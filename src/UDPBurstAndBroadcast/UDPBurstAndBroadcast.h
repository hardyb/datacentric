//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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


#ifndef __INET_UDPBURSTANDBROADCAST_H
#define __INET_UDPBURSTANDBROADCAST_H

#include <vector>
#include <map>

#include "INETDefs.h"
#include "UDPSocket.h"
#include "AppControlMessage_m.h"
#include "DataCentricAppPkt_D.h"
#include "DataCentricNetworkMan.h"
#include "Ieee802154Phy.h"
#include "DropTailQueue.h"


// packet control types
#define FIND_CONTROL_UNIT 0
#define CONTROL_UNIT_DETAILS 1
#define REGISTER_AS_SOURCE 2
#define REGISTER_AS_SINK 3
#define HOME_ENERGY_DATA 4
#define REGISTER_AS_SINK_CONFIRMATION 10



#define SERVICE_DISCOVERY_REQUEST 21
#define SERVICE_DISCOVERY_CONFIRMATION 22
#define BINDING_REQUEST 23
#define BINDING_CONFIRMATION 24




#include "aodv_zigbee_defs.h"


/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBurstAndBroadcast : public cSimpleModule
{
  public:
    enum ChooseDestAddrMode
    {
        ONCE = 1, PER_BURST, PER_SEND
    };
    RoutingData moduleRD;

  protected:
    Ieee802154Phy* mPhyModule;
    DropTailQueue* mQueueModule;
    UDPSocket socket;
    int localPort, destPort;
    int mUpperLayerIn;
    int mUpperLayerOut;
    cMessage *mpUpDownMessage;
    int              mStability;
    //double              mStability;

    typedef struct
    {
        std::string data;
        std::string context;
        simtime_t t;
    } AData;

    struct ABinding
    {
        std::vector<IPvXAddress> AddressList;
        std::vector<AData> DataQueue;
    };

    struct Compare {
        bool operator() (const AData &lhs, const AData &rhs) const
        {
            if ( lhs.data.compare(rhs.data) < 0 ) // lhs.data < rhs.data )
            {
                return true;
            }

            if ( lhs.data == rhs.data
                    && lhs.context.compare(rhs.context) < 0 ) // lhs.context < rhs.context )
            {
                return true;
            }

            return false;
        }
    };

    std::map<AData, ABinding, Compare> mBindingList;
    typedef std::map<AData, ABinding, Compare>::iterator BindingListIter;

    std::map<AData, std::vector<IPvXAddress>, Compare> mServiceList;
    typedef std::map< AData, std::vector<IPvXAddress>, Compare>::iterator ServiceListIter;

    std::map<AData, int, Compare> mBindingTries;
    std::map<AData, int, Compare> mDiscoveryTries;
    typedef std::map<AData, int, Compare>::iterator TriesIter;

    double mServiceDiscoveryTimeOut;
    int mServiceDiscoveryNumTries;
    double mBindingTimeOut;
    int mBindingNumTries;


    // NOT implementing ucast retransmission
    // So use one bool for any neighbours
    struct BTR
    {
        simtime_t expiry;
        bool relayed;
        cPacket* pkt;
        unsigned int retries;
    };
    struct SendLater
    {
        cPacket* pkt;
        IPvXAddress destAddr;
    };

    typedef std::map<uint64, BTR> BcastsType;
    typedef BcastsType::iterator BcastsIterator;
    BcastsType mBTT;
    typedef std::map<cMessage*, SendLater> SendLaterMessageMapType;
    typedef std::map<cMessage*, uint64> BroadcastMessageMapType;
    typedef BroadcastMessageMapType::iterator BroadcastExpiryIterator;
    typedef BroadcastMessageMapType::iterator BroadcastRetryIterator;
    typedef SendLaterMessageMapType::iterator SendLaterMessageMapIterator;
    BroadcastMessageMapType mBroadcastExpiries;
    BroadcastMessageMapType mBroadcastRetries;
    SendLaterMessageMapType mSendLaterMessageMap;

    ChooseDestAddrMode chooseDestAddrMode;
    std::vector<IPvXAddress> destAddresses;
    IPvXAddress destAddr;
    IPvXAddress mBcastAddr;
    IPvXAddress mServerAddr;
    IPvXAddress myAddr;
    bool mIsControlUnit;
    bool mBindWithSource;
    std::vector<AppControlMessage*> mPktsForServer;

    int destAddrRNG;

    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;
    simtime_t delayLimit;
    cMessage *timerNext;
    simtime_t stopTime;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool activeBurst;
    bool isSource;
    bool haveSleepDuration;
    int outputInterface;
    std::vector<int> outputInterfaceMulticastBroadcast;
    cOutVector e2eDelayVec;
    DataCentricNetworkMan* mNetMan;

    struct TheInterest
    {
        std::string interest;
        std::string context;
    };

    std::map<IPvXAddress, TheInterest> mInterestedNodes;
    typedef std::map<IPvXAddress, std::string>::iterator InterestedNodesIterator;


    static int counter; // counter for generating a global number for each packet

    int numSent;
    int numReceived;
    int numDeleted;
    int numDuplicated;
    cStdDev *pktDelay;

    // volatile parameters:
    cPar *messageLengthPar;
    cPar *burstDurationPar;
    cPar *sleepDurationPar;
    cPar *sendIntervalPar;

    //statistics:
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;

    // chooses random destination address
    virtual IPvXAddress chooseDestAddr();
    virtual cPacket *createPacket();
    virtual AppControlMessage *createPacket2(const char *name);
    virtual void handlePacket(cPacket *msg);
    virtual void generateBurst();
    void generatePacket(IPvXAddress &_destAddr, const char *name, int _cntrlType, const char * _interests, const char * _sourceData, const char * _context, double _delay);
    void generatePacket(simtime_t t, IPvXAddress &_destAddr, const char *name, int _cntrlType, const char * _interests, const char * _sourceData, const char * _context, double _delay);
    void sendDownTheNIC();
    void sendPacket(cPacket *payload, const IPvXAddress &_destAddr);


    //virtual void forwardBroadcast(int moduleId, int msgId);
    virtual void forwardBroadcast(cPacket* payload);
    void ProcessPacket(cPacket* payload);
    void DataReceived(AppControlMessage* acm);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    void handleUpperLayerMessage(DataCentricAppPkt* appPkt);
    void SetContext(DataCentricAppPkt* appPkt);
    bool duplicate(int moduleId, int msgId, cPacket *pk);
    virtual void finish();

  public:
    UDPBurstAndBroadcast();
    virtual ~UDPBurstAndBroadcast();
};

#endif

