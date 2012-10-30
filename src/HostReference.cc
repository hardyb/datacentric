


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
            if ( !strcmp(underlyingModule->getParentModule()->getFullName(), "fixhost[4]") )
            {
                string fn = getFullPath();
                cout << "We are here!" << endl;
            }

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


                std::string collaboratorInitiatorFor = par("collaboratorInitiatorFor").stringValue();
                if ( collaboratorInitiatorFor.size() )
                {
                    ev << "Setting collaboratorInitiatorFor in:    " << getFullPath() << std::endl;
                }
                std::string collaboratorFor = par("collaboratorFor").stringValue();
                if ( collaboratorFor.size() )
                {
                    ev << "Setting collaboratorFor in:    " << getFullPath() << std::endl;
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


                //underlyingModule->par("sinkFor").setStringValue(sinkFor);
                this->SetString(underlyingModule->par("sinkFor"), sinkFor.c_str());

                //underlyingModule->par("sourceFor").setStringValue(sourceFor);
                this->SetString(underlyingModule->par("sourceFor"), sourceFor.c_str());

                //underlyingModule->par("collaboratorInitiatorFor").setStringValue(collaboratorInitiatorFor);
                this->SetString(underlyingModule->par("collaboratorInitiatorFor"), collaboratorInitiatorFor.c_str());

                //underlyingModule->par("collaboratorFor").setStringValue(collaboratorFor);
                this->SetString(underlyingModule->par("collaboratorFor"), collaboratorFor.c_str());

                //underlyingModule->par("nodeContext").setStringValue(par("nodeContext").stringValue());
                this->SetString(underlyingModule->par("nodeContext"), par("nodeContext").stringValue());

                //underlyingModule->par("appMode").setStringValue(par("appMode").stringValue());
                this->SetString(underlyingModule->par("appMode"), par("appMode").stringValue());

                //underlyingModule->par("mains").setBoolValue(par("mains").boolValue());
                this->SetBool(underlyingModule->par("mains"), par("mains").boolValue());

                //underlyingModule->par("probabilityDown").setDoubleValue(par("probabilityDown").doubleValue());
                this->SetDouble(underlyingModule->par("probabilityDown"), par("probabilityDown").doubleValue());

                //underlyingModule->par("actionThreads").setStringValue(actionThreads);
                this->SetString(underlyingModule->par("actionThreads"), actionThreads.c_str());

                //underlyingModule->par("nodeStartTime").setDoubleValue(par("nodeStartTime").doubleValue());
                this->SetDouble(underlyingModule->par("nodeStartTime"), par("nodeStartTime").doubleValue());

                //underlyingModule->par("scheduleStartTime").setDoubleValue(par("scheduleStartTime").doubleValue());
                this->SetDouble(underlyingModule->par("scheduleStartTime"), par("scheduleStartTime").doubleValue());

                //underlyingModule->par("timeSinkRegisterWithControlUnit").setDoubleValue(par("timeSinkRegisterWithControlUnit").doubleValue());
                this->SetDouble(underlyingModule->par("timeSinkRegisterWithControlUnit"), par("timeSinkRegisterWithControlUnit").doubleValue());

                //underlyingModule->par("timeSourceRegisterWithControlUnit").setDoubleValue(par("timeSourceRegisterWithControlUnit").doubleValue());
                this->SetDouble(underlyingModule->par("timeSourceRegisterWithControlUnit"), par("timeSourceRegisterWithControlUnit").doubleValue());

                underlyingModule->par("timeSinkBindWithSourceDevice").setDoubleValue(par("timeSinkBindWithSourceDevice").doubleValue());
                this->SetDouble(underlyingModule->par("timeSinkBindWithSourceDevice"), par("timeSinkBindWithSourceDevice").doubleValue());

                //underlyingModule->par("isAppliance").setBoolValue(par("isAppliance").boolValue());
                this->SetBool(underlyingModule->par("isAppliance"), par("isAppliance").boolValue());
            }





        }
        else
        {
            //throw cRuntimeError("underlyingModule has not been set");
        }
    }





}





