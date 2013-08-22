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
#include "DataCentricNetworkLayer.h"

#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "BroadcastMessage_m.h"
#include "DataCentricNetworkLayer.h"
#include "aodv_uu_omnet.h"
//#include "ChannelControl.h"

#include "SpecialDebug.h"

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

//static void cb_record_RREQstats(double stat);
//static void cb_record_RReplystats(double stat);
static void cb_record_RReplyCompletion(uint32 _originator, uint32 _destination);
static void cb_record_RREQInitiation(uint32 _originator, uint32 _destination);
static void cb_record_ProactiveRoute(uint32 _originator);
static void cb_record_Datastats(unsigned char type, double stat);
//void cb_collect_AirFrame(cPacket* p);

DataCentricNetworkMan* netMan;

/*
void cb_record_RREQstats(double stat)
{
    netMan->recordOnePacket(RREQ_STAT);
}

void cb_record_RReplystats(double stat)
{
    netMan->recordOnePacket(RREPLY_STAT);
}


void cb_collect_AirFrame(cPacket* p)
{
    string name = p->getName();
    unsigned int bitLen = p->getBitLength();

    netMan->collectMsgBits(p->getBitLength(), p);


}
*/


void cb_record_RReplyCompletion(uint32 _originator, uint32 _destination)
{
    netMan->removePendingRREQ(_originator, _destination);

}


void cb_record_RREQInitiation(uint32 _originator, uint32 _destination)
{
    netMan->addPendingRREQ(_originator, _destination);

}


void cb_record_ProactiveRoute(uint32 _originator)
{
    if ( _originator )
    {
        netMan->addProactiveRREQ(_originator);
    }
    else
    {
        netMan->clearProactiveRREQ();
    }

}





void cb_record_Datastats(unsigned char type, double stat)
{
    netMan->recordOnePacket(type);


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
    cancelAndDelete(mpUpDownMessage);

    delete pktDelay;
}

void UDPBurstAndBroadcast::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.

    //cModule* mr = this->getParentModule()->getSubmodule("manetrouting");
    //int ind = this->getParentModule()->getIndex();
    //double bcD1 = mr->par("broadcastDelay").doubleValue();
    //double bcD2 = mr->par("broadcastDelay").doubleValue();
    //double bcD3 = mr->par("broadcastDelay").doubleValue();


    if (stage != 4)
        return;

    mServiceDiscoveryTimeOut = par("ServiceDiscoveryTimeOut");
    mServiceDiscoveryNumTries = par("ServiceDiscoveryNumTries");
    mBindingTimeOut = par("BindingTimeOut");
    mBindingNumTries = par("BindingNumTries");
    mBindWithSource = par("bindWithSource");

    moduleRD.top_context = trie_new();
    mpUpDownMessage = 0;

    // comment out if not needed
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

    //setRecordRREQStatsCallBack(cb_record_RREQstats);
    //setRecordRReplyStatsCallBack(cb_record_RReplystats);
    setRecordRReplyCompletionCallBack(cb_record_RReplyCompletion);
    setRecordRREQInitiationCallBack(cb_record_RREQInitiation);
    setRecordProactiveRouteCallBack(cb_record_ProactiveRoute);
    setRecordDataStatsCallBack(cb_record_Datastats);
    //setCollectAirFrameCallBack(cb_collect_AirFrame);

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


cPacket *UDPBurstAndBroadcast::createPacket() // NOT USED
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



AppControlMessage *UDPBurstAndBroadcast::createPacket2(const char *name)
{
    //char msgName[32];
    //sprintf(msgName, "UDPBroadcast-%d", counter++);
    //long msgByteLength = messageLengthPar->longValue();
    //AppControlMessage *payload = new AppControlMessage(msgName);
    AppControlMessage *payload = new AppControlMessage(name);
    //payload->setCntrlType(FIND_CONTROL_UNIT);
    //std::string interests = "\x83\x2\x0";
    //payload->setInterests(interests.c_str());
    //payload->setInterests("\x83\x2\x0");
    //payload->setSourceData("");
    payload->setByteLength(3);
    payload->addPar("sourceId") = getId();
    payload->addPar("msgId") = numSent++;

    return payload;
}

