/**
 * @simple traffic generator to test IEEE 802.15.4 protocols
*/

#ifndef DATACENTRIC_TEST_APP
#define DATACENTRIC_TEST_APP

#include "TrafGenPar.h"
#include "DataCentricAppPkt_m.h"
#include "DataCentricNetworkLayer.h"
//#include "Ieee802154UpperCtrlInfo_m.h"


// DataCentric 'C' associations
#include "RoutingAndAggregation.h"
//#include <INode.h>


#define DATACENTRIC_MODE 1
#define AODV_MODE 2



class DataCentricTestApp : public TrafGenPar
{
  public:
    virtual int    numInitStages    () const { return 4; }
    /*
    typedef struct Program
    {
        string programName;
        int watts;
    }Program;

    typedef struct PeriodOfOperation
    {
        Program program;
        int period;
    }PeriodOfOperation;
    */


    //typedef std::vector<PeriodOfOperation> OperationSchedule;

    //typedef std::map<string, OperationSchedule> SecheduleSet;

    /////////////////////////////////////////////////

    typedef list<ifstream*> ActionStreamHierarchy;
    //typedef struct ActionThread
    //{
    //    ActionStreamHierarchy* ash;
    //    cMessage* msg;
    //};
    typedef map<cMessage*, ActionStreamHierarchy*>::iterator ActionThreadsIterator;


    //SecheduleSet mSecheduleSet;

    // LIFECYCLE
    // this takes care of constructors and destructors

    virtual void initialize(int);
    virtual void finish();
    double NodeStartTime();
    double TimeSinkRegisterWithControlUnit();
    double TimeSourceRegisterWithControlUnit();
    double ScheduleStartTime();
    void processSinkFor(string &temp2);
    void processSourceFor(string &temp1);
    void processActionsFor(string &actionThreadsString);
    std::string contextData;
    signed short currentDemand;
    bool isAppliance;

  protected:

    // OPERATIONS
    virtual void handleSelfMsg(cMessage*);
    //void readFile(char* filePath, OperationSchedule& operationSchedule);
    virtual void handleLowerMsg(cMessage*);



    int getNextAction(ActionThreadsIterator& i);
    //int getReading(ifstream* ifs);
    void processWatts(ActionThreadsIterator& i);
    void startProgram(ActionThreadsIterator& i);
    //void actionReading(int reading);
    //void actionProgram(string program);
    void FileEnd(ActionThreadsIterator& i);
    void SensorReading(ActionThreadsIterator& i, const char* sensorDataName);
    void setCurrentDemand(signed short _demand);
    void processEnvironmentalData(unsigned char* pkt);
    void processDemandData(unsigned char* pkt);
    void processWattsData(unsigned char* pkt);
    void processBidData(unsigned char* pkt);
    void processOccupancyData(unsigned char* pkt);
    void processTemperatureData(unsigned char* pkt);


    virtual void SendTraf(cPacket *msg, const char*);

    // sibling module IDs
    DataCentricNetworkLayer* netModule;
    DataCentricNetworkMan* mNetMan;
    cModule* mNet;

  private:
    bool    m_debug;        // debug switch
    std::vector<std::string> sourcesData;
    std::vector<std::string> sinksData;
    int     mLowerLayerIn;
    int     mLowerLayerOut;

    int     mCurrentTrafficPattern;

    double  mNumTrafficMsgs;
    double  mNumTrafficMsgRcvd;
    double  mNumTrafficMsgNotDelivered;

    const char* m_moduleName;
    simtime_t   sumE2EDelay;
    double  numReceived;
    double  totalByteRecv;
    cOutVector e2eDelayVec;
    cOutVector meanE2EDelayVec;

    cMessage *mpStartMessage;
    //ifstream mActivityFile;
    //ifstream mProgramFile;



    map<cMessage*, ActionStreamHierarchy*> mActionThreads;
    int mAppMode;

    typedef NEIGHBOUR_ADDR ApplianceId;
    typedef unsigned short DemandBid;
    typedef unsigned short Priority;

    std::map<ApplianceId, DemandBid> mBids;
    std::map<ApplianceId, Priority> mPriorities;



};

#endif
