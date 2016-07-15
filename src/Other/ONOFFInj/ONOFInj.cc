#include "ONOFInj.h"
#include "Inj_PDU.h"
#include "Utils.h"
#include "OFMessages.h"
#include "OFInfectionComparator.h"
#include "OFStaticGenerator.h"

#include "InjListener.h"

Define_Module(ONOFInj);

int voiceOF::min_pdu_len = 100;
int voiceOF::max_pdu_len = 500;
double voiceOF::interval = 1.0/30.0;
double voiceOF::idle_time = 9.0;
double voiceOF::burst_time = 3.0;

double dtcpOF::ackT = 0.1;

int clientOF::pdu_len = 1400;
int serverOF::pdu_len = 1400;

ofMsg * voiceOF::getData(ONOFInj * m) {
    return new ofMsg(flowId, intuniform(voiceOF::min_pdu_len, voiceOF::max_pdu_len), ++seq);
}

ofMsg * clientOF::getRq(ONOFInj * m) {
    if (at->isScheduled()) {
        m->cancelEvent(at);
    }
    return new ofReq(flowId, clientOF::pdu_len, ++sent, A, B, nacking, until, reqRate);
}
ofMsg * clientOF::getAck(ONOFInj * m) {
    if (at->isScheduled()) {
        m->cancelEvent(at);
    }
    return new ofAck(flowId, 0, sent, A, B, nacking);
}

ofMsg * serverOF::getData(ONOFInj * m) {
    m->dataDCount++;
    if (at->isScheduled()) {
        m->cancelEvent(at);
    }
    if(nextSNack) {
        nextSNack = false;
        return new ofData(flowId, serverOF::pdu_len,  sN, A, B, nacking);
    } else {
        return new ofData(flowId, serverOF::pdu_len,  ++sent, A, B, nacking);
    }
}
ofMsg * serverOF::getAck(ONOFInj * m) {
    m->dataACount++;
    if (at->isScheduled()) {
        m->cancelEvent(at);
    }
    return new ofData(flowId, 0, sent, A, B, nacking);
}

Inj_PDU * ONOFInj::getPDU(const Flow_t * f, Inj_data * data) {
    Inj_PDU * pdu = new Inj_PDU();
    connID.setQoSId(f->QoS);
    connID.setSrcCepId(f->flowId);
    pdu->setConnId(connID);
    pdu->setSrcAddr(srcAddr);
    pdu->setSrcApn(srcAddr.getApn());
    dstAddr = Address(f->dstAddr.c_str(), dif.c_str());
    pdu->setDstAddr(dstAddr);
    pdu->setDstApn(dstAddr.getApn());
    pdu->xdata = shared_ptr < Inj_data > (data);
    pdu->setByteLength(pdu->xdata->len + header);


    if(ofReq * d = dynamic_cast<ofReq*>(data)) {
        reqCount++;
    //    //cout << simTime() <<"  -  "<<srcAddr << " -> "<< dstAddr << " + "<< f->QoS << " || " << d->seq<<endl;
    //    //cout << "  data req "<< d->A << " | "<<d->B << " -- "<< d->request << " at rate "<< d->rate<<endl;
    } else if(ofAck * d = dynamic_cast<ofAck*>(data)) {
        ackCount++;
    //   //cout << simTime() <<"  -  "<<srcAddr << " -> "<< dstAddr << " + "<< f->QoS << " || " << d->seq<<endl;
     //   //cout << "  data ack "<< d->A << " | "<<d->B <<endl;
    } else if(ofData * d = dynamic_cast<ofData*>(data)) {
        dataCount++;
    //    //cout << simTime() <<"  -  "<<srcAddr << " -> "<< dstAddr << " + "<< f->QoS << " || " << d->seq<<endl;
   /*     if(const serverOF  * sf = dynamic_cast<const serverOF*>(f)){
            //cout << sf->until<<endl;
        }*/
    //    //cout << "  data response "<< d->A << " | "<<d->B <<endl;
    }

    sent++;

   // //cout<< src<<" -- " << received << " / "<< sent<<endl<<endl;
    return pdu;
}

