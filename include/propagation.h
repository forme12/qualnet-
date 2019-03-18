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

#ifndef PROPAGATION_H
#define PROPAGATION_H

#include "main.h"
#include "coordinates.h"
#include "fileio.h"
#include "random.h"
#include "dynamic.h"
#include "terrain.h"
#include "prop_mimo.h"

#include <map>
#include <string>
#include <vector>

#include <algorithm>
using namespace::std;

// /**
// CONSTANT    :: BOLTZMANN_CONSTANT : 1.379e-23
// DESCRIPTION :: Boltzmann constant
// **/
#define BOLTZMANN_CONSTANT 1.379e-23

// /**
// CONSTANT    :: NEGATIVE_PATHLOSS_dB  : -1.0
// DESCRIPTION :: Path loss in dB (used as an invalid value)
// **/
#define NEGATIVE_PATHLOSS_dB -1.0

// /**
// CONSTANT    :: SPEED_OF_LIGHT  : 3.0e8
// DESCRIPTION :: Defines the value of speed of light
// **/
#define SPEED_OF_LIGHT                3.0e8

// /**
// CONSTANT    :: PROP_DEFAULT_PROPAGATION_LIMIT_dBm  : -111.0
// DESCRIPTION :: Default value for propagation limit.
// **/
#define PROP_DEFAULT_PROPAGATION_LIMIT_dBm -111.0

// /**
// CONSTANT    :: PROP_DEFAULT_SHADOWING_MEAN_dB  : 4.0
// DESCRIPTION :: Default mean value for shadowing in dB
// **/
#define PROP_DEFAULT_SHADOWING_MEAN_dB 4.0
//PL_OPAR_PROP
#define PROP_DEFAULT_INTER_CITY_OBSTRUCTION_DENSITY_FACTOR 0.04
#define PROP_DEFAULT_INTRA_CITY_OBSTRUCTION_DENSITY_FACTOR 0.02
#define PROP_DEFAULT_INTER_CITY_FOLIAGE_OBSTRUCTION_DENSITY_FACTOR 0.4
#define PROP_DEFAULT_INTRA_CITY_FOLIAGE_OBSTRUCTION_DENSITY_FACTOR 0.2

// /**
// CONSTANT    :: MAX_NUM_ELEVATION_SAMPLES : 16384
// DESCRIPTION :: Maximum number of sample would be taken.
// **/
#define MAX_NUM_ELEVATION_SAMPLES 16384

// /**
// CONSTANT    :: PROP_DEFAULT_BANDWIDTH_FACTOR : 2.0
// DESCRIPTION :: The bandwidth factor that is used to get the half sum bandwidth.
// **/
#define PROP_DEFAULT_BANDWIDTH_FACTOR 2.0

// /**
// MACRO       :: PROP_NumberChannels(node)
// DESCRIPTION :: Get the number of channel.
// **/
#define PROP_NumberChannels(node) ((node)->numberChannels)

// /**
// MACRO       :: PROP_ChannelWavelength(node, channelIndex)
// DESCRIPTION :: Get wavelength of channel having index channelIndex
// **/
#define PROP_ChannelWavelength(node, channelIndex) \
            ((node)->partitionData->propChannel[(channelIndex)].profile->wavelength)


#define HORIZONTAL 0
#define VERTICAL 1

// /**
// ENUM        :: PathlossModel
// DESCRIPTION :: Different type of path loss.
// **/
enum PathlossModel {
    FREE_SPACE = 0,
    TWO_RAY,
    PL_MATRIX,
    OPAR,
    ITM,
    TIREM,
    OKUMURA_HATA,
    ASAPS,
    RFPS,
    COST231_HATA,
    COST231_WALFISH_IKEGAMI,
    ITU_R,
    URBAN_MODEL_AUTOSELECT,
    STREET_MICROCELL,
    STREET_M_TO_M,
    INDOOR,
    SUBURBAN_FOLIAGE,
    PL_OPAR,
    PL_OPAR_PROP,
    FLAT_BINNING
};

// /**
// ENUM        :: ShadowingModel
// DESCRIPTION :: Different type of shadowing used.
// **/
enum ShadowingModel {
    CONSTANT = 0,
    LOGNORMAL
};

