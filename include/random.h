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

#ifndef QUALNET_RANDOM_H
#define QUALNET_RANDOM_H

#include "types.h"

struct NodeInput;

// /**
// PACKAGE     :: RANDOM NUMBERS
// DESCRIPTION :: This file describes functions to generate pseudo-random number streams.
// **/

// /**
// STRUCT       :: ValueProbabilityPair
// DESCRIPTION  :: Stores one data point in a user defined distribution.
// **/
struct ValueProbabilityPair {
    Float32 value;
    Float32 probability;
};

// /**
// STRUCT       :: ArbitraryDistribution
// DESCRIPTION  :: Stores a user defined distribution.
// **/
struct ArbitraryDistribution {
    char* distributionName;
    Int32 numDistPoints;
    ValueProbabilityPair* values;
};

// /**
// ENUM         :: RandomDistributionType
// DESCRIPTION  :: Random function types
// **/
typedef enum {
    DNULL,
    USER,
    DET,
    UNI,    
    INT_UNI,
    EXP,
    GAU,        //Gaussian- continuous valued
    TPD,        //Truncated Pareto (3 parameters)
    TPD4,        //Truncated Pareto (4 parameters)
    UPD,        //Untruncated Pareto (2 parameters)
    GPD,        //General Untruncated Pareto 
    INT_GAU     //Gaussian- discrete valued, only integers
} RandomDistributionType;

// /**
// ENUM         :: RandomDataType
// DESCRIPTION  :: Used for parsing input strings.
// **/
typedef enum {
    RANDOM_CLOCKTYPE,
    RANDOM_INT,
    RANDOM_DOUBLE
} RandomDataType;

typedef unsigned short RandomSeed[3];

// /**
// API        :: RANDOM_SetSeed
// PURPOSE    :: Chooses from a set of pre-defined independent random seeds.
//               The parameter names here are recommend invariants for use
//               in selecting seeds, but other values could be used instead.
// PARAMETERS ::
// + seed       : RandomSeed : the seed to be set.
// + globalSeed : UInt32     : the scenario's global seed, i.e. SEED in the
//                             .config file, stored in node->globalSeed.
// + nodeId     : UInt32     : the node's ID
// + protocolId : UInt32     : the protocol number, as defined in the layer
//                             header files (e.g. MAC_PROTOCOL_CSMA in mac.h)
// + instanceId : UInt32     : the instance of this protocol, often the
//                             interfaceIndex.
// RETURN :: void :
// **/
void RANDOM_SetSeed(RandomSeed seed,
                    UInt32 globalSeed,
                    UInt32 nodeId = 0,
                    UInt32 protocolId = 0,
                    UInt32 instanceId = 0);

// /**
// API        :: RANDOM_erand
// PURPOSE    :: Returns a uniform distribution in [0.0 .. 1.0]
// PARAMETERS ::
// + seed : RandomSeed : the seed for this random stream.
// RETURN :: double : a random number
// **/
extern double RANDOM_erand(RandomSeed);

// /**
// API        :: RANDOM_jrand
// PURPOSE    :: Returns an integer uniformly distributed between -2^31 and 2^31.
// PARAMETERS ::
// + seed : RandomSeed : the seed for this random stream.
// RETURN :: Int32 : a random number
// **/
extern Int32  RANDOM_jrand(RandomSeed);

// /**
// API        :: RANDOM_nrand
// PURPOSE    :: Returns an integer uniformly distributed between 0 and 2^31.
// PARAMETERS ::
// + seed : RandomSeed : the seed for this random stream.
// RETURN :: Int32 : a random number
// **/
extern Int32  RANDOM_nrand(RandomSeed);


// /**
// API        :: RANDOM_LoadUserDistributions
// PURPOSE    :: Loads all user defined distributions.
// PARAMETERS ::
// + nodeInput   : NodeInput* : the .config file
// RETURN :: void :
// **/
void RANDOM_LoadUserDistributions(const NodeInput* nodeInput);

