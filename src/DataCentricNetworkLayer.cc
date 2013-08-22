/**
 * @short Implementation of a simple packets forward function for IEEE 802.15.4 star network
 *  support device <-> PAN coordinator <-> device transmission
    MAC address translation will be done in MAC layer (refer to Ieee802154Mac::handleUpperMsg())
 * @author Feng Chen
*/

#include <stdio.h>
#include "DataCentricNetworkLayer.h"
#include "Ieee802Ctrl_m.h"
#include "Ieee802154Frame_m.h"
#include "Ieee802154NetworkCtrlInfo_m.h"

#include "SpecialDebug.h"

//#define OLDFRAMEWORK
//#define STATIC_MEMORY


//#include "InterfaceTableAccess.h"
//#include "MACAddress.h"
//#include "Ieee802Ctrl_m.h"
//#include "Ieee802154Phy.h"
//#include "csma802154.h"



//#undef EV
//#define EV (ev.isDisabled()||!m_debug) ? COUT : ev ==> EV is now part of <omnetpp.h>

Define_Module( DataCentricNetworkLayer );

/////////////////////////////// PUBLIC ///////////////////////////////////////

// Framework values mostly extern
static unsigned char forwardingRole[14] = {14, 4, 4, 8, 2, 3, 0, 0, 0, 0, 9, 4, 6, 0};
unsigned char nodeConstraint;
NEIGHBOUR_ADDR thisAddress;
unsigned int moduleIndex;

extern NEIGHBOUR_ADDR excludedInterface; // used as the incoming i/f for debug

#ifndef OLDFRAMEWORK
extern int stateCount;
#endif

extern char queue[100];


double countIFLqiValuesRecorded;
unsigned int routingDelayCount;
char testSeqNo;

//static std::ofstream myfile;


bool DataCentricNetworkLayer::justStartedInitialising = true;
static int currentModuleId;

/*
static char messageName[9][24] =
{
"ADVERT                 ",
"INTEREST               ",
"REINFORCE              ",
"DATA                   ",
"NEIGHBOR_BCAST         ",
"NEIGHBOR_UCAST         ",
"REINFORCE_INTEREST     ",
"COLLABORATION          ",
"REINFORCE_COLLABORATION"
};
*/




#ifndef OLDFRAMEWORK
static void cb_send_message(NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime, uint64_t ID);
static void cb_handle_application_data(unsigned char* _msg, double _creationTime, uint64_t ID);
#else
static void cb_send_message(NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime);
static void cb_handle_application_data(unsigned char* _msg, unsigned int len, double _creationTime);
#endif

static void setTimer(TIME_TYPE timeout, void* relevantObject, void timeout_callback(void* relevantObject));
static void cb_bcast_message(unsigned char* _msg);
//static void write_one_connection(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if);
static void cb_recordNeighbourLqi(Interface* i, State* s);
//static void cb_printNeighbour(Interface* i, State* s);




//static void write_one_gradient(KDGradientNode* g, unsigned char* _name);


//============================= LIFECYCLE ===================================
/**
 * Initialization routine
 */
