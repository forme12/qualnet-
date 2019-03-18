# Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
#                          6100 Center Drive
#                          Suite 1250
#                          Los Angeles, CA 90045
#                          sales@scalable-networks.com
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

# The traffic generators currently available are FTP,
# FTP/GENERIC, TELNET, CBR, MCBR, and HTTP.
# 
# ---------------------------------------------------------------
# 1. FTP
# 
# FTP uses tcplib to simulate the file transfer protocol.  In order to use
# FTP, the following format is needed:
# 
#     FTP <src> <dest> <items to send> <start time>
# 
# where 
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is how many application layer items to send.
#     <start time> is when to start FTP during the simulation.
# 
# If <items to send> is set to 0, FTP will use tcplib to randomly determine
# the amount of application layer items to send.  The size of each item is
# will always be randomly determined by tcplib.  Note that the term "item"
# in the application layer is eqivalent to the term "packet" at the network
# layer and "frame" at the MAC layer.
# 
# 
# EXAMPLE:
# 
#     a) FTP 1 2 10 0S
# 
#        Node 1 sends node 2 ten items at the start of the simulation,
#        with the size of each item randomly determined by tcplib.
# 
#     b) FTP 1 2 0 100S
# 
#        Node 1 sends node 2 the number of items randomly picked by tcplib
#        after 100 seconds into the simulation.  The size of each item is
#        also randomly determined by tcplib.
# 
# 
# ---------------------------------------------------------------
# 2. FTP/GENERIC
# 
# FTP/GENERIC does not use tcplib to simulate file transfer.  Instead,
# the client simply sends the data items to the server without the server
# sending any control information back to the client.  In order to use
# FTP/GENERIC, the following format is needed:
# 
#     FTP/GENERIC <src> <dest> <items to send> <item size> <start time>
#                 <end time>
# 
# where
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is the number of items to send.
#     <item size> is size of each item.
#     <start time> is when to start FTP/GENERIC during the simulation.
#     <end time> is when to terminate FTP/GENERIC during the simulation.
# 
# If <items to send> is set to 0, FTP/GENERIC will run until the specified
# <end time> or until the end of the simuation, which ever comes first.  
# If <end time> is set to 0, FTP/GENERIC will run until all <items to send>
# is transmitted or until the end of simulation, which ever comes first.
# If <items to send> and <end time> are both greater than 0, FTP/GENERIC will
# will run until either <items to send> is done, <end time> is reached, or 
# the simulation ends, which ever comes first.
# 
# EXAMPLE:
# 
#     a) FTP/GENERIC 1 2 10 1460 0S 600S
# 
#        Node 1 sends node 2 ten items of 1460B each at the start of the 
#        simulation up to 600 seconds into the simulation.  If the ten
#        items are sent before 600 seconds elapsed, no other items are 
#        sent.
# 
#     b) FTP/GENERIC 1 2 10 1460 0S 0S
# 
#        Node 1 sends node 2 ten items of 1460B each at the start of the 
#        simulation until the end of the simulation.  If the ten
#        items are sent the simulation ends, no other items are 
#        sent.
# 
#     c) FTP/GENERIC 1 2 0 1460 0S 0S
# 
#        Node 1 continuously sends node 2 items of 1460B each at the 
#        start of the simulation until the end of the simulation.  
# 
# 
# ---------------------------------------------------------------
# 3. TELNET
# 
# TELNET uses tcplib to simulate the telnet protocol.  In order to use
# TELNET, the following format is needed:
# 
#     TELNET <src> dest> <session duration> <start time>
# 
# where
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <session duration> is how long the telnet session will last.
#     <start time> is when to start TELNET during the simulation.
# 
# If <session duration> is set to 0, FTP will use tcplib to randomly determine
# how long the telnet session will last.  The interval between telnet items 
# are determined by tcplib.
# 
# 
# EXAMPLE:
# 
#     a) TELNET 1 2 100S 0S
# 
#        Node 1 sends node 2 telnet traffic for a duration of 100 seconds at
#        the start of the simulation.
# 
#     b) TELNET 1 2 0S 0S
# 
#        Node 1 sends node 2 telnet traffic for a duration randomly
#        determined by tcplib at the start of the simulation.
# 
# 
# ---------------------------------------------------------------
# 4. CBR
# 
# CBR simulates a constant bit rate generator.  In order to use CBR, the
# following format is needed:
# 
#     CBR <src> <dest> <items to send> <item size> 
#         <interval> <start time> <end time>
# 
# where
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is how many application layer items to send.
#     <item size> is size of each application layer item.
#     <interval> is the interdeparture time between the application layer items.
#     <start time> is when to start CBR during the simulation.
#     <end time> is when to terminate CBR during the simulation.
# 
# If <items to send> is set to 0, CBR will run until the specified
# <end time> or until the end of the simuation, which ever comes first.  
# If <end time> is set to 0, CBR will run until all <items to send>
# is transmitted or until the end of simulation, which ever comes first.
# If <items to send> and <end time> are both greater than 0, CBR will
# will run until either <items to send> is done, <end time> is reached, or 
# the simulation ends, which ever comes first.
# 
# EXAMPLE:
# 
#     a) CBR 1 2 10 1460 1S 0S 600S
# 
#        Node 1 sends node 2 ten items of 1460B each at the start of the 
#        simulation up to 600 seconds into the simulation.  The interdeparture
#        time for each item is 1 second.  If the ten items are sent before 
#        600 seconds elapsed, no other items are sent.
# 
#     b) CBR 1 2 0 1460 1S 0S 600S
# 
#        Node 1 continuously sends node 2 items of 1460B each at the start of
#        the simulation up to 600 seconds into the simulation.
#        The interdeparture time for each item is 1 second.
#       
#     c) CBR 1 2 0 1460 1S 0S 0S
# 
#        Node 1 continuously sends node 2 items of 1460B each at the start of
#        the simulation up to the end of the simulation.
#        The interdeparture time for each item is 1 second.
#
#
# ---------------------------------------------------------------
# 5. MCBR
#
# MCBR simulates a multicast constant bit rate traffic generator.
# It functions the same as CBR, except that a multicast destination
# address is allowed.  MCBR follows the syntax of CBR, with the
# obvious exception that "MCBR" is specified first, not "CBR".
#       
# 
# ---------------------------------------------------------------
# 6. HTTP
# 
# HTTP simulates single-TCP connection web servers and clients.  Bruce Mah
# has gathered packet traces of HTTP network conversations, and produced
# CDFs for "the size of HTTP items retrieved, number of items per 'Web page',
# think time, and user browsing behavior."  
# (http://www.employees.org/~bmah/Software/Insane/)
# 
# This model has been implemented for QualNet, and the following format
# describes its use for servers:
# 
#     HTTPD <address>
# 
# where
# 
#     <address> is the node address of a node which will be serving
#               Web pages.
# 
# For HTTP clients, the  following format is used:
# 
#     HTTP <address> <num_of_server> <server_1> ... <server_n> <start> <thresh>
# 
# where
# 
#     <address> is the node address of the node on which this client resides
#     <num_of_server> is the number of server addresses which will follow
#     <server_1>
#     .
#     .
#     <server_n> are the node addresses of the servers which this client
#                will choose between when requesting pages.  There must
#                be "HTTPD <address>" lines existing separately for each of
#                these addresses.
#     <start> is the start time for when the client will begin requesting
#             pages
#     <thresh> is a ceiling (specified in units of time) on the amount of
#              "think time" that will be allowed for a client.  The 
#              network-trace based amount of time modulo this threshold
#              is used to determine think time.
# 
# EXAMPLE:
# 
#     HTTPD 2
#     HTTPD 5
#     HTTPD 8
#     HTTPD 11
#     HTTP 1 3 2 5 11 10S 120S
# 
#     There are HTTP servers on nodes 2, 5, 8, and 11.  There is an HTTP
#     client on node 1.  This client chooses between servers {2, 5, 11} only
#     when requesting web pages.  It begins browsing after 10S of simulation
#     time have passed, and will "think" (remain idle) for at most 2 minutes
#     of simulation time, at a time.

CBR 1 3 100 512 1S 1S 0S PRECEDENCE 1