// /**
// ENUM        :: FadingModel
// DESCRIPTION :: Different type of fading used.
// **/
enum FadingModel {
    NONE = 0,
    RICEAN
};

// /**
// ENUM        :: propagationEnvironment
// DESCRIPTION :: Different type of propagation environment.
// **/
enum PropagationEnvironment {
    OPEN_RURAL = 0,
    QUASI_OPEN_RURAL,
    SUBURBAN,
    URBAN,
    METROPOLITAN  // building heights greater than 15 m
};

// /**
// ENUM        :: LoSIndicator
// DESCRIPTION :: Indicated if the path is Line of sight OR non-Line of sight
// **/
enum LoSIndicator {
    LOS = 0,
    NLOS
};


// /**
// ENUM        :: SuburbanTerrainType
// DESCRIPTION :: Terrain types for Suburban-foliage model
// **/

enum SuburbanTerrainType{
    HILLY_TERRAIN_WITH_MOD_TO_HEAVY_TREE_DENSITY = 0,
    FLAT_TERRAIN_WITH_MOD_TO_HEAVY_TREE_DENSITY,
    FLAT_TERRAIN_WITH_LIGHT_TREE_DENSITY
};


// /**
// ENUM        :: IndoorLinkType
// DESCRIPTION :: Link types for Indoor model
// **/

enum IndoorLinkType {
    RR = 0, //Room-to-Room
    CC,     //Corridor-to-Corridor
    OO,     //Open area-to-Open area
    RC,     //Room-to-Corridor
    CO,     //Corridor-to-Open area
    RO      //Room-to-Open area
};

enum ObstructionType {
    OBSTRUCTION_BUILDING = 0,
    OBSTRUCTION_FOLIAGE
};

// /**
// ENUM        :: LinkType
// DESCRIPTION :: Link types for model
// **/
enum LinkType {
    OUTDOOR_ONLY = 0,
    INDOOR_ONLY,
    HETEROGENEOUS
};

// /**
// STRUCT      :: PropPathProfile
// DESCRIPTION :: Structure that keeps track of all propertice of a path.
// **/
struct PropPathProfile {
    clocktype   propDelay;
    double      distance;
    Orientation txDOA;
    Orientation rxDOA;
    double      rxPower_dBm;        // rx power at receiver
    double      pathloss_dB;
    double      fading_dB;
    double      channelReal;        // for cooperative comm
    double      channelImag;        // for cooperative comm
    double      rxFrequency;        // frequency at receiver side
    Coordinates fromPosition;
    Coordinates toPosition;
    int         sequenceNum;
    double      weatherPathloss_dB; // pathloss due to weather
    int         weatherSequenceNum;
};

struct PropGridMatrix {
    std::map<int, PropPathProfile> gridMatrix;
};

// /**
// STRUCT      :: PropChannel
// DESCRIPTION :: structure of a channel.
// **/
struct PropChannel {
    int       numNodes;
    Node**    nodeList;

    int       numNodesWithLI; // LI: Limited Interference
    Node**    nodeListWithLI;

    int       profileIndex;

    int maxNumNodes;
    PropProfile* profile;
};

//PL_OPAR_PROP
struct Obstruction {
    ObstructionType obstructiontype;
    Coordinates southwestOrLowerLeft;
    Coordinates northeastOrUpperRight;
    double intraCityObstructionDensityFactor;
    double interCityObstructionDensityFactor;
};

//PL_OPAR_PROP
struct PathlossArea {
    PathlossModel pathlossModel;
    PathlossModel pathlossModelPrimary;
    Coordinates southwestOrLowerLeft;
    Coordinates northeastOrUpperRight;
};

struct pathLossMatrixValue{
    clocktype simTime;
    string values;
};
// /**
// STRUCT      :: PropProfile
// DESCRIPTION :: Main structure of propagation profile
// **/
struct PropProfile {
    int       profileIndex;

    double    propLimit_dB;
    D_Float64    propMaxDistance;
    //double    propCommunicationProximity;
    //double    propProfileUpdateRatio;
    D_Float64 propCommunicationProximity;
    D_Float64 propProfileUpdateRatio;
    double    frequency;
    double    antennaHeight;
    double    wavelength;

