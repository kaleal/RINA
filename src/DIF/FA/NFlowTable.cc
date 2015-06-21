// The MIT License (MIT)
//
// Copyright (c) 2014-2016 Brno University of Technology, PRISTINE project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "NFlowTable.h"

//Statistic collextion
const char* SIG_STAT_FT_SIZE             = "FT_FlowTableSize";

Define_Module(NFlowTable);

void NFlowTable::initialize()
{
    //Inits
    initSignalsAndListeners();
    //Watchers
    WATCH_LIST(FaiTable);
}

std::string NFlowTable::info() const {
    std::ostringstream os;
    os << "id=" << this->getId() << endl;
    /*
    for(TFTConstIter it = FaiTable.begin(); it != FaiTable.end(); ++it )
    {
        FAITableEntry tft = *it;
        os << tft << endl;
    }
    */
    return os.str();
}

NFlowTableEntry* NFlowTable::findEntryByDstAddressAndFwd(const APN& apname) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getDstAddr().getApname() == apname
                && tft.getAllocateStatus() == NFlowTableEntry::FORWARDED)
            return &(*it);
    }
    return NULL;
}

NFlowTableEntry* NFlowTable::findEntryBySrcAddressAndFwd(const APN& apname) {
    //EV << "Look APN> " << ipcAddr << endl;
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        //EV << "XXXX " << tft.getCFlow()->getSrcAddr().getIpcAddress() << endl
        //   << "YYYY " << (tft.getAllocateStatus() == FAITableEntry::FORWARDED) << endl;
        if (tft.getCFlow()->getSrcAddr().getIpcAddress() == apname
                && tft.getAllocateStatus() == NFlowTableEntry::FORWARDED)
            return &(*it);
    }
    return NULL;

}

NFlowTableEntry* NFlowTable::findEntryByApns(const APN& srcApn, const APN& dstApn) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getSrcApni().getApn() == srcApn
                && tft.getCFlow()->getDstApni().getApn() == dstApn
                && tft.getAllocateStatus() == NFlowTableEntry::FORWARDED)
            return &(*it);
    }
    return NULL;
}

TFAIPtrs NFlowTable::findEntriesByDstNeighborAndFwd(const APN& apname) {
    TFAIPtrs list;
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getDstNeighbor().getApname() == apname
                && tft.getAllocateStatus() == NFlowTableEntry::FORWARDING)
            list.push_back(&(*it));
    }
    return list;
}

NFlowTableEntry* NFlowTable::findEntryByInvokeId(long invId) {
    if (!invId) return NULL;
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getAllocInvokeId() == invId)
            return &(*it);
    }
    return NULL;
}

TFAIPtrs NFlowTable::findEntriesAffectedByMgmt(const Flow* flow) {
    TFAIPtrs list;
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getSrcAddr().getApname() == flow->getSrcApni().getApn()
            && tft.getCFlow()->getDstAddr().getApname() == flow->getDstApni().getApn()
            && tft.getAllocateStatus() == NFlowTableEntry::ALLOC_PEND)
            list.push_back(&(*it));
    }
    return list;
}

//XXX: Vesely - This search does not yield exact intended match!
NFlowTableEntry* NFlowTable::findMgmtEntryByDstNeighbor(const Address& addr) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getDstNeighbor() == addr
            && tft.getCFlow()->isManagementFlowLocalToIPCP())
            return &(*it);
    }
    return NULL;
}

void NFlowTable::handleMessage(cMessage *msg)
{
}

void NFlowTable::insertNew(Flow* flow) {
    this->insert(NFlowTableEntry(flow));
    updateDisplayString();
    //EV << "FT emits signal " << (long)FaiTable.size() << endl;
    emit(sigStatFTSize, (long)FaiTable.size());
}

void NFlowTable::insert(const NFlowTableEntry& entry) {
    Enter_Method("insertFlow()");
    FaiTable.push_back(entry);
}

void NFlowTable::removeByFlow(Flow* flow) {
    Enter_Method("removeFlow()");
    NFlowTableEntry* entry = findEntryByFlow(flow);
    if (entry)
        FaiTable.remove(*entry);
}

NFlowTableEntry* NFlowTable::findEntryByFlow(const Flow* flow) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (*(tft.getCFlow()) == *flow)
            return &(*it);
    }
    return NULL;
}

NFlowTableEntry* NFlowTable::findEntryByFai(FAIBase* fai) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getFai() == fai)
            return &(*it);
    }
    return NULL;
}

void NFlowTable::changeAllocStatus(Flow* flow, NFlowTableEntry::EAllocateStatus status) {
    NFlowTableEntry* fte = findEntryByFlow(flow);
    if (fte) {
        fte->setAllocateStatus(status);
    }
    else
        EV << "findByFlow() returned NULL" << endl;
}

void NFlowTable::changeAllocStatus(FAIBase* fai, NFlowTableEntry::EAllocateStatus status) {
    NFlowTableEntry* fte = findEntryByFai(fai);
    if (fte)
        fte->setAllocateStatus(status);
    else
        EV << "findByFai() returned NULL" << endl;
}

void NFlowTable::setFaiToFlow(FAIBase* fai, Flow* flow) {
    NFlowTableEntry* fte = findEntryByFlow(flow);
    fte->setFai(fai);
}

void NFlowTable::updateDisplayString() {
    // display number of flows
    cDisplayString& disp = getDisplayString();
    disp.setTagArg("t", 1, "t");
    std::ostringstream os;
    os << "records: " << getSize();
    disp.setTagArg("t", 0, os.str().c_str());
}

void NFlowTable::initSignalsAndListeners() {
    sigStatFTSize = registerSignal(SIG_STAT_FT_SIZE);
}

NFlowTableEntry* NFlowTable::findMgmtEntry(const Flow* flow) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->isManagementFlowLocalToIPCP())
            return &(*it);
    }
    return NULL;
}

NFlowTableEntry* NFlowTable::findMgmtEntryByDstAddr(const Address& addr) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getDstAddr() == addr
            && tft.getCFlow()->isManagementFlowLocalToIPCP())
            return &(*it);
    }
    return NULL;
}

NFlowTableEntry* NFlowTable::findMgmtEntryByDstApni(const APN& dstApn) {
    for(TFTIter it = FaiTable.begin(); it != FaiTable.end(); ++it) {
        NFlowTableEntry tft = *it;
        if (tft.getCFlow()->getDstApni().getApn() == dstApn
            && tft.getCFlow()->isManagementFlowLocalToIPCP())
            return &(*it);
    }
    return NULL;
}

const unsigned int NFlowTable::getSize() const {
    return FaiTable.size();
}