void DataCentricNetworkLayer::initialize(int aStage)
{
    cSimpleModule::initialize(aStage); //DO NOT DELETE!!

    SetCurrentModuleInCLanguageFramework();
    if (0 == aStage)
    {
        numSent = 0;

        //mpStartMessage = new cMessage("StartMessage");
        mpUpDownMessage = new cMessage("UpDownMessage");
        mMessageForTesting_1 = new cMessage("mMessageForTesting_1");

        //new cMessage()
        // WirelessMacBase stuff...
        mUpperLayerIn  = findGate("upperLayerIn");
        mUpperLayerOut = findGate("upperLayerOut");
        mLowerLayerIn  = findGate("lowerLayerIn");
        mLowerLayerOut = findGate("lowerLayerOut");
        controlPackets.setName("ControlPackets");
        neighbourLqis.setName("neighbourLqis");
        RangeLqis.setName("RangeLqis");
        TotalInterestArrivalsVector.setName("TotalInterestArrivals");
        InterestInterArrivalTimesVector.setName("InterestInterArrivalTimesVector");
        InterestInterDepartureTimesVector.setName("InterestInterDepartureTimesVector");
        AdvertInterDepartureTimesVector.setName("AdvertInterDepartureTimesVector");
        StabilityVector.setName("StabilityVector");

        ostringstream os;
        os << "seenLqisInTime_" << this->getIndex();
        string vectorName = os.str();
        seenLqisInTime.setName(vectorName.c_str());


        mTotalInterestArrivals = 0.0;

        for ( unsigned int i = 10; i < 270; i += 10 )
        {
            mNeighboursInLqiRange[i] = 0;
        }

        m_moduleName    = getParentModule()->getFullName();
        mpNb = NotificationBoardAccess().get();
        mpNb->subscribe(this, NF_LINK_BREAK);

        m_debug                     = par("debug");
        isPANCoor                   = par("isPANCoor");

        mMeanDownTime = par("meanDownTime");
        if ( mMeanDownTime == 0.2 )
        {
            mMeanDownTime = 0.2;

        }
        mMeanDownTimeInterval = par("meanDownTimeInterval");

        mMeanDownTimeSeconds = mMeanDownTimeInterval * mMeanDownTime;
        mMeanUpTimeSeconds = mMeanDownTimeInterval * (1.0-mMeanDownTime);




        numForward      = 0;
        mInterestFirstArrivalTime_TESTINGONLY = SIMTIME_ZERO;
        mLengthLatestInterestArrivalPeriod_TESTINGONLY = SIMTIME_ZERO;
        mLastInterestArrivalTime = SIMTIME_ZERO;
        mLastInterestDepartureTime = SIMTIME_ZERO;
        mLastAdvertDepartureTime = SIMTIME_ZERO;

        routingDelayCount = 0;
        testSeqNo = 0;


        // ORIGINAL DATA CENTRIC STUFF

#ifdef STATIC_MEMORY
        memset(moduleRD.context_array_ptr, 0, sizeof(moduleRD.context_array_ptr));
        memset(moduleRD.copy_packet_ptr, 0, sizeof(moduleRD.copy_packet_ptr));
        memset(moduleRD.deletion_packet_ptr, 0, sizeof(moduleRD.deletion_packet_ptr));
        memset(moduleRD.gradient_ptrs_array, 0, sizeof(moduleRD.gradient_ptrs_array));
        memset(moduleRD.interface_list_ptr_array, 0, sizeof(moduleRD.interface_list_ptr_array));
        memset(moduleRD.interface_node_ptrs_array, 0, sizeof(moduleRD.interface_node_ptrs_array));
        memset(moduleRD.interface_ptrs_array, 0, sizeof(moduleRD.interface_ptrs_array));
        memset(moduleRD.packet_queue_ptr_array, 0, sizeof(moduleRD.packet_queue_ptr_array));
        memset(moduleRD.state_array_ptr, 0, sizeof(moduleRD.state_array_ptr));
        memset(moduleRD.trie_array_ptr, 0, sizeof(moduleRD.trie_array_ptr));
#endif

        moduleRD.grTree = NULL;
        moduleRD.pktQ = NULL;
        moduleRD.interfaceTree = NULL;
        moduleRD.stateTree = NULL;
        moduleRD.kdRoot = NULL;
        moduleRD.role[0] = NULL;
        moduleRD.role[1] = NULL;
        moduleRD.role[2] = NULL;
        moduleRD.role[3] = NULL;
        moduleRD.top_context = trie_new();
        moduleRD.top_state = trie_new();
        moduleRD.pkts_received = 0;
        moduleRD.pkts_ignored = 0;




/*
#define MAX_COPY_PACKETS 6
struct new_packet copy_packet[MAX_COPY_PACKETS];
struct new_packet* copy_packet_ptr[MAX_COPY_PACKETS];

#define MAX_INTERFACES 12
struct InterfaceNode interface_node_array[MAX_INTERFACES];
struct InterfaceNode* interface_node_ptrs_array[MAX_INTERFACES];
struct Interface interface_array[MAX_INTERFACES];
struct Interface* interface_ptrs_array[MAX_INTERFACES];

#define MAX_GRADIENTS 15
struct KDGradientNode gradients_array[MAX_GRADIENTS];
struct KDGradientNode* gradient_ptrs_array[MAX_GRADIENTS];

#define MAX_INTERFACE_LIST 20
struct InterfaceList interface_list_array[MAX_INTERFACE_LIST];
struct InterfaceList* interface_list_ptr_array[MAX_INTERFACE_LIST];

struct PacketQueue packet_queue_array[MAX_COPY_PACKETS];
struct PacketQueue* packet_queue_ptr_array[MAX_COPY_PACKETS];

struct QueueDeletion deletion_packet[MAX_COPY_PACKETS];
struct QueueDeletion* deletion_packet_ptr[MAX_COPY_PACKETS];

#define MAX_STATES 7
struct State state_array[MAX_STATES];
struct State* state_array_ptr[MAX_STATES];

#define MAX_CONTEXTS 9
struct context context_array[MAX_CONTEXTS];
struct context* context_array_ptr[MAX_CONTEXTS];

#define MAX_TRIES 16
struct trie trie_array[MAX_TRIES];
struct trie* trie_array_ptr[MAX_TRIES];
*/














        rd = &(moduleRD);
        rd->role[0] = (unsigned char*)malloc(forwardingRole[0]);
        memcpy(rd->role[0], forwardingRole, forwardingRole[0]);
        nodeConstraintValue         = par("nodeConstraint");
        mStability         = par("Stability");
        mRoutingDelay         = par("routingDelay");
        setMessageCallBack(cb_send_message);
        setBroadcastCallBack(cb_bcast_message);
        setApplicationCallBack(cb_handle_application_data);
        setTimerCallBack(setTimer);

        mRegularCheckMessage = new cMessage("mRegularCheckMessage");
        //scheduleAt(simTime()+2.0, mRegularCheckMessage);
        //scheduleAt(simTime() + StartTime(), mpStartMessage);



    }

    if (1 == aStage)
    {
        testValue = 0;
        mBatteryModule = (BasicBattery*)this->getParentModule()->getSubmodule("battery");
        cModule* nicModule = this->getParentModule()->getSubmodule("nic");
        cModule* macModule = check_and_cast<cModule*>(nicModule->getSubmodule("mac"));
        mPhyModule = check_and_cast<Ieee802154Phy*>(nicModule->getSubmodule("phy"));
        mQueueModule = check_and_cast<DropTailQueue*>(nicModule->getSubmodule("ifq"));
        mPhyModule->disableModule();
        cSimulation* sim =  cSimulation::getActiveSimulation();
        mNetMan = check_and_cast<DataCentricNetworkMan*>(sim->getModuleByPath("DataCentricNet.dataCentricNetworkMan"));


        string tempAddressString = macModule->par("address");
        MACAddress addrObj(tempAddressString.c_str());
        //MACAddress addrObj(mTheAddressString.c_str());
        //mTheAddressString = tempAddressString;
        mTheAddressString = addrObj.str();
        mAddress = addrObj.getInt();

        mNetMan->mNetModules[mAddress] = this;














        //mAddress &= 0x0000FFFFFFFFFFFF;


        WriteModuleListFile();
        //int thePoisson = poisson(mMeanUpTimeSeconds);
        // SPECIAL TEMPORARY CODE
        // ======================
        //int thePoisson = mMeanUpTimeSeconds;
        //COUT << "POISSON " << this->getParentModule()->getFullName() << ": " << thePoisson << endl;



        // For the moment comment out to disable stability feature
        //scheduleAt(simTime() + 1.0, mpUpDownMessage);









        // OLD STUFF
        //if ( !strcmp(this->getParentModule()->getFullName(), "host[52]") )
        //{
        //    scheduleAt(simTime() + 2.0, mpUpDownMessage);
        //}
        //else
        //{
        //    scheduleAt(simTime() + thePoisson, mpUpDownMessage);
            //scheduleAt(simTime() + poisson(mMeanUpTimeSeconds), mpUpDownMessage);
        //}

        //if ( !strcmp(this->getParentModule()->getFullName(), "host[5]") )
        //{
            //scheduleAt(simTime() + 5.0, mMessageForTesting_1);
        //}









    }


}

void DataCentricNetworkLayer::finish()
{
    SetCurrentModuleInCLanguageFramework();

    double capacity_mW_sec = mBatteryModule->par("capacity").doubleValue() * 60 * 60 * mBatteryModule->par("voltage").doubleValue();
    double residualCapacity_mW_sec = mBatteryModule->GetEnergy();
    double usage_mW_sec = capacity_mW_sec - residualCapacity_mW_sec;

    recordScalar("num of pkts forwarded", numForward);

    if ( rd->interfaceTree )
    {
        unsigned int totNeighbors = CountInterfaceNodes(rd->interfaceTree);
        recordScalar("NumberOfNeighbours", totNeighbors);
        double totalLqi =  TotalNeighborLqi(rd->interfaceTree);

        unsigned int maxlqi = (unsigned int)rd->interfaceTree->i->lqi;
        unsigned int minlqi = (unsigned int)rd->interfaceTree->i->lqi;
        MinMaxNeighborLqi(rd->interfaceTree, &maxlqi, &minlqi);

#ifndef OLDFRAMEWORK
        stateCount = 0; // no semantic error, but don't we need an extern
        traverse(rd->top_state, queue, 0, countState);
        recordScalar("NumberOfStates", stateCount);
#endif

        double averageLqi = (totalLqi / totNeighbors);
        recordScalar("TotalNeighborLqi", totalLqi);
        recordScalar("AverageNeighborLqi", averageLqi);
        recordScalar("MaxNeighborLqi", maxlqi);
        recordScalar("MinNeighborLqi", minlqi);
        recordScalar("TEST_LengthLatestInterestArrivalPeriod", mLengthLatestInterestArrivalPeriod_TESTINGONLY);

        countIFLqiValuesRecorded = 0;
        TraversInterfaceNodes(rd->interfaceTree, 0, cb_recordNeighbourLqi);

        for ( unsigned int i = 10; i < 270; i += 10 )
        {
            double lqi = (double)i;
            double numInRange = (double)mNeighboursInLqiRange[i];
            RangeLqis.recordWithTimestamp(numInRange, lqi);
        }
    }
    else
    {
        recordScalar("NumberOfNeighbours", 0);
        recordScalar("TotalNeighborLqi", 0);
        recordScalar("AverageNeighborLqi", 0);
        recordScalar("MaxNeighborLqi", 0);
        recordScalar("MinNeighborLqi", 0);
        recordScalar("TEST_LengthLatestInterestArrivalPeriod", mLengthLatestInterestArrivalPeriod_TESTINGONLY);
        for ( unsigned int i = 10; i < 270; i += 10 )
        {
            double lqi = (double)i;
            RangeLqis.recordWithTimestamp(0, lqi);
        }

    }


    double pkts_ignored = (double)rd->pkts_ignored;
    double pkts_received = (double)rd->pkts_received;
    double ignoreRatio = pkts_ignored / pkts_received;
    recordScalar("ignoreRatio", ignoreRatio);

    recordScalar("numSeenLqis", seenLqis.getCount());
    recordScalar("meanSeenLqis", seenLqis.getMean());
    recordScalar("stdevSeenLqis", seenLqis.getStddev());

    cancelAndDelete(mpUpDownMessage);
    cancelAndDelete(mMessageForTesting_1);
    cancelAndDelete(mRegularCheckMessage);

    for (TimeoutMessages2Iterator i = mTimeoutMessages2.begin();
            i != mTimeoutMessages2.end(); ++i)
    {
        cancelAndDelete((*i));
    }



}


