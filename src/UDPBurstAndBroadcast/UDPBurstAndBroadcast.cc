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

static void cb_record_RREQstats(double stat);
static void cb_record_RReplystats(double stat);
static void cb_record_RReplyCompletion(uint32 _originator, uint32 _destination);
static void cb_record_RREQInitiation(uint32 _originator, uint32 _destination);
static void cb_record_Datastats(unsigned char type, double stat);
DataCentricNetworkMan* netMan;


void cb_record_RREQstats(double stat)
{
    netMan->updateControlPacketData(RREQ_STAT, false);
}

void cb_record_RReplystats(double stat)
{
    netMan->updateControlPacketData(RREPLY_STAT, false);
}

void cb_record_RReplyCompletion(uint32 _originator, uint32 _destination)
{
    //netMan->updateControlPacketData(RREPLY_STAT, false);
    netMan->removePendingRREQ(_originator, _destination);

}


void cb_record_RREQInitiation(uint32 _originator, uint32 _destination)
{
    //netMan->updateControlPacketData(RREPLY_STAT, false);
    netMan->addPendingRREQ(_originator, _destination);

}



void cb_record_Datastats(unsigned char type, double stat)
{
    netMan->updateControlPacketData(type, false);


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

    moduleRD.top_context = trie_new();
    mpUpDownMessage = new cMessage("UpDownMessage");
    scheduleAt(simTime() + 1800.0, mpUpDownMessage); // first check in half an hour


    cModule* app = this->getParentModule()->getSubmodule("app");
    bool excludeFromStability = false;
    excludeFromStability = !excludeFromStability ? strcmp(app->par("sinkFor").stringValue(), "") : true;
    excludeFromStability = !excludeFromStability ? strcmp(app->par("sourceFor").stringValue(), "") : true;
    excludeFromStability = !excludeFromStability ? strcmp(app->par("collaboratorInitiatorFor").stringValue(), "") : true;
    excludeFromStability = !excludeFromStability ? strcmp(app->par("collaboratorFor").stringValue(), "") : true;

    mStability = 0;
    if ( !excludeFromStability )
    {
        mStability         = par("Instability");
    }

    cModule* wlan = this->getParentModule()->getSubmodule("wlan");
    mQueueModule = check_and_cast<DropTailQueue*>(wlan->getSubmodule("ifq"));


    mPhyModule = check_and_cast<Ieee802154Phy*>(wlan->getSubmodule("phy"));
    mPhyModule->disableModule();

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

    setRecordRREQStatsCallBack(cb_record_RREQstats);
    setRecordRReplyStatsCallBack(cb_record_RReplystats);
    setRecordRReplyCompletionCallBack(cb_record_RReplyCompletion);
    setRecordRREQInitiationCallBack(cb_record_RREQInitiation);
    setRecordDataStatsCallBack(cb_record_Datastats);

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

    if (strcmp(par("controlUnit").stringValue(),"") != 0)
    {
        mServerAddr = IPvXAddressResolver().resolve(par("controlUnit").stringValue());
    }

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    myAddr = IPvXAddressResolver().resolve(this->getParentModule()->getFullPath().c_str());
    mNetMan->mAppModules[myAddr] = this;

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
    payload->addPar("msgId") = numSent++;

    return payload;
}

