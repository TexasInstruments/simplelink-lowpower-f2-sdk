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

from . import app, request, TEMPLATES, abort, jsonify
from jinja2 import Environment, FileSystemLoader
from requests import Session

template_loader = FileSystemLoader(TEMPLATES)
env = Environment(loader=template_loader)

# Globals
auth_code = None
access_token = None


class GrantTypes:
    IMPLICIT = 'implicit'
    AUTHORIZATION_CODE = 'authorization_code'
    REFRESH = 'refresh'


@app.route('/login', methods=['GET'])
def login():
    client_id = request.args.get('client_id')
    grant_type = request.args.get('grant_type')
    template = None
    if grant_type == GrantTypes.IMPLICIT:
        template = env.get_template('token.html')
    elif grant_type == GrantTypes.AUTHORIZATION_CODE:
        template = env.get_template('token_cg.html')
    else:
        abort(400, description='Bad request. Non valid grant_type was selected')
    return template.render(client_id=client_id)


@app.route('/auth_code', methods=['POST'])
def post_auth_code():
    global auth_code
    auth_code = request.args.get('authorization_code')
    return jsonify({'authorization_code': auth_code})


@app.route('/auth_code', methods=['GET'])
def get_auth_code():
    if not auth_code:
        abort(404, description='Authorization code not found. '
                               'Run API /login?grant_type=authorization')
    else:
        return jsonify({'authorization_code': auth_code})


@app.route('/lwa', methods=['POST'])
def post_lwa_token():
    global access_token
    access_token = request.args.get('access_token')
    return {'access_token': access_token}


def get_lwa_token_implicit():
    if not access_token:
        abort(404, description='Authorization code not found. '
                               'Run API /login?grant_type=implicit')
    return jsonify({
        'access_token': access_token,
        'refresh_token': None
    })


def get_lwa_token_code_grant(session,
                             url,
                             headers,
                             client_id=None,
                             client_secret=None,
                             code=None):
    response = session.post(
        url,
        headers=headers,
        data={'grant_type': 'authorization_code',
              'code': code,
              'client_id': client_id,
              'client_secret': client_secret}
    )
    if response.ok:
        return jsonify({
            'access_token': response.json().get('access_token'),
            'refresh_token': response.json().get('refresh_token')
        })
    else:
        abort(404, description=response.reason)


def get_lwa_token_refresh(session,
                          url,
                          headers,
                          client_id,
                          client_secret,
                          refresh_token):
    response = session.post(
        url,
        headers=headers,
        data={
            'grant_type': 'refresh_token',
            'refresh_token': refresh_token,
            'client_id': client_id,
            'client_secret': client_secret
        }
    )
    if response.ok:
        return jsonify({
            'access_token': response.json().get('access_token'),
            'refresh_token': response.json().get('refresh_token')
        })
    else:
        abort(404, description=response.reason)


@app.route('/lwa', methods=['GET'])
def get_lwa_token():
    grant_type = request.args.get('grant_type')
    client_id = request.args.get('client_id')
    client_secret = request.args.get('client_secret')
    code = request.args.get('authorization_code')
    refresh_token = request.args.get('refresh_token')
    auth_session = Session()
    url = 'https://api.amazon.com/auth/o2/token'
    headers = {
        'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8'
    }
    response = None
    if grant_type == GrantTypes.AUTHORIZATION_CODE:
        if not all([client_id, client_secret, auth_code]):
            abort(400, description=f'Bad request. There is a missing '
                                   f'parameter for grant_type={GrantTypes.AUTHORIZATION_CODE}. '
                                   f'Mandatory parameters: client_id, client_secret, auth_code')
        response = get_lwa_token_code_grant(auth_session,
                                            url,
                                            headers,
                                            client_id,
                                            client_secret,
                                            code)
    elif grant_type == GrantTypes.IMPLICIT:
        response = get_lwa_token_implicit()
    elif grant_type == GrantTypes.REFRESH:
        if not all([client_id, client_secret, refresh_token]):
            abort(400, description=f'Bad request. There is a missing '
                                   f'parameter for grant_type={GrantTypes.REFRESH}. '
                                   f'Mandatory parameters: client_id, client_secret, refresh_token')
        response = get_lwa_token_refresh(auth_session,
                                         url,
                                         headers,
                                         client_id,
                                         client_secret,
                                         refresh_token)
    else:
        abort(400, description='Bad request. Non valid grant_type was selected')
    return response