/*
static void cb_printNeighbour(Interface* i, State* s)
{
    COUT << "Neighbour: " << hex<< i->iName << "\n";


}
*/






static void cb_recordNeighbourLqi(Interface* i, State* s)
{
    DataCentricNetworkLayer* currentModule = check_and_cast<DataCentricNetworkLayer *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));
    unsigned int lqi = (unsigned int)i->lqi;

    //currentModule->neighbourLqis.record(lqi);
    currentModule->neighbourLqis.recordWithTimestamp(countIFLqiValuesRecorded, lqi);
    countIFLqiValuesRecorded += 1;

    //unsigned int numOfTens = lqi / 10;
    //unsigned int range = (numOfTens * 10) + 10;
    //currentModule->mNeighboursInLqiRange[range]++;

    for ( unsigned int i = 10; i < 270; i += 10 )
    {
        if ( lqi <= i )
        {
            currentModule->mNeighboursInLqiRange[i]++;
        }
    }


}



void DataCentricNetworkLayer::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method("receiveChangeNotification(int category, const cPolymorphic *details)");
    SetCurrentModuleInCLanguageFramework();

    std::string fName = this->getParentModule()->getFullName();
    double currentTime = simTime().dbl();
    NullStream() << "Current Time: " << currentTime << "\n";

    Ieee802154Frame * f;
    switch (category)
    {
    case NF_LINK_BREAK:
        f = check_and_cast<Ieee802154Frame *>(details);
        if ( f )
        {
            MACAddress destMac = f->getDstAddr();
            destMac.convert48();
            NEIGHBOUR_ADDR destIF = destMac.getInt();
            DataCentricAppPkt* appPkt = check_and_cast<DataCentricAppPkt *>(f->decapsulate());
            uint64_t moduleId64 = (int)appPkt->par("sourceId");
            uint64_t msgId64 = (int)appPkt->par("msgId");
            uint64_t ID = (moduleId64 << 32) | msgId64;

            unsigned char* pkt = (unsigned char*)malloc(appPkt->getPktData().size());
            std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), pkt);
            switch ( *pkt )
            {
                case DATA:
                    switch ( (pkt[2] & MSB2) )
                    {
                        case PUBLICATION:
#ifndef OLDFRAMEWORK
                            advert_breakage_just_ocurred(pkt, destIF, appPkt->getCreationTime().dbl(), ID);
#endif
                            break;
                        case RECORD:
#ifndef OLDFRAMEWORK
                            interest_breakage_just_ocurred(pkt, destIF, appPkt->getCreationTime().dbl(), ID);
#else
                            interest_breakage_just_ocurred(pkt, destIF, appPkt->getCreationTime().dbl());
#endif
                            break;
                    }
                    break;
            }
            free(pkt);
        }
        break;
    case NF_RADIOSTATE_CHANGED:
        break;
    default:
        break;
    }

}

// OLD COMMENT FROM ABOVE
//case REINFORCE:
//case REINFORCE_INTEREST:
    //int x = 0;
    //InterfaceDown(pkt, destIF);

    /*
     * It may be important to schedule a new event here
     * instead of diving in asynchronously
     *
     * Or may be move shared data like output_pkt into
     * rd an set rd so no conflict occurrs
     *
     * Also may be important to write this aspect into
     * the requirements of the framework
     */



