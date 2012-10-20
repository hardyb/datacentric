/*
 * HostReference.h
 *
 *  Created on: 24 Sep 2012
 *      Author: AndrewHardy
 */

#ifndef HOSTREFERENCE_H_
#define HOSTREFERENCE_H_

#include <omnetpp.h>
#include "DataCentricTestApp.h"


class HostReference  : public cSimpleModule
{
  public:
    virtual int    numInitStages    () const { return 3; }



    virtual void initialize(int);
    virtual void finish();
    void setUnderlyingModule(DataCentricTestApp* _underlyingModule);
    bool hasUnderlyingModule();


  protected:

    // OPERATIONS
    virtual void handleSelfMsg(cMessage*);
    virtual void handleLowerMsg(cMessage*);
    void SetString(cPar& _par, const char * _s);
    void SetDouble(cPar& _par, double _d);
    void SetBool(cPar& _par, bool _b);
    void SetInt(cPar& _par, int _i);




  private:
    bool    m_debug;        // debug switch
    int     mLowerLayerIn;
    int     mLowerLayerOut;

    DataCentricTestApp* underlyingModule;

};

#endif /* HOSTREFERENCE_H_ */