void UDPBurstAndBroadcast::handleMessage(cMessage *msg)
{
    rd = &(moduleRD);

    double cTime = simTime().dbl();

    if (msg == mpUpDownMessage )
    {
        if ( mPhyModule->isEnabled() )
        {
            // stability zero implements never
            // other implements a percentage chance
            int r = intuniform(1,100);
            if ( r <= mStability  )
            {
                COUT << "MODULE GOING DOWN: " << this->getParentModule()->getFullName() << "\n";
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
            COUT << "MODULE COMING UP: " << this->getParentModule()->getFullName() << "\n";

            mPhyModule->enableModule();
            //StabilityVector.record(0.0);
            mNetMan->changeInModulesDown(-1.0);

            mQueueModule->requestPacket(); // reprime the previously cleared nic queue
            scheduleAt(simTime() + 1800.0, mpUpDownMessage); // check again in half an hour
        }

        return;
    }


    ////////////// NEW CODE
    // Check if message is in mDiscoveryTries or mBindingTries
    if ( msg->isName("BindingRetry") )
    {
        AData d;
        d.data = msg->par("Data").stringValue();
        d.context = msg->par("Context").stringValue();
        IPvXAddress addr;
        addr.set(msg->par("Address").stringValue());
        TriesIter bindIt = mBindingTries.find(d);
        if (bindIt != mBindingTries.end() )
        {
            generatePacket(addr, "BindRequest", BINDING_REQUEST, d.data.c_str(), "",
                    d.context.c_str(), 0);
            bindIt->second++;
            if ( bindIt->second < mBindingNumTries )
            {
                scheduleAt(simTime()+mBindingTimeOut, msg);
                return;
            }
        }
        delete msg;
        return;

    }

    if ( msg->isName("DiscoveryRetry") )
    {
        AData d;
        d.data = msg->par("Data").stringValue();
        d.context = msg->par("Context").stringValue();
        TriesIter discoverIt = mDiscoveryTries.find(d);
        if (discoverIt != mDiscoveryTries.end() )
        {
            generatePacket(mBcastAddr, "ServiceDiscovery", SERVICE_DISCOVERY_REQUEST,
                    d.data.c_str(), "", d.context.c_str(), 0);
            discoverIt->second++;
            if ( discoverIt->second < mServiceDiscoveryNumTries )
            {
                scheduleAt(simTime()+mServiceDiscoveryTimeOut, msg);
                return;
            }
        }
        delete msg;
        return;
    }
    ///////////////////////


    SendLaterMessageMapIterator laterIt = mSendLaterMessageMap.find(msg);
    if (laterIt != mSendLaterMessageMap.end() )
    {
        IPvXAddress addr = mSendLaterMessageMap[msg].destAddr;
        cPacket *pkt = mSendLaterMessageMap[msg].pkt;
        sendPacket(pkt, addr);
        mSendLaterMessageMap.erase(msg);
        delete msg;
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
        handleUpperLayerMessage(appPkt); // message deleted in this method
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
            handlePacket(PK(msg)); // PK(msg) deleted in this method or its callees
        }
        else if (msg->getKind() == UDP_I_ERROR)
        {
            EV << "Ignoring UDP error report\n";
            delete msg;
        }
        else
        {
            error("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
            delete msg;
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
    //unsigned char* index = x;

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
        //COUT << "\n" << "DATA SENT ORIG CREATE TIME:     " << currentPktCreationTime << "\n";
        //SendDataWithLongestContext(appPkt);

        theData.resize(appPkt->getPktData().size(), 0);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), theData.begin());

        // MOVE THIS BIT INTO FRAMEWORK
        //getLongestContextTrie(rd->top_context, temp, temp, index);
        getLongestContextTrie(rd->top_context, temp, temp, x);
        //context = x;

        // MORE WORK - Mechanism to decide whether server based or direct to src binding based
        if ( mBindWithSource )
        {
            ////////////// NEW CODE
            // HEREHERE
            // find binding
            AData d;
            d.data = theData;
            d.context = (const char*)x;
            d.t = simulation.getSimTime();
            //d.t = 0; // IMPROVE ZERO PKT START HACK

            for (BindingListIter i = mBindingList.begin();
                    i != mBindingList.end(); ++i)
            {
                pair<std::string::iterator,std::string::iterator> res;
                std::string currentData = i->first.data; // i->first.data.begin() returns unwanted const_iter
                res = std::mismatch(currentData.begin(), currentData.end(), d.data.begin());
                if (res.first == currentData.end())
                {
                    std::string currentContext = i->first.context; // i->first.context.begin() returns unwanted const_iter
                    res = std::mismatch(currentContext.begin(), currentContext.end(), d.context.begin());
                    if (res.first == currentContext.end())
                    {
                        // both binding data and context are prefix of the
                        // data and context of the packet intended to be sent - CHECKED
                        if ( i->second.AddressList.empty() )
                        {
                            i->second.DataQueue.push_back(d);
                        }
                        else
                        {
                            for (std::vector<IPvXAddress>::iterator it = i->second.AddressList.begin();
                                    it != i->second.AddressList.end(); ++it)
                            {
                                COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending DATA to: " << (*it).get4() << "\n";
                                generatePacket(*it, "EnergyData", HOME_ENERGY_DATA, "",
                                        theData.c_str(), (const char*)x, 0);
                                // IMPROVE ZERO PKT START HACK
                                //COUT << "Time: 0 At: " << myAddr.get4() << ", Sending DATA to: " << (*it).get4() << "\n";
                                //generatePacket(0, *it, "EnergyData", HOME_ENERGY_DATA, "",
                                //        theData.c_str(), (const char*)x, 0);
                            }
                        }
                        break;
                    }
                }
            }








            /////////////////////////////////
        }
        else
        {
            COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending DATA to: " << mServerAddr.get4() << "\n";
            generatePacket(mServerAddr, "EnergyData", HOME_ENERGY_DATA, "",
                    theData.c_str(), (const char*)x, 0);
            // IMPROVE ZERO PKT START HACK
            //COUT << "Time: 0" << " At: " << myAddr.get4() << ", Sending DATA to: " << mServerAddr.get4() << "\n";
            //generatePacket(0, mServerAddr, "EnergyData", HOME_ENERGY_DATA, "",
            //        theData.c_str(), (const char*)x, 0);
            mNetMan->addPendingDataPkt();
        }
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
        if ( this->getParentModule()->getIndex() == 3 )
            cout << "Ind 3" << endl;
        //if ( simTime().dbl() > 0 )
        //    cout << "Index: " << this->getParentModule()->getIndex() << endl;
        mPhyModule->enableModule();
        //generatePacket(mBcastAddr, "ServiceDiscovery", FIND_CONTROL_UNIT, "", "");
        //mNetMan->recordOnePacket(DISCOVERY_STAT);
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
        if ( this->getParentModule()->getIndex() == 3 )
            cout << "Ind 3" << endl;

        // TEMP COMMENT OUT

        if ( mBindWithSource )
        {
            ////////////// NEW CODE
            // MORE WORK OR DONE?????????????
            sourceData.resize(appPkt->getPktData().size(), 0);
            std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sourceData.begin());
            getLongestContextTrie(rd->top_context, temp, temp, x);
            AData d;
            ABinding b;
            d.data = sourceData;
            d.context = (const char*)x;
            mBindingList[d] = b;
            ////////////////////////////////////
        }
        else
        {
            if ( mServerAddr.isUnspecified() )
            {
                generatePacket(mBcastAddr, "ServiceDiscovery", FIND_CONTROL_UNIT, "", "", "", 0);
                mNetMan->recordOnePacket(DISCOVERY_STAT);
            }
        }




        //sourceData.resize(appPkt->getPktData().size(), 0);
        //std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sourceData.begin());
        //generatePacket(mServerAddr, "BindRequest", REGISTER_AS_SOURCE, "", sourceData.c_str());
        //mNetMan->recordOnePacket(REGISTER_STAT);
        break;
    case SINK_MESSAGE:
        //if ( simTime().dbl() > 0 )
        //    cout << "Index: " << this->getParentModule()->getIndex() << endl;
        sinkData.resize(appPkt->getPktData().size(), 0);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());
        getLongestContextTrie(rd->top_context, temp, temp, x);

        if ( mBindWithSource )
        {
            ////////////// NEW CODE
            generatePacket(mBcastAddr, "ServiceDiscovery",


                    SERVICE_DISCOVERY_REQUEST, sinkData.c_str(), "", (const char*)x, 0);
            if ( mServiceDiscoveryNumTries > 1 )
            {
                AData d;
                d.data = sinkData.c_str();
                d.context = (const char*)x;
                mDiscoveryTries[d] = 1;
                cMessage* m = new cMessage("DiscoveryRetry");

                // One assignment method
                //m->addPar("Data") = sinkData.c_str();
                //m->addPar("Context") = (const char*)x;

                // Another assignment method
                //m->addPar("Data").setStringValue(sinkData.c_str());
                //m->addPar("Context").setStringValue((const char*)x);

                // Yet another assignment method
                m->addPar("Data").setStringValue(d.data.c_str());
                m->addPar("Context").setStringValue(d.context.c_str());

                //m->addPar("Data") = sinkData;
                //m->addPar("Context") = (const char*)x;
                //std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());


                // rewrite 1
                //d.data = m->par("Data").str();
                //d.context = m->par("Context").str();
                //d.t = 0; // place holder


                // rewrite 2
                //d.data = m->par("Data");
                //d.context = m->par("Context");
                //d.t = 0; // place holder

                // rewrite 3
                //std::copy(d.data.begin(), d.data.end(), m->par("Data").str().begin());
                //std::copy(d.context.begin(), d.context.end(), m->par("Context").str().begin());
                //d.t = 0; // place holder

                // rewrite 4
                //addr.set(msg->par("Address").stringValue());
                ///d.data = m->par("Data").stringValue();
                //d.context = m->par("Context").stringValue();
                //d.t = 0; // place holder

                //HERE
                scheduleAt(simTime()+mServiceDiscoveryTimeOut, m);
            }
            ////////////////////////////////////////////////////////////////
        }
        else
        {
            if ( mIsControlUnit )
            {
                mInterestedNodes[myAddr].interest = sinkData;
                mInterestedNodes[myAddr].context = (const char*)x;
            }
            else
            {
                if ( mServerAddr.isUnspecified() )
                {
                    generatePacket(mBcastAddr, "ServiceDiscovery", FIND_CONTROL_UNIT, "", "", "", 0);
                    mNetMan->recordOnePacket(DISCOVERY_STAT);
                }
                COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending REGISTER_AS_SINK to: " << mServerAddr.get4() << "\n";
                generatePacket(mServerAddr, "BindRequest", REGISTER_AS_SINK,
                        sinkData.c_str(), "", (const char*)x, 0);
                mNetMan->addPendingRegistration(myAddr.get4().getInt());
                mNetMan->recordOnePacket(REGISTER_STAT);
            }
        }
        break;
    default:
        break;
    }

    delete appPkt;
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
        delete pk;
        return;
    }

    // Broadcast - consider noting, forwarding or handling
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
            delete pk;
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
            forwardBroadcast(pk); // forwards a copy

            m = new cMessage("");
            mBroadcastRetries[m] = bcast;
            scheduleAt(simTime()+0.25, m);

            m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);

            ProcessPacket(dynamic_cast<AppControlMessage*>(pk)); // pk deleted by function
        }
        return;
    }

    // Unicast packet for us so handle if not already handled
    if ( dynamic_cast<AppControlMessage*>(pk) )
    {
        if ( !duplicate(moduleId, msgId, pk) )
        {
            ProcessPacket(dynamic_cast<AppControlMessage*>(pk)); // pk deleted by function
        }
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
            //EV << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            //emit(outOfOrderPkSignal, pk);
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
            //COUT << "MODULE GOING DOWN: " << fName << "\n";
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

void UDPBurstAndBroadcast::generatePacket(IPvXAddress &_destAddr, const char *name, int _cntrlType, const char * _interests, const char * _sourceData, const char * _context, double _delay)
{
    generatePacket(simulation.getSimTime(),
                                _destAddr,
                                name,
                                _cntrlType,
                                _interests,
                                _sourceData,
                                _context,
                                _delay);
}

void UDPBurstAndBroadcast::generatePacket(simtime_t t, IPvXAddress &_destAddr, const char *name, int _cntrlType, const char * _interests, const char * _sourceData, const char * _context, double _userdelay)
{
    AppControlMessage *payload = createPacket2(name);
    payload->setTimestamp(t);
    //payload->setTimestamp();
    payload->setCntrlType(_cntrlType);
    payload->setInterests(_interests);
    payload->setSourceData(_sourceData);
    payload->setContext(_context);

    if ( _destAddr.isUnspecified() )
    {
        mPktsForServer.push_back(payload);
        return;
    }

    // prepare broadcast case
    double broadcastJitter = 0;
    if ( _destAddr.get4().isLimitedBroadcastAddress() )
    {
        broadcastJitter = par("broadcastJitter");
        uint64 moduleId64 = (int)payload->par("sourceId");
        uint64 msgId64 = (int)payload->par("msgId");
        uint64 bcast = (moduleId64 << 32) | msgId64;
        if ( mBTT.find(bcast) == mBTT.end() )
        {
            cMessage* m;
            BTR btr;
            btr.expiry = simTime()+_userdelay+1.5;
            btr.relayed = false;
            btr.pkt = payload->dup();
            btr.retries = 0;
            mBTT[bcast] = btr;

            m = new cMessage("");
            mBroadcastRetries[m] = bcast;
            scheduleAt(simTime()+_userdelay+0.25, m);

            m = new cMessage("");
            mBroadcastExpiries[m] = bcast;
            scheduleAt(btr.expiry, m);
        }
    }

    // calculate overall sending delay
    double overallDelay = _userdelay + broadcastJitter;

    // send now or later
    if ( overallDelay )
    {
        SendLater sendLater;
        sendLater.destAddr = _destAddr;
        sendLater.pkt = payload;
        cMessage* m = new cMessage("");
        mSendLaterMessageMap[m] = sendLater;
        scheduleAt(simTime()+overallDelay, m);
    }
    else
    {
        sendPacket(payload, _destAddr);
        emit(sentPkSignal, payload);
        //numSent++;
    }

}





void UDPBurstAndBroadcast::generateBurst() // NOT USED
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
    //recordScalar("FailedRREQs", (double)mNetMan->numPendingRREQs());
    //recordScalar("FailedRegistrations", (double)mNetMan->numPendingRegistrations());
    //recordScalar("LinkFailures", (double)mNetMan->numAODVAllLineBreaksValue());
    //recordScalar("DataArrivals", (double)mNetMan->numAODVDataArrivalValue());






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


    // cannot use senddelayed as with udp delay only available a number of programming layers below
    // forward broadcast with jitter
    double broadcastJitter = par("broadcastJitter");
    SendLater sendLater;
    sendLater.destAddr = mBcastAddr;
    sendLater.pkt = payload;
    cMessage* m = new cMessage("");
    mSendLaterMessageMap[m] = sendLater;
    scheduleAt(simTime()+broadcastJitter, m);



    //sendPacket(payload, mBcastAddr);
    mNetMan->recordOnePacket(DISCOVERY_STAT);  // keep this here jitter is short, essentially it's gone!

}


