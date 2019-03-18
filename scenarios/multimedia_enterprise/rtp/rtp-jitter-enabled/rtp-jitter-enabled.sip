# Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
#                          6100 Center Drive
#                          Suite 1250
#                          Los Angeles, CA 90045
#                          sales@scalable-networks.com
#
# This source code is licensed, not sold, and is subject to a written
# license agreement.  Among other things, no portion of this source
# code may be copied, transmitted, disclosed, displayed, distributed,
# translated, used as the basis for a derivative work, or used, in
# whole or in part, for any program or purpose other than its intended
# use in compliance with the license agreement as part of the QualNet
# software.  This source code and certain of the algorithms contained
# within it are confidential trade secrets of Scalable Network
# Technologies, Inc. and may not be used as the basis for any other
# software, hardware, product or service.
#


1   192.0.1.1   one         rediffmail.com   2    192.0.1.2
2   192.0.1.2   two         rediffmail.com   2    192.0.1.2
3   192.0.2.2   three       rediffmail.com   2    192.0.1.2
4   192.0.4.2   four        rediffmail.com   2    192.0.1.2
5   192.0.5.2   five        rediffmail.com   2    192.0.1.2


#
# SIP-ALIAS-ADDRESS-FILE is a plain text file containing User Agent and
# Proxy node details in a specified format. It is necessary for the SIP 
# operation.
#
#
# The exact syntax of the file is as given below :
#
# 
# Each node of the SIP network will be represented by one or more lines
# depending on whether it has a singe interface or multiple interface 
# according to the following format.
#
# There are six columns in each row - 
# column1 - nodeId of the node
# column2 - IP ADDR for node interface
# column3 - alias name for the host
# column4 - sip domain which provides service to this user
# column5 - nodeId of the proxy configured for this domain
# column6 - IP ADDR of the proxy
# 
# For example a single line can look as: 
# <UA_id>   <UA_ip>  <host alias>  <domain>    <proxy_id> <proxy_ip_address>
#
#    1   192.10.10.1  stephen       snt.com        5       192.168.100.10
#
#
#--------------------------------------------------------------------------------