    PathlossModel pathlossModel;

    float elevationSamplingDistance;
    int   climate;
    double refractivity;
    double conductivity;
    double permittivity;
    double humidity;
    int   polarization;
    char polarizationString[5];

    ShadowingModel shadowingModel;
    double shadowingMean_dB;

    FadingModel fadingModel;
    double kFactor;
    double dopplerFrequency;

    double  baseDopplerFrequency;
    D_Int32     samplingRate;
    int     numGaussianComponents;
    double* gaussianComponent1;
    double* gaussianComponent2;

    int       numChannelsInMatrix;
    int*      channelIndexArray;
    int       numNodesInMatrix;
    vector<pathLossMatrixValue>      matrixList;

    void *propGlobalVar;

    // high frequency
    double monthofyear;
    double dayofmonth;
    double timeofday;
    double Tindex;
    double mintoa;
    double hfTxpower;
    double reqSnr;
    double manMadenoise;
    double hfbandwidth;
    double reqPercentageDay;
    void   *TxantennaData;
    void   *RxantennaData;

    // ASAPS-sepcific
    char asapsPath[MAX_STRING_LENGTH];

    // motion
    BOOL motionEffectsEnabled;

    // OKUMURA_HATA, COST231_Hata, COST231_Walfish_Ikegami,ITU-R
    PropagationEnvironment propagationEnvironment;

    //COST231_Walfish_Ikegami
    double roofHeight;
    double streetWidth;
    double buildingSeparation;

    //Added for URBAN-AUTOSELECT-MODEL
    double RelativeNodeOrientation;
    double MaxRoofHeight;
    double MinRoofHeight;

    //Street_Microcell model, Indoor
    LoSIndicator losIndicator;

    //Street_M_to_M model
    int Num_builings_in_path;

    //Suburban-Foliage Model
    SuburbanTerrainType suburbanTerrainType;

    // PL_OPAR
    PathlossModel pathlossModelPrimary;

    // PL_OPAR_PROP
    Obstruction* obstructions;
    int numObstructions;

    PathlossArea* pathlossArea;
    int numPathlossAreas;

    // MIMO channel profile
    MIMO_ChannelProfile mimoProfile;

    TERRAIN::ConstructionMaterials  constructionMaterials;
    BOOL enableChannelOverlapCheck;
};


// /**
// STRUCT      :: PropData
// DESCRIPTION :: Main structure of propagation data.
// **/
struct PropData {
    int   numPhysListenable;
    int   numPhysListening;
    BOOL* phyListening;
    BOOL  limitedInterference;

    RandomDistribution<double> shadowingDistribution;
    int  nodeListId;

    int  numSignals;

    PropRxInfo* rxSignalList;

    double fadingStretchingFactor;

    PropPathProfile* pathProfile;

    void *propVar;
    int numPathLossCalculation;
};

// /**
// STRUCT      :: PropTxInfo
// DESCRIPTION :: This structure is used for fields related to channel layer
//                information that need to be sent with a message.
// **/
struct PropTxInfo {
    clocktype txStartTime; // signal airborne time
    clocktype duration;    // signal duration
    double  txPower_dBm;   // transmit power in dBm
    int     dataRate;      // data rate
    short   phyIndex;      // transmitter index
    short   numReferenced; // number of receivers referencing this message
    int     patternIndex;  // antenna pattern
    int     sequenceNum;   // needed for checking if the path profile is up to date
    int     txNodeId;      // required for distributed memory parallel
    Node*   txNode;
    Coordinates position;
    Coordinates lastPosition;  // temporary substitute for velocity
    double      speed;         // ditto
    Orientation orientation;
    unsigned char txPhyModel;  // The PhyModel of the transmitter

    //AntennaModelType antennaModelType; // used by kernel
    Orientation      steeringAngle;    // used by kernel

    // MIMO related parameters
    int           txNumAtnaElmts;    //Number of transmit antenna elements
    double        txAtnaElmtSpace;   //Transmit antenna element space

};