void DataCentricNetworkLayer::sendDownTheNIC()
{
    Enter_Method("sendDownTheNIC()");

    double randomStability = uniform(1,255);
    if ( randomStability <= mStability  )
    {
        if ( mPhyModule->isEnabled() )
        {
            //COUT << "MODULE GOING DOWN: " << fName << endl;
            //TraversInterfaceNodes(rd->interfaceTree, 0, cb_printNeighbour);

            mPhyModule->disableModule();
            mNetMan->changeInModulesDown(1.0);

            freeKDGradientNode(moduleRD.grTree);
            freeInterfaceNode(moduleRD.interfaceTree);
            // also free packet queue
            trie_free(moduleRD.top_context);
            trie_free(moduleRD.top_state);
            moduleRD.grTree = NULL;
            moduleRD.pktQ = NULL;
            moduleRD.interfaceTree = NULL;
            moduleRD.top_context = trie_new();
            moduleRD.top_state = trie_new();
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

}

void DataCentricNetworkLayer::handleMessage(cMessage* msg)
{
    SetCurrentModuleInCLanguageFramework();

    std::string fName = this->getParentModule()->getFullName();
    double currentTime = simTime().dbl();

    NullStream() << "Current Time: " << currentTime << "\n";

    if ( msg->isName("FrameworkTimeout") )
    {

        void* relevantObject = msg->par("relevantObject").pointerValue();
        void (*timerCB) (void*) = (void (*) (void*))msg->par("callBack").pointerValue();

        // Remove the timer before executing the call back in case
        // the call back tries to add another one.
        // if we remove after then the call back may reschedule this
        // message and we will try to delete a currently scheduled message
        TimeoutMessages2Iterator i2 = mTimeoutMessages2.find(msg);
        if ( i2 != mTimeoutMessages2.end() )
        {
            mTimeoutMessages2.erase(msg);
        }

        delete msg;

        timerCB(relevantObject);

        return;



        /*
        TimeoutMessagesIterator i = mTimeoutMessages.find(relevantObject);
        if ( i != mTimeoutMessages.end() )
        {
            mTimeoutMessages.erase(i);
            delete msg;
        }

        return;
        */

    }


    if ( msg == mMessageForTesting_1 )
    {
        //TraversInterfaceNodes(rd->interfaceTree, 0, cb_printNeighbour);
        //return;

        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char temp[30];
        unsigned char x[20];
        string i = "\x2\x2\x0";
        int datalen = strlen(i.c_str());
        memcpy(x, i.c_str(), datalen);
        x[datalen] = DOT;
        getShortestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        weAreSinkFor(x, ++testSeqNo);

        // mMessageForTesting_1 not used any way
#ifndef OLDFRAMEWORK
        UcastAllBestGradients(rd->top_state, 0);
#endif

        scheduleAt(simTime() + 10.0, mMessageForTesting_1);

        return;
    }


    if (msg == mpUpDownMessage )
    {
        //if ( uniform(0,1) <= mStability  )
        //{
            /*
            if ( mPhyModule->isEnabled() )
            {
                COUT << "MODULE GOING DOWN: " << fName << endl;
                //TraversInterfaceNodes(rd->interfaceTree, 0, cb_printNeighbour);

                mPhyModule->disableModule();
                mNetMan->changeInModulesDown(1.0);

                freeKDGradientNode(moduleRD.grTree);
                freeInterfaceNode(moduleRD.interfaceTree);
                // also free packet queue
                trie_free(moduleRD.top_context);
                trie_free(moduleRD.top_state);
                moduleRD.grTree = NULL;
                moduleRD.pktQ = NULL;
                moduleRD.interfaceTree = NULL;
                moduleRD.top_context = trie_new();
                moduleRD.top_state = trie_new();
                //mQueueModule->dropAll();
                mQueueModule->clear();

                std::string s;
                std::ostringstream ss;
                ss.clear();
                ss.str(s);
                ss << ".\\" << std::hex << std::uppercase << thisAddress << "Connections.txt";
                std::remove(ss.str().c_str());
                //scheduleAt(simTime() + 1.0, mpUpDownMessage);
            }
            else
            */

            if ( !mPhyModule->isEnabled() )
            {
                COUT << "MODULE COMING UP: " << fName << "\n";
                //scheduleAt(simTime() + 0.5, mMessageForTesting_1);

                mPhyModule->enableModule();
                StabilityVector.record(0.0);
                mNetMan->changeInModulesDown(-1.0);

                mQueueModule->requestPacket(); // reprime the previously cleared nic queue
                StartUp();
            }
        //}

        /*
        if ( mPhyModule->isEnabled() )
        {
            StabilityVector.record(0.0);
        }
        else
        {
            StabilityVector.record(1.0);
        }
        */


        //scheduleAt(simTime() + 1.0, mpUpDownMessage);
        // Single fixed message, do not delete,
        // module taken down on occasion then this msg rescheduled by sendDownTheNIC()
        // module brought back up again on delivery in this section
        return;
    }

    if ( msg == mRegularCheckMessage )
    {
        /*
        regular_checks();

        string s;
        ostringstream ss;
        ss.clear();
        ss.str(s);
        ss << ".\\" << hex << uppercase << thisAddress << "Connections.txt";


        scheduleAt(simTime()+2.0, mRegularCheckMessage); // Ownership PASSED on
        return;
        */
    }

    DataCentricAppPkt* appPkt = check_and_cast<DataCentricAppPkt *>(msg);


    if ( !strcmp(this->getParentModule()->getFullName(), "host[5]") )
    {
        int x = 5;
        NullStream() << "x: " << x << "\n";

    }


    if (msg->getArrivalGateId() == mUpperLayerIn)
    {
        handleUpperLayerMessage(appPkt); // Ownership PASSED on
    }

    if (msg->getArrivalGateId() == mLowerLayerIn)
    {
        handleLowerLayerMessage(appPkt); // Ownership PASSED on
    }

}


void DataCentricNetworkLayer::SetCurrentModuleInCLanguageFramework()
{
    // Framework is non-object-oriented C Code.  In the Omnet++
    // simulation we must repeatedly reset the C Code to reflect the module
    // currently being simulated.
    // In the real world a copy of the C Code will be deployed in each device

    //nodeConstraint = nodeConstraintValue;
    //nodeConstraint = (unsigned int)(255.0 * mStability);
    nodeConstraint = (unsigned int)mStability;
    currentModuleId = this->getId();
    moduleIndex = this->getParentModule()->getIndex();
    thisAddress = mAddress;
    rd = &(moduleRD);
}


void DataCentricNetworkLayer::handleLowerLayerMessage(DataCentricAppPkt* appPkt)
{
    //Ieee802Ctrl *incomingControlInfo = check_and_cast<Ieee802Ctrl*>(appPkt->getControlInfo());
    Ieee802154NetworkCtrlInfo *incomingControlInfo = check_and_cast<Ieee802154NetworkCtrlInfo*>(appPkt->getControlInfo());
    currentPktCreationTime = appPkt->getCreationTime();

    //incomingControlInfo->get
    uint64 previousAddress = incomingControlInfo->getNetwAddr();
    MACAddress prevAddr(previousAddress);
    double powerRec = incomingControlInfo->getPowerRec();
    double snr = incomingControlInfo->getSnr();
    unsigned int rssi = incomingControlInfo->getRssi();
    unsigned char lqi = incomingControlInfo->getLqi();

    string fn(this->getParentModule()->getFullName());
    EV << "=====================================================" << endl;
    EV << "To: " << fn << endl;
    string foundNode;
    if ( this->FindNode(prevAddr.str(), foundNode))
    {
        EV << "From: " << foundNode << endl;
    }
    else
    {
        EV << "From: " << prevAddr.str() << endl;
    }
    EV << "      -  Power: " << powerRec << endl;
    EV << "      -  Snr:   " << snr << endl;
    EV << "      -  RSSI:  " << rssi << endl;
    EV << "      -  LQI:   " << (unsigned int)lqi << endl;
    EV << "=====================================================" << endl;

    //uint64 previousAddress = incomingControlInfo->getSrc().getInt();
    unsigned char* pkt = (unsigned char*)malloc(appPkt->getPktData().size());
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), pkt);

    switch ( pkt[0] )
    {
        case INTEREST:
            if ( SIMTIME_ZERO == mLastInterestArrivalTime )
            {
                mLastInterestArrivalTime = simTime();
            }
            else
            {
                InterestInterArrivalTimesVector.record(simTime() - mLastInterestArrivalTime);
                mLastInterestArrivalTime = simTime();
            }

            mTotalInterestArrivals++;
            TotalInterestArrivalsVector.record(mTotalInterestArrivals);
            if ( SIMTIME_ZERO == mInterestFirstArrivalTime_TESTINGONLY )
            {
                mInterestFirstArrivalTime_TESTINGONLY = simTime();
            }
            else
            {
                if ( (simTime()-mInterestFirstArrivalTime_TESTINGONLY) > 1.0 )//Check this
                {
                    mInterestFirstArrivalTime_TESTINGONLY = simTime();
                    mLengthLatestInterestArrivalPeriod_TESTINGONLY = SIMTIME_ZERO;
                    mTotalInterestArrivals = 0.0;
                }
                else
                {
                    mLengthLatestInterestArrivalPeriod_TESTINGONLY =
                            (simTime() - mInterestFirstArrivalTime_TESTINGONLY);
                }
            }
            break;
        default:
            break;
    }
    double creationTime = appPkt->getCreationTime().dbl();

    uint64_t ID = 0; // only ucasts have these parameters and only data packets use it
    if ( appPkt->hasPar("sourceId") && appPkt->hasPar("msgId") )
    {
        uint64_t moduleId64 = (int)appPkt->par("sourceId");
        uint64_t msgId64 = (int)appPkt->par("msgId");
        ID = (moduleId64 << 32) | msgId64;
    }

    bool success = mNetMan->lqiVec.record((double)lqi);
    seenLqis.collect((double)lqi);
    seenLqisInTime.record((double)lqi);

#ifndef OLDFRAMEWORK
    handle_message(pkt, previousAddress, lqi, creationTime, ID);
#else
    handle_message(pkt, previousAddress, lqi, creationTime);
#endif
    free(pkt);
    delete appPkt;
}


void DataCentricNetworkLayer::handleUpperLayerMessage(DataCentricAppPkt* appPkt)
{
    int inde;
    switch ( appPkt->getKind() )
    {
        case DATA_PACKET:
            appPkt->addPar("sourceId") = getId();
            appPkt->addPar("msgId") = numSent++;
            currentPktCreationTime = simTime();
            COUT << "\n" << "DATA SENT ORIG CREATE TIME:     " << currentPktCreationTime << "\n";
            SendDataWithLongestContext(appPkt); // ownership NOT passed on
            break;
        case STARTUP_MESSAGE:
            if ( this->getParentModule()->getIndex() == 3 )
                cout << "Ind 3" << endl;
            StartUpModule();
            break;
        case CONTEXT_MESSAGE:
            SetContext(appPkt); // ownership not passed on
            break;
        case SOURCE_MESSAGE:
            if ( this->getParentModule()->getIndex() == 3 )
                cout << "Ind 3" << endl;
            //inde = this->getParentModule()->getIndex();
            SetSourceWithLongestContext(appPkt); // ownership NOT passed on
            break;
        case SINK_MESSAGE:
            SetSinkAsIs(appPkt); // ownership NOT passed on
            //SetSinkWithShortestContext(appPkt); // ownership NOT passed on
            break;
        case COLLABORATOR_INITITOR_MESSAGE:
            SetCollaboratorInitiatorWithShortestContext(appPkt); // ownership NOT passed on
            break;
        case COLLABORATOR_MESSAGE:
            SetCollaboratorWithShortestContext(appPkt); // ownership NOT passed on
            break;
        default:
            break;
    }

    delete appPkt;
}


