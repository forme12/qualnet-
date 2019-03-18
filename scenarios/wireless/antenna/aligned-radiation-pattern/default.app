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


#FTP/GENERIC 1 2 3 512 10 150S
#FTP 1 2 10 150S
#TELNET 3 4 10S 150S

CBR 1 2 10000 512   5S    50S 0S
CBR 2 1 10000 512 2.5S 69S 0S

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
#         <interval> <start time> <end time> 
#         [TOS <tos-value> | DSCP <dscp-value> |
#         PRECEDENCE <precedence-value>] [RSVP-TE]

# 
# where
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is how many application layer items to send.
#     <item size> is size of each application layer item.
#     <interval> is the interdeparture time between the application layer
#               items.
#     <start time> is when to start CBR during the simulation.
#     <end time> is when to terminate CBR during the simulation.
#     <tos-value> is the value of the TOS bits of the IP header.
#     <dscp-value> is the value of the DSCP bits of the IP header.
#     <precedence-value> is the contents of the Precedence bits of IP header.
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
#                 <end time>
#                 [TOS <tos-value> | DSCP <dscp-value> |
#                 PRECEDENCE <precedence-value>]
# 
# where
# 
#     <src> is the client node.
#     <dest> is the server node.
#     <items to send> is the number of items to send.
#     <item size> is size of each item.
#     <start time> is when to start FTP/GENERIC during the simulation.
#     <end time> is when to terminate FTP/GENERIC during the simulation.
#     <tos-value> is the value of the TOS bits of the IP header.
#     <dscp-value> is the value of the DSCP bits of the IP header.
#     <precedence-value> is the contents of the Precedence bits of IP header.
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
#           [TOS <tos-value> | DSCP <dscp-value> |
#           PRECEDENCE <precedence-value>]
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
#    <tos-value> is the value of the TOS bits of the IP header.
#    <dscp-value> is the value of the DSCP bits of the IP header.
#    <precedence-value> is the contents of the Precedence bits of IP header.
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
# MCBR stands for Multicast CBR (constant-bit-rate) traffic. It operates
# identically to CBR, with the exceptions that the destination must be a
# multicast address, the type of service is not supported (TOS is always
# set to 0), and RSVP-TE is not an option.  To use MCBR, a multicast
# routing must be also be configured.
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
#                             CONSTRAINT <bandwidth> <delay>
#                             [TOS <tos-value> | DSCP <dscp-value> |
#                             PRECEDENCE <precedence-value>]
#
#             <bandwidth>        ::= integer
#             <delay>            ::= <time_value>
#             <tos-value>        ::= <TOS value in IP header>
#             <dscp-value>       ::= <DSCP value in IP header>
#             <precedence-value> ::= <Precedence value in IP header>
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
#             CONSTRAINT 300000 69MS PRECEDENCE 2
#
# TRAFFIC-GEN 1 5 DET 5 DET 400 RND UNI 250 250 UNI 20MS 20MS 1 NOLB
#             CONSTRAINT 300000 69MS RETRY-INTERVAL 1S
#
# TRAFFIC-TRACE 2 4 DET 0 DET 100 TRC soccer.trc LB 30000 500000 DELAY
#
#
# ---------------------------------------------------------------
# 9. SUPER-APPLICATION
# 
# SUPER-APPLICATION is a generic application. SUPER-APPLICATION can simulate
# both UDP and TCP flows. 2-way flow (request-response) is supported for UDP 
# based applications. Request packets travel from source to destination 
# whereas response packets travel from destination to source. In order to use 
# SUPER-APPLICATION, the following format is needed:
# 
#     SUPER-APPLICATION <src> <dest> <Keyword 1> <value 1> 
#   <Keyword 2> <value 2>  ..... <Keyword n> <value n> 
# 
# The following are the required keyword and value pairs:
#   
#     DELIVERY-TYPE <RELIABLE|UNRELIABLE> : Specifies  TCP or UDP
#     START-TIME <time_distribution_type> : specifies application start time.
#     DURATION <time_distribution_type>   : specifies application duration.
#     REQUEST-NUM <int_distribution_type> :is the number of items to send.
#     REQUEST-SIZE <int_distribution_type> : is size of each item.
#     REQUEST-INTERVAL <time_distribution_type> : is inter-departure time 
#   between requests items.     
#
# The following optional keyword and value pairs are also supported:    
#     REQUEST-TOS [TOS <tos-value> | DSCP <dscp-value> | PRECEDENCE 
#   <precedence-value>] :  <tos-value> is the value of the TOS bits of 
#   the IP header.<dscp-value> is the value of the DSCP bits of the IP 
#   header.<precedence-value> is the contents of the Precedence bits of 
#   IP header. When no REQUEST-TOS is specified, default value of 
#   APP_DEFAULT_TOS ( Ox00) is used.
#     REPLY-TOS [TOS <tos-value> | DSCP <dscp-value> | PRECEDENCE 
#   <precedence-value>] :  <tos-value> is the value of the TOS bits of 
#   the IP header.<dscp-value> is the value of the DSCP bits of the IP 
#   header.<precedence-value> is the contents of the Precedence bits of 
#   IP header. When no REPLY-TOS is specified, default value of 
#   APP_DEFAULT_TOS ( Ox00) is used.
#     REPLY-PROCESS <YES|NO> : indicates if reply is to be sent. When 
#   REPLY-PROCESS is YES, then all reply related keywords are required.
#     REPLY-NUM <int_distribution_type> : is the number of reply packets to 
#   send per request
#     REPLY-SIZE <int_distribution_type> : is the size of reply packets
#     REPLY-PROCESS-DELAY <time_distribution_type> : is the wait time at the 
#   server for sending the first response packet when a request is received.
#     REPLY-INTERDEPARTURE-DELAY <time_distribution_type>: is the delay between 
#   the reply packets.
#     FRAGMENT-SIZE <bytes> : is the max UDP application fragment size. Packets will
#       be broken into multiple fragments when packet size exceeds FRAGMENT-SIZE.
#     SOURCE-PORT <port number> : is the port number to be used at the source. 
#   The application chooses a free source port when user does not specify a 
#   source port. User specified ports must be unique.
#     DESTINATION-PORT <port number> : is the port number to be used at the 
#   destination. The application uses the it's default well known port when 
#   destination port is not specified by the user. User specified ports must 
#   be unique.
#
# Where
#             <time_distribution_type> ::= DET time_value |
#                                          UNI time_value time_value |
#                                          EXP time_value
#
#         <time_value> ::= integer [ NS | MS | S | M | H | D ]
#
#             <int_distribution_type> ::= DET integer |
#                                         UNI integer integer |
#                                         EXP integer         
#
# EXAMPLE:
# 
#     a) SUPER-APPLICATION 1 2 DELIVERY-TYPE RELIABLE START-TIME DET 60S DURATION 
#    DET 10S REQUEST-NUM DET 5 REQUEST-SIZE DET 1460  REQUEST-INTERVAL DET 0S 
#    REQUEST-TOS PRECEDENCE 0 REPLY-PROCESS NO 
# 
#        Node 1 sends to node 2 five items of 1460B each using TCP from 60 seconds 
#    simulation time till 70 seconds simulation time. If five items are sent 
#    before 10 seconds elapsed, no other items are sent. Precedence is set 
#    to 0.
#
#     b) SUPER-APPLICATION 1 2 DELIVERY-TYPE UNRELIABLE START-TIME DET 10S DURATION  
#    DET 21S REQUEST-NUM DET 3 REQUEST-SIZE DET 123  REQUEST-INTERVAL DET 1S 
#    REQUEST-TOS PRECEDENCE 1 REPLY-PROCESS YES FRAGMENT-SIZE 200 DESTINATION-PORT 
#    1751 SOURCE-PORT 2345 REPLY-NUM DET 2 REPLY-SIZE DET 140 REPLY-PROCESS-DELAY 
#    DET 1S REPLY-INTERDEPARTURE-DELAY DET 0.1S 
# 
#        Node 1 sends to node 2 three (request) packets of 123 bytes each using UDP.
#    Node 1 starts sending at 10 seconds simulation time and sends for a 
#    duration of 21 seconds. Packets are sent with a 1 sec interval with 
#    precedence set to 1. Node 1 uses port 2345 whereas Node 2 uses port 1751. 
#    The fragment size is bigger than packet size, so no fragmentation takes 
#    place. On receiving the request packets, Node 2 replies with 2 packets of 
#    140 bytes each, for every request packet received. On receiving a 
#    request packet, node 2 waits 1 second before sending the first of 2
#    reply packets for that request packet. The second reply packet is 
#    sent 0.1 seconds after the first one.
#
# ---------------------------------------------------------------
#
# Behavior & Limitations 
#
# a) Response is only supported with DELIVERY-TYPE UNRELIABLE.
#
# b) Default value for REPLY-PROCESS is NO. Replies are not processed unless 
#    this is explicitly set to YES.
#
# c) Both client and server print out all possible statistics even if they are 
#    not collected. This will be changed in the next release. Not collected 
#    statistics are set to zero. Example: The client reports "Number of reply 
#    packets sent = 0" since this stat is only collected at the server.
#  
# d) The REPLY-INTERDEPARTURE-DELAY and REQUEST-INTERVAL is applicable between
#    packets and not fragments. If fragmentation occurs then all fragments are
#    sent back to back.
#
# e) 0 second duration indicates "till end of simulation". 
#
# --------------------------------------------------------------------------
# 10. VBR
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
# 11. VOIP
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
#          [TOS <tos-value> | DSCP <dscp-value> |
#          PRECEDENCE <precedence-value>]
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
#    <tos-value>        : Value of the TOS bits of the IP header.
#    <dscp-value>       : Value of the TOS bits of the IP header.
#    <precedence-value> : Value of the TOS bits of the IP header.
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
#               TOS 0
#
#       Here, the TOS field of the IP header is explicitly set to 0
#
# --------------------------------------------------------------------------
#  MGEN 
#  (Included as a part of HLA & Threaded Communication Module)
#
#  MGEN stands for multi generator. It is an application layer protocol that 
#  provides the ability to undertake IP network performance test and measurement
#  using the underlying UDP/IP protocol. Basically it simulates a toolset which
#  generates real-time traffic patterns so that the network can be loaded in a 
#  variety of ways. The generated traffic can also be received and logged for 
#  analyses. Script files are used to drive the generated loading patterns over
#  the course of time. Similarly log file is used to log the received
#  traffic for post-test analysis.
#           
#  To use MGEN, input must be given in the following format :
#            
#      mgen <nodeId> [<IP_version>] <txlog> <input script_file_name> <output>
#                       
#
#
#   where,
#      
#      nodeId = NodeId of the mgen enabled node. 
#      <IP_version> = ipv4 | ipv6
#      <txlog> = Keyword necessary to enable transmission logging for a node
#      input = Keyword preceding the input driver script file
#      <script_file_name> = Script file, conventionally is given suffix of "mgn"
#      output = Used to log receive events .Here one log file is generated
#               for each node and it follows a standard naming convention:
#               mgen_<nodeId>.log and does not take the name from user.
#               The results from successive runs are overwriteen.
#      
#   Note : 1. Each generating node must be accompanied by it's driver script
#             file,that specifies the traffic pattern to be generated over
#             time and the intended receiver(s) of that traffic.
#
#          2. A receiving node that wishes to dynamically join or leave 
#             a multicast group also must be accompanied with this script
#             file. Dynamic listening to a port also can be done from this
#             file(To be implemented).
#
#          3. A node can't receive traffic generated by itself, that is 
#             loopback is not currently supported.
#
#          4. Currently only IP version 4 is supported.
#           
#          5. Output keyword must be given for all the nodes for which traffic
#             log is to be generated.
#          
#          6. "txlog" must be given before any "input" or "output" keyword
#              and preferably follow the sequence mentioned above. 
#      
#      
#  EXAMPLE:
# 
#  a)  mgen 1 ipv4 input default.mgn
#      mgen 2 ipv4
#      mgen 4 ipv4
# 
#      Node 1 acts as an mgen traffic generator and sends mgen packets to 
#      the destination that it reads from the default.mgn script file. 
#      Node 2, 4 are only mgen enabled, but whether generated traffic is sent
#      to node 2 or 4 (but not itself) depends on what is written in the
#      associated script file.
# 
#  b)  mgen 2 ipv4 input default.mgn 
#      mgen 6 ipv4
#      mgen 8 ipv4
#        
#      Here Node 2 sends mgen packets to either of node 6 or node 8 or both
#      as per the destination IP specified in script file.      
#      
#  c)  mgen 1 ipv4 input default.mgn txlog output default.trc 
#      mgen 6 ipv4
#      mgen 8 ipv4
#         
#      Node 1 generates traffic as per the pattern specified in the driver
#      script file. The sending events as well as the receive events are 
#      logged into the file - default.trc
#
#        Sample MGEN script file (default.mgn)  is included in the bin folder of
#        QualNet. MGEN4 is the currently supported version of MGEN, however
#        previously supported MGEN3 is also available.
#
#        MGEN 3 & MGEN 4 work mutually exclusively and at a given time
#        only one version should be compiled for use with a given QualNet instance.
#
#
#-------------------------------------------------------------------------------

