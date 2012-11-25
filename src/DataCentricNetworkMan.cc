
#include "DataCentricTestApp.h"

// Possible problem with this
//#include "DataCentricNetworkMan.h"

#include <stdio.h>
#include <string.h>


//#include "SpecialDebug.h"



#include "aodv_uu_omnet.h"
#include "IPv4.h"
#include "UDPBurstAndBroadcast.h"




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
        totControlPacketsThisRun = 0;

        mExpectedDataArrivals = par("expectedDataArrivals");

        numHelloPackets = 0;
        numHelloBackPackets = 0;
        bcastNumInterestPackets = 0;
        ucastNumInterestPackets = 0;
        numAdvertPackets = 0;
        numReinforcementPackets = 0;
        numBreakagePackets = 0;
        numModulesDown = 0;
        numRREQPackets = 0;
        numRERRPackets = 0;
        numDiscoveryPackets = 0;
        numRegisterPackets = 0;
        numRReplyPackets = 0;

        // Unicast forwarded packets (this includes all RREP, data & RERR I think
        numAODVDataPackets = 0;
        numAODVDataLineBreaks = 0;
        numAODVAllLineBreaks = 0;
        numAODVDataArrival = 0;
        numPendingDataPackets = 0;
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
        RERRPacketFrequency.setName("RERRPacketFrequency");

        DiscoveryPacketFrequency.setName("DiscoveryPacketFrequency");
        RegisterPacketFrequency.setName("RegisterPacketFrequency");
        RReplyPacketFrequency.setName("RReplyPacketFrequency");

        // Unicast forwarded packets (this includes all RREP, data & RERR I think
        AODVDataPacketFrequency.setName("AODVDataPacketFrequency");
        AODVDataLineBreakVector.setName("AODVDataLineBreakVector");
        AODVAllLineBreakVector.setName("AODVAllLineBreakVector");
        DataArrivalsVector.setName("DataArrivalsVector");
        PendingDataPacketsVector.setName("PendingDataPacketsVector");



        demandVector.setName("demandVector");
        pendingRREQVector.setName("pendingRREQVector");
        pendingRegistrationVector.setName("pendingRegistrationVector");




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
        demand += (*i)->actualDemand;
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
        RERRPacketFrequency.record(numRERRPackets);
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
        numRERRPackets = 0;
        numDiscoveryPackets = 0;
        numRegisterPackets = 0;
        numRReplyPackets = 0;
        numAODVDataPackets = 0;
    }


}

void DataCentricNetworkMan::finish()
{
    //recordScalar("numDataArrivals", numDataArrivals);

    // Additional scalar recording for scatter charts in multi simulation runs
    recordScalar("FailedRREQs", (double)numPendingRREQs());
    recordScalar("FailedRegistrations", (double)numPendingRegistrations());
    recordScalar("LinkFailures", (double)numAODVAllLineBreaksValue());
    recordScalar("DataArrivals", (double)numAODVDataArrivalValue());
    recordScalar("ProactiveRREQs", (double)numProactiveRREQ());
    recordScalar("DataFailures", (double)(mExpectedDataArrivals-numAODVDataArrivalValue()));

    unsigned int numFixHosts = getParentModule()->par("numFixHosts");
    recordScalar("ProactiveRREQFailures", (double)(numFixHosts-numProactiveRREQ()));

    recordScalar("numControlPackets", totControlPacketsThisRun);// pos change text tot for run you see

    recordScalar("MeanE2EDelay", E2EDelayStats.getMean());
    recordScalar("StdDevE2EDelay", E2EDelayStats.getStddev());
    recordScalar("MaxE2EDelay", E2EDelayStats.getMax());
    recordScalar("MinE2EDelay", E2EDelayStats.getMin());





    //recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
}