// /**
// STRUCT      :: PropRxInfo
// DESCRIPTION :: This structure is used for fields related to channel layer
//                information that need to be received with a message.
// **/
struct PropRxInfo {
    NodeAddress txNodeId;
    short       txPhyIndex;
    clocktype   rxStartTime; // signal arrival time
    clocktype   duration;    // signal duration
    BOOL        distorted;
    double      rxPower_dBm;   // signal power before adding receiver antenna gain
    double      pathloss_dB;
    double      fading_dB;
    int         channelIndex;
    double      channelReal;   // for cooperative comm
    double      channelImag;   // for cooperative comm
    double      frequency;     // frequency at receiver side
    double      bandwidth;
    Orientation txDOA;
    Orientation rxDOA;

    // MIMO related parameters
    int         txNumAtnaElmts;    //Number of transmit antenna elements
    double      txAtnaElmtSpace;   //Transmit antenna element space

    Message*    txMsg;
    PropRxInfo* prev;
    PropRxInfo* next;
};

// /**
// FUNCTION        :: PROP_GlobalInit
// PURPOSE         :: Initialization function for propagation
//                    This function is called from each partition,
//                    not from each node
// PARAMETERS      ::
// + partitionData  : PartitionData* : structure shared among nodes
// + nodeInput      : NodeInput*     : structure containing contents of input file
// RETURN          :: void :
void PROP_GlobalInit(PartitionData *partitionData, NodeInput *nodeInput);

// /**
// FUNCTION        :: PROP_PartitionlInit
// PURPOSE         :: Initialize some partition specific data structures.
//                    This function is called from each partition, not from each node
//                    This function is only called for non-MPI
// PARAMETERS      ::
// + partitionData  : PartitionData* : structure shared among nodes
// + nodeInput      : NodeInput*     : structure containing contents of input file
// RETURN          :: void :
void PROP_PartitionInit(PartitionData *partitionData, NodeInput *nodeInput);

// /**
// FUNCTION       :: PROP_Init
// PURPOSE        :: Initialization function for propagation functions.
//                   This function is called from each node.
// PARAMETERS     ::
// + node          : Node*      : node being initialized.
// + channelIndex  : int        : channel being initialized.
// + nodeInput     : NodeInput* : structure containing contents of input file
// RETURN         :: void :
// **/
void PROP_Init(Node *node, int channelIndex, NodeInput *nodeInput);

// /**
// FUNCTION         :: PROP_ProcessEvent
// PURPOSE          :: To receive message.
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + msg             : Message* : message received by the layer
// RETURN           :: void :
// **/
void PROP_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION         :: PROP_Finalize
// PURPOSE          :: To collect various result.
// PARAMETERS       ::
// + node            : Node*    : node for which results are to be collected
// RETURN           :: void :
// **/
void PROP_Finalize(Node *node);

// /**
// API              :: PROP_PathlossFreeSpace
// PURPOSE          :: Calculates pathloss using free space model.
// PARAMETERS       ::
// + distance        : double           : distance (meters) between two nodes
// + wavelength      : double           : wavelength used for propagation.
// RETURN           :: double           : pathloss in db
// **/
double PROP_PathlossFreeSpace(double distance,
                              double waveLength);

// /**
// API              :: PROP_PathlossTwoRay
// PURPOSE          :: To calculate path loss of a channel.
// PARAMETERS       ::
// + distance        : double           : distance (meters) between two nodes
// + wavelength      : double           : wavelength used for propagation.
// + txAntennaHeight : float            : tranmitting antenna hight.
// + rxAntennaHeight : float            : receiving antenna hight.
// RETURN           :: double           : pathloss in db
// **/
double PROP_PathlossTwoRay(double distance,
                           double waveLength,
                           float txAntennaHeight,
                           float rxAntennaHeight);

// /**
// API                  :: PROP_PathlossOpar
// PURPOSE              :: Calculates extra path attenuation using opar model.
// PARAMETERS           ::
// + distance            : double           : distance (meters) between two nodes
// + OverlappingDistance : double           : overlapping distance
// + frequency           : double           : frequency used for propagation.
// + obstructiontype     : ObstructionType  : obstruction type
// RETURN               :: double           : extra path attenuation in db
// **/


