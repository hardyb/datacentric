



package datacentric;
simple DataCentricNetworkLayer
{
    parameters:
        // debug switch
        bool debug; 				// debug switch
        bool isPANCoor;
        int nodeConstraint = default(0); // NOT USED?
        double Stability;
        //double startTime;
		
		// volatile double routingDelay @unit("s") = default(uniform(0s,0.005s)); 
		// if we are using replicas we don't want volatile        
		double routingDelay @unit("s") = default(uniform(0.040s,0.042s));        
		//double routingDelay @unit("s") = default(uniform(0s,0.005s));        
        
        int meanDownTimeInterval = default(0); // NOT USED? Frequency with which down time occurs in seonds
        double meanDownTime = default(0); // NOT USED? Proportion of the time the nodes is down as a fraction
        bool ignoreFirstDataPackets = default(false); // when measuring ongoing transmissions
        int pathCostMethod = default(0); // PC_ADD_LQI 0, PC_MULT_LQI 1, PC_LOWEST_MAX_LINK 2
        bool XBeeModule = default(false); // use XBee pkt size, digi config, etc
        bool RSSI_Test = default(false); // use just to test RSSI
        
    gates:
        input upperLayerIn; 			// from the upper layer
        output upperLayerOut; 		// to the upper layer
        input lowerLayerIn; 			// from the physical layer
        output lowerLayerOut; 			// to the physical layer
}
