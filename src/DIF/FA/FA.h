//
// Copyright © 2014 PRISTINE Consortium (http://ict-pristine.eu)
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
/**
 * @file FlowAllocator.h
 * @author Marcel Marek (imarek@fit.vutbr.cz)
 * @date Apr 29, 2014
 * @brief
 * @detail
 */

#ifndef FA_H_
#define FA_H_

//Standard libraries
#include <omnetpp.h>
#include <string>
#include "FABase.h"
#include "FAListeners.h"

#include "FAI.h"
#include "Flow.h"
#include "FAITable.h"
#include "RINASignals.h"
#include "ModuleAccess.h"

#include "EFCP.h"

const int RANDOM_NUMBER_GENERATOR = 0;
const int MAX_PORTID = 65535;
const int MAX_CEPID  = 65535;

//class EFCP;

class FA : public FABase
{
  public:
    FA();
    virtual ~FA();

    virtual void receiveAllocateRequest(cObject* obj);
    virtual void receiveAllocateResponsePositive(cObject* obj);
    virtual void receiveAllocateResponseNegative(cObject* obj);
    virtual void receiveDeallocateRequest(cObject* obj);
    virtual void receiveCreateFlowRequest(cObject* obj);

    virtual void deinstantiateFai(Flow* flow);

    bool invokeNewFlowRequestPolicy(Flow* flow);

    //Signals
    //simsignal_t sigFAIAllocReq;
    simsignal_t sigFAAllocResNega;
    simsignal_t sigFAAllocResPosi;
    simsignal_t sigFACreReqFwd;
    simsignal_t sigFACreResNega;
    simsignal_t sigFACreResPosi;

    //Listeners
    LisFAAllocReq* lisAllocReq;
    LisFAAllocResPosi* lisAllocResPosi;
    LisFAAllocResNega* lisAllocResNega;
    LisFADeallocReq* lisDeallocReq;
    LisFACreReq*  lisCreReq;
    LisFACreRes* lisCreRes;
    LisFADelReq*  lisDelReq;
    LisFADelRes* lisDelRes;

  protected:
    //SimpleModule overloads
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  private:
    FAITable* FaiTable;
    EFCP* efcp;
    bool isMalformedFlow(Flow* flow);
    bool isAppLocal(Flow* flow);
    FAI* createFAI(Flow* flow);
    //void registerIRMSigs();
    void insertNewFTRecord(Flow* flow);

    void initSignalsAndListeners();

    void signalizeAllocateResponseNegative(Flow* flow);
    void signalizeCreateFlowRequestForward(Flow* flow);
    void signalizeCreateFlowResponseNegative(Flow* flow);
};

#endif /* FLOWALLOCATOR_H_ */
