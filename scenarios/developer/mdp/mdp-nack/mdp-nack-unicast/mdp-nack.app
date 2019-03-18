# Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
#                          6701 Center Drive West
#                          Suite 520
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

CBR 1 2 0 512 1S 10S 150S MDP-ENABLED
SUPER-APPLICATION 1 3 DELIVERY-TYPE UNRELIABLE START-TIME DET 10S DURATION DET 150S REQUEST-NUM DET 0 REQUEST-SIZE DET 512 REQUEST-INTERVAL DET 1S REQUEST-TOS PRECEDENCE 0 REPLY-PROCESS NO MDP-ENABLED
TRAFFIC-GEN 1 2 DET 10S DET 150S RND UNI 512 512 UNI 1S 1S 1 NOLB MDP-ENABLED
TRAFFIC-TRACE 1 5 DET 10S DET 150S TRC soccer.trc LB 30000 500000 DELAY MDP-ENABLED
VBR 1 3 512 1S 10S 150S  MDP-ENABLED