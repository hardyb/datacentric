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


#define FIND_CONTROL_UNIT 0




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

  protected:
    UDPSocket socket;
    int localPort, destPort;

    // NOT implementing ucast retransmission
    // So use one bool for any neighbours
    struct BTR
    {
        simtime_t expiry;
        bool relayed;
    };
    typedef std::map<uint64, BTR> BcastsType;
    typedef BcastsType::iterator BcastsIterator;
    BcastsType mBTT;
    typedef std::map<cMessage*, uint64> BroadcastExpiryType;
    typedef BroadcastExpiryType::iterator BroadcastExpiryIterator;
    BroadcastExpiryType mBroadcastExpiries;

    ChooseDestAddrMode chooseDestAddrMode;
    std::vector<IPvXAddress> destAddresses;
    IPvXAddress destAddr;
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

    std::map<IPvXAddress, std::string> mInterestedNodes;
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
    virtual cPacket *createPacket2();
    virtual void processPacket(cPacket *msg);
    virtual void generateBurst();
    //virtual void forwardBroadcast(int moduleId, int msgId);
    virtual void forwardBroadcast(cPacket* payload);
    void ProcessIfAppControlPacket(cPacket* payload);

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  public:
    UDPBurstAndBroadcast();
    virtual ~UDPBurstAndBroadcast();
};

#endif
