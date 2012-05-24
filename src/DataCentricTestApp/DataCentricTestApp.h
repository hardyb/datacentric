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
#include <INode.h>





class DataCentricTestApp : public TrafGenPar
{
  public:
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

    typedef std::vector<PeriodOfOperation> OperationSchedule;

    typedef std::map<string, OperationSchedule> SecheduleSet;

    SecheduleSet mSecheduleSet;

    // LIFECYCLE
    // this takes care of constructors and destructors

    virtual void initialize(int);
    virtual void finish();
    double StartTime();

  protected:

    // OPERATIONS
    virtual void handleSelfMsg(cMessage*);
    void readFile(char* filePath, OperationSchedule& operationSchedule);
    virtual void handleLowerMsg(cMessage*);



    int getNextAction(ifstream* ifs);
    int getReading();
    string getProgram();
    void actionReading(int reading);
    void actionProgram(string program);

    virtual void SendTraf(cPacket *msg, const char*);

    // sibling module IDs
    DataCentricNetworkLayer* netModule;

  private:
    bool    m_debug;        // debug switch
    std::vector<std::string> sourcesData;
    std::vector<std::string> sinksData;
    std::string contextData;
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
    ifstream mActivityFile;
    ifstream mProgramFile;


    typedef list<ifstream*> ActionStreamHierarchy;
    typedef struct ActionThread
    {
        ActionStreamHierarchy* ash;
        cMessage* msg;
    };
    //set<ActionThread*> mActionThreads;
    map<cMessage*, ActionStreamHierarchy*> mActionThreads;
    typedef map<cMessage*, ActionStreamHierarchy*>::iterator ActionThreadsIterator;




};

#endif
