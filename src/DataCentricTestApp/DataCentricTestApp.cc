
#include "DataCentricTestApp.h"
#include <stdio.h>
#include <string.h>
#include "StationaryMobility.h"
#include "HostReference.h"

#include "SpecialDebug.h"


#define UNKNOWN_ACTIVITY 0
#define SENSOR_READING 1
#define SET_PROGRAM 2
#define WATTS 3
#define ACTION_FILE_END 4
#define OCCUPANCY_READING 5
#define TEMP_READING 6






//#undef EV
//#define EV (ev.isDisabled() || !m_debug) ? COUT : ev ==> EV is now part of <omnetpp.h>

Define_Module(DataCentricTestApp);




int escapeBuffer(unsigned char* pkt, unsigned int len, unsigned char* out)
{
int i = 0;
while ( len )
{
        switch (*(pkt))
        {
            case 0x27:
                out[i++] = 0x27;
                out[i++] = 0x27;
                break;
            case 0x0:
                out[i++] = 0x27;
                out[i++] = 0x1;
                break;
            case 0xFF:
                out[i++] = 0x27;
                out[i++] = 0x2;
                break;
            default:
                out[i++] = *pkt;
                break;
        }
        pkt++;
        len--;
}
out[i] = 0x0;

return i;
}









int unescapeBuffer(unsigned char* pkt, unsigned char* out)
{
bool escaped = false;
int i = 0;
while ( *pkt )
{
    if ( !escaped )
    {
        if ( *(pkt) == 0x27 )
        {
            escaped = true;
        }
        else
        {
            out[i++] = *(pkt);
        }
    }
    else
    {
        switch (*(pkt))
        {
            case 0x27:
                out[i++] = 0x27;
                break;
            case 0x1:
                out[i++] = 0x00;
                break;
            case 0x2:
                out[i++] = 0xFF;
                break;
            default:
                out[i++] = *pkt;
                break;
        }
        escaped = false;
    }
    pkt++;
}

return i;
}








