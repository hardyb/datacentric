//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2007 Universidad de Málaga
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "UDPBurstAndBroadcast.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "BroadcastMessage_m.h"
#include "DataCentricNetworkLayer.h"
#include "aodv_uu_omnet.h"



EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("ChooseDestAddrMode");
    if (!e) enums.getInstance()->add(e = new cEnum("ChooseDestAddrMode"));
    e->insert(UDPBurstAndBroadcast::ONCE, "once");
    e->insert(UDPBurstAndBroadcast::PER_BURST, "perBurst");
    e->insert(UDPBurstAndBroadcast::PER_SEND, "perSend");
);

Define_Module(UDPBurstAndBroadcast);

int UDPBurstAndBroadcast::counter;

simsignal_t UDPBurstAndBroadcast::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBurstAndBroadcast::rcvdPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBurstAndBroadcast::outOfOrderPkSignal = SIMSIGNAL_NULL;
simsignal_t UDPBurstAndBroadcast::dropPkSignal = SIMSIGNAL_NULL;

static void cb_record_stats(double stat);
DataCentricNetworkMan* netMan;


void cb_record_stats(double stat)
{
    netMan->updateControlPacketData(255, false);

}


UDPBurstAndBroadcast::UDPBurstAndBroadcast()
{
    messageLengthPar = NULL;
    burstDurationPar = NULL;
    sleepDurationPar = NULL;
    sendIntervalPar = NULL;
    timerNext = NULL;
    outputInterface = -1;
    outputInterfaceMulticastBroadcast.clear();
    pktDelay = new cStdDev("burst pkt delay");
}

UDPBurstAndBroadcast::~UDPBurstAndBroadcast()
{
    cancelAndDelete(timerNext);
    delete pktDelay;
}

void UDPBurstAndBroadcast::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    counter = 0;
    numSent = 0;
    numReceived = 0;
    numDeleted = 0;
    numDuplicated = 0;

    mUpperLayerIn  = findGate("datacentricAppIn");
    mUpperLayerOut = findGate("datacentricAppOut");

    delayLimit = par("delayLimit");
    simtime_t startTime = par("startTime");
    stopTime = par("stopTime");

    messageLengthPar = &par("messageLength");
    burstDurationPar = &par("burstDuration");
    sleepDurationPar = &par("sleepDuration");
    sendIntervalPar = &par("sendInterval");
    nextSleep = startTime;
    nextBurst = startTime;
    nextPkt = startTime;
    e2eDelayVec.setName("End-to-end delay");

    setRecordStatsCallBack(cb_record_stats);

    cSimulation* sim =  cSimulation::getActiveSimulation();
    mNetMan = check_and_cast<DataCentricNetworkMan*>(sim->getModuleByPath("csma802154net.dataCentricNetworkMan"));
    netMan = mNetMan;

    destAddrRNG = par("destAddrRNG");
    const char *addrModeStr = par("chooseDestAddrMode").stringValue();
    int addrMode = cEnum::get("ChooseDestAddrMode")->lookup(addrModeStr);
    if (addrMode == -1)
        throw cRuntimeError("Invalid chooseDestAddrMode: '%s'", addrModeStr);
    chooseDestAddrMode = (ChooseDestAddrMode)addrMode;

    WATCH(numSent);
    WATCH(numReceived);
    WATCH(numDeleted);
    WATCH(numDuplicated);

    localPort = par("localPort");
    destPort = par("destPort");

    mIsControlUnit = par("isControlUnit").boolValue();

    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort);
    if (par("setBroadcast").boolValue())
        socket.setBroadcast(true);

    if (strcmp(par("outputInterface").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        InterfaceEntry *ie = ift->getInterfaceByName(par("outputInterface").stringValue());
        if (ie == NULL)
            throw cRuntimeError(this, "Invalid output interface name : %s",par("outputInterface").stringValue());
        outputInterface = ie->getInterfaceId();
    }

    outputInterfaceMulticastBroadcast.clear();
    if (strcmp(par("outputInterfaceMulticastBroadcast").stringValue(),"") != 0)
    {
        IInterfaceTable* ift = InterfaceTableAccess().get();
        const char *ports = par("outputInterfaceMulticastBroadcast");
        cStringTokenizer tokenizer(ports);
        const char *token;
        while ((token = tokenizer.nextToken()) != NULL)
        {

            InterfaceEntry *ie = ift->getInterfaceByName(token);
            if (ie == NULL)
                throw cRuntimeError(this, "Invalid output interface name : %s",token);
            outputInterfaceMulticastBroadcast.push_back(ie->getInterfaceId());
        }
    }

    mBcastAddr = IPv4Address::ALLONES_ADDRESS;
    //mServerAddr = 0x0;
    // should return isUnspecified() true
    mServerAddr = IPvXAddressResolver().resolve("csma802154net.fixhost[115]");


    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    while ((token = tokenizer.nextToken()) != NULL)
    {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }

    isSource = !destAddresses.empty();

    if (isSource)
    {
        if (chooseDestAddrMode == ONCE)
            destAddr = chooseDestAddr();

        activeBurst = true;

        timerNext = new cMessage("UDPBasicBurstTimer");
        scheduleAt(startTime, timerNext);
    }

    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");
    outOfOrderPkSignal = registerSignal("outOfOrderPk");
    dropPkSignal = registerSignal("dropPk");
}

