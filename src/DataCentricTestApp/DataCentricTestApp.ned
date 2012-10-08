package datacentric.DataCentricTestApp;
import  inet.applications.trafgen.TrafGenPar;

simple DataCentricTestApp extends TrafGenPar
{
    parameters:
        @class(DataCentricTestApp);
        bool debug;
        string xxx = default("hello");
        
        string sinkFor = default("");
        string sourceFor = default("");// = "\x48\x49\x54";
        string nodeContext = default("");
        string appMode = default("");
        bool mains = default(true); // TO BE DISCONTINUED?
        double probabilityDown = default(0); // TO BE DISCONTINUED?
        string actionThreads = default("");
        
        //sourceFor[5]: string;
        //int x[5];
        double nodeStartTime = default(0.0);
        double scheduleStartTime = default(0.0);
        bool setParametersDirectly = default(false);
        
        double timeSinkRegisterWithControlUnit = default(0.0);
        double timeSourceRegisterWithControlUnit = default(0.0);
        double timeSinkBindWithSourceDevice = default(0.0);
        bool isAppliance = default(false);
        double otherAction1 = default(0.0);
        double otherAction2 = default(0.0);
        
        
        //sinks[numSinks] : 
        //byte y;
        //char x;
        
   
}
