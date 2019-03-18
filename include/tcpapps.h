// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive
//                          Suite 1250
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/*
 *
 * Ported from TCPLIB.  This file contains the declarations
 * of the functions defined in telnet.c, ftp.c, etc.
 */

/*
 * DO NOT MODIFY THIS FILE.  IT IS AUTOMATICALLY GENERATED
 * BY THE TCPLIBGEN PROGRAMS.  ALL CHANGES HERE WILL BE DELETED.
 */

/*
 * Copyright (c) 1991 University of Southern California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of Southern California. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
*/


/*
 * telnet
*/
extern int telnet_pktsize(RandomSeed seed);
extern float telnet_interarrival(RandomSeed seed);
extern float telnet_duration(RandomSeed seed);

/*
 * ftp
*/
extern int ftp_nitems(RandomSeed seed);
extern int ftp_itemsize(RandomSeed seed);
extern int ftp_ctlsize(RandomSeed seed);

/*
 * nntp
*/
extern int nntp_nitems();
extern int nntp_itemsize();

/*
 * smtp
*/
extern int smtp_itemsize();

/*
 * phone
*/
extern float phone_talkspurt();
extern float phone_pause();

