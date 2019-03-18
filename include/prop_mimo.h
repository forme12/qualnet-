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
// PACKAGE     :: PROPAGATION
// DESCRIPTION :: This file describes data structures and functions used by propagation models.
// **/

#ifndef PROP_MIMO_H
#define PROP_MIMO_H

#include "util_mini_matrix.h"

/* enum MIMO_ChannelModel 
 * MIMO spatial channel model types
 */
enum MIMO_ChannelModel {
    MIMO_TGN_A, 
    MIMO_TGN_B, 
    MIMO_TGN_C, 
    MIMO_TGN_D, 
    MIMO_TGN_E, 
    MIMO_TGN_F, 
    MIMO_SUI_1,
    MIMO_SUI_2,
    MIMO_SUI_3,
    MIMO_SUI_4,
    MIMO_SUI_5,
    MIMO_SUI_6,
};


const size_t MIMO_NUM_MPATH_TGNA = 1;
const size_t MIMO_NUM_MPATH_TGNB = 9;
const size_t MIMO_NUM_MPATH_TGNC = 14;
const size_t MIMO_NUM_MPATH_TGND = 18;
const size_t MIMO_NUM_MPATH_TGNE = 18;
const size_t MIMO_NUM_MPATH_TGNF = 18;
const size_t MIMO_NUM_MPATH_SUI  = 3;

/* struct MIMO_MpathProfile
 * MIMO spatial channel model multipath profile,
 * Define average power reduction and delay for each multipath component.
 */
struct MIMO_MpathProfile {
    double  avgPower;           //in DB
    double  rltDelay;           //in ns
};

//TGn model 'A'
//Flat fading with 0 ns rms delay spread (one tap at 0 ns delay model)
const MIMO_MpathProfile MIMO_MProf_TgnA[MIMO_NUM_MPATH_TGNA] = 
    { {0.0, 0.0} };

//TGn model 'B'
//Typical residential environment, 15 ns rms delay spread
const MIMO_MpathProfile MIMO_MProf_TgnB[MIMO_NUM_MPATH_TGNB] = 
    { { 0.0,  0.0  }, 
      {-5.42, 10.0e-9 },
      {-2.51, 20.0e-9 },
      {-5.89, 30.0e-9 }, 
      {-9.16, 40.0e-9 }, 
      {-12.51,50.0e-9 }, 
      {-15.61,60.0e-9 }, 
      {-18.71,70.0e-9 },
      {-21.81,80.0e-9 } };

//TGn model 'C'
//Typical residential or small office environment, 30 ns rms delay spread
const MIMO_MpathProfile MIMO_MProf_TgnC[MIMO_NUM_MPATH_TGNC] = 
    { {  0.0,   0.0     }, 
      {-2.17,   10.0e-9 }, 
      {-4.34,   20.0e-9 }, 
      {-6.51,   30.0e-9 }, 
      {-8.68,   40.0e-9 }, 
      {-10.85,  50.0e-9 }, 
      {-4.38,   60.0e-9 }, 
      {-6.56,   70.0e-9 }, 
      {-8.73,   80.0e-9 }, 
      {-10.90,  90.0e-9 }, 
      {-13.71, 110.0e-9 }, 
      {-15.88, 140.0e-9 }, 
      {-18.05, 170.0e-9 }, 
      {-20.22, 200.0e-9 } };

//TGn model 'D'
//Medbo model A - Typical office environment, 50 ns rms delay spread
const MIMO_MpathProfile MIMO_MProf_TgnD[MIMO_NUM_MPATH_TGND] = 
    { {0.0,   0.0    }, 
      {-0.9,  10.0e-9}, 
      {-1.7,  20.0e-9}, 
      {-2.6,  30.0e-9}, 
      {-3.5,  40.0e-9}, 
      {-4.3,  50.0e-9}, 
      {-5.2,  60.0e-9}, 
      {-6.1,  70.0e-9}, 
      {-6.9,  80.0e-9}, 
      {-7.8,  90.0e-9}, 
      {-4.7, 110.0e-9}, 
      {-7.3, 140.0e-9}, 
      {-9.9, 170.0e-9}, 
      {-12.5,200.0e-9}, 
      {-13.7,240.0e-9}, 
      {-18.0,290.0e-9}, 
      {-22.4,340.0e-9}, 
      {-26.7,390.0e-9} };

//TGn model 'E'
//Medbo model B - Typical large open space and office environments,
//100 ns rms delay spread
const MIMO_MpathProfile MIMO_MProf_TgnE[MIMO_NUM_MPATH_TGNE] = 
    { {-2.5,  0.0    }, 
      {-3.0,  10.0e-9}, 
      {-3.5,  20.0e-9}, 
      {-3.9,  30.0e-9}, 
      {0.0,   50.0e-9}, 
      {-1.3,  80.0e-9}, 
      {-2.6, 110.0e-9}, 
      {-3.9, 140.0e-9}, 
      {-3.4, 180.0e-9}, 
      {-5.6, 230.0e-9}, 
      {-7.7, 280.0e-9}, 
      {-9.9, 330.0e-9}, 
      {-12.1,380.0e-9}, 
      {-14.3,430.0e-9}, 
      {-15.4,490.0e-9}, 
      {-18.4,560.0e-9}, 
      {-20.7,640.0e-9}, 
      {-24.6,730.0e-9} };

