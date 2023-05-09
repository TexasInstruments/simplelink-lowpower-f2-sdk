#!/usr/bin/env python3

#
# Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

import os
import sys
import platform
import re
import argparse
import subprocess
import logging
import requests
import webbrowser
import time
import threading

# This file is intended to be used as an entry point for installation and environment setup
# Add the Workspace root to the PYTHONPATH
SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
LWA_PATH = '{base}/lwa'.format(base=SCRIPT_PATH)
WORKSPACE_ROOT = os.path.abspath('{base}/../..'.format(base=SCRIPT_PATH))
CURRENT_OS = platform.system()

# Add the workspace root to the sys paths
sys.path.append(WORKSPACE_ROOT)

# Load the app configuration
from apps.device_registration import app_config
from apps.device_registration.libs.errors import AuthorizationCodeGrantError, RefreshTokenError

# Initialize logging
logging.basicConfig(level=logging.INFO, format='%(levelname)7s %(asctime)s [%(filename)13s:%(lineno)4d] %(message)s')
logger = logging.getLogger(__name__)


class GrantTypes:
    IMPLICIT = 'implicit'
    AUTHORIZATION_CODE = 'authorization_code'
    REFRESH = 'refresh'


def __run_command(command, timeout=3600, cwd=None):
    in_dir = None
    if cwd:
        in_dir = f'{cwd}'
    try:
        cmd = command.split(' ')
        output = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout, cwd=in_dir)
        if output.returncode != 0:
            return output.returncode, output.stderr.decode('utf-8')
        return output.returncode, output.stdout.decode('utf-8')
    except Exception as err:
        return -1, err


def __check(rc, output, message=None, regerr=None):
    re_type = type(re.compile(r''))
    if not (rc != 0 or (regerr and isinstance(regerr, re_type) and output and regerr.search(output))):
        return
    if message:
        raise RuntimeError("%s. rc is: <%s>; output is: <%s>" % (message, rc, repr(output)))
    raise RuntimeError("rc is: <%s>; output is: <%s>" % (rc, repr(output)))


def get_authorization_code(timeout=120):
    t_init = time.time()
    while time.time() - t_init < timeout:
        # Keep polling every second for LWA token until a valid response is returned
        # or the timeout expires
        response = requests.get('http://localhost:8000/auth_code')
        if response.ok:
            try:
                return response
            except AttributeError:
                pass
        time.sleep(1)
    return None


def launch_lwa(grant_url=None):
    from apps.device_registration.lwa.index import app
    flask_thread = threading.Thread(target=app.run, kwargs={'host': 'localhost',
                                                            'port': 8000})
    flask_thread.daemon = True
    flask_thread.start()
    time.sleep(2)
    logger.info('Launching browser to get the LWA token.')
    if grant_url is not None:
        webbrowser.open(grant_url)


def get_lwa_token_implicit_grant(grant_url, timeout=120):
    # Open the url
    launch_lwa(grant_url)
    t_init = time.time()
    while time.time() - t_init < timeout:
        # Keep polling every second for LWA token until a valid response is returned
        # or the timeout expires
        response = requests.get('http://localhost:8000/lwa',
                                params={'grant_type': GrantTypes.IMPLICIT})
        if response.ok:
            try:
                return response.json()
            except AttributeError:
                pass
        time.sleep(1)
    return None


def get_lwa_token_auth_code_grant(grant_url, client_id, client_secret, timeout=120):
    # Open the url
    launch_lwa(grant_url)
    # Get the authorization code
    auth_code_resp = get_authorization_code()
    if auth_code_resp.ok:
        t_init = time.time()
        while time.time() - t_init < timeout:
            # Keep polling every second for LWA token until a valid response is returned
            # or the timeout expires
            response = requests.get('http://localhost:8000/lwa',
                                    params={'grant_type': GrantTypes.AUTHORIZATION_CODE,
                                            'client_id': client_id,
                                            'client_secret': client_secret,
                                            'authorization_code': auth_code_resp.json().get('authorization_code')
                                            }
                                    )
            if response.ok:
                try:
                    return response.json()
                except AttributeError:
                    pass
            time.sleep(1)
        return None
    else:
        raise AuthorizationCodeGrantError('Not able to get the Authorization code from LWA SDK.')


