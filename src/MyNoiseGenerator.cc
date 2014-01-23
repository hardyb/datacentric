/*
 * MyNoiseGenerator.cc
 *
 *  Created on: 11 Nov 2013
 *      Author: AndrewHardy
 */

#include "MyNoiseGenerator.h"
#include "FWMath.h"
#include "Ieee802154Phy.h"
#include "DataCentricNetworkLayer.h"
#include "DataCentricNetworkMan.h"


//Define_Module( MyNoiseGenerator );
Register_Class( MyNoiseGenerator );


extern DataCentricNetworkMan* netWkMan; // change name


//cModule* MyNoiseGenerator::radio = NULL;

//double MyNoiseGenerator::nLevel = 0;
simsignal_t MyNoiseGenerator::changeLevelNoise = SIMSIGNAL_NULL;
//MyNoiseGenerator MyNoiseGenerator::noiseGeneratorManager = MyNoiseGenerator();
//MyNoiseGenerator* MyNoiseGenerator::noiseGeneratorManager = 0;
//MyNoiseGenerator MyNoiseGenerator::noiseGeneratorManager = MyNoiseGenerator();

//MyNoiseGenerator MyNoiseGenerator::noiseGeneratorManager = MyNoiseGenerator;

//MyNoiseGenerator MyNoiseGenerator::noiseGeneratorManager;


//cComponent


//void MyNoiseGenerator::handleMessage(cMessage *msg)
//{
//    if ( msg == mNoiseMessage )
//    {
//        nLevel = nLevel != 0 ? 0 : -40;
//        scheduleAt(simTime() + 0.05, mNoiseMessage);
//        //scheduleAt(simTime() + uniform(0,0.1), mNoiseMessage);
//    }


//}


//void MyNoiseGenerator::initialize()
//{
//    mNoiseMessage = new cMessage("noiseMessage");

    //scheduleAt(simTime() + 0.05, mNoiseMessage);
    //scheduleAt(simTime() + uniform(0,0.1), mNoiseMessage);


//}


//void MyNoiseGenerator::initialize(int stage)
//{
//    cSimpleModule::initialize(stage);

    //changeLevelNoise = registerSignal("changeLevelNoise");



//}

//MyNoiseGenerator::MyNoiseGenerator()
//{
//    nLevel = 0;
    //changeLevelNoise = registerSignal("changeLevelNoise");

    //changeLevelNoise = registerSignal("changeLevelNoise");

//}


//MyNoiseGenerator::MyNoiseGenerator(const MyNoiseGenerator&)
//{
//
//}


void MyNoiseGenerator::initializeFrom(cModule *radioModule)
{
    //if ( !radio )
    //{
        radio = radioModule;
    //}

    netWkMan->myNoiseGenerators.push_back(this);

    //myNoiseGenerators

    //DataCentricNetworkLayer* c = check_and_cast<DataCentricNetworkLayer*>
    //                (radioModule->getParentModule()->getParentModule()->getSubmodule("net"));
    //c->myNoise = this;
    //c->par("");
    //c->par("");



    changeLevelNoise = radio->registerSignal("changeLevelNoise80215");
    //nLevel = 0;
    nLevel = FWMath::dBm2mW(-99);

}


double MyNoiseGenerator::noiseLevel()
{
    //float b = 10;
    //sqrt(-2 * pow(b,2) * log(uniform(0,1)) );

    //double db = uniform(-93, -95); - 3/12 got through (thought it would be 100% collisions)
    //double db = uniform(-93, -96); - 10/12 got through
    //double db = uniform(-91, -95.5);
    //double db = uniform(-95, -94);

    // can use this while testing to see effect of only updating noise
    // when reading the noise
    //double noiseLower = netWkMan->par("noiseLower").doubleValue();
    //double noiseUpper = netWkMan->par("noiseUpper").doubleValue();
    //double db = uniform(noiseLower, noiseUpper);
    //double db = uniform(noiseUpper, noiseLower);
    //nLevel = FWMath::dBm2mW(db);

    return nLevel;

}



void MyNoiseGenerator::setNoiseLevel(double _nLevel)
{

    nLevel = FWMath::dBm2mW(_nLevel);
    //nLevel = _nLevel;
    //cObject *obj = 0;

    // signal the radio here
    //emit(changeLevelNoise, obj);
    //emit(changeLevelNoise, 0);
    //emit(changeLevelNoise, NULL);
    //noiseGeneratorManager.emit(noiseGeneratorManager.changeLevelNoise, 0);
    //radio->emit(changeLevelNoise, radio);


    cObject *obj = 0;
    radio->emit(changeLevelNoise, obj);




}

