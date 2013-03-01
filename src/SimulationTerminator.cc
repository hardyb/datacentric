
#include "cSimulation.h"
#include "cmodule.h"
#include "cexception.h"
#include "SimulationTerminator.h"
#include <string>
using namespace std;



bool pleaseTerminate;
string terminationReasonString;


void setTerminate(const char* s, int _terminationReason)
{
    int tReason = cSimulation::getActiveSimulation()->getContextModule()->par("simAppTerminationReason").longValue();
    if ( _terminationReason == tReason )
    {
        pleaseTerminate = true;
        terminationReasonString = s;
    }
}
void considerTerminateTheSimulation()
{
    if ( pleaseTerminate )
    {
        throw cTerminationException(terminationReasonString.c_str());
    }
}
