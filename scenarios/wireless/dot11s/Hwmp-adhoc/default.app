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
#      10 MP ......... 20 MP ......... 30 MP
#       .                .               .
#       .                .                .
#       .                 .                .
#      40 MP  .........  50 MP .......... 60 MP


# Between 10, 60
CBR 10 60 100 1000 100MS 25 0
FTP/GENERIC 10 60 100 512 45 0

# Between 40, 30
CBR 40 30 100 1000 100MS 65 0
FTP/GENERIC 40 30 100 512 85 0