IPvXAddress UDPBurstAndBroadcast::chooseDestAddr()
{
    if (destAddresses.size() == 1)
        return destAddresses[0];

    int k = genk_intrand(destAddrRNG, destAddresses.size());
    return destAddresses[k];
}


cPacket *UDPBurstAndBroadcast::createPacket()
{
    char msgName[32];
    sprintf(msgName, "UDPAppData-%d", counter++);
    long msgByteLength = messageLengthPar->longValue();
    cPacket *payload = new cPacket(msgName);
    payload->setByteLength(msgByteLength);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;

    return payload;
}



AppControlMessage *UDPBurstAndBroadcast::createPacket2()
{
    char msgName[32];
    sprintf(msgName, "UDPBroadcast-%d", counter++);
    //long msgByteLength = messageLengthPar->longValue();
    AppControlMessage *payload = new AppControlMessage(msgName);
    payload->setCntrlType(FIND_CONTROL_UNIT);
    //std::string interests = "\x83\x2\x0";
    //payload->setInterests(interests.c_str());
    payload->setInterests("\x83\x2\x0");
    payload->setSourceData("");
    payload->setByteLength(3);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent;

    return payload;
}

void UDPBurstAndBroadcast::handleMessage(cMessage *msg)
{
    BroadcastExpiryIterator i = mBroadcastExpiries.find(msg);
    if (i != mBroadcastExpiries.end() )
    {
        mBTT.erase(mBroadcastExpiries[msg]);
        mBroadcastExpiries.erase(msg);
        delete msg;
        this->getParentModule()->bubble("Erase BTR");
        return;
    }

    if (msg->getArrivalGateId() == mUpperLayerIn)
    {
        DataCentricAppPkt* appPkt = check_and_cast<DataCentricAppPkt *>(msg);
        handleUpperLayerMessage(appPkt);
    }
    else
    {
        if (msg->isSelfMessage())
        {
            //if (stopTime <= 0 || simTime() < stopTime)
            //{
                // send and reschedule next sending
                //if (isSource) // if the node is a sink, don't generate messages
                //    generateBurst();
            //}
        }
        else if (msg->getKind() == UDP_I_DATA)
        {
            // process incoming packet
            handlePacket(PK(msg));
        }
        else if (msg->getKind() == UDP_I_ERROR)
        {
            EV << "Ignoring UDP error report\n";
            delete msg;
        }
        else
        {
            error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
        }

    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
        getDisplayString().setTagArg("t", 0, buf);
    }
}





