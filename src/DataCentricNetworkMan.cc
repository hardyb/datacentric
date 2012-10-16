
#include "DataCentricTestApp.h"

// Possible problem with this
//#include "DataCentricNetworkMan.h"

#include <stdio.h>
#include <string.h>









/*
static cNEDValue ned_choose(cComponent *context, cNEDValue argv[], int argc)
{
int index = (int)argv[0];
for ( int i = 0; i < index; i++ )
{

}
if (index < 0 || index >= argc-1)
throw cRuntimeError("choose(): index %d is out of range", index);
return argv[index+1];
}
Define_NED_Function(ned_choose, "any choose(int index, ...)");
Here, the value of argv[0] is read using the typecast operator that maps to longValue
*/

























Define_Module(DataCentricNetworkMan);

void DataCentricNetworkMan::initialize(int aStage)
{
    cSimpleModule::initialize(aStage); //DO NOT DELETE!!

    if (0 == aStage)
    {
        numDataArrivals = 0;
        std::string fName = this->getFullPath();
        std::string fName2 = this->getFullPath();
        numControlPackets = 0;

        numHelloPackets = 0;
        numHelloBackPackets = 0;
        bcastNumInterestPackets = 0;
        ucastNumInterestPackets = 0;
        numAdvertPackets = 0;
        numReinforcementPackets = 0;
        numBreakagePackets = 0;
        numModulesDown = 0;
        numRREQPackets = 0;
        numDiscoveryPackets = 0;
        numRegisterPackets = 0;
        numRReplyPackets = 0;
        numAODVDataPackets = 0;

        mDemand = 0;




        controlPackets.setName("ControlPackets");
        controlPacketFrequency.setName("controlPacketFrequency");
        mpControlPacketFrequencyMessage = new cMessage("ControlPacketFrequencyMessage");
        mpDemandMessage = new cMessage("DemandMessage");


        helloPacketFrequency.setName("helloPacketFrequency");
        helloBackPacketFrequency.setName("helloBackPacketFrequency");
        bcastInterestPacketFrequency.setName("bcastInterestPacketFrequency");
        ucastInterestPacketFrequency.setName("ucastInterestPacketFrequency");
        advertPacketFrequency.setName("advertPacketFrequency");
        reinforcementPacketFrequency.setName("reinforcementPacketFrequency");
        dataPacketE2EDelay.setName("dataPacketE2EDelay");
        breakagePacketFrequency.setName("breakagePacketFrequency");
        modulesDownVector.setName("modulesDownVector");
        RREQPacketFrequency.setName("RREQPacketFrequency");
        DiscoveryPacketFrequency.setName("DiscoveryPacketFrequency");
        RegisterPacketFrequency.setName("RegisterPacketFrequency");
        RReplyPacketFrequency.setName("RReplyPacketFrequency");
        AODVDataPacketFrequency.setName("AODVDataPacketFrequency");

        demandVector.setName("demandVector");
        pendingRREQVector.setName("pendingRREQVector");




        /*
         * A region has top left coord (x,y) and width and height w, h
         * A region may also have a context / location name e.g. \6\4
         *
         * Example
         *
         * region 1 - 0,0 w=18 h=18
         * region 2 - 18,0 w=17 h=18
         * region 3 - 0,18 w=18 h=17
         * region 4 - 18,18 w=17 h=17
         *
         * where shall we store this data?
         * we need to set context from ini file
         *
         * FOR NOW...
         * Lets do a hard-coded example
         *
         * For the main ongoing code we need to think a little deeper
         * about the hierarchy, overlap etc etc
         *
         */

        Region r6;
        r6.x =0;
        r6.y =0;
        r6.z =0;
        r6.w =10;
        r6.h =12;
        r6.d =3.5;
        memcpy(r6.context, "\x06\x00", 3);
        mRegions.push_back(r6);

        Region r61;
        r61.x =0;
        r61.y =0;
        r61.z =0;
        r61.w =5;
        r61.h =6;
        r61.d =3.5;
        memcpy(r61.context, "\x06\x01\x00", 3);
        mRegions.push_back(r61);

        Region r62;
        r62.x =5;
        r62.y =0;
        r62.z =0;
        r62.w =5;
        r62.h =6;
        r62.d =3.5;
        memcpy(r62.context, "\x06\x02\x00", 3);
        mRegions.push_back(r62);

        Region r63;
        r63.x =0;
        r63.y =6;
        r63.z =0;
        r63.w =5;
        r63.h =6;
        r63.d =3.5;
        memcpy(r63.context, "\x06\x03\x00", 3);
        mRegions.push_back(r63);

        Region r64;
        r64.x =5;
        r64.y =6;
        r64.z =0;
        r64.w =5;
        r64.h =6;
        r64.d =3.5;
        memcpy(r64.context, "\x06\x04\x00", 3);
        mRegions.push_back(r64);




        //netModule = check_and_cast<DataCentricNetworkLayer*>(this->getParentModule()->getSubmodule("net"));
        //mpStartMessage = new cMessage("StartMessage");
        //m_debug             = par("debug");
        //contextData = par("nodeContext").stringValue();
        //std::string temp1 = par("sourceFor").stringValue();
        //sourcesData = cStringTokenizer(temp1.c_str()).asVector();
        //std::string temp2 = par("sinkFor").stringValue();
        //sinksData = cStringTokenizer(temp2.c_str()).asVector();
        //mLowerLayerIn        = findGate("lowerLayerIn");
        //mLowerLayerOut       = findGate("lowerLayerOut");
        //m_moduleName        = getParentModule()->getFullName();
        //sumE2EDelay         = 0;
        //numReceived         = 0;
        //mNumTrafficMsgs     = 0;
        //totalByteRecv           = 0;
        //e2eDelayVec.setName("End-to-end delay");
        //meanE2EDelayVec.setName("Mean end-to-end delay");
        //scheduleAt(simTime() + StartTime(), mpStartMessage);

        traverseModule(*getParentModule());



    }

    if (1 == aStage)
    {

    }

    if (2 == aStage)
    {
        bool isSources;
        string sinksString = par("sinks").stringValue();
        isSources = false;
        setSinkOrSources(sinksString, isSources);

        string sourcesString = par("sources").stringValue();
        isSources = true;
        setSinkOrSources(sourcesString, isSources);

    }

}


