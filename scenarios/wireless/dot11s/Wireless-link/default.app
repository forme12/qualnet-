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
#
#      102           (             )           202
#       |           (               )           |
#       |          (  Mesh network   )          |
#      101 ---- 100  with 10 mobile   200 ---- 201
#       |          (     nodes       )          |
#       |           (               )           |
#      103            (            )           203



# Between wired nodes
CBR 102 202 100 1000 100MS 25 0
CBR 202 101 100 1000 100MS 45 0
FTP/GENERIC 102 202 100 512 65 0

# Between mesh and wired nodes
CBR 1 202 100 1000 100MS 85 0
CBR 202 2 100 1000 100MS 105 0
FTP/GENERIC 2 202 100 512 125 0
