#include "GDC_R1.h"

namespace GDC {
    using namespace std;

    Register_Class(R1);


    void R1::startMyNeis() {
        if(Im.a <= nPod) { mySons = nTor; }
        else { mySons = nEdge; }

        myNeisSize = mySons + nSpines;
        myNeis = new nodeNeig[myNeisSize];

        for(ushort i = 0; i< nSpines; i++) {
            auto & n = myNeis[i];
            Addr n_addr = Addr(0, i+1);
            n.init(n_addr, Address(n_addr.toString().c_str(), myAddress.getDifName().getName().c_str()));

            linkInfo li(linkId(Im, n_addr), false, -1);
            linksStatus[li.link] = li;
        }
        for(ushort i = 0; i< mySons; i++) {
            auto & n = myNeis[i+nSpines];
            Addr n_addr = Addr(Im.a, nFab + i + 1);
            n.init(n_addr, Address(n_addr.toString().c_str(), myAddress.getDifName().getName().c_str()));

            linkInfo li(linkId(Im, n_addr), false, -1);
            linksStatus[li.link] = li;
        }
    }

    ushort R1::getNeiId(const Addr & addr) {
        if(addr.a == 0 && addr.b > 0 && addr.b <= nSpines) {
            return addr.b-1;
        } else if(addr.a == Im.a && addr.b > nFab && addr.b <= nFab+mySons) {
            return nSpines + addr.b - 1 - nFab;
        }

        return myNeisSize;
    }

