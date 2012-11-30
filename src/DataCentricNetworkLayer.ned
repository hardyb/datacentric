



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
		volatile double routingDelay @unit("s") = default(uniform(0s,0.005s));        
        int meanDownTimeInterval = default(0); // NOT USED? Frequency with which down time occurs in seonds
        double meanDownTime = default(0); // NOT USED? Proportion of the time the nodes is down as a fraction
    gates:
        input upperLayerIn; 			// from the upper layer
        output upperLayerOut; 		// to the upper layer
        input lowerLayerIn; 			// from the physical layer
        output lowerLayerOut; 			// to the physical layer
}
