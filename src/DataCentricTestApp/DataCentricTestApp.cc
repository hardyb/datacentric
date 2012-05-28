
#include "DataCentricTestApp.h"
#include <stdio.h>
#include <string.h>


#define UNKNOWN_ACTIVITY 0
#define SENSOR_READING 1
#define SET_PROGRAM 2
#define WATTS 3
#define FILE_END 4



//#undef EV
//#define EV (ev.isDisabled() || !m_debug) ? std::cout : ev ==> EV is now part of <omnetpp.h>

Define_Module(DataCentricTestApp);

void DataCentricTestApp::initialize(int aStage)
{
    TrafGenPar::initialize(aStage);


    EV << getParentModule()->getFullName() << ": initializing DataCentricTestApp, stage=" << aStage << std::endl;
    if (0 == aStage)
    {
        netModule = check_and_cast<DataCentricNetworkLayer*>(this->getParentModule()->getSubmodule("net"));

        //mpStartMessage = new cMessage("StartMessage");
        m_debug             = par("debug");
        //std::string temp1 = par("sourceFor").stringValue();
        //sourcesData = cStringTokenizer(temp1.c_str()).asVector();
        //std::string temp2 = par("sinkFor").stringValue();
        //sinksData = cStringTokenizer(temp2.c_str()).asVector();

        mLowerLayerIn        = findGate("lowerLayerIn");
        mLowerLayerOut       = findGate("lowerLayerOut");
        m_moduleName        = getParentModule()->getFullName();
        sumE2EDelay         = 0;
        numReceived         = 0;
        mNumTrafficMsgs     = 0;
        totalByteRecv           = 0;
        e2eDelayVec.setName("End-to-end delay");
        meanE2EDelayVec.setName("Mean end-to-end delay");

        //ifstream scheduleFile;
        //string activities = par("activities").stringValue();
        //mActivityFile.open(activities.c_str());



        string actionThreadsString = par("actionThreads").stringValue();
        std::vector<std::string> actionThreads = cStringTokenizer(actionThreadsString.c_str()).asVector();
        for (std::vector<std::string>::iterator i = actionThreads.begin();
                i != actionThreads.end(); ++i)
        {
            ifstream* actionStream = new ifstream();
            actionStream->open(i->c_str());
            ActionStreamHierarchy* ash = new ActionStreamHierarchy();
            ash->push_back(actionStream);
            cMessage* m = new cMessage(i->c_str());
            mActionThreads[m] = ash;
            scheduleAt(simTime() + 1.0, m);
        }

        contextData = par("nodeContext").stringValue();
        DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt->getPktData().insert(appPkt->getPktData().end(), contextData.begin(), contextData.end());
        appPkt->setKind(CONTEXT_MESSAGE);
        send(appPkt, mLowerLayerOut);

        std::string temp1 = par("sourceFor").stringValue();
        if ( temp1.size() )
        {
            DataCentricAppPkt* appPkt1 = new DataCentricAppPkt("DataCentricAppPkt");
            appPkt1->getPktData().insert(appPkt1->getPktData().end(), temp1.begin(), temp1.end());
            appPkt1->setKind(SOURCE_MESSAGE);
            send(appPkt1, mLowerLayerOut);
        }

        std::string temp2 = par("sinkFor").stringValue();
        if ( temp2.size() )
        {
            DataCentricAppPkt* appPkt2 = new DataCentricAppPkt("DataCentricAppPkt");
            appPkt2->getPktData().insert(appPkt2->getPktData().end(), temp2.begin(), temp2.end());
            appPkt2->setKind(SINK_MESSAGE);
            send(appPkt2, mLowerLayerOut);
        }

        DataCentricAppPkt* appPkt3 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt3->setKind(STARTUP_MESSAGE);
        sendDelayed(appPkt3, StartTime(), mLowerLayerOut);

    }


}

void DataCentricTestApp::finish()
{
    recordScalar("trafficSent", mNumTrafficMsgs);
    recordScalar("total bytes received", totalByteRecv);
    //recordScalar("total time", simTime() - FirstPacketTime());
    //recordScalar("goodput (Bytes/s)", totalByteRecv / (simTime() - FirstPacketTime()));
}

