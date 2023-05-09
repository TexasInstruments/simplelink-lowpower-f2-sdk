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

import logging
from requests import Session
from urllib3 import disable_warnings, exceptions

_logger_name = 'base_http_client'
logger = logging.getLogger(_logger_name)


class HttpAbstractClient(Session):

    def __init__(self):
        super(HttpAbstractClient, self).__init__()
        self.auth = self.set_auth()
        self.__url = self.set_connection_url()
        self.validate_connection()
        disable_warnings(exceptions.InsecureRequestWarning)

    # Override this method
    def set_connection_url(self):
        """
        Sets the base connection URL for the HTTP client.
        :return:
        """
        raise NotImplementedError("set_connection_url method is not implemented.")

    # Override this method
    def validate_connection(self):
        """
        Validates that credentials and URL is valid.
        :return:
        """
        raise NotImplementedError("validate_connection method is not implemented.")

    # Override this method
    def set_auth(self):
        """
        Set the authentication method.
        :return:
        """
        raise NotImplementedError("set_auth method is not implemented")

    def get_connection_url(self):
        """
        Returns the used base connection url.
        :return: str
        """
        return self.__url

    def get(self, url, **kwargs):
        logger.debug("GET {url}".format(url=url))
        if kwargs.get("params"):
            logger.debug("Parameters: {params}".format(params=kwargs.get("params")))
        if kwargs.get("headers"):
            logger.debug("Headers: {headers}".format(headers=kwargs.get("headers")))
        if 'https://' not in url and 'http://' not in url:
            url = self.__url + url
        response = super(HttpAbstractClient, self).get(url=url, **kwargs)
        logger.debug("GET {request}".format(request=response.request))
        logger.debug("GET response:\n"
                     "\tHeaders:\t{headers}\n"
                     "\tStatus:\t{status}\n"
                     "\tBody:\t{body}".format(headers=response.headers,
                                              status=response.status_code,
                                              body=response.text))
        return response

    def post(self, url, **kwargs):
        logger.debug("POST {url}".format(url=url))
        if kwargs.get("params"):
            logger.debug("Parameters: {params}".format(params=kwargs.get("params")))
        if kwargs.get("json"):
            logger.debug("JSON Body: {json}".format(json=kwargs.get("json")))
        if kwargs.get("headers"):
            logger.debug("Headers: {headers}".format(headers=kwargs.get("headers")))
        if 'https://' not in url and 'http://' not in url:
            url = self.__url + url
        response = super(HttpAbstractClient, self).post(url=url, **kwargs)
        logger.debug("POST {request}".format(request=response.request))
        logger.debug("POST response:\n"
                     "\tHeaders:\t{headers}\n"
                     "\tStatus:\t{status}\n"
                     "\tBody:\t{body}\n"
                     "\tReason\t{reason}".format(headers=response.headers,
                                                 status=response.status_code,
                                                 body=response.text,
                                                 reason=response.reason))
        return response

    def delete(self, url, **kwargs):
        logger.debug("DELETE {url}".format(url=url))
        if kwargs.get("params"):
            logger.debug("Parameters: {params}".format(params=kwargs.get("params")))
        if kwargs.get("json"):
            logger.debug("JSON Body: {json}".format(json=kwargs.get("json")))
        if kwargs.get("headers"):
            logger.debug("Headers: {headers}".format(headers=kwargs.get("headers")))
        if 'https://' not in url and 'http://' not in url:
            url = self.__url + url
        response = super(HttpAbstractClient, self).delete(url=url, **kwargs)
        logger.debug("DELETE {request}".format(request=response.request))
        logger.debug("DELETE response:\n"
                     "\tHeaders:\t{headers}\n"
                     "\tStatus:\t{status}\n"
                     "\tBody:\t{body}".format(headers=response.headers,
                                              status=response.status_code,
                                              body=response.text))
        return response

    def put(self, url, **kwargs):
        logger.debug("PUT {url}".format(url=url))
        if kwargs.get("params"):
            logger.debug("Parameters: {params}".format(params=kwargs.get("params")))
        if kwargs.get("json"):
            logger.debug("JSON Body: {json}".format(json=kwargs.get("json")))
        if kwargs.get("headers"):
            logger.debug("Headers: {headers}".format(headers=kwargs.get("headers")))
        if 'https://' not in url and 'http://' not in url:
            url = self.__url + url
        response = super(HttpAbstractClient, self).put(url=url, **kwargs)
        logger.debug("PUT {request}".format(request=response.request))
        logger.debug("PUT response:\n"
                     "\tHeaders:\t{headers}\n"
                     "\tStatus:\t{status}\n"
                     "\tBody:\t{body}".format(headers=response.headers,
                                              status=response.status_code,
                                              body=response.text))
        return response
