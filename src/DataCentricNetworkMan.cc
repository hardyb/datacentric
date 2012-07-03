
#include "DataCentricNetworkMan.h"
#include <stdio.h>
#include <string.h>



Define_Module(DataCentricNetworkMan);

void DataCentricNetworkMan::initialize(int aStage)
{
    cSimpleModule::initialize(aStage); //DO NOT DELETE!!

    if (0 == aStage)
    {
        std::string fName = this->getFullPath();
        std::string fName2 = this->getFullPath();
        numControlPackets = 0;

        numHelloPackets = 0;
        numHelloBackPackets = 0;
        bcastNumInterestPackets = 0;
        ucastNumInterestPackets = 0;
        numAdvertPackets = 0;
        numReinforcementPackets = 0;


        controlPackets.setName("ControlPackets");
        controlPacketFrequency.setName("controlPacketFrequency");
        mpControlPacketFrequencyMessage = new cMessage("ControlPacketFrequencyMessage");


        helloPacketFrequency.setName("helloPacketFrequency");
        helloBackPacketFrequency.setName("helloBackPacketFrequency");
        bcastInterestPacketFrequency.setName("bcastInterestPacketFrequency");
        ucastInterestPacketFrequency.setName("ucastInterestPacketFrequency");
        advertPacketFrequency.setName("advertPacketFrequency");
        reinforcementPacketFrequency.setName("reinforcementPacketFrequency");


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


    }


}

void DataCentricNetworkMan::finish()
{
    //recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
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