void DataCentricNetworkLayer::SendDataAsIs(DataCentricAppPkt* appPkt)
{
    unsigned char* data = (unsigned char*)malloc(appPkt->getPktData().size());
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), data);
    uint64_t moduleId64 = (int)appPkt->par("sourceId");
    uint64_t msgId64 = (int)appPkt->par("msgId");
    uint64_t ID = (moduleId64 << 32) | msgId64;

#ifndef OLDFRAMEWORK
    send_data(appPkt->getPktData().size(), data, simTime().dbl(), ID);
#else
    send_data(appPkt->getPktData().size(), data, simTime().dbl());
#endif
    free(data);
}



void DataCentricNetworkLayer::SendDataWithLongestContext(DataCentricAppPkt* appPkt)
{
    // MOVE THIS BIT INTO FRAMEWORK
    unsigned char temp[30];
    unsigned char x[20];
    unsigned char* index = x;
    int stateLen = appPkt->getPktData().size();
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), index);
    index += stateLen++;
    *index = DOT;
    //x[stateLen] = DOT;
    //getLongestContextTrie(rd->top_context, temp, temp, &(x[stateLen+1]));
    getLongestContextTrie(rd->top_context, temp, temp, ++index);
    stateLen += strlen((const char*)index);
    unsigned char* data = (unsigned char*)malloc(stateLen);
    memcpy(data, x, stateLen);
    uint64_t moduleId64 = (int)appPkt->par("sourceId");
    uint64_t msgId64 = (int)appPkt->par("msgId");
    uint64_t ID = (moduleId64 << 32) | msgId64;

#ifndef OLDFRAMEWORK
    send_data(stateLen, data, simTime().dbl(), ID);
#else
    send_data(stateLen, data, simTime().dbl());
#endif
    free(data);
}




void DataCentricNetworkLayer::StartUpModule()
{
    mPhyModule->enableModule();
    StartUp();
}





void DataCentricNetworkLayer::SetContext(DataCentricAppPkt* appPkt)
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


void DataCentricNetworkLayer::SetSourceWithLongestContext(DataCentricAppPkt* appPkt)
{
    string sourceData;
    sourceData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sourceData.begin());
    //int size = appPkt->getPktData().size();
    //size = sourceData.size();
    std::vector<std::string> sourcesData = cStringTokenizer(sourceData.c_str(), "\xFE").asVector();
    unsigned char temp[30];
    for (std::vector<std::string>::iterator i = sourcesData.begin();
            i != sourcesData.end(); ++i)
    {
        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = DOT;
        getLongestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        weAreSourceFor(x, 0);
    }

}



void DataCentricNetworkLayer::SetSinkWithShortestContext(DataCentricAppPkt* appPkt)
{
    string sinkData;
    sinkData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());
    //int size = appPkt->getPktData().size();
    //size = sinkData.size();
    std::vector<std::string> sinksData = cStringTokenizer(sinkData.c_str(), "\xFE").asVector();
    unsigned char temp[30];
    for (std::vector<std::string>::iterator i = sinksData.begin();
            i != sinksData.end(); ++i)
    {
        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = DOT;
        getShortestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        weAreSinkFor(x, 0);
    }

}



void DataCentricNetworkLayer::SetSinkAsIs(DataCentricAppPkt* appPkt)
{
    string sinkData;
    sinkData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), sinkData.begin());
    //int size = appPkt->getPktData().size();
    //size = sinkData.size();
    std::vector<std::string> sinksData = cStringTokenizer(sinkData.c_str(), "\xFE").asVector();
    unsigned char temp[30];
    for (std::vector<std::string>::iterator i = sinksData.begin();
            i != sinksData.end(); ++i)
    {
        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = 0;
        weAreSinkFor(x, 0);
    }

}


void DataCentricNetworkLayer::SetCollaboratorInitiatorWithShortestContext(DataCentricAppPkt* appPkt)
{
    string collabData;
    collabData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), collabData.begin());
    //int size = appPkt->getPktData().size();
    //size = collabData.size();
    std::vector<std::string> collabsData = cStringTokenizer(collabData.c_str(), "\xFE").asVector();
    unsigned char temp[30];
    for (std::vector<std::string>::iterator i = collabsData.begin();
            i != collabsData.end(); ++i)
    {
        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = DOT;
        getShortestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        weAreCollaboratorInitiatorFor(x, 0);
    }

}






void DataCentricNetworkLayer::SetCollaboratorWithShortestContext(DataCentricAppPkt* appPkt)
{
    string collabData;
    collabData.resize(appPkt->getPktData().size(), 0);
    std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), collabData.begin());
    //int size = appPkt->getPktData().size();
    //size = collabData.size();
    std::vector<std::string> collabsData = cStringTokenizer(collabData.c_str(), "\xFE").asVector();
    unsigned char temp[30];
    for (std::vector<std::string>::iterator i = collabsData.begin();
            i != collabsData.end(); ++i)
    {
        // MOVE THIS BIT INTO FRAMEWORK
        unsigned char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = DOT;
        getShortestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        weAreCollaboratorFor(x, 0);
    }

}





double DataCentricNetworkLayer::StartTime()
{
    return par("startTime").doubleValue();
}


void DataCentricNetworkLayer::WriteModuleListFile()
{
    if ( justStartedInitialising )
    {
        std::remove("C:\\GradientData\\ModuleList.txt");
        justStartedInitialising = false;
    }
    std::ofstream myfile;
    myfile.open ("C:\\GradientData\\ModuleList.txt", std::ios::app);
    myfile << "NODE" << std::endl;
    //std::string fp = getFullPath();
    //std::string fp = this->mAddress;
    myfile << hex << uppercase << this->mAddress << std::endl;
    cModule* pm = this->getParentModule();

    char image[100];
    char* image_i = image;
    std::sprintf(image_i, "%s", pm->getDisplayString().getTagArg("i", 0));
    myfile << "IMAGE" << std::endl;
    myfile << IMAGEPATH << image_i << ".png" << std::endl;

    char x[10];
    char y[10];
    char* x_i = x;
    char* y_i = y;
    std::sprintf(x_i, "%s", pm->getDisplayString().getTagArg("p", 0));
    std::sprintf(y_i, "%s", pm->getDisplayString().getTagArg("p", 1));
    myfile << "POSITION" << std::endl;
    myfile << x_i << std::endl;
    myfile << y_i << std::endl;

    //int n = gateSize("gate");
    //for ( int i = 0; i < n; i++ )
    //{
    //    cGate* g = gate("gate$o",i);
    //    cGate*  ng = g->getNextGate();
    //    cModule* connectedMod = ng->getOwnerModule();
    //    std::string connectedModFullPath = connectedMod->getFullPath();
    //    myfile << "CONNECTION" << std::endl;
    //    myfile << fp << std::endl;
    //    myfile << connectedModFullPath << std::endl;
    //}

    myfile.close();

}


