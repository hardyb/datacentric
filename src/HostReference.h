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




  private:
    bool    m_debug;        // debug switch
    int     mLowerLayerIn;
    int     mLowerLayerOut;

    DataCentricTestApp* underlyingModule;

};

#endif /* HOSTREFERENCE_H_ */
