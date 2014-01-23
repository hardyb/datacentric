/*
 * MyNoiseGenerator.h
 *
 *  Created on: 11 Nov 2013
 *      Author: AndrewHardy
 */

#ifndef MYNOISEGENERATOR_H_
#define MYNOISEGENERATOR_H_

#include <omnetpp.h>

#include "cdataratechannel.h"
#include "compat.h"
#include "csimplemodule.h"
#include "INoiseGenerator.h"


//class MyNoiseGenerator;


//class MyNoiseGenerator : public INoiseGenerator , public cSimpleModule
class MyNoiseGenerator : public INoiseGenerator //, public cSimpleModule
{
public:
    /**
     * Allows parameters to be read from the module parameters of a
     * module that contains this object.
     */
    //static double nLevel;
    double nLevel;
    //static MyNoiseGenerator noiseGeneratorManager;

    //MyNoiseGenerator();

    //void initialize(int stage);

    //void handleMessage(cMessage *msg);
    //void initialize();

    void initializeFrom(cModule *radioModule);
    double noiseLevel();
    //static void setNoiseLevel(double _nLevel);
    void setNoiseLevel(double _nLevel);

private:
    //static cModule* radio;
    cModule* radio;
    //cMessage* mNoiseMessage;

protected:
    static simsignal_t changeLevelNoise;
    //static simsignal_t changeLevelNoise;

};















#endif /* MYNOISEGENERATOR_H_ */