void DataCentricNetworkMan::setSinkOrSources(string &sinksString, bool isSources)
{
    std::vector<std::string> sinks = cStringTokenizer(sinksString.c_str(), ",").asVector();
    int numItems = sinks.size();

    //if ( sinks.size() != 5 )
    if ( (sinks.size() % 5) )
    {
        //throw cRuntimeError("items in sinks must be multiple of 3");
        throw cRuntimeError("items in sinks or sources must be 5");
    }

    for ( int i = 0; i < sinks.size(); i += 5)
    {
        string _context = sinks[i+0];
        int _from = atoi(sinks[i+1].c_str());
        int _to = atoi(sinks[i+2].c_str());
        string _data = sinks[i+3];
        string _actions = sinks[i+4];
        int numInRegion = 0;
        for (std::vector<DataCentricTestApp*>::iterator i = mNodeArray.begin();
                i != mNodeArray.end(); ++i)
        {
            std::vector<std::string> contextDataSet = cStringTokenizer((*i)->contextData.c_str()).asVector();
            for (std::vector<std::string>::iterator it = contextDataSet.begin();
                    it != contextDataSet.end(); ++it)
            {
                if ( (*it) == _context )
                {
                    if ( (_from <= numInRegion) && (numInRegion <= _to) )
                    {
                        if ( isSources )
                        {
                            (*i)->processSourceFor(_data);
                            (*i)->processActionsFor(_actions);
                        }
                        else
                        {
                            (*i)->processSinkFor(_data);
                        }
                    }
                    numInRegion++;
                }
            }
        }

    }

    //cSimulation::getActiveSimulation();
    //cSimulation::getActiveEnvir();



}



void DataCentricNetworkMan::traverseModule(const cModule& m)
{
    for (cSubModIterator iter(m); !iter.end(); iter++)
    {
        ev << "Traversing:    " << iter()->getFullName() << endl;
        cModule* sm = dynamic_cast<cModule*>(iter());
        if ( sm )
        {
            DataCentricTestApp* dcApp = dynamic_cast<DataCentricTestApp*>(sm->getSubmodule("app"));
            if ( dcApp )
            {
                ev << "Found:    " << dcApp->getFullName() << endl;
                mNodeArray.push_back(dcApp);
                //mNodeArray[mNodeArrayIndex++] = mn;
            }

        }
        //else
        //{
        //    traverseModule(*iter());
        //}
    }

}




void DataCentricNetworkMan::addAppliance(DataCentricTestApp* _appliance)
{
    Enter_Method("addAppliance(DataCentricTestApp* _appliance)");
    mAppliances.insert(_appliance);

}


signed short DataCentricNetworkMan::getTotalDemand()
{
    Enter_Method("getTotalDemand()");
    // just in case local and remote call are different

    return totalDemand();

}


signed short DataCentricNetworkMan::totalDemand()
{
    signed short demand = 0;
    for ( std::set<DataCentricTestApp*>::iterator i = mAppliances.begin(); i != mAppliances.end(); i++ )
    {
        demand += (*i)->currentDemand;
    }

    return demand;
}


void DataCentricNetworkMan::recordDemandLocal()
{
    demandVector.record((double)totalDemand());
    this->cancelEvent(mpDemandMessage);
    scheduleAt(simTime() + 300, mpDemandMessage);
}



void DataCentricNetworkMan::recordDemand()
{
    Enter_Method("recordDemand()");

    demandVector.record((double)totalDemand());
    this->cancelEvent(mpDemandMessage);
    scheduleAt(simTime() + 300, mpDemandMessage);
}



