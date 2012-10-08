


#include "HostReference.h"


Define_Module(HostReference);

void HostReference::initialize(int aStage)
{
    cSimpleModule::initialize(aStage); //DO NOT DELETE!!
    if (0 == aStage)
    {
        underlyingModule = NULL;
    }

    if (2 == aStage)
    {
        if ( underlyingModule )
        {
            if ( underlyingModule->par("setParametersDirectly").boolValue()
                    && !par("setHostParametersDirectly").boolValue() )
            {
                std::string s = underlyingModule->getFullPath() + " is set direct and referenced by "
                        + getFullPath() + " which is not set direct";
                throw cRuntimeError(s.c_str());
            }

            if ( !underlyingModule->par("setParametersDirectly").boolValue()
                    && par("setHostParametersDirectly").boolValue() )
            {
                std::string s = underlyingModule->getFullPath() + " is not set direct and referenced by "
                        + getFullPath() + " which is set direct";
                throw cRuntimeError(s.c_str());
            }

            if ( !underlyingModule->par("setParametersDirectly").boolValue()
                    && !par("setHostParametersDirectly").boolValue() )
            {
                std::string sinkFor = par("sinkFor").stringValue();
                if ( sinkFor.size() )
                {
                    ev << "Setting sinkFor in:    " << getFullPath() << std::endl;
                }
                std::string sourceFor = par("sourceFor").stringValue();
                if ( sourceFor.size() )
                {
                    ev << "Setting sourceFor in:    " << getFullPath() << std::endl;
                }
                std::string actionThreads = par("actionThreads").stringValue();
                if ( actionThreads.size() )
                {
                    ev << "Setting actionThreads in:    " << getFullPath() << std::endl;
                }

                ev << "underlyingModule->contextData = ";// << underlyingModule->contextData <<
                for ( std::string::iterator i = underlyingModule->contextData.begin(); i != underlyingModule->contextData.end(); i++ )
                {
                    char cval1 =  (*i);
                    unsigned char cval2 =  (*i);
                    unsigned int val = (unsigned int)cval2;
                    ev << std::hex << std::uppercase << "\\" << val;
                }
                ev << std::endl;

                underlyingModule->par("sinkFor").setStringValue(sinkFor);
                underlyingModule->par("sourceFor").setStringValue(sourceFor);
                underlyingModule->par("nodeContext").setStringValue(par("nodeContext").stringValue());
                underlyingModule->par("appMode").setStringValue(par("appMode").stringValue());
                underlyingModule->par("mains").setBoolValue(par("mains").boolValue());
                underlyingModule->par("probabilityDown").setDoubleValue(par("probabilityDown").doubleValue());
                underlyingModule->par("actionThreads").setStringValue(actionThreads);
                underlyingModule->par("nodeStartTime").setDoubleValue(par("nodeStartTime").doubleValue());
                underlyingModule->par("scheduleStartTime").setDoubleValue(par("scheduleStartTime").doubleValue());

                underlyingModule->par("timeSinkRegisterWithControlUnit").setDoubleValue(par("timeSinkRegisterWithControlUnit").doubleValue());
                underlyingModule->par("timeSourceRegisterWithControlUnit").setDoubleValue(par("timeSourceRegisterWithControlUnit").doubleValue());
                underlyingModule->par("timeSinkBindWithSourceDevice").setDoubleValue(par("timeSinkBindWithSourceDevice").doubleValue());
                underlyingModule->par("isAppliance").setBoolValue(par("isAppliance").boolValue());

            }





        }
        else
        {
            //throw cRuntimeError("underlyingModule has not been set");
        }
    }





}

void HostReference::finish()
{
}


void HostReference::setUnderlyingModule(DataCentricTestApp* _underlyingModule)
{
    Enter_Method("setUnderlyingModule(cModule* _underlyingModule)");

    underlyingModule = _underlyingModule;
}

bool HostReference::hasUnderlyingModule()
{
    Enter_Method("hasUnderlyingModule()");

    return (underlyingModule != NULL);

}



void HostReference::handleLowerMsg(cMessage* apMsg)
{
}

void HostReference::handleSelfMsg(cMessage *apMsg)
{

    //TrafGenPar::handleSelfMsg(apMsg);
}










/*
if ( underlyingModule->par("nodeStartTime").containsValue() )
{
    string fp = underlyingModule->getFullPath();
    ev << fp << ": nodeStartTime HAS value" << std::endl;
}
else
{
    string fp = underlyingModule->getFullPath();
    ev << fp << ": nodeStartTime has NOT value" << std::endl;
}


if ( underlyingModule->par("nodeStartTime").isSet() )
{
    string fp = underlyingModule->getFullPath();
    ev << fp << ": nodeStartTime IS set" << std::endl;
}
else
{
    string fp = underlyingModule->getFullPath();
    ev << fp << ": nodeStartTime NOT set" << std::endl;
}
*/




//string fp = underlyingModule->getFullPath();
//if ( strcmp(underlyingModule->getFullPath().c_str(), "csma802154net.fixhost[115].app") )
//{
//    underlyingModule->par("nodeStartTime").setDoubleValue(par("nodeStartTime").doubleValue());
//}
//else
//{
//    if ( underlyingModule->par("nodeStartTime").doubleValue() != 1.0 )
//    {
//        throw cRuntimeError("115 must be nodeStartTime 1.0");
//    }
//}

