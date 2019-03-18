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

FTP/GENERIC 3 1 0 1000 0S 0S

#This works for TPD
#TRAFFIC-GEN 4 1 DET 10 DET 100 RND TPD 150 250 2 UNI 10MS 20MS 1 NOLB

#TPD4
#TRAFFIC-GEN 4 1 DET 10 DET 100 RND TPD4 150 200 250 2 UNI 10MS 20MS 1 NOLB
#TRAFFIC-GEN 4 1 DET 10 DET 100 RND TPD4 150 200 250 2 UNI 10MS 20MS  1 LB 500 13 DROP
TRAFFIC-GEN 4 1 DET 10 DET 100 RND EXP 150 UNI 10MS 20MS 1 NOLB 

