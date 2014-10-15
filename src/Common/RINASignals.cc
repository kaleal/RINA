/*
 * RINASignals.cc
 *
 *  Created on: 22. 7. 2014
 *      Author: Mordeth
 */

#include "RINASignals.h"

const char* SIG_AE_AllocateRequest              = "AE-AllocateRequest";
const char* SIG_AE_DeallocateRequest            = "AE-DeallocateRequest";
const char* SIG_AE_DataSend                     = "AE-DataSend";

const char* SIG_RIBD_DataSend                   = "RIBd-DataSend";
const char* SIG_RIBD_CreateRequestFlow          = "RIBd-CreateRequestFlow";
const char* SIG_RIBD_CreateFlow                 = "RIBd-CreateFlow";

const char* SIG_CDAP_DateReceive                = "CDAP-DataReceive";

const char* SIG_IRM_AllocateRequest             = "IRM-AllocateRequest";
const char* SIG_IRM_DeallocateRequest           = "IRM-DeallocateRequest";

const char* SIG_AERIBD_AllocateResponsePositive = "AEorRIBd-AllocateResponsePositive";
const char* SIG_AERIBD_AllocateResponseNegative = "AEorRIBd-AllocateResponseNegative";

const char* SIG_FA_CreateFlowResponseNegative   = "FA-CreateFlowResponseNegative";
const char* SIG_FA_CreateFlowResponsePositive   = "FA-CreateFlowResponsePositive";
const char* SIG_FA_CreateFlowRequestForward     = "FA-CreateFlowRequestForward";
const char* SIG_FA_AllocateResponsePositive     = "FA-AllocateResponsePositive";
const char* SIG_FA_AllocateResponseNegative     = "FA-AllocateResponseNegative";

const char* SIG_toFAI_AllocateRequest           = "toFAI-AllocateRequest";
const char* SIG_toFAI_AllocateResponsePositive  = "toFAI-AllocateResponsePositive";
const char* SIG_toFAI_AllocateResponseNegative  = "toFAI-AllocateResponseNegative";
const char* SIG_FAI_AllocateRequest             = "FAI-AllocateRequest";
const char* SIG_FAI_AllocateResponsePositive    = "FAI-AllocateResponsePositive";
const char* SIG_FAI_AllocateResponseNegative    = "FAI-AllocateResponseNegative";
const char* SIG_FAI_CreateFlowRequest           = "FAI-CreateFlowRequest";
const char* SIG_FAI_DeleteFlowRequest           = "FAI-DeleteFlowRequest";
const char* SIG_FAI_CreateFlowResponsePositive  = "FAI-CreateFlowResponsePositive";
const char* SIG_FAI_CreateFlowResponseNegative  = "FAI-CreateFlowResponseNegative";
const char* SIG_FAI_DeleteFlowResponse          = "FAI-DeleteFlowResponse";

const char* SIG_RA_AllocateRequest              = "RA-AllocateRequest";
const char* SIG_RA_DeallocateRequest            = "RA-DeallocateRequest";
const char* SIG_RA_AllocateResponsePositive     = "RA-AllocateResponsePositive";
const char* SIG_RA_AllocateResponseNegative     = "RA-AllocateResponseNegative";
const char* SIG_RA_FlowAllocated                = "RA-FlowAllocated";
const char* SIG_RA_FlowDeallocated              = "RA-FlowDeallocated";

const char* SIG_RIB_CreateRequestFlow           = "RIB-CreateRequestFlow";
const char* SIG_RIB_CreateResponseFlow          = "RIB-CreateResponseFlow";
const char* SIG_RIB_DeleteRequestFlow           = "RIB-DeleteRequestFlow";
const char* SIG_RIB_DeleteResponseFlow          = "RIB-DeleteResponseFlow";