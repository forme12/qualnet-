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
// PACKAGE     :: ANTENNA
// DESCRIPTION :: This file describes data structures and functions used by antenna models.
// **/
// /**
// PROTOCOL     :: Antenna
// SUMMARY      :: This protocol deals with the Antenna signalling and
//                 communication
// LAYER        :: Physical Layer
// STATISTICS   :: None
// CONFIG_PARAM :: ANTENNA-MODEL:  OMNIDIRECTIONAL
//                                 SWITCHED-BEAM
//                                 STEERABLE
//                                 PATTERNED
//
//                 ANTENNA-AZIMUTH-PATTERN-FILE ./default.antenna-azimuth
//                 ANTENNA-ELEVATION-PATTERN-FILE ./default.antenna-elevation
//                 ANTENNA-MODEL-CONFIG-FILE      ./default.antenna-models
//                 ANTENNA-GAIN             0.0
//                 ANTENNA-EFFICIENCY       0.8
//                 ANTENNA-MISMATCH-LOSS    0.3
//                 ANTENNA-CABLE-LOSS       0.0
//                 ANTENNA-CONNECTION-LOSS  0.2
//                 ANTENNA-HEIGHT           1.5
//                 PHY802.11-ESTIMATED-DIRECTIONAL-ANTENNA-GAIN 15.0
//
// VALIDATION   :: None
// IMPLEMENTED_FEATURES :: omnidirectional, switched beam , steerable
//                         antenna and patterned
// OMITTED_FEATURES :: None
// ASSUMPTIONS  :: pre-generated antenna pattern files are required.
// STANDARD     :: None
// CONFIG_PARAM ::
//                 MAC-802.11-DIRECTIONAL-ANTENNA-MODE                 YES |
//                                                                       NO
//                 MAC-802.11-DIRECTION-CACHE-EXPIRATION-TIME          2S
//                 MAC-802.11-DIRECTIONAL-NAV-AOA-DELTA-ANGLE          37.0
//                 MAC-802.11-DIRECTIONAL-SHORT-PACKET-TRANSMIT-LIMIT  4
//
// **/


#ifndef ANTENNA_H
#define ANTENNA_H

// /**
// CONSTANT :: ANTENNA_DEFAULT_HEIGHT : 1.5
// DESCRIPTION :: Default height of the antenna
// **/
#define ANTENNA_DEFAULT_HEIGHT                           1.5

// /**
// CONSTANT :: ANTENNA_DEFAULT_GAIN_dBi : 0.0
// DESCRIPTION :: Default gain of the antenna
// **/
#define ANTENNA_DEFAULT_GAIN_dBi                         0.0

// /**
// CONSTANT :: ANTENNA_DEFAULT_EFFICIENCY : 0.8
// DESCRIPTION :: Default efficiency of the antenna
// **/
#define ANTENNA_DEFAULT_EFFICIENCY                       0.8

// /**
// CONSTANT :: ANTENNA_DEFAULT_MISMATCH_LOSS_dB : 0.3
// DESCRIPTION :: Default mismatch loss of the antenna
// **/
#define ANTENNA_DEFAULT_MISMATCH_LOSS_dB                 0.3

// /**
// CONSTANT    :: ANTENNA_DEFAULT_CONNECTION_LOSS_dB :   0.2
// DESCRIPTION :: Default connection loss of the antenna
// **/
#define ANTENNA_DEFAULT_CONNECTION_LOSS_dB               0.2

// /**
// CONSTANT :: ANTENNA_DEFAULT_CABLE_LOSS_dB : 0.0
// DESCRIPTION :: Default cable loss of the antenna
// **/
#define ANTENNA_DEFAULT_CABLE_LOSS_dB                    0.0

// /**
// CONSTANT :: ANTENNA_LOWEST_GAIN_dBi : -10000.0
// DESCRIPTION :: Default minimum gain of the antenna
// **/
#define ANTENNA_LOWEST_GAIN_dBi                         -10000.0

// /**
// CONSTANT :: ANTENNA_DEFAULT_PATTERN : 0
// DESCRIPTION :: Default Pattern
// **/
#define ANTENNA_DEFAULT_PATTERN                 0

// /**
// CONSTANT :: ANTENNA_OMNIDIRECTIONAL_PATTERN : -1
// DESCRIPTION :: OMNIDIRECTIONAL PATTERN
// **/
#define ANTENNA_OMNIDIRECTIONAL_PATTERN                 -1

