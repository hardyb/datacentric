



#include "DataCentricAppPkt_D.h"

#include "RoutingAndAggregation.h"




Register_Class(DataCentricAppPkt);



const char *DataCentricAppPkt::getDisplayString() const
{
    char msgType = *(getPktData().begin());

    switch ( msgType )
    {
        case 0x04: // bcast - white
            return "b=10,10,oval,white,white,1";
            break;
        case 0x05: // ucast - discontinued?
            return "b=10,10,oval,black,black,1";
            break;
        case 0x01: // interest - green
            return "b=10,10,oval,green,green,1";
            break;
        case 0x06: // reinforce interest - green with black dot
            return "b=10,10,oval,black,green,5";
            break;
        case 0x03: // data - blue
            return "b=10,10,oval,blue,blue,1";
            break;
        case INTEREST_CORRECTION: // INTEREST_CORRECTION
            return "b=10,10,oval,white,green,5";
            break;
        case 0x0A: // Breakage
            return "b=10,10,oval,yellow,yellow,1";
            break;
        default:
            return "b=20,20,rect";
            break;
    }
    //return "b=20,20,rect";
}