void UDPBurstAndBroadcast::handleUpperLayerMessage(DataCentricAppPkt* appPkt)
{

    string sourceData;
    string sinkData;
    string theData;
    switch ( appPkt->getKind() )
    {
    case DATA_PACKET:
        /*
         * If PUB Data
         *      Send to server
         *
         * If RECORD Data
         *      Send to server
         *
         *
         */
        //currentPktCreationTime = simTime();
        //cout << endl << "DATA SENT ORIG CREATE TIME:     " << currentPktCreationTime << endl;
        //SendDataWithLongestContext(appPkt);

        theData.resize(appPkt->getPktData().size(), 0);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), theData.begin());
        generatePacket(mServerAddr, HOME_ENERGY_DATA, "", theData.c_str());
        break;
    case STARTUP_MESSAGE:
        /*
         * Clearly datacentric farmework initialisation is not appropriate
         * we are using AODV
         *
         * However in the datacentric case the DataCentricNetworkLayer is
         * the equivalent to this module and it disables the PhyModule at
         * initialisation and enables it when the STARTUP_MESSAGE is received
         */
        //StartUpModule();
        //generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "");
        //mNetMan->updateControlPacketData(255, false);
        break;
    case CONTEXT_MESSAGE:
        /*
         * Not sure
         *
         *
         */
        //SetContext(appPkt);
        break;
    case SOURCE_MESSAGE:
        /*
         * If source for RECORD
         *      Find server
         *      Register with server?
         *
         * If source for PUB
         *      Find server
         *      Register with server?
         *
         */
        //SetSourceWithLongestContext(appPkt);

        // TEMP COMMENT OUT
        //generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "");
        //mNetMan->updateControlPacketData(255, false);

        //sourceData.resize(appPkt->getPktData().size(), 0);
        //std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sourceData.begin());
        //generatePacket(mServerAddr, REGISTER_AS_SOURCE, "", sourceData.c_str());
        //mNetMan->updateControlPacketData(255, false);
        break;
    case SINK_MESSAGE:
        /*
         * If sink for PUB
         *      Find server
         *      Register for the particular events
         *
         *
         */
        //SetSinkWithShortestContext(appPkt);

        // TEMP COMMENT OUT
        //generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "");
        //mNetMan->updateControlPacketData(255, false);

        sinkData.resize(appPkt->getPktData().size(), 0);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());
        if ( mIsControlUnit )
        {
            mInterestedNodes[myAddr] = sinkData;
        }
        else
        {
            generatePacket(mServerAddr, REGISTER_AS_SINK, sinkData.c_str(), "");
            mNetMan->updateControlPacketData(255, false);
        }
        break;
    default:
        break;
    }



}






void UDPBurstAndBroadcast::handlePacket(cPacket *pk)
{
    if (pk->getKind() == UDP_I_ERROR)
    {
        EV << "UDP error received\n";
        delete pk;
        return;
    }

    // Get source and message Id
    int moduleId;
    int msgId;
    if ( pk->hasPar("sourceId") && pk->hasPar("msgId") )
    {
        moduleId = (int)pk->par("sourceId");
        msgId = (int)pk->par("msgId");

    }
    else
    {
        return;
    }

    // If broadcast consider forwarding
    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication*>(pk->getControlInfo());
    IPvXAddress destAddr = udpCtrl->getDestAddr();
    if ( destAddr.get4().isLimitedBroadcastAddress() )
    {
        uint64 moduleId64 = moduleId;
        uint64 msgId64 = msgId;
        uint64 bcast = (moduleId64 << 32) | msgId64;

        if ( mBTT.find(bcast) != mBTT.end() )
        {
            // What does Zigbee broadcast mechanism do if we never discover
            // that the broadcast was relayed successfully ?????????????
            mBTT[bcast].relayed = true;
        }
        else
        {
            BTR btr;
            btr.expiry = simTime()+1.5;
            btr.relayed = false;
            mBTT[bcast] = btr;
            this->getParentModule()->bubble("Forwarding broadcast");
            forwardBroadcast(pk);
            cMessage* m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);
            ProcessPacket(dynamic_cast<AppControlMessage*>(pk));
        }
        return;
    }

    // now deal with packet itself
    if ( dynamic_cast<AppControlMessage*>(pk) )
    {
        ProcessPacket(dynamic_cast<AppControlMessage*>(pk));
    }

    // STILL ISSUES WITH DELETING PK  -  POORLY STRUCTURED CODE!!
    /*
    if ( !forwarded )
    {
        if ( !duplicate(moduleId, msgId, pk) )
        {
            if ( dataConsumed )
            {
                pktDelay->collect(simTime() - pk->getTimestamp());
                numReceived++;

                simtime_t e2eDelay = simTime() - pk->getTimestamp();
                e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
                mNetMan->addADataPacketE2EDelay(e2eDelay);

                this->getParentModule()->bubble("Received data packet");

                EV << "Received data packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
                emit(rcvdPkSignal, pk);
            }

            delete pk;
        }

    }
    */







        /*
        else
        {
            // duplicate control
            SourceSequence::iterator it = sourceSequence.find(moduleId);
            if (it != sourceSequence.end())
            {
                if (it->second >= msgId)
                {
                    EV << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
                    emit(outOfOrderPkSignal, pk);
                    delete pk;
                    numDuplicated++;
                    return;
                }
                else
                    it->second = msgId;
            }
            else
                sourceSequence[moduleId] = msgId;

            if (delayLimit > 0)
            {
                if (simTime() - pk->getTimestamp() > delayLimit)
                {
                    EV << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
                    emit(dropPkSignal, pk);
                    delete pk;
                    numDeleted++;
                    return;
                }
            }
            pktDelay->collect(simTime() - pk->getTimestamp());
            numReceived++;

            simtime_t e2eDelay = simTime() - pk->getTimestamp();
            e2eDelayVec.record(SIMTIME_DBL(e2eDelay));

            this->getParentModule()->bubble("Received packet");
        }
        */


}