void UDPBurstAndBroadcast::handleMessage(cMessage *msg)
{
    if (msg == mpUpDownMessage )
    {
        if ( mPhyModule->isEnabled() )
        {
            // stability zero implements never
            // other implements a percentage chance
            int r = intuniform(1,100);
            if ( r <= mStability  )
            {
                cout << "MODULE GOING DOWN: " << this->getParentModule()->getFullName() << endl;
                //TraversInterfaceNodes(rd->interfaceTree, 0, cb_printNeighbour);

                mPhyModule->disableModule();
                mNetMan->changeInModulesDown(1.0);

                //mQueueModule->dropAll();
                mQueueModule->clear();

                scheduleAt(simTime() + 1800.0, mpUpDownMessage); // down for half an hour
            }
            else
            {
                scheduleAt(simTime() + 1800.0, mpUpDownMessage); // check again in half an hour
            }
        }
        else
        {
            cout << "MODULE COMING UP: " << this->getParentModule()->getFullName() << endl;

            mPhyModule->enableModule();
            //StabilityVector.record(0.0);
            mNetMan->changeInModulesDown(-1.0);

            mQueueModule->requestPacket(); // reprime the previously cleared nic queue
            scheduleAt(simTime() + 1800.0, mpUpDownMessage); // check again in half an hour
        }

        return;
    }

    SendLaterMessageMapIterator laterIt = mSendLaterMessageMap.find(msg);
    if (laterIt != mSendLaterMessageMap.end() )
    {
        IPvXAddress addr = mSendLaterMessageMap[msg].destAddr;
        cPacket *pkt = mSendLaterMessageMap[msg].pkt;
        sendPacket(pkt, addr);
        return;
    }

    BroadcastRetryIterator retryIt = mBroadcastRetries.find(msg);
    if (retryIt != mBroadcastRetries.end() )
    {
        if ( !mBTT[mBroadcastRetries[msg]].relayed && mBTT[mBroadcastRetries[msg]].retries < 2 )
        {
            //forwardBroadcast(mBTT[mBroadcastRetries[msg]].pkt);
            //mBTT[mBroadcastRetries[msg]].pkt = mBTT[mBroadcastRetries[msg]].pkt->dup();
            mBTT[mBroadcastRetries[msg]].retries++;
            this->getParentModule()->bubble("Broadcast retry");
        }
        else
        {
            mBroadcastRetries.erase(msg);
            delete msg;
            this->getParentModule()->bubble("Erase broadcast retry");
        }
        return;
    }

    BroadcastExpiryIterator expiryIt = mBroadcastExpiries.find(msg);
    if (expiryIt != mBroadcastExpiries.end() )
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
    string context;
    string theData;
    unsigned char temp[30];
    unsigned char x[20];
    unsigned char* index = x;

    rd = &(moduleRD);
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

        // MOVE THIS BIT INTO FRAMEWORK
        //getLongestContextTrie(rd->top_context, temp, temp, index);
        getLongestContextTrie(rd->top_context, temp, temp, x);
        //context = x;

        std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending DATA to: " << mServerAddr.get4() << std::endl;
        generatePacket(mServerAddr, HOME_ENERGY_DATA, "", theData.c_str(), (const char*)x, 0);
        mNetMan->addPendingDataPkt();
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
        mPhyModule->enableModule();
        //generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "");
        //mNetMan->updateControlPacketData(DISCOVERY_STAT, false);
        break;
    case CONTEXT_MESSAGE:
        /*
         * Not sure
         *
         *
         */
        SetContext(appPkt);
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

        if ( mServerAddr.isUnspecified() )
        {
            generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "", "", 0);
            mNetMan->updateControlPacketData(DISCOVERY_STAT, false);
        }

        //sourceData.resize(appPkt->getPktData().size(), 0);
        //std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sourceData.begin());
        //generatePacket(mServerAddr, REGISTER_AS_SOURCE, "", sourceData.c_str());
        //mNetMan->updateControlPacketData(REGISTER_STAT, false);
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
        if ( mServerAddr.isUnspecified() )
        {
            generatePacket(mBcastAddr, FIND_CONTROL_UNIT, "", "", "", 0);
            mNetMan->updateControlPacketData(DISCOVERY_STAT, false);
        }

        sinkData.resize(appPkt->getPktData().size(), 0);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());

        // MOVE THIS BIT INTO FRAMEWORK
        //getLongestContextTrie(rd->top_context, temp, temp, index);
        getLongestContextTrie(rd->top_context, temp, temp, x);
        //context = x;




        if ( mIsControlUnit )
        {
            mInterestedNodes[myAddr].interest = sinkData;
            mInterestedNodes[myAddr].context = (const char*)x;
        }
        else
        {
            std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending REGISTER_AS_SINK to: " << mServerAddr.get4() << std::endl;
            generatePacket(mServerAddr, REGISTER_AS_SINK, sinkData.c_str(), "", (const char*)x, 0);
            //IPv4Address pending = myAddr.get4();
            mNetMan->addPendingRegistration(myAddr.get4().getInt());
            mNetMan->updateControlPacketData(REGISTER_STAT, false);
        }
        break;
    default:
        break;
    }



}




void UDPBurstAndBroadcast::SetContext(DataCentricAppPkt* appPkt)
{
    string contextData;
    contextData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), contextData.begin());
    //int size = appPkt->getPktData().size();
    //size = contextData.size();
    unsigned char x[30];
    unsigned int contextLen = strlen(contextData.c_str());
    memcpy(x, contextData.c_str(), contextLen);
    x[contextLen] = 0;
    trie_add(rd->top_context, x, CONTEXT);
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
            cMessage* m;
            BTR btr;
            btr.expiry = simTime()+1.5;
            btr.relayed = false;
            btr.pkt = pk->dup();
            btr.retries = 0;
            mBTT[bcast] = btr;
            this->getParentModule()->bubble("Forwarding broadcast");
            forwardBroadcast(pk);

            m = new cMessage("");
            mBroadcastRetries[m] = bcast;
            scheduleAt(simTime()+0.25, m);

            m = new cMessage("");
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