void DataCentricTestApp::initialize(int aStage)
{
    TrafGenPar::initialize(aStage); //DO NOT DELETE!!


    EV << getParentModule()->getFullName() << ": initializing DataCentricTestApp, stage=" << aStage << std::endl;
    if (0 == aStage)
    {
        actualDemand = 0;
        mDownTime = 0;
        mLastOnTime = 0;
        mLastOffTime = 0;
        mOriginalNextDemandActionTime = 0;
        mDemandActionMessage = 0;
        appState = APPSTATE_IDLE;


        // TODO
        string fn = this->getParentModule()->getFullName();
        if ( !strcmp(this->getParentModule()->getFullName(), "fixhost[115]") )
        {
            COUT << "We are here!" << "\n";
        }

        mBeenSetDirect = false;

        mBeenSetDirect = !mBeenSetDirect ? par("setParametersDirectly").boolValue() : true;
        mBeenSetDirect = !mBeenSetDirect ? par("isAppliance").boolValue() : true;
        //mBeenSetDirect = !mBeenSetDirect ? par("debug").boolValue() : true;
        mBeenSetDirect = !mBeenSetDirect ? strcmp(par("actionThreads").stringValue(), "") : true;
        mBeenSetDirect = !mBeenSetDirect ? strcmp(par("sourceFor").stringValue(), "") : true;
        mBeenSetDirect = !mBeenSetDirect ? strcmp(par("sinkFor").stringValue(), "") : true;
        mBeenSetDirect = !mBeenSetDirect ? strcmp(par("collaboratorInitiatorFor").stringValue(), "") : true;
        mBeenSetDirect = !mBeenSetDirect ? strcmp(par("collaboratorFor").stringValue(), "") : true;
        //mBeenSetDirect = !mBeenSetDirect ? strcmp(par("appMode").stringValue(), "") : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("nodeStartTime").doubleValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("timeSinkRegisterWithControlUnit").doubleValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("timeSourceRegisterWithControlUnit").doubleValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("timeSinkBindWithSourceDevice").doubleValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("scheduleStartTime").doubleValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("mains").boolValue() : true;
        mBeenSetDirect =  !mBeenSetDirect ? par("probabilityDown").doubleValue() : true;

        // nodeContext is always set from region

        // Need to write something about debug and appMode

        if ( mBeenSetDirect )
        {
            COUT << "We are here!" << "\n";
        }






















        /*
        if ( par("nodeStartTime").isSet() )
        {
            string fp = getFullPath();
            ev << fp << ": nodeStartTime IS set" << std::endl;
        }
        else
        {
            string fp = getFullPath();
            ev << fp << ": nodeStartTime NOT set" << std::endl;
        }
        */


        cSimulation* sim =  cSimulation::getActiveSimulation();
        if ( sim->getModuleByPath("DataCentricNet.dataCentricNetworkMan") )
        {
            mNetMan = check_and_cast<DataCentricNetworkMan*>(sim->getModuleByPath("DataCentricNet.dataCentricNetworkMan"));
            mNet = sim->getModuleByPath("DataCentricNet");
            netModule = check_and_cast<DataCentricNetworkLayer*>(this->getParentModule()->getSubmodule("net"));
        }
        else
        {
            mNetMan = check_and_cast<DataCentricNetworkMan*>(sim->getModuleByPath("csma802154net.dataCentricNetworkMan"));
            mNet = sim->getModuleByPath("csma802154net");
        }



        //mpStartMessage = new cMessage("StartMessage");
        m_debug             = par("debug");

        if ( !strcmp(par("appMode").stringValue(), "AODV") )
        {
            mAppMode = AODV_MODE;
        }
        else
        {
            mAppMode = DATACENTRIC_MODE;
        }

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
        demandVec.setName("demandVec");
        meanE2EDelayVec.setName("Mean end-to-end delay");

    }

    if (1 == aStage)
    {
        // RECENTLY MOVED THIS TO STAGE 1
        // SO WATCH CAREFULLY


        /*
        std::vector<std::string> actionThreads = cStringTokenizer(actionThreadsString.c_str()).asVector();
        for (std::vector<std::string>::iterator i = actionThreads.begin();
                i != actionThreads.end(); ++i)
        {
            ifstream* actionStream = new ifstream();
            actionStream->open(i->c_str());
            if ( actionStream->is_open() )
            {
                ActionStreamHierarchy* ash = new ActionStreamHierarchy();
                ash->push_back(actionStream);
                cMessage* m = new cMessage(i->c_str());
                mActionThreads[m] = ash;
                scheduleAt(simTime() + ScheduleStartTime(), m);
            }
            else
            {
                throw cRuntimeError("Cannot open actionThread: '%s' ", i->c_str());
            }
        }
        */



        /*
         * A region has top left coord (x,y) and width and height w, h
         * A region may also have a context / location name e.g. \6\4
         *
         * Example
         *
         * region 1 - 0,0 w=18 h=18
         * region 2 - 18,0 w=17 h=18
         * region 3 - 0,18 w=18 h=17
         * region 4 - 18,18 w=17 h=17
         *
         * where shall we store this data?
         * we need to set context from ini file
         *
         * WE HAVE DONE HARDCODED EXAMPLE IN NetMan'
         *
         * BELOW QUICK AND DIRTY START FOR SETTING CONTEXT IN THIS NODE
         *
         */

        // Probably no longer take from ini file
        /*
         * If the context(s) set here then skip region check
         * else create multiple contexts through region check
         *
         * next use tokenizer and make multiple calls to
         * appPkt->setKind(CONTEXT_MESSAGE);
         * send(appPkt, mLowerLayerOut);
         */
        contextData = par("nodeContext").stringValue();
        if ( !strcmp(contextData.c_str(), "") )
        {
            StationaryMobility* mob = check_and_cast<StationaryMobility*>(this->getParentModule()->getSubmodule("mobility"));
            double x = mob->par("initialX");
            double y = mob->par("initialY");
            double z = mob->par("initialZ");
            for (cSubModIterator iter(*mNet); !iter.end(); iter++)
            {
                if ( iter()->isName("region") )
                {
                    //ev << "Traversing:    " << iter()->getFullName() << endl;
                    double _x = iter()->par("x");
                    double _y = iter()->par("y");
                    double _z = iter()->par("z");
                    double _w = iter()->par("w");
                    double _h = iter()->par("h");
                    double _d = iter()->par("d");
                    string _c = iter()->par("context").stringValue();
                    if (       x >= _x
                            && y >= _y
                            && z >= _z
                            && x < (_x + _w)
                            && y < (_y + _h)
                            && z < (_z + _d)      )
                    {
                        contextData = contextData + _c + " ";
                        if ( mBeenSetDirect )
                        {
                            // In this app one or more relevent parameters have been set directly
                            // so DO NOT connect with the next available regionHost
                            // Relevant parameters should be set directly or via regionHost
                            // never both
                            string fn = this->getParentModule()->getFullName();
                            fn = this->getParentModule()->getFullName();
                            continue;
                        }
                        for (cSubModIterator iter2(*(iter())); !iter2.end(); iter2++)
                        {
                            if ( iter2()->isName("regionHost") )
                            {
                                //ev << "Traversing:    " << iter2()->getFullName() << endl;
                                HostReference* hr = check_and_cast<HostReference*>(iter2());
                                if ( !hr->hasUnderlyingModule() )
                                {
                                    ev << "Adding this app to:    " << iter2()->getFullPath() << endl;
                                    hr->setUnderlyingModule(this);
                                    break;
                                }
                            }
                        }

                    }
                }
            }

            /*
            for (DataCentricNetworkMan::RegionsIterator i = mNetMan->mRegions.begin();
                    i != mNetMan->mRegions.end(); ++i)
            {
                if (       x >= i->x
                        && y >= i->y
                        && z >= i->z
                        && x < (i->x + i->w)
                        && y < (i->y + i->h)
                        && z < (i->z + i->d)      )
                {
                    //string temp((const char*)i->context);
                    //contextData = temp;
                    // check out below
                    contextData = contextData + i->context + " ";

                    // what do we put in here - there may be multiple ones
                    //par("nodeContext").setStringValue(contextData.c_str());

                    //DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
                    //appPkt->getPktData().insert(appPkt->getPktData().end(), contextData.begin(), contextData.end());
                    //appPkt->setKind(CONTEXT_MESSAGE);
                    //send(appPkt, mLowerLayerOut);

                    //break;
                    //
                    // need to think it through but we think NOT break
                    // cause need context object at every hierarchy layer
                    // e.g.
                    // property 6
                    // room1 6-1
                    // room2 6-2
                    //
                    // but need to take care regarding hierarchy and overlap
                    // when proper code is done in DataCentricNetworkMan
                    //
                }
            }
            */
        }

        string fn = this->getParentModule()->getFullName();
        par("nodeContext").setStringValue(contextData.c_str());
        ev << "Context: " << getFullName() << ": " << contextData << endl;

        // should we do the tokenization here
        // or in datacentricnetworklayer like for sources and sinks?
        std::vector<std::string> contextDataSet = cStringTokenizer(contextData.c_str()).asVector();
        for (std::vector<std::string>::iterator i = contextDataSet.begin();
                i != contextDataSet.end(); ++i)
        {
            DataCentricAppPkt* appPkt = new DataCentricAppPkt("DataCentricAppPkt");
            appPkt->getPktData().insert(appPkt->getPktData().end(), i->begin(), i->end());
            appPkt->setKind(CONTEXT_MESSAGE);
            send(appPkt, mLowerLayerOut);
        }

    }

    if (3 == aStage)
    {
        isAppliance = par("isAppliance").boolValue();
        if ( isAppliance )
        {
            mNetMan->addAppliance(this);
        }

        string actionThreadsString = par("actionThreads").stringValue();
        processActionsFor(actionThreadsString);

        std::string temp1 = par("sourceFor").stringValue();
        processSourceFor(temp1);

        std::string temp2 = par("sinkFor").stringValue();
        processSinkFor(temp2);

        std::string temp3 = par("collaboratorInitiatorFor").stringValue();
        processCollaboratorInitiatorFor(temp3);

        std::string temp4 = par("collaboratorFor").stringValue();
        processCollaboratorFor(temp4);






        DataCentricAppPkt* appPkt3 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt3->setKind(STARTUP_MESSAGE);
        sendDelayed(appPkt3, NodeStartTime(), mLowerLayerOut);

    }






}



