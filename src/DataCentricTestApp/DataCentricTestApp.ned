package datacentric.DataCentricTestApp;
import  inet.applications.trafgen.TrafGenPar;

simple DataCentricTestApp extends TrafGenPar
{
    parameters:
        @class(DataCentricTestApp);
        
        // IMPORTANT
        // #######################
        // These parameters are being set indirectly through a host reference
        // which resides in a geographical region module
        //
        // Since there is no way in omnet to know whether a parameter
        // has been set from default or set programatically through the reference
        // we must impose a manual rule that in this ned file parameters MUST
        // have the following defaults
        //
        // int    -  0
        // double - 0.0
        // bool   - false
        // string - ""
        //
        // Also if we set setParametersDirectly true then
        // the host referene model will ignore this node
         
        bool debug = default(flase);
        string sinkFor = default("");
        string collaboratorInitiatorFor = default("");
        string collaboratorFor = default("");
        string sourceFor = default("");// = "\x48\x49\x54";
        bool participate = default(true);
        string nodeContext = default("");
        string appMode = default("");
        bool mains = default(false); // TO BE DISCONTINUED?
        double probabilityDown = default(0); // TO BE DISCONTINUED?
        string actionThreads = default("");
        int none = 0;
        int firstPacket = 1;
        int allPackets = 2;
        int other = 3;
        int simAppTerminationReason @enum(none,firstPacket,allPackets,other) = default(none);
        bool isControlUnit = default(false); // Whether this app/node is the server control unit
        bool isRoot = default(false); // Proactive RREQ
        double first_proactiveRreqTimeout = default(0);// 0 seconds
        string controlUnit = default("");
        
        //sourceFor[5]: string;
        //int x[5];
        double nodeStartTime = default(0.0);
        double scheduleStartTime = default(0.0);
        
        bool setParametersDirectly = default(false);
        
        double timeSinkRegisterWithControlUnit = default(0.0);
        double timeSourceRegisterWithControlUnit = default(0.0);
        double timeSinkBindWithSourceDevice = default(0.0);
        bool isAppliance = default(false);
        
}
