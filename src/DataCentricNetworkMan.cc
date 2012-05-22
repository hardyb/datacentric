
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
        controlPackets.setName("ControlPackets");
        controlPacketFrequency.setName("controlPacketFrequency");
        mpControlPacketFrequencyMessage = new cMessage("ControlPacketFrequencyMessage");



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
        if ( numControlPackets != 0 )
        {
            scheduleAt(simTime() + 0.005, mpControlPacketFrequencyMessage);
        }
        numControlPackets = 0;


    }


}

void DataCentricNetworkMan::finish()
{
    //recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
}



void DataCentricNetworkMan::updateControlPacketData()
{
    Enter_Method("updateControlPacketData()");
    if ( !mpControlPacketFrequencyMessage->isScheduled() )
    {
        scheduleAt(simTime() + 0.005, mpControlPacketFrequencyMessage);
    }
    numControlPackets++;
    controlPackets.record(numControlPackets);
}