//TGn model 'F'
// Medbo model C - Large open space (indoor and outdoor),
// 150 ns rms delay spread
const MIMO_MpathProfile MIMO_MProf_TgnF[MIMO_NUM_MPATH_TGNF] = 
    { {-3.3,   0.0    }, 
      {-3.6,   10.0e-9}, 
      {-3.9,   20.0e-9}, 
      {-4.2,   30.0e-9}, 
      { 0.0,   50.0e-9}, 
      {-0.9,   80.0e-9}, 
      {-1.7,  110.0e-9}, 
      {-2.6,  140.0e-9}, 
      {-1.5,  180.0e-9}, 
      {-3.0,  230.0e-9}, 
      {-4.4,  280.0e-9}, 
      {-5.9,  330.0e-9}, 
      {-5.3,  400.0e-9}, 
      {-7.9,  490.0e-9}, 
      {-9.4,  600.0e-9}, 
      {-13.2, 730.0e-9}, 
      {-16.3, 880.0e-9}, 
      {-21.2,1050.0e-9} };

const MIMO_MpathProfile MIMO_MProf_Sui_1[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-15.0, 400.0e-9 },
      {-20.0, 900.0e-9 } };

const MIMO_MpathProfile MIMO_MProf_Sui_2[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-12.0, 400.0e-9 },
      {-15.0, 1.1e-9   } };

const MIMO_MpathProfile MIMO_MProf_Sui_3[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-5.0,  400.0e-9 },
      {-10.0, 900.0e-9 } };

const MIMO_MpathProfile MIMO_MProf_Sui_4[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-4.0,  1.5e-6   },
      {-8.0,  4.0e-6   } };

const MIMO_MpathProfile MIMO_MProf_Sui_5[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-5.0,  4.0e-6   },
      {-10.0, 10.0e-6  } };

const MIMO_MpathProfile MIMO_MProf_Sui_6[MIMO_NUM_MPATH_SUI] = 
    { { 0.0,  0.0      },
      {-10.0, 14.0e-6  },
      {-14.0, 20.0e-6  } };

/* struct MIMO_ChannelProfile 
 * MIMO channel profile
 */
struct MIMO_ChannelProfile {
    size_t              numMpathCpnts;  //Number of multipath components
    const MIMO_MpathProfile*  mpath;   //Pointer to the multipath profile

    MIMO_ChannelProfile() 
    {
        numMpathCpnts = MIMO_NUM_MPATH_TGNA;
        mpath  = MIMO_MProf_TgnA;
    }

    void SetModel (MIMO_ChannelModel model) 
    {
        switch (model) {
            case MIMO_TGN_A: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNA;
                mpath  = MIMO_MProf_TgnA;
                break;
            case MIMO_TGN_B: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNB;
                mpath  = MIMO_MProf_TgnB;
                break;
            case MIMO_TGN_C: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNC;
                mpath  = MIMO_MProf_TgnC;
                break;
            case MIMO_TGN_D: 
                numMpathCpnts = MIMO_NUM_MPATH_TGND;
                mpath  = MIMO_MProf_TgnD;
                break;
            case MIMO_TGN_E: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNE;
                mpath  = MIMO_MProf_TgnE;
                break;
            case MIMO_TGN_F: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNF;
                mpath  = MIMO_MProf_TgnF;
                break;
            case MIMO_SUI_1:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_1;
                break;
            case MIMO_SUI_2:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_2;
                break;
            case MIMO_SUI_3:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_3;
                break;
            case MIMO_SUI_4:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_4;
                break;
            case MIMO_SUI_5:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_5;
                break;
            case MIMO_SUI_6:
                numMpathCpnts = MIMO_NUM_MPATH_SUI;
                mpath  = MIMO_MProf_Sui_6;
                break;
            default: 
                numMpathCpnts = MIMO_NUM_MPATH_TGNA;
                mpath  = MIMO_MProf_TgnA;
        }
    }
};

/**********************************************************
 * 802.11n MIMO APIs
 **********************************************************/
/*
 * MIMO_EstimateChnlMatrix
 * Get the estimation of MIMO channel matrix
 * Arguments
 *   propProfile:       PropProfile:    channel propagation profile
 *   seed:              RandomSeed:     Random seed
 *   txAtnaElmtSpace:   double:         transmit antenna element space  
 *   txNumAtnaElmts:    int:            number of tranmit antenna elements
 *   rxAtnaElmtSpace:   double:         receive antenna element space  
 *   rxNumAtnaElmts:    int:            number of receive antenna space
 *   angleOfDeparture:  Orientation:    angle of signal departure
 *   angleOfArrival:    Orientation:    angle of signal arrival
 * Return
 *   CplxMiniMatrix:    channel estimation matrix
 */
CplxMiniMatrix MIMO_EstimateChnlMatrix(
        PropProfile* propProfile,
        RandomSeed&  seed,
        double       txAtnaElmtSpace,
        int          txNumAtnaElmts,
        double       rxAtnaElmtSpace,
        int          rxNumAtnaElmts,
        Orientation  angleOfDeparture,
        Orientation  angleOfArrival);

/* MIMO_EstimateChnlCapacity
 * Estimate MIMO channel capacity
 * Arguments
 * numSs::          size_t:         number of spatial streams
 * channelMatrix::  CplxMiniMatrix: channel matrix estimations
 * SINR::           double:         signal to noise ratio
 * bandwidth:       double:         channel bandwidth
 * Return::         double:         estimated capacity
 */
double MIMO_EstimateChnlCapacity(
        std::size_t numSs,
        CplxMiniMatrix channelMatrix,
        double SINR,
        double bandwidth);

#endif // PROP_MIMO_H