// /**
// CONSTANT :: ANTENNA_PATTERN_NOT_SET : -2
// DESCRIPTION :: Const for Pattern of antenna not set
// **/
#define ANTENNA_PATTERN_NOT_SET                         -2

// /**
// CONSTANT :: AZIMUTH_INDEX : 0
// DESCRIPTION :: Const for azimuth index of antenna Pattern
// **/
#define AZIMUTH_INDEX                   0

// /**
// CONSTANT :: ELEVATION_INDEX : 1
// DESCRIPTION :: Const for elevation index of antenna Pattern
// **/
#define ELEVATION_INDEX                 1

// /**
// CONSTANT :: MAX_ANTENNA_NUM_LINES : 30
// DESCRIPTION :: Const for the line number in the antennaModelInput
// **/
#define MAX_ANTENNA_NUM_LINES           30

// /**
// CONSTANT :: AZIMUTH_ELEVATION_INDEX : 2
// DESCRIPTION :: Const for the memory allocation of azimuth and elevation
//                gain array.
// **/
#define AZIMUTH_ELEVATION_INDEX         2

// /**
// CONSTANT :: NSMA_PATTERN_START_LINE_NUMBER : 10
// DESCRIPTION :: Const represents the basic pattern
//                starting point in NSMA file
// **/
#define NSMA_PATTERN_START_LINE_NUMBER        10

// /**
// CONSTANT :: NSMA_MAX_STARTLINE :          41
// DESCRIPTION :: Const represents the Revised pattern
//                max line number where the revised NSMA 
//                pattern can start.
// **/
#define NSMA_MAX_STARTLINE                   41


// /**
// FUNCTION :: ANTENNA_Init
// LAYER :: Physical Layer.
// PURPOSE :: Initialize antennas.
// PARAMETERS ::
// + node : Node* : node being initialized.
// + phyIndex  : int : interface for which physical to be
//                     initialized.
// + nodeInput : const NodeInput* : structure containing contents of input
//                                  file.
// RETURN :: void : NULL
// **/

void ANTENNA_Init(
     Node* node,
     int   phyIndex,
     const NodeInput* nodeInput);


// /**
// FUNCTION :: ANTENNA_ReadPatterns
// LAYER  :: Physical Layer.
// PURPOSE :: Read in the azimuth pattern file.
// PARAMETERS ::
// + node : Node* : node being used.
// + phyIndex : int : interface for which physical to be
//                    initialized.
// + antennaInput : const NodeInput* : structure containing contents of
//                                     input file.
// + numPatterns  : int* : contains the number of patterns
//                         in the pattern file.
// + steerablePatternSetRepeatSectorAngle : int* : contains
//                                          PatternSetRepeatSectorAngle
//                                          for steerable antenna.
// + pattern_dB : float*** : array used to store the gain values
//                           of the pattern file.
// + azimuthPlane : BOOL : shows whether the file is azimuth
//                         file or elevation file.
// RETURN :: void : NULL
// **/

void ANTENNA_ReadPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int* numPatterns,
     int* steerablePatternSetRepeatSectorAngle,
     float*** pattern_dB,
     BOOL azimuthPlane);


// /**
// FUNCTION :: ANTENNA_ReadNsmaPatterns
// LAYER :: Physical Layer
// PURPOSE :: Read in the NSMA pattern file.
// PARAMETERS ::
// + node : Node*  : node being used.
// + phyIndex : int : interface for which physical
//                    to be initialized.
// +antennaInput : NodeInput* : structure containing contents of
//                              input file.
// +numPatterns : int : number of patterns in the file.
// +azimuthPattern_dB : float*** : pattern_dB array for
//                                 azimuth gains.
// +azimuthResolution : int* : azimuth resolution
//                     and azimuth range.
// +elevationPattern_dB : float*** : pattern_dB array
//                                   for elevation gains.
// +elevationResolution : int* : elevation resolution
//                     and elevation range.
// +NSMAPatternVersion* :  version version of NSMA pattern
// RETURN :: void : NULL
// **/

void ANTENNA_ReadNsmaPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int numPatterns,
     float*** azimuthPattern_dB,
     int* azimuthResolution,
     float*** elevationPattern_dB,
     int* elevationResolution,
     NSMAPatternVersion* version);

