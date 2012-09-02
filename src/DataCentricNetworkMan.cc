
#include "DataCentricNetworkMan.h"
#include <stdio.h>
#include <string.h>



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





        controlPackets.setName("ControlPackets");
        controlPacketFrequency.setName("controlPacketFrequency");
        mpControlPacketFrequencyMessage = new cMessage("ControlPacketFrequencyMessage");


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
        r6.w =35;
        r6.h =35;
        memcpy(r6.context, "\x06\x00", 3);
        mRegions.push_back(r6);

        Region r61;
        r61.x =0;
        r61.y =0;
        r61.w =18;
        r61.h =18;
        memcpy(r61.context, "\x06\x01\x00", 3);
        mRegions.push_back(r61);

        Region r62;
        r62.x =18;
        r62.y =0;
        r62.w =17;
        r62.h =18;
        memcpy(r62.context, "\x06\x02\x00", 3);
        mRegions.push_back(r62);

        Region r63;
        r63.x =0;
        r63.y =18;
        r63.w =18;
        r63.h =17;
        memcpy(r63.context, "\x06\x03\x00", 3);
        mRegions.push_back(r63);

        Region r64;
        r64.x =18;
        r64.y =18;
        r64.w =17;
        r64.h =17;
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
    }


}


void DataCentricNetworkMan::handleMessage(cMessage* msg)
{
    //nodeConstraint = nodeConstraintValue;
    //currentModuleId = this->getId();
    //thisAddress = mAddress;
    //rd = &(moduleRD);



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