void UDPBurstAndBroadcast::DataReceived(AppControlMessage* acm)
{
    pktDelay->collect(simTime() - acm->getTimestamp());
    numReceived++;

    double now = simTime().dbl();
    double pktTime = acm->getTimestamp().dbl();

    simtime_t e2eDelay = simTime() - acm->getTimestamp();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    mNetMan->addADataPacketE2EDelay(e2eDelay);
    //mNetMan->recordOnePacket(AODV_DATA_ARRIVAL);

    this->getParentModule()->bubble("Received data packet");

    EV << "Received data packet: " << UDPSocket::getReceivedPacketInfo(acm) << endl;
    emit(rcvdPkSignal, acm);

    string sd = acm->getSourceData();
    string con = acm->getContext();

    //char delim = 0xFF;
    //string delim = "\xFF";
    //acm->getso
    //sd.be
    //char theData[50];
    //memcpy(theData, sd.c_str(), sd.size());
    //theData+=sd.size();
    //memcpy(theData, sd.c_str(), sd.size());

    DataCentricAppPkt* appPkt = new DataCentricAppPkt("Data_DataCentricAppPkt");
    appPkt->setKind(DATA_PACKET);

    //appPkt->getPktData().insert(appPkt->getPktData().end(), sd.size(), sd.c_str()[0]);
    //appPkt->getPktData().insert(appPkt->getPktData().end(), 1, 0xFF);
    //appPkt->getPktData().insert(appPkt->getPktData().end(), con.size(), con.c_str()[0]);

    appPkt->getPktData().insert(appPkt->getPktData().end(), sd.begin(), sd.end());
    appPkt->getPktData().insert(appPkt->getPktData().end(), 1, 0xFF);
    appPkt->getPktData().insert(appPkt->getPktData().end(), con.begin(), con.end());

    send(appPkt, mUpperLayerOut);


}