bool UDPBurstAndBroadcast::duplicate(int moduleId, int msgId, cPacket *pk)
{
    SourceSequence::iterator it = sourceSequence.find(moduleId);
    if (it != sourceSequence.end())
    {
        if (it->second >= msgId)
        {
            EV << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(outOfOrderPkSignal, pk);
            delete pk;
            numDuplicated++;
            return true;
        }
        else
        {
            it->second = msgId;

        }
    }
    else
    {
        sourceSequence[moduleId] = msgId;
    }

    return false;

    /*
    if (delayLimit > 0)
    {
        if (simTime() - pk->getTimestamp() > delayLimit)
        {
            EV << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(dropPkSignal, pk);
            delete pk;
            numDeleted++;
            return;
        }
    }
    pktDelay->collect(simTime() - pk->getTimestamp());
    numReceived++;

    simtime_t e2eDelay = simTime() - pk->getTimestamp();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));

    this->getParentModule()->bubble("Received packet");
    */




}



void UDPBurstAndBroadcast::sendPacket(cPacket *payload, const IPvXAddress &_destAddr)
{
    if (!outputInterfaceMulticastBroadcast.empty() && (_destAddr.isMulticast() || (!_destAddr.isIPv6() && _destAddr.get4() == IPv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i< outputInterfaceMulticastBroadcast.size(); i++)
        {
            if (outputInterfaceMulticastBroadcast.size()-i > 1)
            {
                socket.sendTo(payload->dup(), _destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
            }
            else
            {
                socket.sendTo(payload, _destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
            }
        }
    }
    else
    {
        socket.sendTo(payload, _destAddr, destPort,outputInterface);
    }

}




void UDPBurstAndBroadcast::generatePacket(IPvXAddress &_destAddr, int _cntrlType, const char * _interests, const char * _sourceData)
{
    AppControlMessage *payload = createPacket2();
    payload->setTimestamp();
    payload->setCntrlType(_cntrlType);
    payload->setInterests(_interests);
    payload->setSourceData(_sourceData);
    emit(sentPkSignal, payload);

    if ( _destAddr.isUnspecified() )
    {
        mPktsForServer.push_back(payload);
        return;
    }

    //socket.sendTo(payload, _destAddr, destPort, outputInterface);
    sendPacket(payload, _destAddr);

    numSent++;

    if ( _destAddr.get4().isLimitedBroadcastAddress() )
    {
        uint64 moduleId64 = (int)payload->par("sourceId");
        uint64 msgId64 = (int)payload->par("msgId");
        uint64 bcast = (moduleId64 << 32) | msgId64;
        if ( mBTT.find(bcast) == mBTT.end() )
        {
            BTR btr;
            btr.expiry = simTime()+1.5;
            btr.relayed = false;
            mBTT[bcast] = btr;
            cMessage* m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);
        }
    }

}





void UDPBurstAndBroadcast::generateBurst()
{
    simtime_t now = simTime();

    if (nextPkt < now)
        nextPkt = now;

    double sendInterval = sendIntervalPar->doubleValue();
    if (sendInterval <= 0.0)
        throw cRuntimeError("The sendInterval parameter must be bigger than 0");
    nextPkt += sendInterval;

    if (activeBurst && nextBurst <= now) // new burst
    {
        double burstDuration = burstDurationPar->doubleValue();
        if (burstDuration < 0.0)
            throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
        double sleepDuration = sleepDurationPar->doubleValue();

        if (burstDuration == 0.0)
            activeBurst = false;
        else
        {
            if (sleepDuration < 0.0)
                throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
            nextSleep = now + burstDuration;
            nextBurst = nextSleep + sleepDuration;
        }

        if (chooseDestAddrMode == PER_BURST)
            destAddr = chooseDestAddr();
    }

    if (chooseDestAddrMode == PER_SEND)
        destAddr = chooseDestAddr();

    cPacket *payload = createPacket();
    payload->setTimestamp();
    emit(sentPkSignal, payload);

    // Check address type
    if (!outputInterfaceMulticastBroadcast.empty() && (destAddr.isMulticast() || (!destAddr.isIPv6() && destAddr.get4() == IPv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i< outputInterfaceMulticastBroadcast.size(); i++)
        {
            if (outputInterfaceMulticastBroadcast.size()-i > 1)
                socket.sendTo(payload->dup(), destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
            else
                socket.sendTo(payload, destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
        }
    }
    else
        socket.sendTo(payload, destAddr, destPort,outputInterface);

    numSent++;

    if ( destAddr.get4().isLimitedBroadcastAddress() )
    {
        uint64 moduleId64 = (int)payload->par("sourceId");
        uint64 msgId64 = (int)payload->par("msgId");
        uint64 bcast = (moduleId64 << 32) | msgId64;
        if ( mBTT.find(bcast) == mBTT.end() )
        {
            BTR btr;
            btr.expiry = simTime()+1.5;
            btr.relayed = false;
            mBTT[bcast] = btr;
            cMessage* m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);
        }
    }

    // Next timer
    if (activeBurst && nextPkt >= nextSleep)
        nextPkt = nextBurst;

    scheduleAt(nextPkt, timerNext);
}

void UDPBurstAndBroadcast::finish()
{
    recordScalar("Total sent", numSent);
    recordScalar("Total received", numReceived);
    recordScalar("Total deleted", numDeleted);
    recordScalar("Mean delay", pktDelay->getMean());
    recordScalar("Min delay", pktDelay->getMin());
    recordScalar("Max delay", pktDelay->getMax());
    recordScalar("Deviation delay", pktDelay->getStddev());
}



//void UDPBurstAndBroadcast::forwardBroadcast(int moduleId, int msgId)
//void UDPBurstAndBroadcast::forwardBroadcast(int moduleId, int msgId, const char* name, long bytelen)
void UDPBurstAndBroadcast::forwardBroadcast(cPacket* _payload)
{
    //char msgName[32];
    //sprintf(msgName, "UDPBasicAppData-%d", counter++);
    //long msgByteLength = messageLengthPar->longValue();

    //cPacket *payload = new cPacket(msgName);
    //payload->setByteLength(msgByteLength);
    //payload->addPar("sourceId") = moduleId;
    //payload->addPar("msgId") = msgId;
    //payload->setTimestamp(); // this was wrong any way

    cPacket* payload = _payload->dup();
    //AppControlMessage* payload = dynamic_cast<AppControlMessage*>(_payload->dup());
    //int moduleId = (int)payload->par("sourceId");
    //int msgId = (int)payload->par("msgId");
    //int ct = payload->getCntrlType();
    //std::string interests = payload->getInterests();
    //std::string sources = payload->getSourceData();
    //int len = payload->getByteLength();

    destAddr = 0xFFFFFFFF;

    // Check address type
    /*
    if (!outputInterfaceMulticastBroadcast.empty() && (destAddr.isMulticast() || (!destAddr.isIPv6() && destAddr.get4() == IPv4Address::ALLONES_ADDRESS)))
    {
        for (unsigned int i = 0; i< outputInterfaceMulticastBroadcast.size(); i++)
        {
            if (outputInterfaceMulticastBroadcast.size()-i > 1)
                socket.sendTo(payload->dup(), destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
            else
                socket.sendTo(payload, destAddr, destPort,outputInterfaceMulticastBroadcast[i]);
        }
    }
    else
    */

    //socket.sendTo(payload, destAddr, destPort,outputInterface);
    //socket.sendTo(payload, mBcastAddr, destPort,outputInterface);
    sendPacket(payload, mBcastAddr);
    mNetMan->updateControlPacketData(255, false);

}


void UDPBurstAndBroadcast::DataReceived(cPacket *pk)
{
    pktDelay->collect(simTime() - pk->getTimestamp());
    numReceived++;

    simtime_t e2eDelay = simTime() - pk->getTimestamp();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    mNetMan->addADataPacketE2EDelay(e2eDelay);

    this->getParentModule()->bubble("Received data packet");

    EV << "Received data packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
}


void UDPBurstAndBroadcast::ProcessPacket(cPacket *pk)
{
    if ( !dynamic_cast<AppControlMessage*>(pk) )
        return;


    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication*>(pk->getControlInfo());
    IPvXAddress destAddr = udpCtrl->getDestAddr();

    IPvXAddress origAddr;
    //origAddr = udpCtrl->getSrcAddr();
    int moduleId = (int)pk->par("sourceId");
    string srcFP = cSimulation::getActiveSimulation()->getModule(moduleId)->getParentModule()->getFullPath();
    origAddr = IPvXAddressResolver().resolve(srcFP.c_str());

    AppControlMessage* acm = dynamic_cast<AppControlMessage*>(pk);
    std::string sd;
    switch ( acm->getCntrlType() )
    {
        case FIND_CONTROL_UNIT:
            if ( mIsControlUnit )
            {
                generatePacket(origAddr, CONTROL_UNIT_DETAILS, getParentModule()->getFullPath().c_str(), "");
                mNetMan->updateControlPacketData(255, false);
            }
            break;
        case CONTROL_UNIT_DETAILS:
            mServerAddr = IPvXAddressResolver().resolve(acm->getInterests());
            for (std::vector<AppControlMessage*>::iterator i = mPktsForServer.begin();
                    i != mPktsForServer.end(); ++i)
            {
                //socket.sendTo(*i, mServerAddr, destPort, outputInterface);
                sendPacket(*i, mServerAddr);
                numSent++;
            }
            break;
        case REGISTER_AS_SOURCE:
            break;
        case REGISTER_AS_SINK:
            mInterestedNodes[origAddr] = acm->getInterests();
            break;
        case HOME_ENERGY_DATA:
            if ( mIsControlUnit )
            {
                sd = acm->getSourceData();
                for (std::map<IPvXAddress, std::string>::iterator i = mInterestedNodes.begin();
                        i != mInterestedNodes.end(); ++i)
                {
                    // this prefixing matching mechanism
                    // does not take into account data and context portions
                    // of naming scheme!!!
                    pair<std::string::iterator,std::string::iterator> res;
                    res = std::mismatch(i->second.begin(), i->second.end(), sd.begin());
                    if (res.first == i->second.end())
                    {
                        // i->second is a prefix of sd.
                        if ( i->first == myAddr)
                        {
                            DataReceived(pk);
                        }
                        else
                        {
                            socket.sendTo(acm->dup(), i->first, destPort, outputInterface);
                            sendPacket(acm->dup(), i->first);
                            numSent++;
                        }
                    }

                }

            }
            else
            {
                DataReceived(pk);
            }

            //std::string foo("foo");
            //std::string foobar("foobar");
            //auto res = std::mismatch(foo.begin(), foo.end(), foobar.begin());
            //if (res.first == foo.end())
            //{
                // foo is a prefix of foobar.
            //}
            break;
        default:
            break;
    }

    delete pk;

    // ignore duplicates for the moment
    //if ( !duplicate(moduleId, msgId, pk) )
    //{
    //}


}