void ONOFInj::initialize() {
    //Init ini/fin times
    double iniT = par("ini").doubleValue();
    if (iniT < 0 || par("infectedIPC").stdstringValue() == "") {
        return;
    }
    fin = par("fin").doubleValue();

    //Get IPCP
    cModule * ipc = this->getParentModule()->getSubmodule(
            par("infectedIPC").stringValue());
    if (ipc == nullptr) {
        return;
    }

    //Connect to RMT
    cModule * rmt = ipc->getSubmodule("relayAndMux")->getSubmodule("rmt");
    rmt->addGate("infGate", cGate::INOUT, false);
    cGate * modIn = rmt->gateHalf("infGate", cGate::INPUT);
    cGate * modOut = rmt->gateHalf("infGate", cGate::OUTPUT);
    cGate * In = gateHalf("g", cGate::INPUT);
    cGate * Out = gateHalf("g", cGate::OUTPUT);
    modOut->connectTo(In);
    Out->connectTo(modIn);

    //Conect comparator
    cModule *ac = ipc->getSubmodule("resourceAllocator")->getSubmodule(
            "addressComparator");
    if (OFInfectionComparator * oac = dynamic_cast<OFInfectionComparator*>(ac)) {
        oac->p = this;
    }

    //Conect pdufwgen
    cModule *fwdg = ipc->getSubmodule("resourceAllocator")->getSubmodule(
            "pduFwdGenerator");
    if (OFStaticGenerator::OFStaticGenerator * ofwdg =
            dynamic_cast<OFStaticGenerator::OFStaticGenerator*>(fwdg)) {
        ofwdg->inj = this;
    }

    //Init node info
    dif = ipc->par("difName").stdstringValue();
    src = ipc->par("ipcAddress").stdstringValue();
    srcAddr = Address(src.c_str(), dif.c_str());
    dstAddr = Address("", dif.c_str());
    nextFlowId = 0;
    connID.setDstCepId(-1);
    header = par("header_size").longValue();
    received = 0;
    sent = 0;

    dtcpOF::ackT = par("aktT").doubleValue();

    string nodes_raw = par("nodes").stdstringValue();
    vector<string> nodes = split(nodes_raw, ' ');

    string voiceQoS = par("voice_qos").stdstringValue();
    int voiceFlows = par("voice_flows").longValue();
    voiceOF::min_pdu_len = par("voice_min_pdu_len").longValue();
    voiceOF::max_pdu_len = par("voice_max_pdu_len").longValue();
    voiceOF::interval = par("voice_interval").doubleValue();
    voiceOF::idle_time = par("voice_idle_time").doubleValue();
    voiceOF::burst_time = par("voice_burst_time").doubleValue();

    string clientQoS = par("client_qos").stdstringValue();
    int clientFlows = par("client_flows").longValue();
    int clientMinData = par("client_min_data").longValue();
    int clientMaxData = par("client_max_data").longValue();
    int clientMinRate = par("client_min_rate").doubleValue();
    int clientMaxRate = par("client_max_rate").doubleValue();
    int clientIdleTime = par("client_idle_time").doubleValue();
    clientOF::pdu_len = par("client_pdu_len").longValue();

    for (const string & n : nodes) {
        if (n == "" || n == src) {
            continue;
        }
        for (int i = 0; i < voiceFlows; i++) {
            voiceOF * f = new voiceOF(nextFlowId, n, voiceQoS);
            vFlows[nextFlowId] = f;
            f->At->f = f;
            nextFlowId++;
            scheduleAt(
                    iniT
                            + uniform(0.0,
                                    voiceOF::idle_time + voiceOF::burst_time),
                    f->At);
        }

        for (int i = 0; i < clientFlows; i++) {
            clientOF * f = new clientOF(nextFlowId, n, clientQoS, clientMinData,
                    clientMaxData, clientMinRate, clientMaxRate,
                    clientIdleTime);
            cFlows[n][nextFlowId] = f;
            f->At->f = f;
            f->rt->f = f;
            f->srt->f = f;
            f->at->f = f;
            nextFlowId++;
            scheduleAt(iniT + uniform(0.0, clientIdleTime), f->At);
        }
    }

    reqCount=0;
    ackCount =0;
    dataCount = 0;
    dataDCount = 0;
    dataACount = 0;
}

