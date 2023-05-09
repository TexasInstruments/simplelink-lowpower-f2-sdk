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

from flask import Flask, request, abort, jsonify
import os


# Get the workspace root
WORKSPACE = os.path.abspath(
    '{script_path}/..'.format(script_path=os
                              .path
                              .dirname(os
                                       .path
                                       .realpath(__file__))))
# Initialize Templates path
TEMPLATES = '{workspace}/templates'.format(workspace=WORKSPACE)

# Initialize Flask application
app = Flask(__name__, static_url_path='/static')
app.config['JSONIFY_PRETTYPRINT_REGULAR'] = False
