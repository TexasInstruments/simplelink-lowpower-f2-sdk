Application Directory
-

All developed code intended to be executed by an end-user
must be placed inside sid_pc_link/apps directory.

Definitions
-
- base_app.py: Import this file in the beginning of your main file.
    - This file contains the next functions:
        - add_lib: append an Amazon developed library for usage of this application.
        - load_dependencies: load all the added libraries.
        - load_config: Load all the specific configuration for an application.
        The configuration file must be named: app_config.json and placed in the
        application root folder (This function is called at import time, there is no need
        to call it again in the application code).
        - get_config: Load all the configuration values in app_config.json file.

Sample Application
-
- sid_pc_link/apps/device_registration
    - app_config.json: Configurable parameters for the loaded libraries.
    - app_config.py: Libraries configuration and logging configuration.
    The logging will affect all the specific modules logging as well.
    - main.py: Application entry point.
