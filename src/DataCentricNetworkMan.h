/**
 * @simple data centric network manager
*/

#ifndef DATACENTRIC_NETWORKMAN
#define DATACENTRIC_NETWORKMAN

#include <omnetpp.h>

// DataCentric 'C' associations
#include "RoutingAndAggregation.h"

//#include "DataCentricNetworkLayer.h" // Cyclic dependancy may need some attention

class DataCentricNetworkLayer;


class DataCentricNetworkMan : public cSimpleModule
{
  public:
    virtual void initialize(int);
    virtual void finish();
    void updateControlPacketData(unsigned char type, bool ucast);
    void addADataPacketE2EDelay(simtime_t delay);
    void changeInModulesDown(double adjustment);

    typedef struct
    {
        double x;
        double y;
        double w;
        double h;
        char context[20];
    }Region;

    typedef std::vector<Region> Regions;
    typedef std::vector<Region>::iterator RegionsIterator;
    Regions mRegions;

    typedef std::map<uint64_t, DataCentricNetworkLayer*> NetModules;
    typedef std::map<uint64_t, DataCentricNetworkLayer*>::iterator NetModulesIterator;
    NetModules mNetModules;

  protected:
    void handleMessage(cMessage*);

    double numControlPackets;

    double numHelloPackets;
    double numHelloBackPackets;
    double bcastNumInterestPackets;
    double ucastNumInterestPackets;
    double numAdvertPackets;
    double numReinforcementPackets;
    double numBreakagePackets;
    double numModulesDown;





    cOutVector controlPackets;
    cOutVector controlPacketFrequency;
    cOutVector helloPacketFrequency;
    cOutVector helloBackPacketFrequency;
    cOutVector bcastInterestPacketFrequency;
    cOutVector ucastInterestPacketFrequency;
    cOutVector advertPacketFrequency;
    cOutVector reinforcementPacketFrequency;
    cOutVector dataPacketE2EDelay;
    cOutVector breakagePacketFrequency;
    cOutVector modulesDownVector;


    unsigned int numDataArrivals;


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
