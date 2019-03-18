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
// PACKAGE :: Product Information
// DESCRIPTION :: This file contains APIs for product information.
//                Most APIs are implemented as static functions of
//                the Product class.
// **/


#ifndef _PRODUCT_INFO_H_
#define _PRODUCT_INFO_H_

#include "types.h"
#include <string>

// /**
// ENUM         :: ProductType
// DESCRIPTION  :: Defines various products.
// **/
enum ProductType
{
    PRODUCT_QUALNET,
    PRODUCT_EXATA,
    PRODUCT_EXATA_CYBER,
    PRODUCT_CES,
    PRODUCT_JNE,
    PRODUCT_VISNET
};

// /**
// ENUM         :: ProductKernelType
// DESCRIPTION  :: Defines product kernel which should be either QualNet
//                 or EXATA now.
// **/
enum ProductKernelType
{
    KERNEL_QUALNET,
    KERNEL_EXATA
};

// /**
// CLASS        :: Product
// DESCRIPTION  :: A class which provide general information about
//                 the product. It allows several products to be built
//                 from the same kernel while using product-appropriate
//                 naming.  Features are implemented as static functions,
///                so an object of the class does not need to be instantiated.
// **/
class Product
{
public:

    /// Returns the product type.
    static ProductType GetProduct();

    /// Retrieves the installation directory of the product.
    /// Returns TRUE if found successfully, FALSE otherwise.
    static BOOL GetProductHome(std::string &val);

    /// Determines the QUALNET_HOME (or EXATA_HOME, etc.) directory
    /// Returns the full path to the product home directory or an empty string if it cannot be determined.
    static std::string DetermineProductHomeFromExecutionPath();

    /// Products are either based on EXata or QualNet.
    static ProductKernelType GetProductKernel();

    /// For products like CES, this is the underlying product's name,
    /// whereas GetProductName will return the derived product's name.
    static const char* GetKernelName();

    /// For products like CES, this is the underlying product's version,
    /// whereas GetCurrentVersion will return the CES version.
    static const char* GetKernelVersion();

    /// The environment variable used for finding files within the installation.
    static const char* GetProductName();
    
    /// The name of the executable program, usually lowercase product name.
    static const char* GetCommandName();
    
    /// The environment variable used for finding files within the installation.
    static const char* GetHomeVariable();

    /// The current product version in numeric form.
    static Float32 GetVersion();

    /// The current product version as a string.
    static const char* GetVersionString();

    /// The default installation folder (usually lower case product name)
    static const char* GetInstallationPath();

    /// Return the product feature name used in licensing
    static const char* GetProductFeature();
};

#endif // _PRODUCT_INFO_H_
