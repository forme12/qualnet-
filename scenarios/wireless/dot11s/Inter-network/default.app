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
#       2 WN -----  1 Wired Node ---- 3 WN
#                   |
#                   |
#                   |
#       60 MP .... 10 MPP .... 20 MP ..... 30 MP
#                               :           :
#                               :           :
#                              40 MP ..... 50 MP


# Between 30, 3
CBR 30 3 100 100 100MS 125 0
CBR 3 30 100 100 100MS 155 0
FTP/GENERIC 30 3 100 512 185 0

# Between 50, 2
CBR 50 2 100 1000 100MS 225 0
CBR 2 50 100 1000 100MS 255 0
FTP/GENERIC 50 2 100 512 285 0