void DataCentricNetworkMan::handleMessage(cMessage* msg)
{
    //nodeConstraint = nodeConstraintValue;
    //currentModuleId = this->getId();
    //thisAddress = mAddress;
    //rd = &(moduleRD);

    if ( msg == mpDemandMessage )
    {
        recordDemandLocal();
    }


    if ( msg == mpControlPacketFrequencyMessage )
    {
        controlPacketFrequency.record(numControlPackets);

        helloPacketFrequency.record(numHelloPackets);
        helloBackPacketFrequency.record(numHelloBackPackets);
        bcastInterestPacketFrequency.record(bcastNumInterestPackets);
        ucastInterestPacketFrequency.record(ucastNumInterestPackets);
        advertPacketFrequency.record(numAdvertPackets);
        reinforcementPacketFrequency.record(numReinforcementPackets);
        breakagePacketFrequency.record(numBreakagePackets);
        RREQPacketFrequency.record(numRREQPackets);
        DiscoveryPacketFrequency.record(numDiscoveryPackets);
        RegisterPacketFrequency.record(numRegisterPackets);
        RReplyPacketFrequency.record(numRReplyPackets);
        AODVDataPacketFrequency.record(numAODVDataPackets);


        //cOutVector RReplyPacketFrequency;
        //cOutVector AODVDataPacketFrequency;

        //double numRReplyPackets;
        //double numAODVDataPackets;




        if ( numControlPackets != 0 )
        {
            scheduleAt(simTime() + 0.005, mpControlPacketFrequencyMessage);
        }
        numControlPackets = 0;
        numHelloPackets = 0;
        numHelloBackPackets = 0;
        bcastNumInterestPackets = 0;
        ucastNumInterestPackets = 0;
        numAdvertPackets = 0;
        numReinforcementPackets = 0;
        numBreakagePackets = 0;
        numRREQPackets = 0;
        numDiscoveryPackets = 0;
        numRegisterPackets = 0;
        numRReplyPackets = 0;
        numAODVDataPackets = 0;
    }


}

void DataCentricNetworkMan::finish()
{
    recordScalar("numDataArrivals", numDataArrivals);
    //recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
}

void DataCentricNetworkMan::addADataPacketE2EDelay(simtime_t delay)
{
    Enter_Method("addADataPacketE2EDelay(simtime_t delay)");

    dataPacketE2EDelay.record(delay);
    numDataArrivals++;

}


void DataCentricNetworkMan::changeInModulesDown(double adjustment)
{
    Enter_Method("changeInModulesDown(double adjustment)");

    numModulesDown += adjustment;
    modulesDownVector.record(numModulesDown);

}


void DataCentricNetworkMan::addDemand(signed int _demand)
{
    Enter_Method("addDemand(signed int)");

    // negative deamnd is supply
    //mDemand += _demand;

    // possibly discontinueds


}



void DataCentricNetworkMan::addPendingRREQ(uint32 _originator, uint32 _destination)
{
    Enter_Method("addPendingRREQ(uint32 _originator, uint32 _destination)");

    PendingRREQ pr;
    pr.originator = _originator;
    pr.destination = _destination;
    mPendingRREQSet.insert(pr);
    mPendingRREQSet.size();
    pendingRREQVector.record((double)mPendingRREQSet.size());


}


void DataCentricNetworkMan::removePendingRREQ(uint32 _originator, uint32 _destination)
{
    Enter_Method("removePendingRREQ(uint32 _originator, uint32 _destination)");

    PendingRREQ pr;
    pr.originator = _originator;
    pr.destination = _destination;
    mPendingRREQSet.erase(pr);
    pendingRREQVector.record((double)mPendingRREQSet.size());

}



signed int DataCentricNetworkMan::getDemand()
{
    Enter_Method("getDemand()");

    // negative deamnd is supply
    //return mDemand;

    // possibly discontinueds
}


void DataCentricNetworkMan::updateControlPacketData(unsigned char type, bool ucast)
{
    Enter_Method("updateControlPacketData(unsigned char type, bool ucast)");
    if ( !mpControlPacketFrequencyMessage->isScheduled() )
    {
        scheduleAt(simTime() + 0.005, mpControlPacketFrequencyMessage);
    }

    switch ( type )
    {
    case ADVERT:
        numAdvertPackets++;
        break;
    case INTEREST:
        if ( ucast )
        {
            ucastNumInterestPackets++;
        }
        else
        {
            bcastNumInterestPackets++;
        }
        break;
    case REINFORCE:
    case REINFORCE_INTEREST:
        numReinforcementPackets++;
        break;
    case NEIGHBOR_UCAST:
        numHelloBackPackets++;
        break;
    case NEIGHBOR_BCAST:
        numHelloPackets++;
        break;
    case BREAKAGE:
        numBreakagePackets++;
        break;
    case RREQ_STAT:
        numRREQPackets++;
        break;
    case DISCOVERY_STAT:
        numDiscoveryPackets++;
        break;
    case REGISTER_STAT:
        numRegisterPackets++;
        break;
    case RREPLY_STAT:
        numRReplyPackets++;
        break;
    case AODV_DATA_STAT:
        numAODVDataPackets++;
        break;
    }




    switch ( type )
    {
        case DATA:
            break;
        default:
            numControlPackets++;
            controlPackets.record(numControlPackets);
            break;
    }

}