void UDPBurstAndBroadcast::sendDownTheNIC()
{
    Enter_Method("sendDownTheNIC()");

    return;

    /*

    double simtimelimit;
    //simtimelimit = cSimulation::getActiveEnvir()->getConfig()->getAsDouble(CFGID_SIM_TIME_LIMIT);

    std::stringstream ss(ev.getConfig()->getConfigValue("sim-time-limit"));
    ss >> simtimelimit;


    double randomStability = uniform(1,255);
    if ( randomStability <= mStability  )
    {
        if ( mPhyModule->isEnabled() )
        {
            //cout << "MODULE GOING DOWN: " << fName << endl;
            //TraversInterfaceNodes(rd->interfaceTree, 0, cb_printNeighbour);

            mPhyModule->disableModule();
            mNetMan->changeInModulesDown(1.0);

            // IMPORTANT NEED TO DO SOMETHING DIFFERENT HERE
            // SINCE CHANGES IN INETMANET INTEGRATION BRANCH
            // FOR DropTailQueue
            //mQueueModule->dropAll();
            mQueueModule->clear();



            std::string s;
            std::ostringstream ss;
            ss.clear();
            ss.str(s);
            ss << ".\\" << std::hex << std::uppercase << thisAddress << "Connections.txt";
            std::remove(ss.str().c_str());

            StabilityVector.record(1.0);
            scheduleAt(simTime() + 2.0, mpUpDownMessage);
        }
    }



    return;
    */


}