// /**
// FUNCTION :: ANTENNA_ReadRevisedNsmaPatterns
// LAYER :: Physical Layer
// PURPOSE :: Read in the Revised NSMA pattern file.
// PARAMETERS ::
// + node : Node*  : node being used.
// + phyIndex : int : interface for which physical
//                    to be initialized.
// +antennaInput : NodeInput* : structure containing contents of
//                              input file.
// +numPatterns : int : number of patterns in the file.
// +azimuthPattern_dB : float*** : pattern_dB array for
//                                 azimuth gains.
// +aziRatio : float : the ratio of azimuth resolution
//                     and azimuth range.
// +elevationPattern_dB : float*** : pattern_dB array
//                                   for elevation gains.
// +elvRatio : float : the ratio of elevation resolution
//                     and elevation range.
// RETURN :: void : NULL
// **/

void ANTENNA_ReadRevisedNsmaPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     int numPatterns,
     float*** azimuthPattern_dB,
     float aziRatio,
     float*** elevationPattern_dB,
    float elvRatio);


// /**
// FUNCTION :: ANTENNA_Read3DAsciiPatterns
// LAYER :: Physical Layer.
// PURPOSE :: Used to read ASCII 3D pattern file.
// PARAMETERS ::
// + node : Node* : node being used.
// + phyIndex : int : interface for which physical
//                    to be initialized.
// +antennaInput : NodeInput* : structure containing contents of
//                              input file.
// +antennaPatterns : AntennaPattern* : Pointer to Global Antenna Pattern
//                                      structure.
// RETURN :: void : NULL
// **/

void ANTENNA_Read3DAsciiPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     AntennaPattern* antennaPatterns);


// /**
// FUNCTION :: ANTENNA_Read2DAsciiPatterns
// LAYER :: Physical Layer.
// PURPOSE :: Used to read ASCII 2D pattern file.
// PARAMETERS ::
// + node : Node* : node being used.
// + phyIndex : int : interface for which physical
//                    to be initialized.
// +antennaInput : NodeInput* : structure containing contents of
//                              input file.
// +antennaPatterns : AntennaPattern* : Pointer to Global Antenna Pattern
//                                      structure.
// +azimuthPlane : BOOL : A boolean variable to differentiate the file
//                        azimuth or elevation.
// +conversionParameter : const float : conversion parameter to change
//                                      the dB values in dBi.
// RETURN :: void : NULL
// **/

void ANTENNA_Read2DAsciiPatterns(
     Node* node,
     int phyIndex,
     const NodeInput* antennaInput,
     AntennaPattern* antennaPatterns,
     BOOL azimuthPlane,
     const float conversionParameter);


// /**
// FUNCTION :: ANTENNA_OmniDirectionalInit
// LAYER :: Physical Layer.
// PURPOSE :: Initialize omnidirectional antenna
//            from the antenna model file.
// PARAMETERS ::
// + node : Node* : node being initialized.
// + nodeInput : const NodeInput* : pointer to node input
// + phyIndex : int : interface for which physical to be
//                    initialized.
// + antennaModel : const  AntennaModelGlobal* : pointer to AntennaModelGlobal
//                                               structure.
// RETURN :: void : NULL
// **/

void ANTENNA_OmniDirectionalInit(
     Node* node,
     const NodeInput* nodeInput,
     int phyIndex,
     const  AntennaModelGlobal* antennaModel);


// /**
// FUNCTION :: ANTENNA_OmniDirectionalInitFromConfigFile
// LAYER :: Physical Layer.
// PURPOSE : Initialize omnidirectional antenna
//           from the default.config file.
// PARAMETERS ::
// + node : Node* : node being initialized.
// + phyIndex : int : interface for which physical to be
//                    initialized.
// + nodeInput : const NodeInput* : structure containing contents of input
//                                  file.
// RETURN :: void : NULL
// **/

void ANTENNA_OmniDirectionalInitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);



// /**
// FUNCTION :: ANTENNA_InitFromConfigFile
// LAYER :: Physical Layer.
// PURPOSE : Initialize antenna from the default.config file.
// PARAMETERS ::
// + node : Node* : node being initialized.
// + phyIndex : int : interface for which physical to be
//                    initialized.
// + nodeInput : const NodeInput* : structure containing contents of input
//                                  file.
// RETURN :: void : NULL
// **/