// /**
// CLASS        :: RandomDistribution
// DESCRIPTION  :: a template class for generating various types of random
//                 distributions with different return types.  Primarily
//                 used by models that allow the user to specify the type
//                 of distribution as a configuration option.
// **/
template <class T>
class RandomDistribution
{
public:
    char* userDefinedDistributionName;

    // /**
    // API        :: RandomDistribution.init
    // PURPOSE    :: Initializes the random distribution
    // PARAMETERS ::
    // RETURN :: void :
    // **/
    void init() {
        value1 = (T) 0;
        value2 = (T) 0;
        value3 = (T) 0;
        alpha  = 0.0;
        randomSeed[0] = 0;
        randomSeed[1] = 0;
        randomSeed[2] = 0;
        type = DNULL;
    }

    // /**
    // API        :: RandomDistribution.setDistributionUniform
    // PURPOSE    :: Sets the distribution to uniform.  With this function,
    //               even integer types return values between [min, max),
    //               meaning to get a boolean distribution, use 0, 2.
    // PARAMETERS ::
    // + min : T : the low end of the range
    // + max : T : the high end of the range
    // RETURN :: void :
    // **/
    void setDistributionUniform(T min, T max);

    // /**
    // API        :: RandomDistribution.setDistributionUniformInteger
    // PURPOSE    :: This one gives [min, max] for integers.
    // PARAMETERS ::
    // + min : T : the low end of the range
    // + max : T : the high end of the range
    // RETURN :: void :
    // **/
    void setDistributionUniformInteger(T min, T max);

    // /**
    // API        :: RandomDistribution.setDistributionExponential
    // PURPOSE    :: Sets the distribution to exponential with the given mean.
    // PARAMETERS ::
    // + mean : T : the mean value of the distribution
    // RETURN :: void :
    // **/
    void setDistributionExponential(T mean);

    // /**
    // API        :: RandomDistribution.setDistributionGaussian
    // PURPOSE    :: Sets the distribution to Gaussian with the given sigma
    // PARAMETERS ::
    // + sigma : T : the sigma value
    // RETURN :: void :
    // **/
    void setDistributionGaussian(double sigma);

    // /**
    // API        :: RandomDistribution.setDistributionGaussianInt
    // PURPOSE    :: Sets the distribution to Gaussian with the given sigma
    // PARAMETERS ::
    // + sigma : T : the sigma value
    // RETURN :: void :
    // **/
    void setDistributionGaussianInt(double sigma);

    // /**
    // API        :: RandomDistribution.setDistributionPareto
    // PURPOSE    :: Sets the distribution to the truncated Pareto distribution
    // PARAMETERS ::
    // + val1  : T      : the low end of the range
    // + val2  : T      : the high end of the range
    // + alpha : double : the alpha value
    // RETURN :: void :
    // **/
    void setDistributionPareto(T val1, T val2, double alpha);

    // /**
    // API        :: RandomDistribution.setDistributionPareto4
    // PURPOSE    :: Sets the distribution to the truncated Pareto distribution
    // PARAMETERS ::
    // + val1  : T      : the minimum value of Pareto distribution 
    // + val2  : T      : the low end of the range
    // + val3  : T      : the high end of the range
    // + alpha : double : the alpha value
    // RETURN :: void :
    // **/
    void setDistributionPareto4(T val1, T val2, T val3, double alpha);

    // /**
    // API        :: RandomDistribution.setDistributionGeneralPareto
    // PURPOSE    :: Sets the distribution to general Pareto distribution
    // PARAMETERS ::
    // + val1  : T      : the low end of the range
    // + val2  : T      : the high end of the range
    // + alpha : double : the alpha value
    // RETURN :: void :
    // **/
    void setDistributionGeneralPareto (T val1, double alpha);