double PROP_PathlossOpar(double distance,
                         double OverlappingDistance,
                         double frequency,
                         ObstructionType obstructiontype);

// /**
// API              :: PROP_CalculatePathloss
// PURPOSE          :: To calculate path loss of a channel.
// PARAMETERS       ::
// + node            : Node*            : Node that is
//                                        being instantiated in
// + txNodeId        : NodeId           : including for debugging
// + rxNodeId        : NodeId           : including for debugging
// + channelIndex    : int              : channel number.
// + wavelength      : double           : wavelength used for propagation.
// + txAntennaHeight : float            : tranmitting antenna hight.
// + rxAntennaHeight : float            : receiving antenna hight.
// + pathProfile     : PropPathProfile* : characteristics of path.
// + forBinning      : bool             : disables some features to support
//                                        flat binning
// RETURN           :: void :
// **/
void PROP_CalculatePathloss(
    Node* node,
    NodeId txNodeId,
    NodeId rxNodeId,
    int channelIndex,
    double wavelength,
    float txAntennaHeight,
    float rxAntennaHeight,
    PropPathProfile *pathProfile,
    double* pathloss_dB,
    bool forBinning = false);

// /**
// API              :: PROP_CalculateFading
// PURPOSE          :: To calculate fading between two node.
// PARAMETERS       ::
// + propTxInfo      : PropTxInfo* : Information about the transmitter
// + node2           : Node*       : receiver
// + channelIndex    : int         : channel number
// + currentTime     : clocktype   : current simulation time
// + fading_dB       : float*      : calculated fading store here.
// + channelReal     : double*     : for cooperative comm
// + channelImag     : double*     : for cooperative comm
// RETURN           :: void :
// **/
void PROP_CalculateFading(
    Message* signalMsg,
    PropTxInfo* propTxInfo,
    Node* node2,
    int channelIndex,
    clocktype currentTime,
    float* fading_dB,
    double* channelReal,
    double* channelImag);


// /**
// API              :: PROP_CalculateRxPowerAndPropagationDelay
// PURPOSE          :: This function will be called by QualNet wireless
//                     propagation code to calculate rxPower and prop delay
//                     for a specific signal from a specific tx node to
//                     a specific rx node.
// PARAMETERS       ::
// + msg             : Message*    : Signal to be propagated
// + channelIndex    : int         : Channel that the signal is propagated
// + propChannel     : PropChannel*: Info of the propagation channel
// + propTxInfo      : PropTxInfo* : Transmission parameers of the tx node
// + txNode          : Node*       : Point to the Tx node
// + rxNode          : Node*       : Point to the Rx node
// + pathProfile     : PropPathProfile* : For returning results
// RETURN           :: BOOL : If FALSE, indicate the two nodes cannot comm
//                            TRUE means two nodes can communicate
// **/
BOOL PROP_CalculateRxPowerAndPropagationDelay(
         Message* msg,
         int channelIndex,
         PropChannel* propChannel,
         PropTxInfo* propTxInfo,
         Node* txNode,
         Node* rxNode,
         PropPathProfile* pathProfile);

// /**
// API              :: PROP_CalculateRxPowerAndPropagationDelay
// PURPOSE          :: This function will be called by QualNet wireless
//                     propagation code to calculate rxPower and prop delay
//                     for a specific signal from a specific tx node to
//                     a specific rx node.
// PARAMETERS       ::
// + msg             : Message*    : Signal to be propagated
// + channelIndex    : int         : Channel that the signal is propagated
// + propChannel     : PropChannel*: Info of the propagation channel
// + propTxInfo      : PropTxInfo* : Transmission parameers of the tx node
// + txNode          : Node*       : Point to the Tx node
// + rxNode          : Node*       : Point to the Rx node
// + pathProfile     : PropPathProfile* : For returning results
// RETURN           :: BOOL : If FALSE, indicate the two nodes cannot comm
//                            TRUE means two nodes can communicate
// **/
BOOL PROP_DefaultCalculateRxPowerAndPropagationDelay(
         Message* msg,
         int channelIndex,
         PropChannel* propChannel,
         PropTxInfo* propTxInfo,
         Node* txNode,
         Node* rxNode,
         PropPathProfile* pathProfile);