void ONOFInj::handleMessage(cMessage *msg) {
    Enter_Method_Silent();
    if (actTimer * m = dynamic_cast<actTimer*>(msg)) {
        if (voiceOF * f = dynamic_cast<voiceOF *>(m->f)) {
            actVoice(m, f); return;
        } else if (clientOF * f = dynamic_cast<clientOF *>(m->f)) {
            actClient(m, f); return;
        } else if (serverOF * f = dynamic_cast<serverOF *>(m->f)) {
            actServer(m, f); return;
        }
    } else if (retransTimer * m = dynamic_cast<retransTimer *>(msg)) {
        if (clientOF * f = dynamic_cast<clientOF *>(m->f)) {
            retClient(m, f); return;
        } else if (serverOF * f = dynamic_cast<serverOF *>(m->f)) {
            retServer(m, f); return;
        }
    } else if (selretransTimer * m = dynamic_cast<selretransTimer *>(msg)) {
        if (serverOF * f = dynamic_cast<serverOF *>(m->f)) {
            selRetServer(m, f); return;
        }
    } else if (ackTimer * m = dynamic_cast<ackTimer *>(msg)) {
        if (clientOF * f = dynamic_cast<clientOF *>(m->f)) {
            rackClient(m, f); return;
        } else if (serverOF * f = dynamic_cast<serverOF *>(m->f)) {
            rackServer(m, f); return;
        }
    }
    //////cout << "Something went wrong!!" <<endl;
}
void ONOFInj::finish() {
    for (auto f : vFlows) {
        cancelAndDelete(f.second->At);
        delete f.second;
    }
    for (auto f : cFlows) {
        for (auto ff : f.second) {
            cancelAndDelete(ff.second->At);
            cancelAndDelete(ff.second->rt);
            cancelAndDelete(ff.second->srt);
            cancelAndDelete(ff.second->at);
            delete ff.second;
        }
    }
    for (auto f : sFlows) {
        for (auto ff : f.second) {
            cancelAndDelete(ff.second->At);
            cancelAndDelete(ff.second->rt);
            cancelAndDelete(ff.second->srt);
            cancelAndDelete(ff.second->at);
            delete ff.second;
        }
    }
    cout << " received " << received << endl;
    cout << " sent " << sent << endl;
    cout << reqCount<<" "<< ackCount<<" "<< dataCount <<" "<<dataDCount <<" "<<dataACount << endl;
}

void ONOFInj::actVoice(actTimer * m, voiceOF * f) {
    if (!f->state) { // Currently in OFF mode
        if (simTime() >= fin) {
            return;
        }
        f->state = true;

        double t = pareto_shifted(5.0, voiceOF::burst_time * 0.8, 0.0);
        if (t > voiceOF::burst_time * 3.0) {
            t = voiceOF::burst_time * 3.0;
        }
        f->remaining = ceil(t / voiceOF::interval);
        actVoice(m, f);
        return;
    }
    if (f->remaining > 0) { // Send data and schedule next call
        RMTPort * p = ports[f->dstAddr][f->QoS];
        if (p != nullptr && p->isOutputReady()) { // Check if output is ready, if not postpone
            f->remaining--;
            send(getPDU(f, f->getData(this)), "g$o");
        }
        scheduleAt(simTime() + voiceOF::interval, m);
    } else { // Time to change to OFF and schedule next ON
        if (simTime() >= fin) {
            return;
        }
        f->state = false;
        double t = pareto_shifted(5.0, voiceOF::idle_time * 0.8, 0.0);
        if (t > voiceOF::idle_time * 3.0) {
            t = voiceOF::idle_time * 3.0;
        }
        scheduleAt(simTime() + t, m);
    }

}
void ONOFInj::actClient(actTimer * m, clientOF * f) {
    if (simTime() >= fin) {
        return;
    }

    f->reqRate = uniform(f->minRate, f->maxRate);
    f->until += intuniform(f->minData, f->maxData);
    f->next_time = simTime().dbl() + f->idle_time;
    send(getPDU(f, f->getRq(this)), "g$o");

   // //cout<< "At "<< simTime() << " - New data request : " << f->until<< " at rate "<< f->reqRate << " | next at "<<f->next_time<<endl;
    InjListener::instance->dataRequestStarted(src, f->dstAddr, f->flowId);

    if (!f->rt->isScheduled()) {
        ////cout << "A"<<endl;
        scheduleAt(simTime() + f->retT, f->rt);
    }
}
void ONOFInj::actServer(actTimer * m, serverOF * f) {
    if (f->nextSNack || f->until > f->sent) {
        RMTPort * p = ports[m->f->dstAddr][m->f->QoS];
        if (p != nullptr && p->isOutputReady()) { // Check if output is ready, if not postpone
            if(f->acked == f->sent){
                f->last_sent = simTime().dbl();
            }

         //  ////cout << "Server sent data"<<endl;
            send(getPDU(f, f->getData(this)), "g$o");
            if (!f->rt->isScheduled()) {
                ////cout << "B"<<endl;
                scheduleAt(simTime() + f->retT, f->rt);
            }
        }
        if(!f->At->isScheduled()){
            scheduleAt(simTime() + f->interval, m);
        }
    }
}

