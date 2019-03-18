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

// /**
// PACKAGE     :: PHY_SYNCHRONIZATION
// DESCRIPTION :: This file describes data structures and functions
//                used by signal synchronization in
//                phy models and propagation models.
// **/

#ifndef PHY_SYNC_H
#define PHY_SYNC_H

//#define PHY_SYNC

typedef enum {
    PHY_MODULATION_DEFAULT,
    PHY_MODULATION_BPSK,   // (Dinary Phase Shift Keying)
    PHY_MODULATION_DBPSK,  // (Differential Binary Phase Shift Keying)
    PHY_MODULATION_QPSK,   // (Quadrature Phase Shift Keying) 
    PHY_MODULATION_DQPSK,  // (Differential Quadrature Phase-shift Keying)
    PHY_MODULATION_OQPSK,  // (Offset quadrature Phase-shift Keying)
    PHY_MODULATION_8PSK,   // (eight Phase Shift Keying)
    PHY_MODULATION_16PSK,  // (sixteen phase shift keying)
    PHY_MODULATION_32PSK,  // (thirty two phase shift keying)
    PHY_MODULATION_64PSK,  // (sixty four phase shift keying)
    PHY_MODULATION_128PSK, // (one hundred twenty eignt phase shift keying)
    PHY_MODULATION_256PSK, // (two hundreds fifty six phase shift keying)
    PHY_MODULATION_4QAM,   // (4-ary quadrature amplitude modulation)
    PHY_MODULATION_8QAM,   // (8-ary quadrature amplitude modulation)
    PHY_MODULATION_16QAM,  // (16-ary quadrature amplitude modulation)
    PHY_MODULATION_32QAM,  // (32-ary quadrature amplitude modulation)
    PHY_MODULATION_64QAM,  // (64-ary quadrature amplitude modulation)
    PHY_MODULATION_128QAM, // (128-ary quadrature amplitude modulation)
    PHY_MODULATION_256QAM, // (256-ary quadrature amplitude modulation)
    PHY_MODULATION_FSK,    // (frequency shift keying)
    PHY_MODULATION_MFSK,   // (Multiple frequency-shift keying)
    PHY_MODULATION_4FSK,   // (4 frequency-shift keying)
    PHY_MODULATION_8FSK,   // (8 frequency-shift keying)
    PHY_MODULATION_16FSK,  // (16 frequency-shift keying)
    PHY_MODULATION_32FSK,  // (32 frequency-shift keying)
    PHY_MODULATION_64FSK,  // (64 frequency-shift keying)
    PHY_MODULATION_128FSK, // (128 frequency-shift keying)
    PHY_MODULATION_256FSK, // (256 frequency-shift keying)
    PHY_MODULATION_GMSK,   // (Gaussian Minimum Keying)
    PHY_MODULATION_MSK,    // (Minimum Shift Keying)
    PHY_MODULATION_CPFSK,  // (Continuous Phase Frequency Shift Keying)

    PHY_MODULATION_NONE    // Must be the last one
} PhyModulation;

extern const char* modulationNames[PHY_MODULATION_NONE];

typedef enum {
    PHY_ENCODING_DEFAULT,
    PHY_ENCODING_CONVOLUTION,
    PHY_ENCODING_REEDSOLOMON,
    PHY_ENCODING_REEDMULLER, 
    PHY_ENCODING_BINARYGOLAY,
    PHY_ENCODING_TERNARYGOLAY, 
    PHY_ENCODING_EXTENDEDTERNARYGOLAY,
    PHY_ENCODING_HADAMARD, 
    PHY_ENCODING_HAGELBARGER, 
    PHY_ENCODING_HAMMING, 
    PHY_ENCODING_BCH,   // (Bose, Ray-Chaudhuri, Hocquenghem)
    PHY_ENCODING_LDPC,  // (low density parity check code)
    PHY_ENCODING_STBC,  // (space time block code) 
    PHY_ENCODING_STTC,  // (space time trellis code)
    PHY_ENCODING_DSTC,  // (differential space time code)
    PHY_ENCODING_DUALMODULARREDUNDANCY,
    PHY_ENCODING_CYCLIC,
    PHY_ENCODING_LT, 
    PHY_ENCODING_TURBO,
    PHY_ENCODING_RAPTOR,
    PHY_ENCODING_SPARSEGRAPH, 
    PHY_ENCODING_TORNADO, 
    PHY_ENCODING_WALSH,                 
    
    PHY_ENCODING_NONE  // Must be the last one
} PhyEncoding;

extern const char* encodingNames[PHY_ENCODING_NONE];

// For Block codes: Lower 32-bit field is N; higher 32-bit field is K.
//                  If N==1, then K is meaningless (it must implicitly be 1)
// For Cyclic codes: Lower 32-bit field is the degree
typedef UInt64 PhyCodingParameter;
// typedef struct {
//    UInt32 N;   // output encoding sequence length
//    UInt32 K;   // input data length
// } PhyCodingParameter;

#endif // PHY_SYNC_H
