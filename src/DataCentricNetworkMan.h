/**
 * @simple data centric network manager
*/

#ifndef DATACENTRIC_NETWORKMAN
#define DATACENTRIC_NETWORKMAN

#include <omnetpp.h>

// DataCentric 'C' associations
#include "RoutingAndAggregation.h"

class DataCentricNetworkMan : public cSimpleModule
{
  public:
    virtual void initialize(int);
    virtual void finish();
    void updateControlPacketData(unsigned char type, bool ucast);

  protected:
    void handleMessage(cMessage*);

    double numControlPackets;

    double numHelloPackets;
    double numHelloBackPackets;
    double bcastNumInterestPackets;
    double ucastNumInterestPackets;
    double numAdvertPackets;
    double numReinforcementPackets;





    cOutVector controlPackets;
    cOutVector controlPacketFrequency;
    cOutVector helloPacketFrequency;
    cOutVector helloBackPacketFrequency;
    cOutVector bcastInterestPacketFrequency;
    cOutVector ucastInterestPacketFrequency;
    cOutVector advertPacketFrequency;
    cOutVector reinforcementPacketFrequency;


    cMessage *mpControlPacketFrequencyMessage;
    // OPERATIONS
    //virtual void handleSelfMsg(cMessage*);
    //virtual void handleLowerMsg(cMessage*);


    // sibling module IDs
    //DataCentricNetworkLayer* netModule;

  private:
    //bool    m_debug;        // debug switch
    //std::vector<std::string> sourcesData;
    //std::vector<std::string> sinksData;
    //std::string contextData;
    //int     mLowerLayerIn;
    //int     mLowerLayerOut;
    //int     mCurrentTrafficPattern;
    //double  mNumTrafficMsgs;
    //double  mNumTrafficMsgRcvd;
    //double  mNumTrafficMsgNotDelivered;
    //const char* m_moduleName;
    //simtime_t   sumE2EDelay;
    //double  numReceived;
    //double  totalByteRecv;
    //cOutVector e2eDelayVec;
    //cOutVector meanE2EDelayVec;
    //cMessage *mpStartMessage;


};

#endif