void ONOFInj::retClient(retransTimer * m, clientOF * f) {
    f->sent = f->acked;
    send(getPDU(f, f->getRq(this)), "g$o");
    scheduleAt(simTime() + f->retT, m);
}
void ONOFInj::retServer(retransTimer * m, serverOF * f) {
    f->sent = f->acked;
    //send(getPDU(f, f->getData(this)), "g$o");
    actServer(f->At, f);
    ////cout << "C"<<endl;
    if(!m->isScheduled()) {
        scheduleAt(simTime() + f->retT, m);
    }
}

void ONOFInj::selRetServer(selretransTimer * m, serverOF * f) {
    f->nextSNack = true;
    scheduleAt(simTime() + f->retT, m);
    if(!f->At->isScheduled()) {
        actServer(f->At, f);
    }
}

void ONOFInj::rackClient(ackTimer * m, clientOF * f) {
    //f->last_sent = simTime().dbl();
    send(getPDU(m->f, f->getAck(this)), "g$o");
}
void ONOFInj::rackServer(ackTimer * m, serverOF * f) {
    //cout << " acking?"<<endl;
    //f->last_sent = simTime().dbl();
    send(getPDU(m->f, f->getAck(this)), "g$o");
}

void ONOFInj::receiveData(const string & _src, const string & _qos, shared_ptr<Inj_data> data) {
    Enter_Method_Silent();

    if(uniform(0.0,1.0)> par("dropProb").doubleValue()) {
        //cout << simTime() << " --- ";
        if(src == "1") {
            if (shared_ptr<ofReq> lData = dynamic_pointer_cast < ofReq > (data)) {
                //cout << " Lost request "<< lData->A<<endl;
            } else if (shared_ptr<ofAck> lData = dynamic_pointer_cast < ofAck > (data)) {
                //cout << " Lost ack "<< lData->A<<endl;
            }
        } else {
            if (shared_ptr<ofData> lData = dynamic_pointer_cast < ofData > (data)) {
                //cout << " Lost data "<< lData->seq <<endl;
            }
        }
        return; }

    received++;

    if (shared_ptr<ofReq> lData = dynamic_pointer_cast < ofReq > (data)) { // Data request from client
        if(sFlows[_src][lData->flow] == nullptr) {
            serverOF * f = new serverOF(lData->flow, _src, _qos);
            f->At->f = f;
            f->rt->f = f;
            f->at->f = f;
            f->srt->f = f;
            sFlows[_src][lData->flow] = f;
        }

        //Set "new request"
        serverOF * f = sFlows[_src][lData->flow];
        f->until = lData->request;
        f->interval = 1.0/lData->rate;
        //cout << simTime() << " --- ";
        //cout << " Until "<< f->until<<endl;
       // //cout << " Interval "<< f->interval<<endl;
        if(!f->At->isScheduled()) {
            scheduleAt(simTime(), f->At);
        }
    }
    if (shared_ptr<ofAck> lData = dynamic_pointer_cast < ofAck > (data)) { // Ack from client
        serverOF * f = sFlows[_src][lData->flow];

        //Recompute retransmission time
        f->retT = dtcpOF::ackT + 3*(simTime().dbl() - lData->t0.dbl());

        bool sendAck = false;

        //Recompute A/B - Destination ack/selNack values
        {
            bool reqAction = false;
            seq_t & s = lData->seq;
            seq_t & A = f->A;
            seq_t & B = f->B;
            if(s > A) {
                if(s == A+1) {
                    sendAck = true;
                    f->nacking = false;
                    //Ack
                    A++; B = A+1;
                } else {
                    sendAck = true;
                    B = A; //nAck
                    f->nacking = true;
                    if(f->lastNack != B) {
                        reqAction = true;
                        f->lastNack = B;
                    }
                }
            }
            if(reqAction) {
                if(f->at->isScheduled()) {
                    cancelEvent(f->at);
                    take (f->at);
                    //cout << "here"<<endl;
                    rackServer(f->at, f);
                }
            }
        }

        //Need to retransmit something?
        {
            seq_t & s = f->sent;
            seq_t & k = f->acked;
            seq_t & n = f->sN;
            seq_t & A = lData->A;
            seq_t & B = lData->B;

            seq_t D = A-k;

            if(A == B ) { // nAck
                if(k > A) { // Already acked future PDUs ??
                    //cout << "Something wrong in nAck - 1"<<endl;
                } else if (A > s){ // Nack of not sent PDU??
                    //cout << "Something wrong in nAck - 2"<<endl;
                } else { // On nAck, ack all until A and transmit all after that again
                    seq_t p = k;
                    if(k < A) { k = A; }
                    //cout << simTime() << " --- ";
                    //cout << "nAck  "<< p << " || " << s << "-> "<< k <<endl;
                    s = k;
                    if(!f->At->isScheduled()) { // If not scheduled, start resending
                        actServer(f->At, f);
                    }
                }
            } else if ( B < A) { // selective nAck
                if(n > B) { // Request selective retransmision of older PDU??
                    //cout << "Something wrong in selective nAck - 1 "<< s<< " - " << A << " / "<< B<<endl;
                } else if( n < B ){ // on selective nAck, mark B as "to resend", and ack until A
                    n = B;
                    if(k < A) { k = A; }
                    if(lData->isNack) { // if also nacks reset last sent
                        //cout << simTime() << " --- ";
                        //cout << "sel + nAck  "<< n << " || " << s << "-> "<< k <<endl;
                        s = k;
                      //  //cout << "selective"<<n <<" + nack " << k<<endl;
                    } else {
                        //cout << simTime() << " --- ";
                        //cout << "sel  "<< n << " || " << s << " : "<< k <<endl;
                    }
                    f->nextSNack = true;
                    if(!f->At->isScheduled()) {
                        actServer(f->At, f); // If not scheduled, resend
                    }
                } else {
                    ////cout << " N == B : "<< B <<endl;
                    if(k < A) { k = A; }
                    if(lData->isNack) { // if also nacks reset last sent
                        //cout << "nAck  " << s << "-> "<< k <<endl;
                        s = k;
                    //    //cout << "selective"<<n <<" + nack " << k<<endl;
                    }
                }
                if(!f->srt->isScheduled()) {
                    scheduleAt(simTime()+f->retT, f->srt);
                }
            } else if(k < A) {
                if(f->srt->isScheduled()) {
                    cancelEvent(f->srt);
                }
                //////cout << "Ack"<<endl;
                k = A;
            } // On Ack, ack all until A

            if(D > 0) {
                if(f->rt->isScheduled()) {
                    cancelEvent(f->rt);
                    f->last_sent += D*f->interval;
                    double st = f->last_sent + f->retT;
                    if(st <= simTime().dbl()) {
                        ////cout << "D"<<endl;
                        scheduleAt(simTime(), f->rt);
                    } else {
                        ////cout << "E"<<endl;
                        scheduleAt(st, f->rt);
                    }
                }
            }

          //  //cout << f->sent <<" vs "<< f->acked << endl;
            if(f->sent == f->acked) {
                f->last_sent = simTime().dbl();
                if(f->rt->isScheduled()) {
                    cancelEvent(f->rt);
                }
            }
        }

        if(sendAck && !f->at->isScheduled()) { // If not scheduled, schedule ack
            //////cout << "Server schedule Ack at "<<simTime()+ dtcpOF::ackT<<endl;
            scheduleAt(simTime() + dtcpOF::ackT, f->at);
        }
    } else if (shared_ptr<ofData> lData = dynamic_pointer_cast < ofData > (data)) { // Data from server
        clientOF * f = cFlows[_src][lData->flow];

        //Recompute retransmission time
        f->retT = dtcpOF::ackT + 3*(simTime().dbl() - lData->t0.dbl());

        if(f == nullptr) {
            ////cout << "Null flow at "<< src<<" ?? "<< _src << " + "<< lData->flow << endl;
        }
        //Recompute A/B - Destination ack/selNack values
        {
            bool reqAction = false;
            seq_t & s = lData->seq;
            seq_t & A = f->A;
            seq_t & B = f->B;
            if(A < B) {
                if(s > A) {
                    if(s == B) {
                        f->nacking = false;
                        //Ack
                        A++;
                        B++;
                    } else if(s == A+2) {
                        f->nacking = false;
                        //Selective nAck
                        A += 2;
                        if(f->lastNack != B) {
                            reqAction = true;
                            f->lastNack = B;
                        }
                    } else {
                        f->nacking = true;
                        //nAck
                        B = A;
                        if(f->lastNack != B) {
                            reqAction = true;
                            f->lastNack = B;
                        }
                    }
                }
            } else if(s == B) {
                f->nacking = false;
                B = A+1; // Ack
            } else if ( s == A+1) {
                f->nacking = false;
                A++; //Selective nAck
            } else if(s > A) {
                f->nacking = true;
                //nAck
                if(f->lastNack != B) {
                    reqAction = true;
                    f->lastNack = B;
                }
            }
            if(reqAction) {
                if(f->at->isScheduled()) {
                    cancelEvent(f->at);
                    take (f->at);
                    rackClient(f->at, f);
                }
            } else if(!f->at->isScheduled()) {
                scheduleAt(simTime()+dtcpOF::ackT, f->at);
            }
        }
        //Need to retransmit something?
        {
            seq_t & s = f->sent;
            seq_t & k = f->acked;
            seq_t & A = lData->A;
            seq_t & B = lData->B;
            if(A == B ) { // nAck
                if(k > A) { // Already acked future PDUs ??
                    //////cout << "Something wrong in nAck - 1 "<< A << " / "<< B<<endl;
                } else if (A > s){ // Nack of not sent PDU??
                    //////cout << "Something wrong in nAck - 2 "<< A << " / "<< B<<endl;
                } else { // On nAck, ack all until A and transmit all after that again
                    if(k < A) { k = A; }
                    s = k;
                    if(!f->At->isScheduled() && ! f->rt->isScheduled()) { // If not scheduled, start resending
                        retClient(f->rt, f);
                    }
                }
            } else if(k < A) {
                k = A;
                if(f->rt->isScheduled()) {
                    cancelEvent(f->rt);
                }
            } // On Ack, ack all until A

            if(!f->at->isScheduled()) { // If not scheduled, schedule ack
                scheduleAt(simTime() + dtcpOF::ackT, f->at);
            }
        }


        if(f->A < f->B && f->until == f->A && !f->At->isScheduled()) { // Received last PDU of current request.
            ////cout << f->until <<" vs "<< f->A<<endl;
            InjListener::instance->dataRequestEnded(src, f->dstAddr, f->flowId);
            if(simTime() < f->next_time) {
                //////cout << "schedul" <<endl;
                scheduleAt(f->next_time, f->At);
            } else {
                actClient(f->At, f);
            }
        }
    } else //Voice message
        if (shared_ptr<ofMsg> lData = dynamic_pointer_cast < ofMsg > (data)) {
        InjListener::instance->voiceReceived(_src, src, lData->flow,
                simTime() - lData->t0);
    }
}