bool DataCentricNetworkLayer::FindNode(const string& addressToFind, string& matchingNodeName)
{
    //ev << "Searching for node with:    " << addressToFind << endl;
    cModule* network = this->getParentModule()->getParentModule();
    for (cSubModIterator iter(*network); !iter.end(); iter++)
    {
        string nodeBeingChecked(iter()->getFullName());
        //ev << "Checking:    " << nodeBeingChecked << endl;
        cModule* mac = iter()->getSubmodule("nic") ? iter()->getSubmodule("nic")->getSubmodule("mac") : 0;
        if ( mac )
        {
            string addressFound = mac->par("address");
            if ( addressFound == addressToFind )
            {
                //ev << nodeBeingChecked << " has IP Address: " << addressFound << endl;
                matchingNodeName = nodeBeingChecked;
                return true;
            }
        }
    }
    return false;
}



bool DataCentricNetworkLayer::duplicate(int moduleId, int msgId) //, DataCentricAppPkt* pk)
{
    SourceSequence::iterator it = sourceSequence.find(moduleId);
    if (it != sourceSequence.end())
    {
        if (it->second >= msgId)
        {
            //delete pk;
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

}



/*
static void write_one_gradient(KDGradientNode* g, unsigned char* _name)
{
    myfile << "CONNECTION" << std::endl;
    myfile << hex << uppercase << thisAddress << std::endl;
    for ( int i = 0; _name[i] != 0; i++ )
    {
        //unsigned char ch = (unsigned char)_name[i];
        myfile.width(2);
        myfile.fill('0');
        myfile << hex << uppercase << (unsigned int)_name[i];
    }
    myfile << std::endl;
    myfile << dec << g->key1->action << std::endl;
    myfile << hex << uppercase << g->key2->iName << std::endl;
    myfile << boolalpha << (bool)g->key2->up << std::endl;
    myfile << dec << g->costToDeliver << std::endl;
    myfile << dec<< g->costToObtain << std::endl;
    myfile << boolalpha << (g->key1->bestGradientToDeliver == g) << std::endl;
    myfile << boolalpha <<  deliverReinforced(g) << std::endl;
    myfile << boolalpha << (g->key1->bestGradientToObtain == g) << std::endl;
    myfile << boolalpha <<  obtainReinforced(g) << std::endl;

}
*/


/*
static void write_one_connection(State* s, unsigned char* _data, NEIGHBOUR_ADDR _if)
{

    myfile << "CONNECTION" << std::endl;

    for ( int i = 0; _data[i] != 0; i++ )
    {
        myfile << hex << uppercase << _data[i];
    }
    myfile << std::endl;

    myfile << hex << uppercase << thisAddress << std::endl;
    myfile << "best_deliver_to" << std::endl;
    if (s->bestGradientToDeliver)
    {
        myfile << hex << uppercase << s->bestGradientToDeliver->key2->iName << std::endl;
        myfile << s->bestGradientToDeliver->costToDeliver << std::endl;
    }
    else
    {
        myfile << "no_best_deliver_to" << std::endl;
        myfile << "not applicable" << std::endl;
    }

    myfile << "best_obtain_from" << std::endl;
    if (s->bestGradientToObtain)
    {
        myfile << hex << uppercase << s->bestGradientToObtain->key2->iName << std::endl;
        myfile << s->bestGradientToObtain->costToObtain << std::endl;
    }
    else
    {
        myfile << "no_best_obtain_from" << std::endl;
        myfile << "not applicable" << std::endl;
    }

}
*/



// only one timeout at a time per relevantObject
// relevantObject is owned by the function caller
// further function calls for the same already scheduled object
// cause a reschedule

/*
 * With TimeoutMessages2Iterator only one timeout at a time per combination
 * of relevantObject and call back pointer
 *
 * further function calls for the same already scheduled combination
 * cause a reschedule
 */
static void setTimer(TIME_TYPE timeout, void* relevantObject, void timeout_callback(void* relevantObject))
{
    DataCentricNetworkLayer* currentModule = check_and_cast<DataCentricNetworkLayer *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));

    std::string fName = currentModule->getParentModule()->getFullName();
    double currentTime = simTime().dbl();
    if ( fName == "host[1]" )
    {
        currentTime = simTime().dbl();
    }

    NullStream() << "Current Time: " << currentTime << "\n";

    for (DataCentricNetworkLayer::TimeoutMessages2Iterator i = currentModule->mTimeoutMessages2.begin();
            i != currentModule->mTimeoutMessages2.end(); ++i)
    {
        void* _relevantObject = (*i)->par("relevantObject").pointerValue();
        void (*timerCB) (void*) = (void (*) (void*))(*i)->par("callBack").pointerValue();
        if ( _relevantObject == relevantObject
                && timerCB == timeout_callback )
        {
            currentModule->cancelEvent((*i));
#ifdef XXXXXX
            if ( timeout > 0 )
#endif
                currentModule->scheduleAt(simTime() + timeout, (*i));
            return;
        }
    }

#ifdef XXXXXX
    if ( timeout <= 0 )
        return;
#endif

    cMessage* m = new cMessage("FrameworkTimeout");
    m->addPar("relevantObject").setPointerValue(relevantObject);
    m->addPar("callBack").setPointerValue((void*)timeout_callback);
    currentModule->mTimeoutMessages2.insert(m);
    currentModule->scheduleAt(simTime() + timeout, m);

    return;



    /*
    DataCentricNetworkLayer::TimeoutMessagesIterator i = currentModule->mTimeoutMessages.find(relevantObject);
    if ( i == currentModule->mTimeoutMessages.end() )
    {
        cMessage* m = new cMessage("FrameworkTimeout");
        m->addPar("relevantObject").setPointerValue(relevantObject);
        m->addPar("callBack").setPointerValue((void*)timeout_callback);

        // We need to be careful - may have to change this
        // there may be cases in framework where more than one
        // event is for the same object (but to call back to different
        // call back functions)...
        currentModule->mTimeoutMessages[relevantObject] = m;
        currentModule->scheduleAt(simTime() + timeout, m);
    }
    else
    {
        currentModule->cancelEvent(i->second);
        currentModule->scheduleAt(simTime() + timeout, i->second);
    }
    */




}






#ifndef OLDFRAMEWORK
static void cb_send_message(NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime, uint64_t ID)
#else
static void cb_send_message(NEIGHBOUR_ADDR _interface, unsigned char* _msg, double _creationTime)
#endif
{
    /*
     * This is a generic call back for the data centric routing framework
     * It's implementation in this case is specifically tailored to Omnet++
     */
    DataCentricNetworkLayer* currentModule = check_and_cast<DataCentricNetworkLayer *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));




    //char msgname[20];
    //sprintf(msgname, "%s", messageName[*_msg]);
    //RoutingMessage *msg = new RoutingMessage(msgname);
    //msg->setKind(ROUTING_MESSAGE);
    //unsigned int messageSize = _msg[1] + 4;
    //for( unsigned i = 0; i < messageSize; i++ )
    //{
    //    msg->setData(i, _msg[i]);
    //}
    DataCentricAppPkt* appPkt;

    switch ( *_msg )
    {
        case REINFORCE:
            appPkt = new DataCentricAppPkt("Rein_DataCentricAppPkt");
            break;
        case DATA:
            appPkt = new DataCentricAppPkt("Data_DataCentricAppPkt");
            currentModule->mNetMan->mNetModules[_interface]->sendDownTheNIC();
            break;
        case REINFORCE_INTEREST:
            appPkt = new DataCentricAppPkt("ReinIN_DataCentricAppPkt");
            break;
        default:
            appPkt = new DataCentricAppPkt("DataCentricAppPkt");
            break;
    }