def get_lwa_token_refresh(client_id, client_secret, timeout=120):
    launch_lwa()
    refresh_token = os.getenv('REFRESH_TOKEN')
    if refresh_token is not None:
        t_init = time.time()
        while time.time() - t_init < timeout:
            # Keep polling every second for LWA token until a valid response is returned
            # or the timeout expires
            response = requests.get('http://localhost:8000/lwa',
                                    params={'grant_type': GrantTypes.REFRESH,
                                            'client_id': client_id,
                                            'client_secret': client_secret,
                                            'refresh_token': refresh_token})
            if response.ok:
                try:
                    return response.json()
                except AttributeError:
                    pass
            time.sleep(1)
        return None
    else:
        raise RefreshTokenError('Not able to found a refresh token.')


def update_application_config(access_token, refresh_token):
    if access_token is not None:
        os.environ.update({'LWA_TOKEN': f"Bearer {access_token}"})
    if refresh_token is not None:
        os.environ.update({'REFRESH_TOKEN': refresh_token})
    # Reload the configuration
    app_config.load_config()


def device_registration_bluetooth():
    from apps.device_registration import device_registration
    device_registration.main()


def parse_arguments(args=None):
    parser = argparse.ArgumentParser()
    exclusive_method_group = parser.add_mutually_exclusive_group()
    exclusive_method_group.add_argument('-r', '--reg',
                                        action='store_const',
                                        const='registration',
                                        dest='action',
                                        help='Run the registration.')
    exclusive_method_group.add_argument('-l', '--lwa',
                                        action='store_const',
                                        const='lwa',
                                        dest='action',
                                        help='Get the LWA token.')
    exclusive_method_group.add_argument('-lc', '--lwa-cg',
                                        action='store_const',
                                        const='lwa-cg',
                                        dest='action',
                                        help='Get the LWA token via Authorization Code Grant.')
    exclusive_method_group.add_argument('-rf', '--refresh-token',
                                        action='store_const',
                                        const='refresh-token',
                                        dest='action',
                                        help='Refresh the LWA Token using the previously obtained '
                                             'refresh token.')
    parser.add_argument('-c', '--client-id',
                        default=None,
                        help='Use with -l/--lwa. Introduce your client ID to get a valid LWA token.')
    parser.add_argument('-s', '--client-secret',
                        default=None,
                        required=False,
                        help='Use with -lc/--lwa-cg. Introduce your client Secret to get LWA access and refresh'
                             'tokens.')
    return parser.parse_args(args)


def main():
    options = parse_arguments(sys.argv[1:])
    if options.action == 'registration':
        device_registration_bluetooth()

    elif options.action == 'lwa':
        if options.client_id is None:
            logger.error('No Client ID was introduced. Please write your client ID using --client-id <client-id>')
            return
        url = requests.Request(method='GET',
                               url='http://localhost:8000/login',
                               params={'grant_type': GrantTypes.IMPLICIT,
                                       'client_id': options.client_id}).prepare().url
        token_dict = get_lwa_token_implicit_grant(grant_url=url)
        update_application_config(token_dict.get('access_token'),
                                  token_dict.get('refresh_token'))

    elif options.action == 'lwa-cg':
        if options.client_id is None:
            logger.error('No Client ID was introduced. Please write your client ID using --client-id <client-id>')
            return
        if options.client_secret is None:
            logger.error('No Client secret was introduced. Please write your client Secret '
                         'using --client-secret <client-secret>')
        url = requests.Request(method='GET',
                               url='http://localhost:8000/login',
                               params={'grant_type': GrantTypes.AUTHORIZATION_CODE,
                                       'client_id': options.client_id}).prepare().url
        token_dict = get_lwa_token_auth_code_grant(grant_url=url,
                                                   client_id=options.client_id,
                                                   client_secret=options.client_secret)
        update_application_config(token_dict.get('access_token'),
                                  token_dict.get('refresh_token'))

    elif options.action == 'refresh-token':
        if options.client_id is None:
            logger.error('No Client ID was introduced. Please write your client ID using --client-id <client-id>')
            return
        if options.client_secret is None:
            logger.error('No Client secret was introduced. Please write your client Secret '
                         'using --client-secret <client-secret>')
        token_dict = get_lwa_token_refresh(client_id=options.client_id,
                                           client_secret=options.client_secret)
        update_application_config(token_dict.get('access_token'),
                                  token_dict.get('refresh_token'))

    else:
        logger.error('No valid operation was introduced.')


if __name__ == '__main__':
    main()
