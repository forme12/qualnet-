// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#include "lic.h"
#include <stdlib.h>
#include <iostream>

static const char *extractorFeatureName = "standard_interfaces-full";
static const char *version = "4.0";

// For some reason when the lm library functions set the VENDORCODE they
// sometimes trash the stack. This hack protects the stack until the error
// is fixed.
typedef struct protected_vendor_code_hack
{
    char buf1[4096];
    VENDORCODE code;
    char buf2[4096];
} protected_vendor_code_hack;

LM_HANDLE *licenseCheckout(const char *qualnetLicensePath)
{
    protected_vendor_code_hack *vendor_code = new protected_vendor_code_hack;
    LM_HANDLE *lm_job;
    int ret = lc_new_job(0, lc_new_job_arg2, &(vendor_code->code), &lm_job);
    if( ret ) {
        lc_perror(lm_job, "license checkout failed");
        exit(lc_get_errno(lm_job));
    }

    lc_set_attr(lm_job, LM_A_LICENSE_DEFAULT, (LM_A_VAL_TYPE)qualnetLicensePath);
    lc_set_attr(lm_job, LM_A_CHECK_BADDATE, (LM_A_VAL_TYPE) 1);

    ret = lc_checkout(lm_job, (const LM_CHAR_PTR) extractorFeatureName, (const LM_CHAR_PTR) version, 1, LM_CO_NOWAIT, &(vendor_code->code), LM_DUP_NONE);
    if( ret )
    {
        lc_perror(lm_job, "license checkout failed");
        exit(lc_get_errno(lm_job));
    }

    delete vendor_code;
    return lm_job;
}

void licenseCheckin(LM_HANDLE *lm_job)
{
    lc_checkin(lm_job, (const LM_CHAR_PTR) extractorFeatureName, 0);
    lc_free_job(lm_job);
}