#ifndef OLDFRAMEWORK
    appPkt->addPar("sourceId").setLongValue((int)(ID >> 32));
    appPkt->addPar("msgId").setLongValue((int)(ID & 0xFFFFFFFF));
#endif

    switch ( *_msg )
    {
        case NEIGHBOR_BCAST:
        case NEIGHBOR_UCAST:
        case INTEREST:
        case ADVERT:
        case REINFORCE:
        case REINFORCE_INTEREST:
        case COLLABORATION:
        case REINFORCE_COLLABORATION:
            appPkt->setName("DataCentricControlPkt");
            break;
        default:
            appPkt->setName("DataCentricAppPkt");
            break;
    }


    currentModule->mNetMan->recordOnePacket(*_msg);
    /*
    switch ( *_msg )
    {
        case DATA:
            break;
        default:
            break;
    }
    */




//#define ADVERT 0
//#define INTEREST 1 // Need to look at this - it is being used by two bits of code differently!!!!!!!!!
//#define REINFORCE 2
//#define DATA 3
//#define NEIGHBOR_BCAST 4
//#define NEIGHBOR_UCAST 5
//#define REINFORCE_INTEREST 6
//#define COLLABORATION 7
//#define REINFORCE_COLLABORATION 8
//#define INTEREST_CORRECTION 9
//#define REINFORCE_INTEREST_CANCEL 10

    //appPkt->setDataName("");

    //appPkt->getPktData().insert(appPkt->getPktData().end(), _msg, _msg+(_msg[1] + 4));
    appPkt->getPktData().insert(appPkt->getPktData().end(), _msg, _msg+sizeof_existing_packet(_msg));
    unsigned long long int pktSize = sizeof_existing_packet_withoutDownIF(_msg);
    appPkt->setByteLength(pktSize);

    switch ( *_msg )
    {
        case NEIGHBOR_BCAST:
            //COUT << "Bcast size: " << pktSize;
            break;
        case NEIGHBOR_UCAST:
            COUT << "HelloReply size: " << pktSize << "\n";
            break;
        case INTEREST:
            COUT << "Interest size: " << pktSize << "\n";
            break;
        case ADVERT:
            //COUT << "Advert size: " << pktSize;
            break;
        case REINFORCE:
        case REINFORCE_INTEREST:
            //COUT << "Reinforcement size: " << pktSize;
            break;
        default:
            break;
    }




    unsigned char* data = &(_msg[2]);
    unsigned int len = _msg[1];
    char dataText[40];
    char* dataTextPtr = dataText;
    for ( unsigned int i = 0; i < len; i++ )
    {
        int numChar = std::sprintf(dataTextPtr, "%d-", (unsigned int)*data);
        dataTextPtr += numChar;
        data++;
    }
    //appPkt->setName((const char*)dataText);


    //appPkt->setSendingMAC(currentModule->mAddressString); // awaiting msg compilation
    //appPkt->setCreationTime(currentModule->currentPktCreationTime);
    appPkt->setCreationTime(_creationTime);



    //COUT << endl << "RECREATE ORIG CREATE TIME:      " << currentModule->currentPktCreationTime << endl;
    COUT << "\n" << "RECREATE ORIG CREATE TIME:      " << _creationTime << "\n";

    //Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    Ieee802154NetworkCtrlInfo *controlInfo = new Ieee802154NetworkCtrlInfo();
    //MACAddress destAddr(_interface);
    //controlInfo->setDest(destAddr);
    controlInfo->setNetwAddr(_interface);
    //controlInfo->setEtherType(ETHERTYPE_IPv4);
    appPkt->setControlInfo(controlInfo);

    //appPkt->setKind(8+_msg[0]);
    currentModule->mRoutingDelay = currentModule->par("routingDelay");
    //currentModule->send(appPkt, currentModule->mLowerLayerOut);
    ev << "UCAST   to " << currentModule->getParentModule()->getFullName() << endl;
    int mtype = *_msg;
    string fn = currentModule->getFullName();

    cout << "## sending: " << mtype << " from:  " <<
            hex << currentModule->mAddress << " (" << fn << ") to: " << hex << _interface << endl;

    currentModule->sendDelayed(appPkt, currentModule->mRoutingDelay, currentModule->mLowerLayerOut);

#ifndef STATIC_MEMORY
    sfree(_msg);
#endif
}



static void cb_bcast_message(unsigned char* _msg)
{
    /*
     * This is a generic call back for the data centric routing framework
     * It's implementation in this case is specifically tailored to Omnet++
     */
//check_and_cast<Txc20 *>
    //static cSimulation* cSimulation::getActiveSimulation()->getModule(currentModuleId)

    //cSimpleModule* currentModule = check_and_cast<cSimpleModule *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));

    DataCentricNetworkLayer* currentModule = check_and_cast<DataCentricNetworkLayer *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));



    //int n = currentModule->gateSize("gate");
    //char msgname[20];
    //sprintf(msgname, "%s", messageName[*_msg]);

    //for ( int i = 0; i < n; i++ )
    //{
    //    RoutingMessage *msg = new RoutingMessage(msgname);
    //    msg->setKind(ROUTING_MESSAGE);
    //    for( unsigned i = 0; i < MESSAGE_SIZE; i++ )
    //    {
    //        msg->setData(i, _msg[i]);
    //    }
    //    EV << "Send to gate " << i << " \n";
    //    currentModule->send(msg, "gate$o",i);
    //}

    //controlPackets


    currentModule->mNetMan->recordOnePacket(*_msg);







    DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
    //appPkt->getPktData().insert(appPkt->getPktData().end(), _msg, _msg+(_msg[1] + 4));
    appPkt->getPktData().insert(appPkt->getPktData().end(), _msg, _msg+sizeof_existing_packet(_msg));
    unsigned long long int pktSize = sizeof_existing_packet_withoutDownIF(_msg);
    appPkt->setByteLength(pktSize);


    switch ( *_msg )
    {
        case NEIGHBOR_BCAST:
        case NEIGHBOR_UCAST:
        case INTEREST:
        case ADVERT:
        case REINFORCE:
        case REINFORCE_INTEREST:
        case COLLABORATION:
        case REINFORCE_COLLABORATION:
            appPkt->setName("DataCentricControlPkt");
            break;
        default:
            appPkt->setName("DataCentricAppPkt");
            break;
    }









    switch ( *_msg )
    {
        case NEIGHBOR_BCAST:
            //COUT << "Bcast size: " << pktSize;
            break;
        case NEIGHBOR_UCAST:
            COUT << "HelloReply size: " << pktSize << "\n";
            break;
        case INTEREST:
            COUT << "Interest size: " << pktSize << "\n";
            if ( SIMTIME_ZERO == currentModule->mLastInterestDepartureTime )
            {
                currentModule->mLastInterestDepartureTime = simTime();
            }
            else
            {
                if ( (simTime() - currentModule->mLastInterestDepartureTime) > 1.0 )
                {
                    // short term assumption that new seqno out
                    // in long term try to introduce DEBUG code callbacks
                    // from framework - not compile in real thing
                    currentModule->InterestInterDepartureTimesVector.recordWithTimestamp(
                            currentModule->mLastInterestDepartureTime+0.0001, 0.0);
                    currentModule->InterestInterDepartureTimesVector.recordWithTimestamp(
                            simTime(), 0.0);
                    currentModule->mLastInterestDepartureTime = simTime();
                }
                else
                {
                    currentModule->InterestInterDepartureTimesVector.record(simTime() -
                                                            currentModule->mLastInterestDepartureTime);
                    currentModule->mLastInterestDepartureTime = simTime();
                }
            }
            break;
        case ADVERT:
            COUT << "Advert size: " << pktSize << "\n";
            if ( SIMTIME_ZERO == currentModule->mLastAdvertDepartureTime )
            {
                currentModule->mLastAdvertDepartureTime = simTime();
            }
            else
            {
                if ( (simTime() - currentModule->mLastAdvertDepartureTime) > 1.0 )
                {
                    // short term assumption that new seqno out
                    // in long term try to introduce DEBUG code callbacks
                    // from framework - not compile in real thing
                    currentModule->AdvertInterDepartureTimesVector.recordWithTimestamp(
                            currentModule->mLastAdvertDepartureTime+0.0001, 0.0);
                    currentModule->AdvertInterDepartureTimesVector.recordWithTimestamp(
                            simTime(), 0.0);
                    currentModule->mLastAdvertDepartureTime = simTime();
                }
                else
                {
                    currentModule->AdvertInterDepartureTimesVector.record(simTime() -
                                                            currentModule->mLastAdvertDepartureTime);
                    currentModule->mLastAdvertDepartureTime = simTime();
                }
            }
            break;
        case REINFORCE:
        case REINFORCE_INTEREST:
            //COUT << "Reinforcement size: " << pktSize;
            break;
        default:
            break;
    }


    //appPkt->setCreationTime(simTime());
    //appPkt->setCreationTime(currentModule->currentPktCreationTime);
    //COUT << endl << "RECREATE ORIG CREATE TIME:      " << currentModule->currentPktCreationTime << endl;

    //Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    Ieee802154NetworkCtrlInfo *controlInfo = new Ieee802154NetworkCtrlInfo();
    static MACAddress broadcastAddr("FF:FF:FF:FF:FF:FF");
    //controlInfo->setDest(broadcastAddr);
    controlInfo->setNetwAddr(broadcastAddr.getInt());
    //controlInfo->setEtherType(ETHERTYPE_IPv4);
    appPkt->setControlInfo(controlInfo);

    //appPkt->setKind(8+_msg[0]);

    //appPkt->
    //appPkt->setd
    //        currentModule->getd

    currentModule->mRoutingDelay = currentModule->par("routingDelay");
    COUT << "ROUTING DELAY " << routingDelayCount++ << ": " << currentModule->mRoutingDelay << "\n";
    if ( routingDelayCount == 25 )
    {
        routingDelayCount = 25;
    }

    //currentModule->send(appPkt, currentModule->mLowerLayerOut);
    ev << "BRDCAST to " << currentModule->getParentModule()->getFullName() << endl;
    currentModule->sendDelayed(appPkt, currentModule->mRoutingDelay, currentModule->mLowerLayerOut);