void DataCentricTestApp::processSinkFor(string &temp2)
{
    Enter_Method("processSinkFor(string &temp2)");

    par("sinkFor").setStringValue(temp2.c_str());

    if ( temp2.size() )
    {
        ev << "SinksFor: " << getFullPath() << ": ";
        for ( string::iterator i = temp2.begin(); i != temp2.end(); i++ )
        {
            //char cval1 =  (*i);
            unsigned char cval2 =  (*i);
            unsigned int val = (unsigned int)cval2;
            ev << std::hex << std::uppercase << "\\" << val;
        }
        ev << endl;

        DataCentricAppPkt* appPkt2 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt2->getPktData().insert(appPkt2->getPktData().end(), temp2.begin(), temp2.end());
        appPkt2->setKind(SINK_MESSAGE);
        if ( mAppMode == AODV_MODE )
        {
            //sendDelayed(appPkt2, NodeStartTime()+42.0, mLowerLayerOut);
            sendDelayed(appPkt2, TimeSinkRegisterWithControlUnit(), mLowerLayerOut);
        }
        else
        {
            send(appPkt2, mLowerLayerOut);
        }
    }

}







void DataCentricTestApp::processCollaboratorInitiatorFor(string &temp2)
{
    Enter_Method("processCollaboratorInitiatorFor(string &temp2)");

    par("collaboratorInitiatorFor").setStringValue(temp2.c_str());

    if ( temp2.size() )
    {
        ev << "collaboratorInitiatorFor: " << getFullPath() << ": ";
        for ( string::iterator i = temp2.begin(); i != temp2.end(); i++ )
        {
            //char cval1 =  (*i);
            unsigned char cval2 =  (*i);
            unsigned int val = (unsigned int)cval2;
            ev << std::hex << std::uppercase << "\\" << val;
        }
        ev << endl;

        DataCentricAppPkt* appPkt2 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt2->getPktData().insert(appPkt2->getPktData().end(), temp2.begin(), temp2.end());
        appPkt2->setKind(COLLABORATOR_INITITOR_MESSAGE);
        if ( mAppMode == AODV_MODE )
        {
            //sendDelayed(appPkt2, NodeStartTime()+42.0, mLowerLayerOut);
            sendDelayed(appPkt2, TimeSinkRegisterWithControlUnit(), mLowerLayerOut);
        }
        else
        {
            send(appPkt2, mLowerLayerOut);
        }
    }

}