void ANTENNA_InitFromConfigFile(
     Node* node,
     int phyIndex,
     const NodeInput* nodeInput);


// /**
// FUNCTION :: ANTENNA_IsInOmnidirectionalMode
// LAYER :: Physical Layer
// PURPOSE :: Is antenna in omnidirectional mode.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be use
// RETURN :: BOOL : returns TRUE if antenna is in
//                  omnidirectional mode
// **/

BOOL ANTENNA_IsInOmnidirectionalMode(
    Node *node,
    int phyIndex);

// /**
// FUNCTION :: ANTENNA_ReturnPatternIndex
// LAYER :: Physical Layer
// PURPOSE :: Return nodes current pattern index.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to use
// RETURN :: int : returns pattern index
// **/

int ANTENNA_ReturnPatternIndex(
    Node* node,
    int phyIndex);

// /**
// FUNCTION :: ANTENNA_ReturnHeight
// LAYER :: Physical Layer
// PURPOSE :: Return nodes antenna height.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: float : height in meters
// **/

float ANTENNA_ReturnHeight(
     Node* node,
     int phyIndex);

// /**
// FUNCTION :: ANTENNA_ReturnSystemLossIndB
// LAYER :: Physical Layer
// PURPOSE :: Return systen loss in dB.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: double : loss in dB
// **/

double ANTENNA_ReturnSystemLossIndB(
       Node* node,
       int phyIndex);

// /**
// FUNCTION :: ANTENNA_GainForThisDirection
// LAYER :: Physical Layer
// PURPOSE :: Return gain for this direction in dB.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + DOA : Orientation : direction of antenna
// RETURN :: float : gain in dB
// **/

float ANTENNA_GainForThisDirection(
      Node* node,
      int phyIndex,
      Orientation DOA);


// /**
// FUNCTION :: ANTENNA_GainForThisDirectionWithPatternIndex
// LAYER :: Physical Layer
// PURPOSE ::  Return gain for this direction for the specified pattern in
//             dB.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + patternIndex : int : pattern index to use
// + DOA : Orientation : direction of antenna
// RETURN :: float : gain in dB
// **/

float ANTENNA_GainForThisDirectionWithPatternIndex(
      Node* node,
      int phyIndex,
      int patternIndex,
      Orientation DOA);


// /**
// FUNCTION :: ANTENNA_GainForThisSignal
// LAYER :: Physical Layer
// PURPOSE :: Return gain in dB.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + propRxInfo : PropRxInfo* : receiver propagation info
// RETURN :: float : gain in dB
// **/

float ANTENNA_GainForThisSignal(
      Node* node,
      int phyIndex,
      PropRxInfo* propRxInfo);


// /**
// FUNCTION :: ANTENNA_DefaultGainForThisSignal
// LAYER :: Physical Layer
// PURPOSE :: Return default gain in dB.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + propRxInfo : PropRxInfo* : receiver propagation info
// RETURN :: float : gain in dB
// **/

float ANTENNA_DefaultGainForThisSignal(
      Node* node,
      int phyIndex,
      PropRxInfo *propRxInfo);


// /**
// FUNCTION :: ANTENNA_LockAntennaDirection
// LAYER :: Physical Layer
// PURPOSE :: Lock antenna to current direction.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: void : NULL
// **/

void ANTENNA_LockAntennaDirection(
     Node* node,
     int phyIndex);


// /**
// FUNCTION :: ANTENNA_UnlockAntennaDirection
// LAYER :: Physical Layer
// PURPOSE :: Unlock antenna.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: void : NULL
// **/

void ANTENNA_UnlockAntennaDirection(
     Node* node,
     int phyIndex);


// /**
// FUNCTION :: ANTENNA_DirectionIsLocked
// LAYER :: Physical Layer
// PURPOSE :: Return if direction antenna is locked.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: BOOL : returns TRUE if the antenna direction is
//                  locked
// **/

BOOL ANTENNA_DirectionIsLocked(
    Node *node,
    int phyIndex);

// /**
// FUNCTION :: ANTENNA_IsLocked
// LAYER :: Physical Layer
// PURPOSE :: Return if antenna is locked.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: BOOL : Returns TRUE if antenna is locked.
// **/

BOOL ANTENNA_IsLocked(
    Node *node,
    int phyIndex);