void DataCentricNetworkMan::addADataPacketE2EDelay(simtime_t delay)
{
    Enter_Method("addADataPacketE2EDelay(simtime_t delay)");

    if ( true ) // Add a condition to the definition of successful delivery
    {
        numDataArrivals++; // Total data arrivals in the run
        DataArrivalsVector.record(numDataArrivals); // packets over time
    }

    dataPacketE2EDelay.record(delay);

    E2EDelayStats.collect(delay);

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



void DataCentricNetworkMan::addPendingRegistration(uint32 _originator)
{
    Enter_Method("addPendingRegistration(uint32 _originator)");

    mPendingRegistrationSet.insert(_originator);
    mPendingRegistrationSet.size();
    pendingRegistrationVector.record((double)mPendingRegistrationSet.size());
}


void DataCentricNetworkMan::addProactiveRREQ(uint32 _originator)
{
    Enter_Method("addProactiveRREQ(uint32 _originator)");

    mProactiveRREQSet.insert(_originator);

}

void DataCentricNetworkMan::clearProactiveRREQ()
{
    Enter_Method("clearProactiveRREQ()");

    mProactiveRREQSet.clear();

}


void DataCentricNetworkMan::addPendingDataPkt()
{
    Enter_Method("addPendingDataPkt()");

    numPendingDataPackets++;
    PendingDataPacketsVector.record(numPendingDataPackets);

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



unsigned int DataCentricNetworkMan::numPendingRREQs()
{
    Enter_Method("numPendingRREQs()");

    return mPendingRREQSet.size();

}



void DataCentricNetworkMan::removePendingRegistration(uint32 _originator)
{
    Enter_Method("removePendingRegistration(uint32 _originator)");

    mPendingRegistrationSet.erase(_originator);
    pendingRegistrationVector.record((double)mPendingRegistrationSet.size());

}



unsigned int DataCentricNetworkMan::numPendingRegistrations()
{
    Enter_Method("numPendingRegistrations()");

    return mPendingRegistrationSet.size();

}


unsigned int DataCentricNetworkMan::numProactiveRREQ()
{
    Enter_Method("numProactiveRREQ()");

    return mProactiveRREQSet.size();

}




unsigned int DataCentricNetworkMan::numAODVAllLineBreaksValue()
{
    Enter_Method("numAODVAllLineBreaksValue()");

    return numAODVAllLineBreaks;

}




unsigned int DataCentricNetworkMan::numAODVDataArrivalValue()
{
    Enter_Method("numAODVDataArrivalValue()");

    return numAODVDataArrival;

}



void DataCentricNetworkMan::removePendingDataPkt()
{
    Enter_Method("removePendingDataPkt()");

    numPendingDataPackets--;
    PendingDataPacketsVector.record(numPendingDataPackets);

}



signed int DataCentricNetworkMan::getDemand()
{
    Enter_Method("getDemand()");

    // negative deamnd is supply
    //return mDemand;

    // possibly discontinueds
}


/*
 * This method is called from:
 *      - inetmanet (indirectly - and for the AODV/Zigbee model  -
 *                      IPV4_DATA_STAT, RERR_STAT, AODV_DATA_LINEBREAK, AODV_ALL_LINEBREAK)
 *      - UDPBurstAndBroadcast (Used by AODV/Zigbee model)
 *      - DataCentricNetworkLayer (Used by data centric model)
 *
 *
 */
void DataCentricNetworkMan::recordOnePacket(unsigned char type)
{
    Enter_Method("recordOnePacket(unsigned char type)");


    recordOneDataCentricPacketForTotalInRun(type);
    recordOneDataCentricPacketForFrequencyStats(type);

    recordOneAODVZIGBEEPacketForTotalInRun(type);
    recordOneAODVZIGBEEPacketForFrequencyStats(type);

    // what we need to record in common to both models at end of a run
    /*
     * ACCURACY
     * mean e2e delay
     * num succ delivery
     * num succ delivery (redfined with delay)
     * num fail delivery
     *
     * OVERHEAD
     * num control packets
     * bandwidth
     * energy used
     *
     * PERFORMANCE
     * delay
     * scalability
     *
     */








    // probably only do this for the frequency type stats
    if ( !mpControlPacketFrequencyMessage->isScheduled() )
    {
        scheduleAt(simTime() + 0.005, mpControlPacketFrequencyMessage);
    }


}










/*
 * Statistic types taken from the following headers:
 *      - RoutingAndAggregation.h (data centric packets)
 */
void DataCentricNetworkMan::recordOneDataCentricPacketForTotalInRun(unsigned char type)
{
    // Total over a run - DATACENTRIC Model
    switch ( type )
    {
        case ADVERT: // generate or forward advert
        case INTEREST: // generate or forward interest
        case REINFORCE:  // generate or forward reinforce advert
        case REINFORCE_INTEREST:  // generate or forward reinforce interest
        case NEIGHBOR_UCAST: // generate unicast hello (DISCONTINUED?)
        case NEIGHBOR_BCAST: // generate broadcast hello
        case BREAKAGE: // generate breakage message (DISCONTINUED?)
            totControlPacketsThisRun++;  // Total data centric control packets in the run
            controlPackets.record(totControlPacketsThisRun);  // packets over time
            break;
    }
}




/*
 * Statistic types taken from the following headers:
 *      - RoutingAndAggregation.h (data centric packets)
 */
void DataCentricNetworkMan::recordOneDataCentricPacketForFrequencyStats(unsigned char type)
{
    // Frequency statistics - DATACENTRIC Model
    switch ( type )
    {
    case ADVERT: // generate or forward advert
        numAdvertPackets++; // packets over a frequency interval
        break;
    case INTEREST: // generate or forward interest
        bcastNumInterestPackets++; // packets over a frequency interval
        break;
    case REINFORCE:  // generate or forward reinforce advert
    case REINFORCE_INTEREST:  // generate or forward reinforce interest
        numReinforcementPackets++; // packets over a frequency interval
        break;
    case NEIGHBOR_UCAST: // generate unicast hello (DISCONTINUED?)
        numHelloBackPackets++; // packets over a frequency interval
        break;
    case NEIGHBOR_BCAST: // generate broadcast hello
        numHelloPackets++; // packets over a frequency interval
        break;
    case BREAKAGE: // generate breakage message (DISCONTINUED?)
        numBreakagePackets++; // packets over a frequency interval
        break;
    }

    switch ( type )
    {
        case ADVERT: // generate or forward advert
        case INTEREST: // generate or forward interest
        case REINFORCE:  // generate or forward reinforce advert
        case REINFORCE_INTEREST:  // generate or forward reinforce interest
        case NEIGHBOR_UCAST: // generate unicast hello (DISCONTINUED?)
        case NEIGHBOR_BCAST: // generate broadcast hello
        case BREAKAGE: // generate breakage message (DISCONTINUED?)
            numControlPackets++; // packets over a frequency interval
            break;
    }
}



/*
 * Statistic types taken from the following headers:
 *      - aodv_uu_omnet.h (AODV control packets)
 *      - IPv4.h (data packets)
 *      - UDPBurstAndBroadcast.h (Zigbee discovery, binding etc control packets)
 */
void DataCentricNetworkMan::recordOneAODVZIGBEEPacketForTotalInRun(unsigned char type)
{
    // Total over a run - AODV/Zigbee Model
    switch ( type )
    {
    case AODV_DATA_LINEBREAK: // One AODV data packet only line break occurred
        numAODVDataLineBreaks++; // Total aodv data only packet line breaks in the run
        AODVDataLineBreakVector.record(numAODVDataLineBreaks); // packets over time
        break;
    case AODV_ALL_LINEBREAK: // One AODV packet line break occurred
        numAODVAllLineBreaks++; // Total aodv packet line breaks in the run
        AODVAllLineBreakVector.record(numAODVAllLineBreaks); // packets over time
        break;
    }

    // Total over a run - AODV/Zigbee Model
    switch ( type )
    {
    case DISCOVERY_STAT: // A Zigbee discover packet generated or forwarded
    case REGISTER_STAT: // A Zigbee registration packet generated
    case RREQ_STAT: // An AODV RREQ generated or forwarded
    case RREPLY_STAT: // An AODV RREP generated or forwarded
    case RERR_STAT: // An AODV RERR generated or forwarded
            totControlPacketsThisRun++;  // Total control packets in the run
            controlPackets.record(totControlPacketsThisRun);  // packets over time
            break;
    }
}



/*
 * Statistic types taken from the following headers:
 *      - aodv_uu_omnet.h (AODV control packets)
 *      - IPv4.h (data packets)
 *      - UDPBurstAndBroadcast.h (Zigbee discovery, binding etc control packets)
 */
void DataCentricNetworkMan::recordOneAODVZIGBEEPacketForFrequencyStats(unsigned char type)
{
    // Frequency statistics - AODV/ZIGBEE Model only
    switch ( type )
    {
    case DISCOVERY_STAT: // A Zigbee discover packet generated or forwarded
        numDiscoveryPackets++; // packets over a frequency interval
        break;
    case REGISTER_STAT: // A Zigbee registration packet generated
        numRegisterPackets++; // packets over a frequency interval
        break;
    case RREQ_STAT: // An AODV RREQ generated or forwarded
        numRREQPackets++; // packets over a frequency interval
        break;
    case RREPLY_STAT: // An AODV RREP generated or forwarded
        numRReplyPackets++; // packets over a frequency interval
        break;
    case IPV4_DATA_STAT: // An AODV Data packet generated or fowarded
        numAODVDataPackets++; // packets over a frequency interval
        break;
    case RERR_STAT: // An AODV RERR generated or forwarded
        numRERRPackets++; // packets over a frequency interval
        break;
    }

}