void HostReference::SetString(cPar& _par, const char * _s)
{
    if ( strcmp(_par.stringValue(), "")
            && strcmp(_s, "") )
    {
        // the underlying host has already been set non-default
        // this host ref is trying to set additional non-default again
        std::string s = underlyingModule->getFullPath() + " (" + _par.getName() + ") has already been set non-" +
                "default somewhere else, and now " + getFullPath() +
                " is trying to set it non-default as well.";
        throw cRuntimeError(s.c_str());
        return;
    }

    if ( !strcmp(_par.stringValue(), "")
            && strcmp(_s, "") )
    {
        // the underlying host is still set to default
        // this host ref is trying to set first non-default, so that is fine
        _par.setStringValue(_s);
        return;
    }

    if ( strcmp(_par.stringValue(), "")
            && !strcmp(_s, "") )
    {
        // the underlying host has already been set non-default
        // this additional host ref referencing same host has been left as default
        // and so can be ignored
        return;
    }
}

void HostReference::SetDouble(cPar& _par, double _d)
{
    if ( _par.doubleValue() != 0.0
            && _d != 0.0 )
    {
        // the underlying host has already been set non-default
        // this host ref is trying to set additional non-default again
        std::string s = underlyingModule->getFullPath() + " (" + _par.getName() + ") has already been set non-" +
                "default somewhere else, and now " + getFullPath() +
                " is trying to set it non-default as well.";
        throw cRuntimeError(s.c_str());
        return;
    }

    if ( _par.doubleValue() == 0.0
            && _d != 0.0 )
    {
        // the underlying host is still set to default
        // this host ref is trying to set first non-default, so that is fine
        _par.setDoubleValue(_d);
        return;
    }

    if ( _par.doubleValue() != 0.0
            && _d == 0.0 )
    {
        // the underlying host has already been set non-default
        // this additional host ref referencing same host has been left as default
        // and so can be ignored
        return;
    }

}

void HostReference::SetBool(cPar& _par, bool _b)
{
    if ( _par.boolValue() != false
            && _b != false )
    {
        // the underlying host has already been set non-default
        // this host ref is trying to set additional non-default again
        std::string s = underlyingModule->getFullPath() + " (" + _par.getName() + ") has already been set non-" +
                "default somewhere else, and now " + getFullPath() +
                " is trying to set it non-default as well.";
        throw cRuntimeError(s.c_str());
        return;
    }

    if ( _par.boolValue() == false
            && _b != false )
    {
        // the underlying host is still set to default
        // this host ref is trying to set first non-default, so that is fine
        _par.setBoolValue(_b);
        return;
    }

    if ( _par.boolValue() != false
            && _b == false )
    {
        // the underlying host has already been set non-default
        // this additional host ref referencing same host has been left as default
        // and so can be ignored
        return;
    }

}

void HostReference::SetInt(cPar& _par, int _i)
{
    if ( _par.longValue() != 0
            && _i != 0 )
    {
        // the underlying host has already been set non-default
        // this host ref is trying to set additional non-default again
        std::string s = underlyingModule->getFullPath() + " (" + _par.getName() + ") has already been set non-" +
                "default somewhere else, and now " + getFullPath() +
                " is trying to set it non-default as well.";
        throw cRuntimeError(s.c_str());
        return;
    }

    if ( _par.longValue() == 0
            && _i != 0 )
    {
        // the underlying host is still set to default
        // this host ref is trying to set first non-default, so that is fine
        _par.setLongValue(_i);
        return;
    }

    if ( _par.longValue() != 0
            && _i == 0 )
    {
        // the underlying host has already been set non-default
        // this additional host ref referencing same host has been left as default
        // and so can be ignored
        return;
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