void DataCentricTestApp::processCollaboratorFor(string &temp2)
{
    Enter_Method("processCollaboratorFor(string &temp2)");

    par("collaboratorFor").setStringValue(temp2.c_str());

    if ( temp2.size() )
    {
        ev << "collaboratorFor: " << getFullPath() << ": ";
        for ( string::iterator i = temp2.begin(); i != temp2.end(); i++ )
        {
            //char cval1 =  (*i);
            unsigned char cval2 =  (*i);
            unsigned int val = (unsigned int)cval2;
            ev << std::hex << std::uppercase << "\\" << val;
        }
        ev << endl;

        DataCentricAppPkt* appPkt2 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt2->getPktData().insert(appPkt2->getPktData().end(), temp2.begin(), temp2.end());
        appPkt2->setKind(COLLABORATOR_MESSAGE);
        if ( mAppMode == AODV_MODE )
        {
            //sendDelayed(appPkt2, NodeStartTime()+42.0, mLowerLayerOut);
            sendDelayed(appPkt2, TimeSinkRegisterWithControlUnit(), mLowerLayerOut);
        }
        else
        {
            send(appPkt2, mLowerLayerOut);
        }
    }

}



void DataCentricTestApp::processSourceFor(string &temp1)
{
    Enter_Method("processSourceFor(string &temp1)");

    par("sourceFor").setStringValue(temp1.c_str());

    if ( temp1.size() )
    {
        ev << "SourcesFor: " << getFullPath() << ": ";
        for ( string::iterator i = temp1.begin(); i != temp1.end(); i++ )
        {
            //char cval1 =  (*i);
            unsigned char cval2 =  (*i);
            unsigned int val = (unsigned int)cval2;
            ev << std::hex << std::uppercase << "\\" << val;
        }
        ev << endl;

        DataCentricAppPkt* appPkt1 = new DataCentricAppPkt("DataCentricAppPkt");
        appPkt1->getPktData().insert(appPkt1->getPktData().end(), temp1.begin(), temp1.end());
        appPkt1->setKind(SOURCE_MESSAGE);
        if ( mAppMode == AODV_MODE )
        {
            //sendDelayed(appPkt1, NodeStartTime()+42.0, mLowerLayerOut);
            sendDelayed(appPkt1, TimeSourceRegisterWithControlUnit(), mLowerLayerOut);
        }
        else
        {
            send(appPkt1, mLowerLayerOut);
        }
    }

}


