###########################################################################
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

# ---------------------------------------------------------------
# The traffic generators currently available are CBR, FTP,
# FTP/GENERIC, HTTP, LOOKUP, MCBR, TELNET, TRAFFIC-GEN, TRAFFIC-TRACE,
# VBR, and VOIP.
# ---------------------------------------------------------------
#
# 1. CBR
#
# CBR simulates a constant bit rate generator.  In order to use CBR, the
# following format is needed:
#
#     CBR <src> <dest> <items to send> <item size>
#         <interval> <start time> <end time> [tos] [RSVP-TE]
#
# where
#
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is how many application layer items to send.
#     <item size> is size of each application layer item.
#     <interval> is the interdeparture time between the application layer
#                items.
#     <start time> is when to start CBR during the simulation.
#     <end time> is when to terminate CBR during the simulation.
#     [tos] (optional) is the contents of the TOS field of the IP header
#     [RSVP-TE] (optional) specifies that CBR will use RSVP-TE to send
#               its packets over an MPLS network
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
# 2. FTP
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
# 3. FTP/GENERIC
#
# FTP/GENERIC does not use tcplib to simulate file transfer.  Instead,
# the client simply sends the data items to the server without the server
# sending any control information back to the client.  In order to use
# FTP/GENERIC, the following format is needed:
#
#     FTP/GENERIC <src> <dest> <items to send> <item size> <start time>
#                 <end time> [tos]
#
# where
#
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is the number of items to send.
#     <item size> is size of each item.
#     <start time> is when to start FTP/GENERIC during the simulation.
#     <end time> is when to terminate FTP/GENERIC during the simulation.
#     [tos] (optional) is the contents of the TOS field of the IP header
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
# 4. HTTP
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
#     HTTP <address> <num_of_server> <server_1> ... <server_n> <start>
#          <thresh>
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
#
#
# --------------------------------------------------------------------------
# 5. LOOKUP
#
#    LOOKUP is an unreliable query/response application that can be used
#    to simulate applications such as DNS lookup or ping.
#
#    To use LOOKUP, the following format is needed:
#
#    LOOKUP <src> <dest> <num requests to send> <request size> <reply size>
#           <request interval> <reply delay> <start time> <end time>
#
# where
#
#    <src> is the client node.
#    <dest> is the destination node.
#    <num requests to send> is how many request packets to send.
#    <request size> is the size of each request packet.
#    <reply size> is the size of each reply packet.
#    <request interval> is the interdeparture interval between request
#                       packets.
#    <reply delay> is the delay after receiving a request packet before
#                  responding with a reply packet.
#    <start time> is when to start LOOKUP during the simulation.
#    <end time> is when to stop LOOKUP during the simulation.
#
# EXAMPLE:
#
#       a) LOOKUP 1 2 10 64 512 1S 0S 0S 10S
#
#       Node 1 sends to node 2 10 requests of 64B each every 1 second.
#       Node 2 will reply to node 1 immediately after receiving a request
#       packet from node 1.  The size of each reply packet is 512B.
#       LOOKUP starts at 0 seconds into simulation and stops after 10 seconds
#       into simulation.
#
#       b) LOOKUP 1 2 10 64 512 1S 1MS 10S 20S
#
#       Node 1 sends to node 2 10 requests of 64B each every 1 second.
#       Node 2 will reply to node 1 after 1 millisecond once a request
#       packet is received from node 1.  The size of each reply packet is
#       512B.  LOOKUP starts at 10 seconds into simulation and stops after
#       20 seconds into simulation.
#
#
# --------------------------------------------------------------------------
# 6. MCBR
#
# MCBR simulates a multicast constant bit rate traffic generator.
# It functions the same as CBR, except that a multicast destination
# address is allowed.  MCBR follows the syntax of CBR, with the
# obvious exception that "MCBR" is specified first, not "CBR".
#
#
# ---------------------------------------------------------------
# 7. TELNET
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
# If <session duration> is set to 0, FTP will use tcplib to randomly
# determine how long the telnet session will last.  The interval between
# telnet items are determined by tcplib.
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
# 8. TRAFFIC-GEN and TRAFFIC-TRACE
#
# TRAFFIC-GEN simulates a random-distribution based traffic generator.
# Available random distribution functions are uniform(UNI), exponential
# (EXP) and deterministic(DET) functions. These random distributions
# are applicable to session property and traffic property. Also, the
# application is capable of shaping traffic by using either leaky bucket
# (LB), dual leaky bucket (DLB).
#
# In order to use TRAFFIC-GEN, the following format is needed:
#
#     TRAFFIC-GEN <src> <dest> <session_property> <traffic_property>
#                 <leaky_bucket_property> [<qos_property>[<retry_property>]]
#
#
# TRAFFIC-TRACE simulates a trace file-based traffic generator.  Trace
# files must have two columns; the first for time interval between
# subsequent data, and the second for each data length.
#
# In order to use TRAFFIC-TRACE, the following format is needed:
#
#     TRAFFIC-TRACE <src> <dest> <session_property> <traffic_property>
#                  <leaky_bucket_property> [<qos_property>[<retry_property>]]
#
# where
#
#     <src> is the client node
#     <dest> is the server node.
#     <session_property> is the session property of a connection. Session
#             property is divided in two parts: start time and duration of
#             session. Both of them are controlled by random distribution.
#             Format of session property is
#                 <session_property> ::= <start_time> <duration>
#
#             <start_time> ::= <time_distribution_type>
#             <duration> ::=   <time_distribution_type>
#
#             <time_distribution_type> ::= DET time_value |
#                                          UNI time_value time_value |
#                                          EXP time_value
#
#             <time_value> ::= integer [ NS | MS | S | M | H | D ]
#
#     <traffic_property> describes how traffic are to be feeded into the
#             network. This property differs for TRAFFIC-GEN and
#             TRAFFIC-TRACE applications.
#
#             TRAFFIC-GEN:
#             Traffic property is divided into three parts: length
#             of data segment, interdeparture time between two consecutive
#             data segment and generation probability of a data segment.
#             Format of traffic property is
#                 <traffic_property> ::=
#                            RND <data_length> <data_interval> <gen_prob>
#
#             <data_length>   ::= <int_distribution_type>
#             <data_interval> ::= <time_distribution_type>
#             <gen_prob>      ::= float within 0 to 1.0
#
#             <int_rnd_dist> ::= DET integer |
#                                UNI integer integer |
#                                EXP integer
#
#             TRAFFIC-TRACE:
#             Traffic property is controlled by the trace file.
#             Format of trace property is
#                 <traffic_property> ::= TRC trace_file_name
#
#     <leaky_bucket_property> Leaky bucket property shapes traffic by using
#             either leaky bucket (LB) or dual leaky bucket (DLB). When
#             shaping traffic through (D)LB, application can either delay
#             or drop application data depending on last parameter of leaky
#             bucket property; DROP or DELAY.
#             Format of leaky bucket property is
#                 <leaky_bucket_property> ::=
#                         NOLB |
#                         LB bucket_size token_rate (DROP|DELAY) |
#                         DLB bucket_size token_rate peak_rate (DROP|DELAY)
#
#     <qos_property> is used to described the QoS property will be used to
#             specify QoS constraints and to find QoS path. If QoS property
#             is omitted, application will inject traffic without searching
#             for a QoS path.
#             Format of qos property is
#                  <qos_property> ::=
#                             CONSTRAINT <bandwidth> <delay> [<queue number>]
#
#             <bandwidth>    ::= integer
#             <delay>        ::= <time_value>
#             <queue number> ::= integer
#
#     <retry_property> is used to retry the session again if it unables to
#             find a QoS path in it's previous try.
#             Format of retry property is
#                 <retry_property> ::= RETRY-INTERVAL <interval>
#
#             <interval> ::= <time_value>
#
# EXAMPLE:
#
# TRAFFIC-GEN 1 4 DET 10 DET 100 RND UNI 250 250 UNI 20MS 20MS 1 NOLB
#
# TRAFFIC-GEN 2 5 DET 120 DET 400 RND UNI 250 250 UNI 20MS 20MS 1 NOLB
#             CONSTRAINT 300000 69MS 2
#
# TRAFFIC-GEN 1 5 DET 5 DET 400 RND UNI 250 250 UNI 20MS 20MS 1 NOLB
#             CONSTRAINT 300000 69MS RETRY-INTERVAL 1S
#
# TRAFFIC-TRACE 2 4 DET 0 DET 100 TRC soccer.trc LB 30000 500000 DELAY
#
#
# --------------------------------------------------------------------------
# 9. VBR
#
# VBR simulates a variable bit rate traffic generator.  In order to use VBR,
# the following format is needed:
#
#    VBR <src> <dest> <item size><interval> <start time> <end time>
#
# where:
#
#     <src> is the client node.
#     <dest> is the server node.
#     <item size> is the size of each application layer item.
#     <interval> data packets are sent at variable rate which is a
#                function of this interval.
#     <start time> is when to start VBR during the simulation.
#     <end time> is when to terminate VBR during the simulation.
#
# EXAMPLE:
#
#      VBR 1 2 1460 1S 5S 0S
#
#      Node 1 sends packets of size 1460 bytes to node 2 at a variable
#      bit rate of 1 second (which is a function of the mean interval)
#      starting from 5 seconds of simulation till the end of simulation.
#
#
# --------------------------------------------------------------------------
# 10. VOIP
#
#    VOIP simulates IP telephony in H323 network. The initiator and receiver
#    generates real time traffic with an exponential distribution function.
#    That simulates a real life telephone conversation.
#
#    The format of the VOIP application without TOS support:
#
#    VOIP  <initiator> <receiver> <avg_talk_time> <start_time> <end_time>
#          [<call_status>] [<packetization_interval>]
#
#    The format of the VOIP application with TOS support:
#
#    VOIP  <initiator> <receiver> <avg_talk_time> <start_time> <end_time>
#          [CALL-STATUS <call_status>]
#          [PACKETIZATION-INTERVAL <packetization_interval>]
#          [TOS <tos>]
#
#    <initiator>     : The terminal node that initiates the call
#    <receiver>      : The terminal node to which the call is destined
#    <avg_talk_time> : Average talking time when a terminal starts talking
#    <start_time>    : When initiating call requests to the receiver for the
#                      voice conversation
#    <end_time>      : When conversation is over.
#
#    <call_status>   : The status may be ACCEPT or REJECT. When this is set
#                      as ACCEPT, the receiver will receive the call.
#                      The setting of REJECT indicates the receiver will not
#                      receive the call and the call request will be
#                      rejected. That means there will be no conversation.
#                      This is optional and when not set, the call will be
#                      received by default.
#
#    <packetization_interval> :
#                      This is the interval at which VOIP application sends
#                      packets to RTP. The default value is 20MS if not set.
#                      Otherwise, the user can specify a value in between
#                      20MS to 200MS.
#
#    <tos>           : Value of the TOS field in the IP header
#
# EXAMPLE:
#
#       a) VOIP 1 19 10S 1M 5M
#
#       Node 1 initiates a call to node 19 with average talking time of 10
#       seconds. The call is initiated after 1 minute into the simulation
#       and terminates after 5 minutes from the start. The call is received
#       by the receiver.
#
#       b) VOIP 3 18 10S 4M 0 ACCEPT
#
#       This connection continues until the end of simulation and the call
#       is accepted by the receiver.
#
#       c) VOIP 4 17 10S 4M 5M ACCEPT 30MS
#
#       Here, the packetization interval is explicitly set to 30MS
#
#       d) VOIP 2 20 10S 4M 6M REJECT 20MS
#
#       This call from node 2 is rejected by node 20 and the call is
#       not established.
#
#       e) VOIP 4 17 10S 4M 5M CALL-STATUS ACCEPT PACKETIZATION-INTERVAL 30MS
#
#       Here, the TOS field of the IP header is explicitly set to 1
#

# VOIP 1 3 10S 1M 10M ACCEPT
# VOIP 2 4 10S 1M 10M ACCEPT

CBR 1 8 0 512 0.1S 0S 0S
CBR 1 9 0 512 0.1S 10S 0S