#ifndef STATIC_MEMORY
    sfree(_msg);
#endif
}



#ifndef OLDFRAMEWORK
static void cb_handle_application_data(unsigned char* _msg, double _creationTime, uint64_t ID)
#else
static void cb_handle_application_data(unsigned char* _msg, unsigned int len, double _creationTime)
#endif
{
    // work still to do here
    // packetbuf_copyto(_msg, MESSAGE_SIZE);
    //cSimpleModule* currentModule = check_and_cast<cSimpleModule *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));
    DataCentricNetworkLayer* currentModule = check_and_cast<DataCentricNetworkLayer *>(cSimulation::getActiveSimulation()->getModule(currentModuleId));

    char bubbleText[40];
    char* bubbleTextPtr = bubbleText;
    unsigned char* index = _msg;
    while (*index)
    {
        int numChar = std::sprintf(bubbleTextPtr, "%d-", (unsigned int)*index);
        bubbleTextPtr += numChar;
        index++;
    }

    COUT << "\n" << "SOME KIND OF DATA PKT RECEIVED, ORIG CREATE TIME: " << _creationTime << "\n";
    string fn = currentModule->getFullName();
    double now = simTime().dbl();
    double pktTime = _creationTime;

#ifndef OLDFRAMEWORK
    int moduleId = (int)(ID >> 32);
    int msgId = (int)(ID & 0xFFFFFFFF);

    if ( !currentModule->duplicate(moduleId, msgId) )//, appPkt) )
    {
#endif
        currentModule->getParentModule()->bubble(bubbleText);
        cout << "## " << hex << currentModule->mAddress << " (" << fn << ") received data from:  " <<
                hex << excludedInterface << endl;

        simtime_t endToEndDelay = simTime() - _creationTime;
        COUT <<         "END TO END DELAY:               " << endToEndDelay << "\n";
        currentModule->mNetMan->addADataPacketE2EDelay(endToEndDelay);

        DataCentricAppPkt* appPkt = new DataCentricAppPkt("Data_DataCentricAppPkt");
        appPkt->setKind(DATA_PACKET);
        appPkt->getPktData().insert(appPkt->getPktData().end(), _msg, _msg+strlen((const char*)_msg));
#ifndef OLDFRAMEWORK
        appPkt->addPar("sourceId").setLongValue(moduleId);
        appPkt->addPar("msgId").setLongValue(msgId);
#endif

        currentModule->send(appPkt, currentModule->mUpperLayerOut);
#ifndef OLDFRAMEWORK
    }
    else
    {
        cout << "## " << hex << currentModule->mAddress << " (" << fn << ") discarded duplicate data from:  " <<
                hex << excludedInterface << endl;

    }
#endif

}




/*

void DataCentricNetworkLayer::handleMessage(cMessage* msg)
{
    // only necessary in this simulation due to combined C++ & C
    nodeConstraint = nodeConstraintValue;
    currentModuleId = this->getId();
    rd = &(moduleRD);
    ////////////////////////////////////////////

    DataCentricAppPkt* appPkt = check_and_cast<DataCentricAppPkt *>(msg);
    //appPkt->getDataName();
    //appPkt->getNextHopMAC();


    //InterfaceEntry *ie = ift->getInterfaceById(controlInfo->getInterfaceId());

    //uint64 getInt() const { return address; }



    // coming from App layer
    if (msg->getArrivalGateId() == mUpperLayerIn)
    {
        static MACAddress broadcastAddr("FF:FF:FF:FF:FF:FF");

        // NEW
        Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
        controlInfo->s
        controlInfo->setDest(broadcastAddr);
        controlInfo->setEtherType(ETHERTYPE_IPv4);
        msg->setControlInfo(controlInfo);

        // send out
        //send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
        //send(msg, nicOutBaseGateId + ie->getNetworkLayerGateIndex());
        send(appPkt, mLowerLayerOut);
        ///////////////////////////////////////////

        Ieee802154NetworkCtrlInfo *control_info = new Ieee802154NetworkCtrlInfo();
        control_info->

        //handle_message((unsigned char *)appPkt->getDataName(), previousAddress);
        handle_message((unsigned char *)appPkt->getDataName(), SELF_INTERFACE);



        Ieee802154NetworkCtrlInfo *control_info = new Ieee802154NetworkCtrlInfo();

        if (isPANCoor)      // I'm PAN coordinator, msg is destined for a device
        {
            control_info->setToParent(false);
            control_info->setDestName(appPkt->getDestName());
        }
        else        // should always be sent to PAN coordinator first
        {
            control_info->setToParent(true);
            control_info->setDestName("PAN Coordinator");
        }

        appPkt->setControlInfo(control_info);
        send(appPkt, mLowerLayerOut);

    }

    // coming from MAC layer
    else if (msg->getArrivalGateId() == mLowerLayerIn)
    {
        Ieee802Ctrl *incomingControlInfo = appPkt->getControlInfo();
        uint64 previousAddress = incomingControlInfo->getSrc().getInt();

        handle_message((unsigned char *)appPkt->getDataName(), previousAddress);
    }
    else
    {
        // not defined
    }
}

 */
