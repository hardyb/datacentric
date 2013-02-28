/**
 * @simple data centric network manager
*/

#ifndef DATACENTRIC_NETWORKMAN
#define DATACENTRIC_NETWORKMAN

#include <omnetpp.h>

// DataCentric 'C' associations
#include "RoutingAndAggregation.h"
//#include "DataCentricTestApp.h"
#include "IPvXAddressResolver.h"

class DataCentricTestApp;





//#include "DataCentricNetworkLayer.h" // Cyclic dependancy may need some attention

class DataCentricNetworkLayer;
class UDPBurstAndBroadcast;

class DataCentricNetworkMan : public cSimpleModule
{
  public:
    virtual void initialize(int);
    virtual int numInitStages() const  {return 3;}
    virtual void finish();
    void recordOnePacket(unsigned char type);
    void addADataPacketE2EDelay(simtime_t delay);
    void changeInModulesDown(double adjustment);
    void addDemand(signed int _demand);
    //signed int getDemand();
    void setSinkOrSources(std::string &sinksString, bool isSources);
    void traverseModule(const cModule& m);
    void addAppliance(DataCentricTestApp* _appliance);
    signed short totalDemand(); // local version
    signed short getTotalDemand(); // remote version
    void recordDemand(); // remote version
    void recordDemandLocal(); // local version
    void addPendingRREQ(uint32 _originator, uint32 _destination);
    void removePendingRREQ(uint32 _originator, uint32 _destination);
    unsigned int numPendingRREQs();
    void addPendingRegistration(uint32 _originator);
    void addPendingDataPkt();
    void removePendingRegistration(uint32 _originator);
    unsigned int numPendingRegistrations();
    unsigned int numAODVAllLineBreaksValue();
    unsigned int numDataArrivalsValue();
    void removePendingDataPkt();
    void addProactiveRREQ(uint32 _originator);
    void clearProactiveRREQ();
    unsigned int numProactiveRREQ();
    void collectBits(unsigned int bits);
    void collectMsgBits(unsigned int bits, cPacket* p);




    typedef struct
    {
        double x;
        double y;
        double z;
        double w;
        double h;
        double d;
        char context[20];
    }Region;

    typedef std::vector<Region> Regions;
    typedef std::vector<Region>::iterator RegionsIterator;
    Regions mRegions;

    typedef std::map<uint64_t, DataCentricNetworkLayer*> NetModules;
    typedef std::map<uint64_t, DataCentricNetworkLayer*>::iterator NetModulesIterator;
    NetModules mNetModules;

    typedef std::map<IPvXAddress, UDPBurstAndBroadcast*> AppModules;
    typedef std::map<IPvXAddress, UDPBurstAndBroadcast*>::iterator AppModulesIterator;
    AppModules mAppModules;

  protected:
    void handleMessage(cMessage*);

    double numControlPackets;
    double totControlPacketsThisRun;

    double numHelloPackets;
    double numHelloBackPackets;
    double bcastNumInterestPackets;
    double ucastNumInterestPackets;
    double numAdvertPackets;
    double numReinforcementPackets;
    double numBreakagePackets;
    double numModulesDown;
    double numRREQPackets;
    double numRERRPackets;
    double numDiscoveryPackets;
    double numRegisterPackets;
    double numRReplyPackets;

    // Unicast forwarded packets (this includes all RREP, data & RERR I think
    double numAODVDataPackets;
    double numAODVDataLineBreaks;
    double numAODVAllLineBreaks;
    //double numDataArrival;
    unsigned int numDataArrivals;
    double numPendingDataPackets;

    unsigned int mExpectedDataArrivals;
    unsigned int currentTotalBits;
    double bpsInterval;
    double nextCheckTime;

    signed int mDemand;

    std::vector<DataCentricTestApp*> mNodeArray;

    typedef struct
    {
        uint32 originator;
        uint32 destination;
    }PendingRREQ;

    struct Compare {
        bool operator() (PendingRREQ const &lhs, PendingRREQ const &rhs) const
        {
            if ( lhs.originator < rhs.originator )
            {
                return true;
            }

            if ( lhs.originator == rhs.originator
                    && lhs.destination < rhs.destination )
            {
                return true;
            }

            return false;
        }
    };

    typedef std::set<PendingRREQ, Compare> PendingRREQSet;
    PendingRREQSet mPendingRREQSet; // A buffer to store a pointer to a message and the related receive power.


    typedef std::set<uint32> PendingRegistrationSet;
    PendingRegistrationSet mPendingRegistrationSet;

    typedef std::set<uint32> ProactiveRREQSet;
    ProactiveRREQSet mProactiveRREQSet;

    std::set<std::string> mAirFrameNames;




    cOutVector controlPackets;
    cOutVector controlPacketFrequency;
    cOutVector helloPacketFrequency;
    cOutVector helloBackPacketFrequency;
    cOutVector bcastInterestPacketFrequency;
    cOutVector ucastInterestPacketFrequency;
    cOutVector advertPacketFrequency;
    cOutVector reinforcementPacketFrequency;
    cOutVector dataPacketE2EDelay;
    cStdDev E2EDelayStats;
    cStdDev bpsStats;

    cOutVector breakagePacketFrequency;
    cOutVector modulesDownVector;
    cOutVector RREQPacketFrequency;
    cOutVector RERRPacketFrequency;
    cOutVector DiscoveryPacketFrequency;
    cOutVector RegisterPacketFrequency;
    cOutVector RReplyPacketFrequency;
    cOutVector AODVDataPacketFrequency;
    cOutVector AODVDataLineBreakVector;
    cOutVector AODVAllLineBreakVector;
    cOutVector DataArrivalsVector;
    cOutVector PendingDataPacketsVector;
    cOutVector demandVector;
    cOutVector pendingRREQVector;
    cOutVector pendingRegistrationVector;




    cMessage *mpControlPacketFrequencyMessage;
    cMessage *mpDemandMessage;
    // OPERATIONS
    //virtual void handleSelfMsg(cMessage*);
    //virtual void handleLowerMsg(cMessage*);


    // sibling module IDs
    //DataCentricNetworkLayer* netModule;

  private:
    std::set<DataCentricTestApp*> mAppliances;

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

    void recordOneDataCentricPacketForTotalInRun(unsigned char type);
    void recordOneDataCentricPacketForFrequencyStats(unsigned char type);
    void recordOneAODVZIGBEEPacketForTotalInRun(unsigned char type);
    void recordOneAODVZIGBEEPacketForFrequencyStats(unsigned char type);

};

#endif