void UDPBurstAndBroadcast::ProcessPacket(cPacket *pk)
{
    if ( !dynamic_cast<AppControlMessage*>(pk) )
        return;
    double currentTime = simTime().dbl();
    NullStream() << "Current Time: " << currentTime << "\n";

    UDPDataIndication *udpCtrl = check_and_cast<UDPDataIndication*>(pk->getControlInfo());
    IPvXAddress destAddr = udpCtrl->getDestAddr();

    IPvXAddress origAddr;
    //origAddr = udpCtrl->getSrcAddr();
    int moduleId = (int)pk->par("sourceId");
    string srcFP = cSimulation::getActiveSimulation()->getModule(moduleId)->getParentModule()->getFullPath();
    origAddr = IPvXAddressResolver().resolve(srcFP.c_str());

    AppControlMessage* acm = dynamic_cast<AppControlMessage*>(pk);
    std::string sd;
    std::string intr;
    std::string con;
    bool atLeastOneMatch;
    AData d;
    IPvXAddress addr;
    cMessage* m;
    string theData;
    unsigned char temp[30];
    unsigned char x[20];
    std::vector<IPvXAddress> jj;
    switch ( acm->getCntrlType() )
    {
        case REGISTER_AS_SINK_CONFIRMATION:
            COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                    << ", Received reg conf from: " << origAddr.get4() << "\n";
            break;
        case FIND_CONTROL_UNIT:
            if ( mIsControlUnit )
            {
                generatePacket(origAddr, "ServiceDiscoveryResponse", CONTROL_UNIT_DETAILS,
                        getParentModule()->getFullPath().c_str(), "", "", 0);
                mNetMan->recordOnePacket(DISCOVERY_STAT);
            }
            break;
        case CONTROL_UNIT_DETAILS:
            mServerAddr = IPvXAddressResolver().resolve(acm->getInterests());
            for (std::vector<AppControlMessage*>::iterator i = mPktsForServer.begin();
                    i != mPktsForServer.end(); ++i)
            {
                //socket.sendTo(*i, mServerAddr, destPort, outputInterface);
                //int ctrlT = (*i)->getCntrlType();
                sendPacket(*i, mServerAddr);
                //numSent++;
            }
            break;
        case SERVICE_DISCOVERY_REQUEST: // comes by bcast
            ////////////// NEW CODE
            intr = acm->getInterests();
            con = acm->getContext();
            for (BindingListIter i = mBindingList.begin();
                    i != mBindingList.end(); ++i)
            {
                pair<std::string::iterator,std::string::iterator> res;
                std::string currentData = i->first.data; // i->first.data.begin() returns unwanted const_iter
                res = std::mismatch(intr.begin(), intr.end(), currentData.begin());
                if (res.first == intr.end())
                {
                    std::string currentContext = i->first.context; // i->first.context.begin() returns unwanted const_iter
                    res = std::mismatch(con.begin(), con.end(), currentContext.begin());
                    if (res.first == con.end())
                    {
                        // both interest and context of incoming service discovery are prefix
                        // of at least one binding entry, so reply to discovery and finish - CHECKED
                        generatePacket(origAddr, "ServiceDiscoveryResponse", SERVICE_DISCOVERY_CONFIRMATION,
                                intr.c_str(), getParentModule()->getFullPath().c_str(), con.c_str(), 0);
                        break;
                    }
                }
            }

            ////////////////////////////////////
            break;
        case SERVICE_DISCOVERY_CONFIRMATION: // comes by ucast
            ////////////// NEW CODE
            d.data = acm->getInterests();
            d.context = acm->getContext();
            mDiscoveryTries.erase(d);
            //addr = IPvXAddressResolver().resolve(acm->getSourceData());
            //mServiceList[d].push_back(addr);
            // Or this method?

            if ( !mServiceList[d].insert(origAddr).second )
                break; // duplicate service response

            // if we get a duplicate service response dont send another bind request
            generatePacket(origAddr, "BindRequest", BINDING_REQUEST,
                    d.data.c_str(), "", d.context.c_str(), 0);
            if ( mBindingNumTries > 1 )
            {
                mBindingTries[d] = 1;
                m = new cMessage("BindingRetry");
                m->addPar("Data").setStringValue(d.data.c_str());
                m->addPar("Context").setStringValue(d.context.c_str());
                m->addPar("Address").setStringValue(origAddr.str().c_str());
                scheduleAt(simTime()+mBindingTimeOut, m);
            }
            ////////////////////////////////////
            break;

        case BINDING_REQUEST:
            ////////////// NEW CODE
            intr = acm->getInterests();
            con = acm->getContext();
            atLeastOneMatch = false;
            for (BindingListIter i = mBindingList.begin();
                    i != mBindingList.end(); ++i)
            {
                pair<std::string::iterator,std::string::iterator> res;
                std::string currentData = i->first.data; // i->first.data.begin() returns unwanted const_iter
                res = std::mismatch(intr.begin(), intr.end(), currentData.begin());
                //res = std::mismatch(i->first.data.begin(), i->first.data.end(), intr.begin());
                //if (res.first == i->first.data.end())
                if (res.first == intr.end())
                {
                    std::string currentContext = i->first.context; // i->first.context.begin() returns unwanted const_iter
                    res = std::mismatch(con.begin(), con.end(), currentContext.begin());
                    //res = std::mismatch(i->first.context.begin(), i->first.context.end(), con.begin());
                    //if (res.first == i->first.context.end())
                    if (res.first == con.end())
                    {
                        // both interest and context of incoming bind request are prefixes
                        // of data and context in this bind entry - CHECKED
                        i->second.AddressList.push_back(origAddr);
                        // send out any waiting data pkts to this newly bound address
                        for (std::vector<AData>::iterator it = i->second.DataQueue.begin();
                                it != i->second.DataQueue.end(); ++it)
                        {
                            COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4() << ", Sending DATA to: " << origAddr.get4() << "\n";
                            generatePacket(it->t, origAddr, "EnergyData", HOME_ENERGY_DATA, "",
                                    it->data.c_str(), it->context.c_str(), 0);
                        }
                        atLeastOneMatch = true;
                    }
                }
            }
            if ( atLeastOneMatch )
            {
                generatePacket(origAddr, "BindResponse", BINDING_CONFIRMATION,
                        intr.c_str(), getParentModule()->getFullPath().c_str(), con.c_str(), 0);
            }

            ////////////////////////////////////
            break;
        case BINDING_CONFIRMATION:
            ////////////// NEW CODE
            d.data = acm->getInterests();
            d.context = acm->getContext();
            mBindingTries.erase(d);
            ////////////////////////////////////
            break;
        case REGISTER_AS_SOURCE:
            break;
        case REGISTER_AS_SINK:
            mInterestedNodes[origAddr].interest = acm->getInterests();
            mInterestedNodes[origAddr].context = acm->getContext();
            //origAddr.get4().getInt()
            //IPv4Address pending(origAddr.get4());
            mNetMan->removePendingRegistration(origAddr.get4().getInt());

            COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                    << ", Adding reg for: " << origAddr.get4() << "\n";

            // Send a confirmation to show success, but mainly to get a route
            // during a manual action so known to probably be separated from
            // other traffic.
            if ( par("confirmRegistration").boolValue() )
            {
                COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                        << ", Sending REGISTER_AS_SINK_CONFIRMATION to: " << origAddr.get4() << "\n";
                generatePacket(origAddr, "BindResponse", REGISTER_AS_SINK_CONFIRMATION,
                        acm->getInterests(), "", "", 0);
                mNetMan->recordOnePacket(REGISTER_STAT);
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
                            // both data and context of a recorded interest are prefix
                            // of the incoming data pkt's data and context fields - CHECKED
                            if ( i->first == myAddr)
                            {
                                DataReceived(acm);
                            }
                            else
                            {
                                //socket.sendTo(acm->dup(), i->first, destPort, outputInterface);
                                // THINK THE ABOVE IS OLD AND SHOULD HAVE BEEN REMOVED
                                mNetMan->addPendingDataPkt();
                                sendPacket(acm->dup(), i->first);
                                COUT << "Time: " << simTime().dbl() << " At: " << myAddr.get4()
                                        << ", Forwarding to: " << i->first.get4() << "\n";
                                //numSent++;
                            }
                        }
                    }
                }

            }
            else
            {
                DataReceived(acm);
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

    //delete pk;
    delete acm; //???

    // ignore duplicates for the moment
    //if ( !duplicate(moduleId, msgId, pk) )
    //{
    //}


}