void DataCentricTestApp::handleLowerMsg(cMessage* apMsg)
{
    simtime_t e2eDelay;
    DataCentricAppPkt* tmpPkt = check_and_cast<DataCentricAppPkt *>(apMsg);
    e2eDelay = simTime() - tmpPkt->getCreationTime();
    totalByteRecv += tmpPkt->getByteLength();
    e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    numReceived++;
    sumE2EDelay += e2eDelay;
    meanE2EDelayVec.record(sumE2EDelay/numReceived);
    //EV << "[APP]: a message sent by " << tmpPkt->getDataName() << " arrived at application with delay " << e2eDelay << " s" << std::endl;
    this->getParentModule()->bubble("Data received!");
    delete apMsg;
}

//***************************************************************
// Reimplement this function and use msg type DataCentricAppPkt for app pkts
//***************************************************************
void DataCentricTestApp::handleSelfMsg(cMessage *apMsg)
{

    ActionThreadsIterator i = mActionThreads.find(apMsg);
    if (i != mActionThreads.end() )
    {
        switch ( getNextAction(i) )
        {
            case SENSOR_READING:
                SensorReading(i);
                break;
            case SET_PROGRAM:
                startProgram(i);
                break;
            case WATTS:
                processWatts(i);
                break;
            case FILE_END:
                FileEnd(i);
                break;
            case UNKNOWN_ACTIVITY:
                break;
            default:
                break;
        }

    }

    TrafGenPar::handleSelfMsg(apMsg);
}



int DataCentricTestApp::getNextAction(ActionThreadsIterator& i)
{
    ifstream* ifs = i->second->back();
    if ( !ifs->eof() )
    {
        string action;
        *ifs >> action;
        if ( action == "SENSOR_READING" )
        {
            return SENSOR_READING;
        }
        if ( action == "SET_PROGRAM" )
        {
            return SET_PROGRAM;
        }
        if ( action == "WATTS" )
        {
            return WATTS;
        }
        return UNKNOWN_ACTIVITY;
    }
    else
    {
        return FILE_END;
    }
}


void DataCentricTestApp::FileEnd(ActionThreadsIterator& i)
{
    ifstream* ifs = i->second->back();
    i->second->pop_back();
    ifs->close();
    delete ifs;
    scheduleAt(simTime(), i->first);
}




//int DataCentricTestApp::getReading(ifstream* ifs)
//{
//    int reading;
//    *ifs >> reading;
//    return reading;
//}



void DataCentricTestApp::processWatts(ActionThreadsIterator& i)
{
    unsigned short watts;
    double period;
    ifstream* ifs = i->second->back();
    *ifs >> watts;
    *ifs >> period;

    DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
    std::ostringstream ss;
    ss.clear();
    ss << "\x2\x2";
    ss << (unsigned char)(watts & 0xff);
    ss << (unsigned char)((watts >>8) & 0xff);
    ss << "\x0";
    //ss << "\x2\x2\x" << hex << watts << "\x0";
    std::string s(ss.str());
    unsigned int a = s.size();
    appPkt->getPktData().insert(appPkt->getPktData().end(), s.begin(), s.end());
    appPkt->setKind(DATA_PACKET);
    send(appPkt, mLowerLayerOut);
    scheduleAt(simTime()+period, i->first);


}




void DataCentricTestApp::startProgram(ActionThreadsIterator& i)
{
    string program;
    ifstream* ifs = i->second->back();
    *ifs >> program;

    ifstream* newIfs = new ifstream();
    newIfs->open(program.c_str());
    i->second->push_back(newIfs);
    scheduleAt(simTime(), i->first);

}


void DataCentricTestApp::SensorReading(ActionThreadsIterator& i)
{
    ifstream* ifs = i->second->back();

    unsigned short reading;
    double period;
    *ifs >> reading;
    *ifs >> period;

    DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
    std::ostringstream ss;
    ss.clear();
    ss << "\x3\x1";
    ss << (unsigned char)(reading & 0xff);
    ss << (unsigned char)((reading >>8) & 0xff);
    ss << "\x0";
    std::string s(ss.str());
    appPkt->getPktData().insert(appPkt->getPktData().end(), s.begin(), s.end());
    appPkt->setKind(DATA_PACKET);
    send(appPkt, mLowerLayerOut);

    scheduleAt(simTime() + period, i->first);
}