// /**
// API              :: PROP_MotionObtainfadingStretchingFactor
// PURPOSE          :: Get a stretching factor for fast moving objects.
// PARAMETERS       ::
// + propTxInfo      : PropTxInfo* : Transmitter information
// + receiver        : Node*       : Receiver node.
// + channelIndex    : int         : channel number
// RETURN           :: void :
// **/
void PROP_MotionObtainfadingStretchingFactor(
    PropTxInfo* propTxInfo,
    Node* receiver,
    int   channelIndex);

// API              :: PROP_AddToChannelList
// PURPOSE          :: Add the node to the channel list
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + channelIndex    : int      : channel number
// RETURN           :: void :
// **/
void PROP_AddToChannelList(Node *node, int channelIndex);

// /**
// API              :: PROP_UpdatePathProfiles
// PURPOSE          :: UpdatePathProfiles
// PARAMETERS       ::
// + node            : Node*             : Node that is
//                                         being instantiated in
// RETURN           :: void :
// **/
void PROP_UpdatePathProfiles(Node* node);

// /**
// API                 :: PROP_ReleaseSignal
// PURPOSE             :: Release (transmit) the signal
// PARAMETERS          ::
// + node               : Node*     : Node that is
//                                    being instantiated in
// + msg                : Message*  : Signal to be transmitted
// + phyIndex           : int       : PHY data index
// + channelIndex       : int       : chanel index
// + txPower_dBm        : float     : transmitting power
// + duration           : clocktype : transmission duration
// + delayUntilAirborne : clocktype : delay until airborne
// RETURN              :: void :
// **/
#ifdef PHY_SYNC
void PROP_ReleaseSignal(
    Node *node,
    Message *msg,
    int phyIndex,
    int channelIndex,
    float txPower_dBm,
    clocktype duration,
        clocktype delayUntilAirborne,
        PhyModulation modulation = PHY_MODULATION_DEFAULT,
        PhyEncoding   encoding = PHY_ENCODING_DEFAULT,
        PhyCodingParameter codingParameter = 1);
#else
    void PROP_ReleaseSignal(
        Node* node,
        Message* msg,
        int phyIndex,
        int channelIndex,
        float txPower_dBm,
        clocktype duration,
        clocktype delayUntilAirborne,
        int     txNumAtnaElmts = 1,
        double  txAtnaElmtSpace = 0.0);

#endif // PHY_SYNC

// /**
// API              :: PROP_SubscribeChannel
// PURPOSE          :: Start subscribing (listening to) a channel
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + phyIndex        : int      : interface index
// + channelIndex    : int      : chanel index
// RETURN           :: void :
// **/
void PROP_SubscribeChannel(
    Node *node,
    int phyIndex,
    int channelIndex);

// /**
// API              :: PROP_UnsubscribeChannel
// PURPOSE          :: Stop subscription of (listening to) a channel
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + phyIndex        : int      : interface index
// + channelIndex    : int      : chanel index
// RETURN           :: void :
// **/
void PROP_UnsubscribeChannel(
    Node *node,
    int phyIndex,
    int channelIndex);

// /**
// API              :: PROP_UnreferenceSignal
// PURPOSE          :: Unreference a signal (internal use)
// PARAMETERS       ::
// + node            : Node*    : Node that is
//                                being instantiated in
// + msg             : Message* : Signal to be unreferenced
// RETURN           :: void :
// **/
void PROP_UnreferenceSignal(Node *node, Message *msg);