void DataCentricTestApp::processActionsFor(string &actionThreadsString)
{
    Enter_Method("processActionsFor(string &actionThreadsString)");

    par("actionThreads").setStringValue(actionThreadsString.c_str());

    if ( actionThreadsString.size() )
    {
        ev << "actionThreads: " << getFullName() << ": " << actionThreadsString << endl;
    }

    std::vector<std::string> actionThreads = cStringTokenizer(actionThreadsString.c_str()).asVector();
    for (std::vector<std::string>::iterator i = actionThreads.begin();
            i != actionThreads.end(); ++i)
    {
        string actionsFolder("..\\Common\\Actions\\");
        string actionFile = actionsFolder + *(i);
        ifstream* actionStream = new ifstream();
        actionStream->open(actionFile.c_str());
        if ( actionStream->is_open() )
        {
            ActionStreamHierarchy* ash = new ActionStreamHierarchy();
            ash->push_back(actionStream);
            cMessage* m = new cMessage(i->c_str());
            mActionThreads[m] = ash;

            unsigned int hours;
            unsigned int minutes;
            unsigned int seconds;
            unsigned int finalSeconds;
            *actionStream >> hours;
            if (actionStream->failbit)
            {
                COUT << "fail" << "\n";
            }
            *actionStream >> minutes;
            if (actionStream->failbit)
            {
                COUT << "fail" << "\n";
            }
            *actionStream >> seconds;
            if (actionStream->failbit)
            {
                COUT << "fail" << "\n";
            }
            finalSeconds = (hours*3600)+(minutes*60)+seconds;
            scheduleAt(finalSeconds + ScheduleStartTime(), m);
            //scheduleAt(simTime() + ScheduleStartTime(), m);
            // may be this should be right away then read time first
        }
        else
        {
            throw cRuntimeError("Cannot open actionThread: '%s' ", i->c_str());
        }
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
    DataCentricAppPkt* appPkt = check_and_cast<DataCentricAppPkt *>(apMsg);

    if ( appPkt->getKind() == DATA_PACKET )
    {
        unsigned int _size = appPkt->getPktData().size();
        unsigned char* pkt = (unsigned char*)malloc(_size+1);
        std::copy(appPkt->getPktData().begin(), appPkt->getPktData().end(), pkt);
        pkt[_size] = 0;

        for (unsigned int i = 0; i < _size; i++ )
        {
            int i = 0;
            switch ( pkt[i] )
            {
            case 0x83: // Environmental event
                processEnvironmentalData(++pkt);
                break;
            case 0x02: // Demand Query
            case 0x42: // Demand Collaboration
                processDemandData(++pkt);
                break;
            //case 0x0:
            //    break;
            //case 0x0:
            //    break;


            }

//        string DEMAND_QUERYBASED = "\x2";
//        string DEMAND_COLLABERATIONBASED = "\x42";
//        string ENVIRONMENTAL_EVENTBASED = "\x83";
//        string CURRENT = "\x2";
//        string BID = "\x1";
//        string OCCUPANCY = "\x1";
//        string TEMP = "\x2";



        }

    }

    //e2eDelay = simTime() - tmpPkt->getCreationTime();
    //totalByteRecv += tmpPkt->getByteLength();
    //e2eDelayVec.record(SIMTIME_DBL(e2eDelay));
    numReceived++;
    //sumE2EDelay += e2eDelay;
    //meanE2EDelayVec.record(sumE2EDelay/numReceived);
    //EV << "[APP]: a message sent by " << tmpPkt->getDataName() << " arrived at application with delay " << e2eDelay << " s" << std::endl;
    //this->getParentModule()->bubble("Data received!");
    delete apMsg;
}






void DataCentricTestApp::processEnvironmentalData(unsigned char* pkt)
{
    switch ( *pkt )
    {
    case 0x01: // Occupancy
        processOccupancyData(++pkt);
        break;
    case 0x02: // Temperature
        processTemperatureData(++pkt);
        break;
    }

}



void DataCentricTestApp::processDemandData(unsigned char* pkt)
{
    switch ( *pkt )
    {
    case 0x01: // Bid
        processBidData(++pkt);
        break;
    case 0x02: // Watts
        processWattsData(++pkt);
        break;
    }

}




union signedShortData
{
    unsigned char theBytes[2];
    signed short theSignedShort;
};

union unSignedShortData
{
    unsigned char theBytes[2];
    unsigned short theUnSignedShort;
};


void DataCentricTestApp::processWattsData(unsigned char* pkt)
{
    //signed short d;
    signedShortData w;
    unsigned char out[4];
    *(strchr((char*)pkt, 0xFF)) = 0;
    //for ( int i; i < strlen((const char*)pkt); i++ )
    //{
    //    if ( pkt[i] == 0xFF )
    //    {
    //        pkt[i] = 0x0;
    //        break;
    //    }
    //}

    unsigned int len = unescapeBuffer(pkt, out);
    if ( len != 2 )
        throw cRuntimeError("Watts must be 2 bytes signed short");

    w.theBytes[0] = out[0];
    w.theBytes[1] = out[1];
    //d = w.theSignedShort;
}






void DataCentricTestApp::processBidData(unsigned char* pkt)
{
    unsigned short applianceId;
    unsigned short bid;
    unSignedShortData ai;
    unSignedShortData bd;

    unsigned char out[8];
    *(strchr((char*)pkt, 0xFF)) = 0;

    unsigned int len = unescapeBuffer(pkt, out);
    if ( len != 4 )
        throw cRuntimeError("Bid must be 4 bytes appliance Id unsigned short & bid unsigned short");

    ai.theBytes[0] = out[0];
    ai.theBytes[1] = out[1];
    applianceId = ai.theUnSignedShort;

    bd.theBytes[0] = out[2];
    bd.theBytes[1] = out[3];
    bid = bd.theUnSignedShort;

    processABid(applianceId, bid);
    sendActualDemandPkt();

}







void DataCentricTestApp::processABid(unsigned short _applianceId, unsigned short _bid)
{
    double currentTime = simTime().dbl();
    unsigned short myAddr = netModule->mAddress & 0xFFFF;
    double finishTime = simTime().dbl() + _bid;

    NullStream() << "Current Time: " << currentTime << "\n";

    /*
     * If we remove an entry then...
     *
     * what if it is for this appliance?
     *   -  It is happening because we are processing an action stream entry for zero demand
     *   -  We should be running state otherwise there would be no scheduled action
     *   -
     *
     *
     */

    if ( !_bid )
    {
        COUT << "#" << myAddr << "# Appliance " << _applianceId << " bid zero, removing from list" << "\n";
        removeBidByApplianceId(_applianceId);
        if ( _applianceId == myAddr )
        {
            COUT << "#" << myAddr << "# We are going into IDLE state" << "\n";
            setCurrentDemand(0);
            scheduleAt(mOriginalNextDemandActionTime + mDownTime, mDemandActionMessage);
            appState = APPSTATE_IDLE;
            return;
        }
    }

    if ( _bid )
    {
        // Remove first this way we only ever have one
        // per appliance
        removeBidByApplianceId(_applianceId);
        mBids[finishTime] = _applianceId;
    }

    switch ( appState )
    {
        case APPSTATE_IDLE:
            if ( _applianceId == myAddr )
            {
                if ( mBids.begin()->second == myAddr )
                {
                    COUT << "#" << myAddr << "# Appliance " << _applianceId <<
                            " bid " << _bid << ", RUNNING" << "\n";
                    setCurrentDemand(mRequestedDemand);
                    scheduleAt(mOriginalNextDemandActionTime + mDownTime, mDemandActionMessage);
                    appState = APPSTATE_RUNNING;
                }
                else
                {
                    COUT << "#" << myAddr << "# Appliance " << _applianceId <<
                            " bid " << _bid << ", we are PAUSING for " << mBids.begin()->second << "\n";
                    setCurrentDemand(0);
                    mLastOffTime = simTime().dbl();
                    appState = APPSTATE_PAUSED;
                }
            }
            break;
        case APPSTATE_RUNNING:
            if ( mBids.begin()->second != myAddr )
            {
                COUT << "#" << myAddr << "# Appliance " << _applianceId <<
                        " bid " << _bid << ", we are PAUSING for " << mBids.begin()->second << "\n";
                setCurrentDemand(0);
                mLastOffTime = simTime().dbl();
                cancelEvent(mDemandActionMessage);
                appState = APPSTATE_PAUSED;
            }
            break;
        case APPSTATE_PAUSED:
            if ( mBids.begin()->second == myAddr )
            {
                COUT << "#" << myAddr << "# Appliance " << _applianceId <<
                        " bid " << _bid << ", we are RUNNING again"<< "\n";
                mLastOnTime = simTime().dbl();
                mDownTime += (mLastOnTime - mLastOffTime);
                setCurrentDemand(mRequestedDemand);
                scheduleAt(mOriginalNextDemandActionTime + mDownTime, mDemandActionMessage);
                appState = APPSTATE_RUNNING;
            }
            break;
    }

}






void DataCentricTestApp::processOccupancyData(unsigned char* pkt)
{
    if ( *pkt != 1 )// 1 indicates occupancy else indicates absence
    {
        setCurrentDemand(0);
    }

}



void DataCentricTestApp::processTemperatureData(unsigned char* pkt)
{

}




void DataCentricTestApp::setCurrentDemand(signed short _demand)
{
    actualDemand = _demand;

    demandVec.record((double)actualDemand);
    mNetMan->recordDemand();

}



//***************************************************************
// Reimplement this function and use msg type DataCentricAppPkt for app pkts
//***************************************************************
void DataCentricTestApp::handleSelfMsg(cMessage *apMsg)
{

    ActionThreadsIterator i = mActionThreads.find(apMsg);
    if (i != mActionThreads.end() )
    {
        double t = simTime().dbl();
        NullStream() << "Current Time: " << t << "\n";

        switch ( getNextAction(i) )
        {
            case SENSOR_READING:
                SensorReading(i, "\x83\x2");
                break;
            case OCCUPANCY_READING:
                SensorReading(i, "\x83\x1");
                break;
            case TEMP_READING:
                SensorReading(i, "\x83\x2");
                break;
            case SET_PROGRAM:
                startProgram(i);
                break;
            case WATTS:
                processWatts(i);
                break;
            case ACTION_FILE_END:
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
        if ( action == "OCCUPANCY_READING" )
        {
            return OCCUPANCY_READING;
        }
        if ( action == "TEMP_READING" )
        {
            return TEMP_READING;
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
        return ACTION_FILE_END;
    }
}


void DataCentricTestApp::FileEnd(ActionThreadsIterator& i)
{
    ifstream* ifs = i->second->back();
    i->second->pop_back();
    ifs->close();
    delete ifs;

    if ( i->second->size() )
    {
        scheduleAt(simTime(), i->first);
    }
}




//int DataCentricTestApp::getReading(ifstream* ifs)
//{
//    int reading;
//    *ifs >> reading;
//    return reading;
//}



void DataCentricTestApp::processWatts(ActionThreadsIterator& i)
{
    signed short watts;
    ifstream* ifs = i->second->back();
    *ifs >> watts;
    mRequestedDemand = watts;

    double hours;
    double minutes;
    double seconds;
    double finalSeconds;
    *ifs >> hours;
    *ifs >> minutes;
    *ifs >> seconds;
    finalSeconds = (hours*3600)+(minutes*60)+seconds;



    mOriginalNextDemandActionTime = finalSeconds + ScheduleStartTime();
    mDemandActionMessage = i->first;

    bool operatingBidSystem = true;
    if ( operatingBidSystem )
    {
        unsigned short lengthToBidFor;
        if ( watts == 0 )
        {
            lengthToBidFor = 0;
        }
        else
        {
            // check this works
            lengthToBidFor = (unsigned short)(mOriginalNextDemandActionTime - simTime().dbl());
        }
        unsigned short myAddr = netModule->mAddress & 0xFFFF;


        processABid(myAddr, lengthToBidFor);
        sendActualDemandPkt();
        sendAGivenBidPkt(lengthToBidFor);

    }
    else
    {
        setCurrentDemand(watts);
        sendActualDemandPkt();
        scheduleAt(mOriginalNextDemandActionTime, mDemandActionMessage);
    }

}






void DataCentricTestApp::sendActualDemandPkt()
{
    DataCentricAppPkt* appPkt = new DataCentricAppPkt("Watts");
    std::ostringstream ss;
    ss.clear();
    ss << "\x2\x2";

    signedShortData d;
    d.theSignedShort = actualDemand;
    unsigned char escapedData[8];
    //unsigned int len = escapeBuffer(d.theBytes, 2, escapedData);
    escapeBuffer(d.theBytes, 2, escapedData);
    ss << escapedData;

    ss << "\x0";
    std::string s(ss.str());
    //unsigned int a = s.size();
    appPkt->getPktData().insert(appPkt->getPktData().end(), s.begin(), s.end());
    appPkt->setKind(DATA_PACKET);
    send(appPkt, mLowerLayerOut);
}


void DataCentricTestApp::sendAGivenBidPkt(unsigned short lengthToBidFor)
{
    unsigned short myAddr = netModule->mAddress & 0xFFFF;

    DataCentricAppPkt* appPkt = new DataCentricAppPkt("Bid");
    std::ostringstream ss;
    ss.clear();
    ss << "\x42\x1";

    unSignedShortData d;
    d.theUnSignedShort = myAddr;
    unsigned char escapedData[8];
    //unsigned int len = escapeBuffer(d.theBytes, 2, escapedData);
    escapeBuffer(d.theBytes, 2, escapedData);
    ss << escapedData;

    d.theUnSignedShort = lengthToBidFor;
    //len = escapeBuffer(d.theBytes, 2, escapedData);
    escapeBuffer(d.theBytes, 2, escapedData);
    ss << escapedData;
    ss << "\x0";
    std::string s(ss.str());

    // check size, follow size and content
    //unsigned int a = s.size();
    appPkt->getPktData().insert(appPkt->getPktData().end(), s.begin(), s.end());
    appPkt->setKind(DATA_PACKET);
    send(appPkt, mLowerLayerOut);
}






void DataCentricTestApp::startProgram(ActionThreadsIterator& i)
{
    string program;
    ifstream* ifs = i->second->back();
    *ifs >> program;

    ifstream* newIfs = new ifstream();
    newIfs->open(program.c_str());
    i->second->push_back(newIfs);

    // need to test this out not sure about this line
    scheduleAt(simTime(), i->first);

}


void DataCentricTestApp::SensorReading(ActionThreadsIterator& i, const char* sensorDataName)
{
    ifstream* ifs = i->second->back();

    unsigned short reading;
    //double period;
    *ifs >> reading;
    //*ifs >> period;

    double hours;
    double minutes;
    double seconds;
    double finalSeconds;
    *ifs >> hours;
    *ifs >> minutes;
    *ifs >> seconds;
    finalSeconds = (hours*3600)+(minutes*60)+seconds;


    DataCentricAppPkt* appPkt = new DataCentricAppPkt("Sensor_Data");
    std::ostringstream ss;
    ss.clear();
    //ss << "\x83\x1";
    ss << sensorDataName;
    ss << (unsigned char)(reading & 0xff);
    ss << (unsigned char)((reading >>8) & 0xff);
    ss << "\x0";
    std::string s(ss.str());
    appPkt->getPktData().insert(appPkt->getPktData().end(), s.begin(), s.end());
    appPkt->setKind(DATA_PACKET);
    send(appPkt, mLowerLayerOut);

    //scheduleAt(simTime() + period, i->first);
    scheduleAt(finalSeconds + ScheduleStartTime(), i->first);
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

    /*
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

        //Ieee802154UpperCtrlInfo *control_info = new Ieee802154UpperCtrlInfo();
        //control_info->setDestName(apDest);
        //appPkt->setControlInfo(control_info);

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
*/


}


double DataCentricTestApp::NodeStartTime()
{
    return par("nodeStartTime").doubleValue();
}



double DataCentricTestApp::TimeSinkRegisterWithControlUnit()
{
    double result = par("timeSinkRegisterWithControlUnit").doubleValue();
    return result;

}



double DataCentricTestApp::TimeSourceRegisterWithControlUnit()
{
    return par("timeSourceRegisterWithControlUnit").doubleValue();
}







double DataCentricTestApp::ScheduleStartTime()
{
    return par("scheduleStartTime").doubleValue();
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