//void DataCentricTestApp::start2Program(string program)
//{
//    if ( mProgramFile.is_open() )
//        mProgramFile.close();
//    mProgramFile.open(program.c_str());
//    "\x2\x2\x0"
//}



//void DataCentricTestApp::actionProgramItem(string program)
//{
//    if ( mProgramFile.is_open() )
//        mProgramFile.close();
//    mProgramFile.open(program.c_str());
//    "\x2\x2\x0"
//}


/*
void DataCentricTestApp::readFile(char* filePath, OperationSchedule& operationSchedule)
{
    ifstream scheduleFile;
    scheduleFile.open(filePath);
    while ( !scheduleFile.eof() )
    {
        PeriodOfOperation periodOfOperation;
        Program program;
        scheduleFile >> program.programName;
        periodOfOperation.program = program;
        scheduleFile >> periodOfOperation.period;
        operationSchedule.push_back(periodOfOperation);
    }
}
*/






/** this function has to be redefined in every application derived from the
    TrafGen class.
    Its purpose is to translate the destination (given, for example, as "host[5]")
    to a valid address (MAC, IP, ...) that can be understood by the next lower
    layer.
    It also constructs an appropriate control info block that might be needed by
    the lower layer to process the message.
    In the example, the messages are sent directly to a mac 802.11 layer, address
    and control info are selected accordingly.
*/
void DataCentricTestApp::SendTraf(cPacket* apMsg, const char* apDest)
{
    delete apMsg;

    // for the moment don't use TrafGenPar, but in the future
    // may be have choice between TrafGenPar, ie very regular
    // random hits, and run from schedule file

    return;

    //rd = &(netModule->moduleRD);

    char temp[30];
    for (std::vector<std::string>::iterator i = sourcesData.begin();
            i != sourcesData.end(); ++i)
    {
        // create a new app pkt
        DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");

        appPkt->setBitLength(PacketSize()*8);
        //appPkt->setDataName(i->c_str());
        //appPkt->setDestName(apDest);
        appPkt->setCreationTime(simTime());

        /*Ieee802154UpperCtrlInfo *control_info = new Ieee802154UpperCtrlInfo();
        control_info->setDestName(apDest);
        appPkt->setControlInfo(control_info);*/

        char x[20];
        int datalen = strlen(i->c_str());
        memcpy(x, i->c_str(), datalen);
        x[datalen] = DOT;
        getLongestContextTrie(rd->top_context, temp, temp, &(x[datalen+1]));
        appPkt->getPktData().insert(appPkt->getPktData().end(), x, x+strlen(x));
        mNumTrafficMsgs++;

        appPkt->setKind(DATA_PACKET);
        //send(appPkt, mLowerLayerOut);

        //incoming_packet.message_type = DATA;
        //incoming_packet.length = strlen((char*)_data);
        //incoming_packet.data = _data;
        //incoming_packet.path_value = 0;
        //handle_data(SELF_INTERFACE);

    }

}


double DataCentricTestApp::StartTime()
{
    return par("startTime").doubleValue();
}










// sending data ideas
/*
unsigned char* data = (unsigned char*)malloc(appPkt->getPktData().size());
std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), data);
send_data(appPkt->getPktData().size(), data);
free(data);
incoming_packet.message_type = DATA;
incoming_packet.length = len;
free(incoming_packet.data);
incoming_packet.data = (unsigned char*)malloc(incoming_packet.length+1);
memcpy(incoming_packet.data, _data, incoming_packet.length);
incoming_packet.data[incoming_packet.length] = 0;
incoming_packet.path_value = 0;
incoming_packet.down_interface = UNKNOWN_INTERFACE;
incoming_packet.excepted_interface = UNKNOWN_INTERFACE;
handle_data(SELF_INTERFACE);
*/


/*
operationSchedule.push_back(periodOfOperation);
mSecheduleSet.insert(pair<string, OperationSchedule>("", operationSchedule));
ifstream scheduleFile;
char output[100];
string programIN;
int periodIN;
scheduleFile.open("test.txt");
while ( !scheduleFile.eof() )
{
    PeriodOfOperation periodOfOperation;
    Program program;
    scheduleFile >> program.programName;
    periodOfOperation.program = program;
    scheduleFile >> periodOfOperation.period;
}
*/