// /**
// FUNCTION :: ANTENNA_SetToDefaultMode
// LAYER :: Physical Layer
// PURPOSE :: Set default antenna mode (usally omni).
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// RETURN :: void : NULL
// **/

void ANTENNA_SetToDefaultMode(
     Node* node,
     int phyIndex);


// /**
// FUNCTION :: ANTENNA_SetToBestGainConfigurationForThisSignal
// PURPOSE :: Set antenna for best gain using the Rx info.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + propRxInfo : PropRxInfo* : receiver propagation info
// RETURN :: void : NULL
// **/

void ANTENNA_SetToBestGainConfigurationForThisSignal(
     Node* node,
     int phyIndex,
     PropRxInfo* propRxInfo);


// /**
// FUNCTION :: ANTENNA_SetBestConfigurationForAzimuth
// LAYER :: Physical Layer
// PURPOSE :: Set antenna for best gain using the azimuth.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + azimuth : double : the azimuth
// RETURN :: void : NULL
// **/

void ANTENNA_SetBestConfigurationForAzimuth(
     Node* node,
     int phyIndex,
     double azimuth);

// /**
// FUNCTION :: ANTENNA_GetSteeringAngle
// LAYER :: Physical Layer
// PURPOSE :: Get steering angle of the antenna.
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + angle    : Orientation* : For returning the angle
// RETURN :: void : NULL
// **/

void ANTENNA_GetSteeringAngle(
     Node* node,
     int phyIndex,
     Orientation* angle);

// /**
// FUNCTION :: ANTENNA_SetSteeringAngle
// LAYER :: Physical Layer
// PURPOSE :: Set the steering angle of the antenna
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical to be used
// + angle : Orientation : Steering angle to be
// RETURN :: void : NULL
// **/

void ANTENNA_SetSteeringAngle(
     Node* node,
     int phyIndex,
     Orientation angle);

// /**
// FUNCTION :: ANTENNA_ReturnAsciiPatternFile
// LAYER :: Physical Layer
// PURPOSE :: Read in the ASCII pattern .
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which physical
//                    to be initialized
// + antennaModelInput : const NodeInput* : structure containing
//                                          contents of input
//                                          file
// +antennaPatterns : AntennaPatterns* : Pointer to the global
//                                       antenna pattern structure.
// RETURN :: void : NULL
// **/

void ANTENNA_ReturnAsciiPatternFile(
     Node* node,
     int phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern*  antennaPatterns);


// /**
// FUNCTION :: ANTENNA_ReturnNsmaPatternFile
// LAYER :: Physical Layer
// PURPOSE :: Read in the NSMA pattern .
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which
//                    physical to be initialized
// + antennaModelInput : const NodeInput* : structure containing
//                                          contents of input
//                                          file
// + antennaPatterns : AntennaPatterns* : Pointer to
//                                       the global antenna
//                                       pattern structure.
// RETURN :: void : NULL
// **/

void ANTENNA_ReturnNsmaPatternFile(
     Node* node,
     int  phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern*  antennaPatterns);


// /**
// FUNCTION :: ANTENNA_ReturnTraditionalPatternFile
// LAYER :: Physical Layer.
// PURPOSE :: Used to read Qualnet Traditional pattern file
// PARAMETERS ::
// + node : Node* : node being used
// + phyIndex : int : interface for which
//                    physical to be initialized
// + antennaModelInput : const NodeInput* : structure containing
//                                          contents of input
//                                          file
// +antennaPatterns : AntennaPatterns* : Pointer to
//                                       the global antenna
//                                       pattern structure.
// RETURN :: void : NULL
// **/

void ANTENNA_ReturnTraditionalPatternFile(
     Node* node,
     int phyIndex,
     const NodeInput* antennaModelInput,
     AntennaPattern* antennaPatterns);


// /**
// FUNCTION :: ANTENNA_MakeAntennaModelInput
// LAYER :: Physical Layer.
// PURPOSE :: Reads the antenna configuration parameters into
//            the NodeInput structure.
// PARAMETERS ::
// + node : Node* : node being used
//                  physical to be initialized
// + buf : char* : Path to input file.
// RETURN :: NodeInput * : pointer to nodeInput structure
// **/

NodeInput* ANTENNA_MakeAntennaModelInput(
           const NodeInput* nodeInput,
           char* buf);

#endif
