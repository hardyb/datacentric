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
    virtual int    numInitStages    () const { return 2; }
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
    double ScheduleStartTime();
    void processSinkFor(string &temp2);
    std::string contextData;

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
    void SensorReading(ActionThreadsIterator& i);

    virtual void SendTraf(cPacket *msg, const char*);

    // sibling module IDs
    DataCentricNetworkLayer* netModule;
    DataCentricNetworkMan* mNetMan;

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



};

#endif
