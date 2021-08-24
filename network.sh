#!/bin/bash

# ping -I enp5s0 -c 2 8.8.4.4 > /dev/null 2>&1

# if [ $? -ne 0 ]; then
# 	nmcli c down uuid 9bd8475e-512d-304b-bba1-fdc7abe2c5b0 
# 	nmcli c up uuid 5286ffe3-6dff-4b23-827a-9a8840f9c05a
# 	nmcli radio wifi on
# 	nmcli c down uuid 5286ffe3-6dff-4b23-827a-9a8840f9c05a
# 	nmcli c up uuid 9bd8475e-512d-304b-bba1-fdc7abe2c5b0 
# else

#     nmcli radio wifi off
# fi



nmcli c up d2186a16-cc50-4833-b05c-7c9b0f72fadc 
nmcli c up 04e86645-5e06-4222-b0d9-69e03d7f352e