    vecRawException R1::getExceptions() {
        vecRawException ret;

        if(!preparseFailures()) { return ret; }

        bool imDUp = fnd(R1DeadUp, Im);
        bool imDDown = fnd(R1DeadDown, Im);
        if(imDUp && imDDown) { return ret; }

        //My neis
        vbool myUNeis(nSpines, true);
        vbool myDNeis(mySons, true);
        const setAddr & myFailsU = R1FailsUp[Im];
        const setAddr & myFailsD = R1FailsDown[Im];
        for(const Addr & f : myFailsU) { myUNeis[f.b-1] = false; }
        for(const Addr & f : myFailsD) { myDNeis[f.b-1-nFab] = false; }
        //bool hasUFails = !myFailsU.empty();
        bool hasDFails = !myFailsD.empty();


        Addr tuNei(0,0);
        Addr tdNei(Im.a,0);

        //Dead R2 nodes
        for(const Addr & d : R2Dead) { ret.push_back(RawException(d, vecAddr()) ); }

        //Exceptions for unreachable groups
        for(const uchar & g : unreachGroup) {
            ret.push_back(RawException(Addr(g, 0), vecAddr()) );
        }

        if(!imDDown) {//Can go down
            //Problems within the R2 neis?
            if(hasDFails) {
                for(const Addr & dst : myFailsD) {
                    if(fnd(R2Dead, Im)) { continue; } // R2 dead

                    //Parse Dst fails
                    vbool dstNeis(nFab, true);
                    const setAddr & dstFails = R2Fails[dst];
                    for(const Addr & f : dstFails) { dstNeis[f.b-1] = false; }

                    //Check neis without problems to reach R1s
                    vbool okNeis(mySons, true);
                    const uchar2setuchar & groupFails = R2GroupedFailures[Im.a];
                    for(const auto & fail : groupFails) { okNeis[fail.first-1-nFab] = false; }

                    vecAddr valids;
                    for(uchar i = 0; i< mySons; i++) {
                        if(!myDNeis[i]) { continue; }
                        bool ok = false;
                        if(okNeis[i]) { ok = true; } //all neis of dst are shared
                        else {
                            //Parse Nei fails
                            tdNei.b = i + nFab +1;
                            vbool neiNeis(nFab, true);
                            const setAddr & neiFails = R2Fails[tdNei];
                            for(const Addr & f : neiFails) { neiNeis[f.b-1] = false; }

                            //Check for shared R1 neis between dst and nei i
                            for(uchar j = 0; j < nFab; j++) {
                                if(dstNeis[j] && neiNeis[j]) { ok = true; }
                            }
                        }

                        if(ok) {
                            tdNei.b = i + nFab +1;
                            valids.push_back(tdNei);
                        }
                    }
                    ret.push_back(RawException(dst, valids) );
                }
            }
        } else { //Dead down... go up to neigbour R1s
            //Only check if an R1 connected to the group can be reached,
            //don't find paths to specific R2s
            //high number of paths makes highly probable that any R0 used can reach opt any R2
            Addr tN(Im.a, 0);
            vbool neiUNeis = myUNeis;
            bool liveNeis = false;
            for(uchar i = 0;  i < nFab; i++) {
                if(i == Im.b-1) { continue; }
                tN.b = i+1;
                if(fnd(R1DeadUp, tN) || fnd(R1DeadDown, tN)) { continue; }
                else {
                    liveNeis = true;
                    const setAddr & neiFailsU = R1FailsUp[tN];
                    for(const Addr & f : neiFailsU) { neiUNeis[f.b-1] = false; }
                }
            }

            if(!liveNeis) { // Impossible to reach group with 2 hops
                ret.push_back(RawException(Addr(Im.a, 0), vecAddr()) );
            } else { // Some nei R1 reachable through some R0
                vecAddr valids;
                for(uchar i = 0; i < nSpines; i++) {
                    if(!neiUNeis[i]) { continue; }
                    tuNei.b = i+1;
                    valids.push_back(tuNei);
                }
                ret.push_back(RawException(Addr(Im.a, 0), valids) );
            }

        }

        //Reaching other groups
        if(!imDUp) { //Can go Up
            //Parse groups with R1 Up problems
            uchar2setuchar groupFails;
            for(const auto & fail : R1FailsUp) {
                const Addr & dst = fail.first;
                if(dst.a == Im.a) { continue; } // Skip if R1 in same group
                for(const Addr & sf : fail.second) {
                    if(myUNeis[sf.b]) { // Only consider problems to reachable neis
                        Addr dst = fail.first;
                        groupFails[dst.a].insert(dst.b);
                        break;
                    }
                }
            }

            Addr tN(0,0);
            for(const auto & gfail : groupFails) {
                tN.a= gfail.first;
                const setuchar & gProbs = gfail.second;
                bool gHasProbs = fnd(groupProb, tN.a);

                if(gProbs.size() >= nFab || gHasProbs) {

                    //Shared neis with dst group R1s
                    vbool vNeis[nFab];
                    for(uchar i = 0; i < nFab; i++) { vNeis[i] = myUNeis; }

                    vbool isAlive(nFab, true);
                    for(const uchar i : gProbs) {
                        tN.b = i;
                        if(fnd(R1DeadDown, tN) || fnd(R1DeadUp, tN)) { isAlive[i] = false; }
                        else if(fnd(R1FailsUp, tN)) {
                            //Check problems at dst R1
                            const setAddr & dF = R1FailsUp[tN];
                            for(const Addr f : dF) { vNeis[i-1][f.b-1] = false; }
                        }
                    }

                    vbool validNeis = myUNeis;
                    if(gProbs.size() >= nFab) { //All dst R1 have problems to reach some R0 neis
                        //Check if all R0 neis can be used to reach at least one dst R1
                        bool changes = false;
                        for(uchar j = 0; j < nSpines; j++) {
                            if(!validNeis[j]) { continue; } //Already not a reachable nei
                            uchar vcount = nFab;
                            for(uchar i = 0; i< nFab; i++) {
                                if(!isAlive[i]) { vcount--; }
                                else if(!vNeis[i][j]) { vcount--; }
                            }
                            if(vcount <= 0) {
                                validNeis[j] = false;
                                changes = true;
                            }
                        }
                        if(changes) { //Some R0 neis cannot opt-reach some dst R1
                            vecAddr valids;
                            for(uchar i = 0; i < nSpines; i++) {
                                if(!validNeis[i]) { continue; }
                                tuNei.b = i+1;
                                valids.push_back(tuNei);
                            }
                            tN.b = 0;
                            ret.push_back(RawException(tN, valids) );
                        }
                    }
                    if(gHasProbs) { //Some R2 may be non opt-reachable via all dst R1
                        //Check all problematic dst R2s
                        const uchar2setuchar & gfails = R2GroupedFailures[tN.a];
                        for(const auto & rfails : gfails) {


                            vbool dstNeis(nFab, true);

                            //Check if all dst fails are dead nodes
                            bool allFailsDead = true;
                            for(const uchar & f : rfails.second) {
                                dstNeis[f-1] = false;
                                if(isAlive[f-1]) { allFailsDead = false; }
                            }
                            if(allFailsDead) { continue; } // Skip if only dead R1 errors found

                            //Search for shared neis with dst R1 neis
                            vbool validDstNeis(nSpines, false);
                            for(uchar i = 0; i < nFab; i++) {
                                if(!dstNeis[i]) { continue; }
                                if(!isAlive[i]) { continue; }
                                for(uchar j = 0; j< nSpines; j++) {
                                    if(vNeis[i][j]) { validDstNeis[j] = true; }
                                }
                            }

                            if(validNeis != validDstNeis) { //Valid neis to reach dst R2 differ from default to reach dst group
                                vecAddr valids;
                                for(uchar i = 0; i < nSpines; i++) {
                                    if(!validDstNeis[i]) { continue; }
                                    tuNei.b = i+1;
                                    valids.push_back(tuNei);
                                }
                                tN.b = rfails.first;
                                ret.push_back(RawException(tN, valids) );
                            }
                        }
                    }
                }
            }
        } else { //Dead UP, go down for default
            //Only search for nei R1 up connected
            //high number of paths makes highly probable that any R2 used can reach opt any group/R2
            Addr tN(Im.a, 0);
            vbool neiDNeis = myDNeis;
            bool liveNeis = false;
            for(uchar i = 0; i < nFab; i++) {
                if(i == Im.b-1) { continue; }
                tN.b = i+1;
                if(fnd(R1DeadUp, tN) || fnd(R1DeadDown, tN)) { continue; }
                else {
                    liveNeis = true;
                    const setAddr & neiFailsU = R1FailsUp[tN];
                    for(const Addr & f : neiFailsU) { neiDNeis[f.b-1-nFab] = false; }
                }
            }
            if(!liveNeis) { // Impossible to reach outside with 2 hops
                ret.push_back(RawException(Addr(0, 0), vecAddr()) );
            } else { // Some nei R1 reachable through some R2
                vecAddr valids;
                for(uchar i = 0; i < mySons; i++) {
                    if(!neiDNeis[i]) { continue; }
                    tdNei.b = i+1+nFab;
                    valids.push_back(tdNei);
                }
                ret.push_back(RawException(Addr(0, 0), valids) );
            }
        }


        return ret;
    }
}