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


PURPOSE:-
--------
To Test Short Guard Interval Feature for MAC DOT11n model.


SCENARIO:-
---------
All nodes are placed in a wireless subnet.
Node 1 is configured as AP. Node 2 is configured as BSS stations.


                    1          
                   / 
                 2   

Application:
------------
CBR session is configured between Node 1 and Node 2. 



RUN:-
----
Run '<QUALNET_HOME>/bin/qualnet short_gi_test.config'.



DESCRIPTION OF THE FILES:-
-------------------------
1. short_gi_test.app -  QualNet configuration file for application input.
2. short_gi_test.config - QualNet configuration input file.
3. short_gi_test.nodes - QualNet node placement file for the simulation run.
4. short_gi_test.expected.stat - QualNet statistics collection.
5. README - This file.

