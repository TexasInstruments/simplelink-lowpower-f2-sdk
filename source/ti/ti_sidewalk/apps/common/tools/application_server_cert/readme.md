# Sidewalk Application Server Cert Generation Script

This script can be used to generate an application server cert which can be
used in the ACS console as described in the Sidewalk Quick Start Guide.

# Script setup

```
pip3 install --user -r requirements.txt
```

# Generation of Application Server Cert

```
python3 generate_application_server.py --app_srv_serial <ApplicationServerSerialNumber> --apid <APID>
```

Where APID is vended by the acs console and ApplicationServerSerialNumber is an
4 byte integer that is appended to the APID to generate a message that is then
signed with the application private cert to generate a signature.

Although the script generates a number of files, there are just 2 files are
needed
 - _app-server-ed25519-private.txt_ which is used as in input to ACS console to
   configure AWS cloud (Step 3 of Sidewalk Quick Start Guide)
 - _app-server-ed25519.public.bin_ which is given as an input to provision.py as
   the application server public key