// /**
// API              :: PROP_CalculateInterNodePathLossOnChannel
// PURPOSE          :: Calculate inter-node pathloss, distance values
//                     between all the nodes on a given  channel
//
// PARAMETERS       ::
// + node               : Node*         : any valid node
// + channelIndex       : int           : selected channel instance
// + numNodesOnChannel  : int*          : number of nodes using this channel
// + nodeIdList         : NodeAddress*  : list of (numNodesOnChannel) nodeIds
// + pathloss_dB        : float**       : 2D pathloss array for nodes in
//                                        nodeIdList
// + distance           : float**       : 2D array of inter-node distances
// RETURN               :: void :
// COMMENTS         :: To access values at location [i][j]:
//                      (pathloss_dB + numNodes * i + j).
//
//                      Memory of nodeIdList, pathloss_dB and distance
//                      'should be freed' by users after usage.
//
//                      The pathloss value of a node to itself will be 0.0
//                      For example: If nodes 1, 3, 4 can use channel 1,
//                      then the pathloss values in pathloss_dB are:
//                      NodeIdList: 1     3       4
//                      ------------------------------
//                          1       0.0    97.49   94.33
//                          3      97.49    0.0    91.43
//                          4      94.33   91.43    0.0
// *//
/*
void PROP_CalculateInterNodePathLossOnChannel(
        Node        *node,
        int         channelIndex,
        int         *numNodesOnChannel,
        NodeAddress **nodeIdList,
        float       **pathloss_dB,
        float       **distance);
        */



//
// API              :: PROP_IsLineOfSight
//
// PURPOSE          :: Check if the path is line of sight
//
// PARAMETERS       ::
// + numSamples             : int           : terrain data sample number
// + sampleDistance         : double        : terrain data sample distance
// + terrainProfile         : double*       : terrain profile sample data
// + txHeight               : double        : Tx node height
// + rxHeight               : double        : Rx node height
// + surfaceRefractivity    : double        : earth surface refractivity
// RETURN               :: BOOL :
BOOL PROP_IsLineOfSight (int     numSamples,
                         double  sampleDistance,
                         double* terrainProfile,
                         double  txHeight,
                         double  rxHeight,
                         double  surfaceRefractivity);

// /**
// API                   :: PROP_CalculatePropagationDelay
// PURPOSE               :: Calculate the wireless propagation delay for the
//                          given distance and propagation speed.
// PARAMETERS            ::
// + distance             : double         : Propagation distance
// + propSpeed            : double         : Propagation speed
// + partitionData        : PartitionData* : Partition data
// + channelIndex         : int            : Channel index or -1 for p2p
//                                           links or microwave links
// + coordinateSystemType : int            : Coordinate system type
// + fromPosition         : Coordinates*   : Source position
// + toPosition           : Coordinates*   : Destination position
// RETURN                :: clocktype      : Calculated propagation delay
// COMMENTS              ::
// + partitionData can be used to get the simulation time or terrain data
// + channelIndex indicates the channel for scenarios with multiple channels
//   Wireless p2p link or microwave links don't use propagation channels.
//   -1 will be passed in which indicate p2p/microwave links.
// + fromPosition and toPosition are not used right now. They can be used
//   to calculate location specific delay.
// **/
clocktype PROP_CalculatePropagationDelay(
              double distance,
              double propSpeed,
              const PartitionData* partitionData,
              int channelIndex,
              int coordinateSystemType,
              const Coordinates* fromPosition,
              const Coordinates* toPosition);

// Prototype required in case Wireless library is missing.
double PathlossMatrix(
    Node* node,
    NodeAddress nodeId1,
    NodeAddress nodeId2,
    int channelIndex,
    clocktype currentTime);


// API              :: PROP_ChangeSamplingRate
//
// PURPOSE          :: Change sampling rate and fading stretching factor
//
// PARAMETERS       ::
// + node            : Node   pointer for the updated node
// + channelIndex    : int    channel index
// RETURN           :: NONE :

void PROP_ChangeSamplingRate(Node *node, int channelIndex);

double PROP_ReturnRxPowerForNodePair(
    Node* node,
    Node* txNode,
    Node* rxNode,
    int channelIndex,
    clocktype futureTime,
    BOOL isPredictive,
    double* noise,
    int* bandwidth,
    double* power_dBm,
    double* propLimit_db,
    double* threshold,
    double* phySnrThreshold);

void PROP_CollectPathlossSample(Node *node);
#ifdef ADDON_DB
void PROP_CollectConnectSample(Node *node);
void PROP_CollectConnectSampleParallel(Node* node);
void PROP_FinalizePathlossMatrixOutput(PartitionData* partitionData);
void PROP_OutputConnectTable(PartitionData* partitionData);

void PROP_GetPOPStatus(PartitionData* partitionData, FILE* fp);