void UDPBurstAndBroadcast::sendPacket(cPacket *payload, const IPvXAddress &_destAddr)
{
    /*
    if ( dynamic_cast<AppControlMessage*>(payload) )
    {
        AppControlMessage* appPkt = dynamic_cast<AppControlMessage*>(payload);
        if ( appPkt->getCntrlType() ==  HOME_ENERGY_DATA )
        {
            mNetMan->mAppModules[_destAddr]->sendDownTheNIC();
        }
    }
    */



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



void UDPBurstAndBroadcast::generatePacket(IPvXAddress &_destAddr, int _cntrlType, const char * _interests, const char * _sourceData, const char * _context, double _delay)
{
    AppControlMessage *payload = createPacket2();
    payload->setTimestamp();
    payload->setCntrlType(_cntrlType);
    payload->setInterests(_interests);
    payload->setSourceData(_sourceData);
    payload->setContext(_context);

    if ( _destAddr.isUnspecified() )
    {
        mPktsForServer.push_back(payload);
        return;
    }

    if ( _destAddr.get4().isLimitedBroadcastAddress() )
    {
        uint64 moduleId64 = (int)payload->par("sourceId");
        uint64 msgId64 = (int)payload->par("msgId");
        uint64 bcast = (moduleId64 << 32) | msgId64;
        if ( mBTT.find(bcast) == mBTT.end() )
        {
            cMessage* m;
            BTR btr;
            btr.expiry = simTime()+_delay+1.5;
            btr.relayed = false;
            btr.pkt = payload->dup();
            btr.retries = 0;
            mBTT[bcast] = btr;

            m = new cMessage("");
            mBroadcastRetries[m] = bcast;
            scheduleAt(simTime()+_delay+0.25, m);

            m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);
        }

    }

    if ( _delay )
    {
        SendLater sendLater;
        sendLater.destAddr = _destAddr;
        sendLater.pkt = payload;
        cMessage* m = new cMessage("");
        mSendLaterMessageMap[m] = sendLater;
        scheduleAt(simTime()+_delay, m);
    }
    else
    {
        sendPacket(payload, _destAddr);
        emit(sentPkSignal, payload);
        //numSent++;
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

    //numSent++;

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

    // Additional scalar recording for scatter charts in multi simulation runs
    recordScalar("FailedRREQs", (double)mNetMan->numPendingRREQs());
    recordScalar("FailedRegistrations", (double)mNetMan->numPendingRegistrations());
    recordScalar("LinkFailures", (double)mNetMan->numAODVAllLineBreaksValue());
    recordScalar("DataArrivals", (double)mNetMan->numAODVDataArrivalValue());





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

    //destAddr = 0xFFFFFFFF;
    // this assignment operator appears to have been depracated
    destAddr.set("255.255.255.255");

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
    mNetMan->updateControlPacketData(DISCOVERY_STAT, false);

}


void UDPBurstAndBroadcast::DataReceived(cPacket *pk)
{
    pktDelay->collect(simTime() - pk->getTimestamp());
    numReceived++;

    simtime_t e2eDelay = simTime() - pk->getTimestamp();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    mNetMan->addADataPacketE2EDelay(e2eDelay);
    mNetMan->updateControlPacketData(AODV_DATA_ARRIVAL, false);

    this->getParentModule()->bubble("Received data packet");

    EV << "Received data packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
}


void UDPBurstAndBroadcast::ProcessPacket(cPacket *pk)
{
    if ( !dynamic_cast<AppControlMessage*>(pk) )
        return;

    double currentTime = simTime().dbl();

    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication*>(pk->getControlInfo());
    IPvXAddress destAddr = udpCtrl->getDestAddr();

    IPvXAddress origAddr;
    //origAddr = udpCtrl->getSrcAddr();
    int moduleId = (int)pk->par("sourceId");
    string srcFP = cSimulation::getActiveSimulation()->getModule(moduleId)->getParentModule()->getFullPath();
    origAddr = IPvXAddressResolver().resolve(srcFP.c_str());

    AppControlMessage* acm = dynamic_cast<AppControlMessage*>(pk);
    std::string sd;
    std::string con;
    switch ( acm->getCntrlType() )
    {
        case REGISTER_AS_SINK_CONFIRMATION:
            std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                    << ", Received reg conf from: " << origAddr.get4() << std::endl;
            break;
        case FIND_CONTROL_UNIT:
            if ( mIsControlUnit )
            {
                generatePacket(origAddr, CONTROL_UNIT_DETAILS, getParentModule()->getFullPath().c_str(), "", "", 0.2);
                mNetMan->updateControlPacketData(DISCOVERY_STAT, false);
            }
            break;
        case CONTROL_UNIT_DETAILS:
            mServerAddr = IPvXAddressResolver().resolve(acm->getInterests());
            for (std::vector<AppControlMessage*>::iterator i = mPktsForServer.begin();
                    i != mPktsForServer.end(); ++i)
            {
                //socket.sendTo(*i, mServerAddr, destPort, outputInterface);
                int ctrlT = (*i)->getCntrlType();
                sendPacket(*i, mServerAddr);
                //numSent++;
            }
            break;
        case REGISTER_AS_SOURCE:
            break;
        case REGISTER_AS_SINK:
            mInterestedNodes[origAddr].interest = acm->getInterests();
            mInterestedNodes[origAddr].context = acm->getContext();
            //origAddr.get4().getInt()
            //IPv4Address pending(origAddr.get4());
            mNetMan->removePendingRegistration(origAddr.get4().getInt());

            std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                    << ", Adding reg for: " << origAddr.get4() << std::endl;

            // Send a confirmation to show success, but mainly to get a route
            // during a manual action so known to probably be separated from
            // other traffic.
            if ( par("confirmRegistration").boolValue() )
            {
                std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                        << ", Sending REGISTER_AS_SINK_CONFIRMATION to: " << origAddr.get4() << std::endl;
                generatePacket(origAddr, REGISTER_AS_SINK_CONFIRMATION, acm->getInterests(), "", "", 0);
            }
            break;
        case HOME_ENERGY_DATA:
            mNetMan->removePendingDataPkt();
            if ( mIsControlUnit )
            {
                sd = acm->getSourceData();
                con = acm->getContext();
                for (std::map<IPvXAddress, TheInterest>::iterator i = mInterestedNodes.begin();
                        i != mInterestedNodes.end(); ++i)
                {
                    pair<std::string::iterator,std::string::iterator> res;
                    res = std::mismatch(i->second.interest.begin(), i->second.interest.end(), sd.begin());
                    if (res.first == i->second.interest.end())
                    {
                        res = std::mismatch(i->second.context.begin(), i->second.context.end(), con.begin());
                        if (res.first == i->second.context.end())
                        {
                            // both data and context of incoming packet are prefix
                            // of this node's interest
                            if ( i->first == myAddr)
                            {
                                DataReceived(pk);
                            }
                            else
                            {
                                //socket.sendTo(acm->dup(), i->first, destPort, outputInterface);
                                // THINK THE ABOVE IS OLD AND SHOULD HAVE BEEN REMOVED
                                mNetMan->addPendingDataPkt();
                                sendPacket(acm->dup(), i->first);
                                std::cout << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                                        << ", Forwarding to: " << i->first.get4() << std::endl;
                                //numSent++;
                            }
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
