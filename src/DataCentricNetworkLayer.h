/**
 * @short Implementation of a simple packets forward function for IEEE 802.15.4 star network
 *  support device <-> PAN coordinator <-> device transmission
    MAC address translation will be done in MAC layer (refer to Ieee802154Mac::handleUpperMsg())
 * @author Feng Chen
*/

#ifndef DataCentricNetworkLayer_H
#define DataCentricNetworkLayer_H

#include <omnetpp.h>
#include "BasicBattery.h"

#include "DataCentricAppPkt_D.h"
#include "Ieee802154NetworkCtrlInfo_m.h"
#include "RoutingAndAggregation.h"

#include "Ieee802154Phy.h"
#include "DataCentricNetworkMan.h"
#include "DropTailQueue.h"




#define IMAGEPATH "C:/omnetpp-4.2.1/images/"

#define DATA_PACKET 1
#define STARTUP_MESSAGE 2
#define CONTEXT_MESSAGE 3
#define SOURCE_MESSAGE 4
#define SINK_MESSAGE 5
#define COLLABORATOR_INITITOR_MESSAGE 6
#define COLLABORATOR_MESSAGE 7




class DataCentricNetworkLayer : public cSimpleModule, public INotifiable
{
  public:
    unsigned int testValue;
    virtual int    numInitStages    () const { return 2; }
    virtual void initialize(int);
    virtual void finish();
    void WriteModuleListFile();
    bool FindNode(const string& addressToFind, string& matchingNodeName);
    void sendDownTheNIC();
    bool duplicate(int moduleId, int msgId);//, DataCentricAppPkt* pk);

    double StartTime();

    // DataCentric 'C' associations
    RoutingData moduleRD;
    simtime_t currentPktCreationTime;
    static bool justStartedInitialising;
    cOutVector controlPackets;
    cOutVector neighbourLqis;
    cOutVector RangeLqis;
    cOutVector TotalInterestArrivalsVector;
    cOutVector InterestInterArrivalTimesVector;
    cOutVector StabilityVector;
    double mTotalInterestArrivals;

    cOutVector InterestInterDepartureTimesVector;
    simtime_t          mLastInterestDepartureTime;

    cOutVector AdvertInterDepartureTimesVector;
    simtime_t          mLastAdvertDepartureTime;

    map<unsigned int, unsigned int> mNeighboursInLqiRange;
    map<void*, cMessage*> mTimeoutMessages;
    set<cMessage*> mTimeoutMessages2;



    cStdDev seenLqis;
    cOutVector seenLqisInTime;



    typedef map<void*, cMessage*>::iterator TimeoutMessagesIterator;
    typedef set<cMessage*>::iterator TimeoutMessages2Iterator;

    // sibling module IDs
    //cModule* nicModule;
    //cModule* macModule;
    //cModule* mPhyModule;
    Ieee802154Phy* mPhyModule;
    BasicBattery* mBatteryModule;
    DropTailQueue* mQueueModule;
    DataCentricNetworkMan* mNetMan;

    double mMeanDownTime;
    int mMeanDownTimeInterval;
    double mMeanDownTimeSeconds;
    double mMeanUpTimeSeconds;

    // accessed from C function call back
    int             mLowerLayerOut;
    int             mUpperLayerOut;
    double          mRoutingDelay;
    uint64              mAddress;

  protected:
    // Message handle functions
    void                handleMessage           (cMessage*);
    void SetCurrentModuleInCLanguageFramework();
    void handleLowerLayerMessage(DataCentricAppPkt* appPkt);
    void handleUpperLayerMessage(DataCentricAppPkt* appPkt);
    void SendDataAsIs(DataCentricAppPkt* appPkt);
    void SendDataWithLongestContext(DataCentricAppPkt* appPkt);
    void StartUpModule();
    void SetContext(DataCentricAppPkt* appPkt);
    void SetSourceWithLongestContext(DataCentricAppPkt* appPkt);
    void SetSinkWithShortestContext(DataCentricAppPkt* appPkt);
    void SetSinkAsIs(DataCentricAppPkt* appPkt);
    void SetCollaboratorInitiatorAsIs(DataCentricAppPkt* appPkt);
    void SetCollaboratorInitiatorWithShortestContext(DataCentricAppPkt* appPkt);
    void SetCollaboratorAsIs(DataCentricAppPkt* appPkt);
    void SetCollaboratorWithShortestContext(DataCentricAppPkt* appPkt);



    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    // debugging enabled for this node? Used in the definition of EV
    bool                m_debug;
    bool                isPANCoor;
    unsigned int        nodeConstraintValue;
    double              mStability;
    std::string         mTheAddressString;
    const char*     m_moduleName;
    NotificationBoard* mpNb;

    // module gate ID
    int             mUpperLayerIn;
    int             mLowerLayerIn;

    // for statistical data
    double          numForward;
    simtime_t          mInterestFirstArrivalTime_TESTINGONLY;
    simtime_t          mLengthLatestInterestArrivalPeriod_TESTINGONLY;
    simtime_t          mLastInterestArrivalTime;

    cMessage *mpStartMessage;
    cMessage *mpUpDownMessage;
    cMessage *mMessageForTesting_1;
    cMessage *mRegularCheckMessage;

    int numSent;
    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;

};









#endif