int PROP_ChannelNodesListenOn(Node *node1, Node *node2);
BOOL PROP_CheckChannelForNodes(Node *node1, Node *node2, int channelIndex);
int PROP_ReturnNodeChannelIndex(Node* node, int channelIndex);
// map node id
int MNI(PartitionData* partitionData, int nodeId);

void PROP_FindShorestPath(PartitionData* partitionData, Node* node, int** resultTable);
#endif

// /**
// API         :: PROP_Reset
// PURPOSE     :: Reset previous channel
//              remove/add node to propChannel for signal delivery,
//              in propagation_private.
// PARAMETERS  ::
// + node : Node* : Node that is being instantiated in
// + phyIndex : int  : interface index
// + newChannelListenable : char*  : new channel
// RETURN               :: void :
// **/
void PROP_Reset(Node* node, int phyIndex, char* newChannelListenable);

// /**
// API         :: PROP_AddNodeToList
// PURPOSE     :: add node to propChannel nodeList
// need to make sure that node is not already exists in list before adding.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// RETURN               :: void :
// **/
void PROP_AddNodeToList(Node* node, int channelIndex);

// /**
// API         :: PROP_RemoveNodeFromList
// PURPOSE     :: remove node from propChannel nodeList
// need to make sure that all the interface from that node is not
// listing on that channel before removing.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// RETURN               :: void :
// **/
void PROP_RemoveNodeFromList(Node* node, int channelIndex);

void PROP_RecordSignalRelease(
    Node *node,
    Message *msg,
    int phyIndex,
    int channelIndex,
    float txPower_dBm);

// /**
// API         :: PROP_GetChannelFrequency
// PURPOSE     :: Get channel frequency from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// RETURN               :: double : channel frequency
// **/
double PROP_GetChannelFrequency(Node* node, int channelIndex);

// /**
// API         :: PROP_SetChannelFrequency
// PURPOSE     :: Set channel frequency from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// + channelFrequency : double  : new channel frequency
// RETURN               :: void :
// **/
void PROP_SetChannelFrequency(Node* node,
                              int channelIndex,
                              double channelFrequency);

// /**
// API         :: PROP_GetChannelWavelength
// PURPOSE     :: Get channel wavelength from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// RETURN               :: double : channel wavelength
// **/
double PROP_GetChannelWavelength(Node* node, int channelIndex);

// /**
// API         :: PROP_SetChannelWavelength
// PURPOSE     :: Set channel wavelength from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// + channelWavelength : double  : new channel wavelength
// RETURN               :: void :
// **/
void PROP_SetChannelWavelength(Node* node,
                               int channelIndex,
                               double channelWavelength);

// /**
// API         :: PROP_GetChannelDopplerFrequency
// PURPOSE     :: Get channel doppler freq  from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// RETURN               :: double : channel doppler freq
// **/
double PROP_GetChannelDopplerFrequency(Node* node, int channelIndex);

// /**
// API         :: PROP_SetChannelDopplerFrequency
// PURPOSE     :: Set channel doppler freq from profile for
// PropChannel.
// PARAMETERS  ::
// + node : Node* : the node
// + channelIndex : int  : channel index
// + channelDopplerFrequency : double  : new channel doppler freq
// RETURN               :: void :
// **/
void PROP_SetChannelDopplerFrequency(Node* node,
                                     int channelIndex,
                                     double channelDopplerFrequency);

// /**
// API         :: PROP_FrequencyOverlap
// PURPOSE     :: Check if there is frequency overlap between signal and receiver node.
//
// PARAMETERS  ::
// + txNode : Node* : the Tx node
// + rxNode : Node* : the Rx node
// + txChannelIndex : int  : the Tx channel index
// + rxChannelIndex : int  : the Rx channel index
// + txPhyIndex  : int : the PHY index for the Tx node.
// + rxPhyIndex  : int : the PHY index for the Rx node.
// RETURN      :: BOOL : if there is frequency overlap
// **/

BOOL PROP_FrequencyOverlap(
   Node *txNode,
   Node *rxNode,
   int txChannelIndex,
   int rxChanelIndex,
   int txPhyIndex,
   int rxPhyIndex );

#endif /* PROPAGATION_H */