    // /**
    // API        :: RandomDistribution.setDistributionParetoUntruncated
    // PURPOSE    :: Sets the distribution to the truncated Pareto distribution
    // PARAMETERS ::
    // + alpha : double : the alpha value
    // RETURN :: void :
    // **/
    void setDistributionParetoUntruncated(T val1, double alpha);

    // /**
    // API        :: RandomDistribution.setDistributionDeterministic
    // PURPOSE    :: The distribution will always return val.
    // PARAMETERS ::
    // + val : T : the value to return
    // RETURN :: void :
    // **/
    void setDistributionDeterministic(T val);

    // /**
    // API        :: RandomDistribution.setDistributionNull
    // PURPOSE    :: The distribution will return 0.  This is used for
    //               initialization.
    // PARAMETERS ::
    // RETURN :: void :
    // **/
    void setDistributionNull();

    // /**
    // API        :: RandomDistribution.setDistribution
    // PURPOSE    :: Sets the distribution by parsing string input.
    // PARAMETERS ::
    // + inputString : char*          : the input string, typically from a line
    //                                  in the .app file, for example
    // + printStr    : char*          : usually the name of the calling
    //                                  protocol.  Used for error messages.
    // + dataType    : RandomDataType : the data type of the template class is
    //                                  used to ensure proper parsing of the
    //                                  numbers in the input string.
    // RETURN :: int : returns the number of tokens read from the input string
    // **/
    int setDistribution(const char* inputString,
                        const char* printStr,
                        RandomDataType dataType);

    // /**
    // API        :: RandomDistribution.getRandomNumber
    // PURPOSE    :: These two functions return the next random number from
    //               the defined distribution.
    // PARAMETERS ::
    // + seed : RandomSeed : when the seed parameter is present, it is used in
    //                       place of the member variable.  This allows several
    //                       distributions to share a seed, which is advisable
    //                       only during initialization.
    // + node : Node*      : the node parameter is required to look up user
    //                       defined distributions.
    // RETURN :: T : the random value
    // **/
    T getRandomNumber();
    T getRandomNumber(RandomSeed seed);

    // /**
    // API        :: RandomDistribution.setSeed
    // PURPOSE    :: Calls RANDOM_SetSeed on the member seed.
    // PARAMETERS ::
    // + globalSeed : UInt32     : the scenario's global seed, i.e. SEED in the
    //                             .config file, stored in node->globalSeed.
    // + nodeId     : UInt32     : the node's ID
    // + protocolId : UInt32     : the protocol number, as defined in the layer
    //                             header files (e.g. MAC_PROTOCOL_CSMA in mac.h)
    // + instanceId : UInt32     : the instance of this protocol, often the
    //                             interfaceIndex.
    // RETURN :: void :
    // **/
    void setSeed(UInt32 globalSeed,
                 UInt32 nodeId = 0,
                 UInt32 protocolId = 0,
                 UInt32 instanceId = 0);

    // /**
    // API        :: RandomDistribution.setSeed
    // PURPOSE    :: Copies the parameter seed into the member seed.
    // PARAMETERS ::
    // + seed : RandomSeed : an already initialized seed.
    // RETURN :: void :
    // **/
    void setSeed(RandomSeed seed);

    RandomDistributionType getDistributionType() { return type; }

private:
    T                      value1;
    T                      value2;
    T                      value3;
    double                 alpha;
    RandomSeed             randomSeed;
    RandomDistributionType type;

    T uniform(RandomSeed seed);
    T exponential(RandomSeed seed);

    T pareto(RandomSeed seed);
    T paretoGeneral(RandomSeed seed);
    T paretoUntruncated(RandomSeed seed);

    T gaussian(RandomSeed seed);
    T gaussianInt(RandomSeed seed);

    double processUserDistribution(RandomSeed seed);
    double getNextNumber(ArbitraryDistribution* data,
                         double      randNum);
    T convertToType(char*          valueString,
                    RandomDataType dataType);
};
#endif
