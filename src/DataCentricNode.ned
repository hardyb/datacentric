package datacentric;


//import inet.wpan.linklayer.ieee802154.Ieee802154Nic;
import inet.wpan.linklayer.ieee802154.Ieee802154csmaNic;


//import inet.wpan.networklayer.Ieee802154StarRouting.Ieee802154StarRouting;
import datacentric.DataCentricNetworkLayer;

//import inet.underTest.wpan.battery.Ieee802154Battery;
import inet.battery.models.InetSimpleBattery;
//import inet.wpan.applications.Ieee802154TestApp.Ieee802154TestApp;
import datacentric.DataCentricTestApp.DataCentricTestApp;




import inet.mobility.IMobility;
import inet.base.NotificationBoard;
import inet.wpan.Isabel.Base.Display;


import inet.mobility.models.StationaryMobility;



//
// This module implements simple ZigBee nodes with IEEE 802.15.4 MAC and PHY layer, plus a simple test app layer
//

module DataCentricNode
{
    parameters:
        @node();
        int x @unit("m");
        int y @unit("m");
        //int ss;
        string mobilityType;
        volatile double numHosts;
    gates:
        input radioIn;
    submodules:
        notificationBoard: NotificationBoard {
            parameters:
                @display("p=65,70;i=block/control");

        }
        mobility: StationaryMobility {//<mobilityType> like IMobility {
            parameters:
                initialX = x;
                initialY = y;
                @display("p=65,150;i=block/cogwheel");

        }
        disp: Display {
            parameters:
                @display("p=65,230;i=block/app2");

        }
        // battery module
        battery: InetSimpleBattery{
            parameters:
                @display("p=65,310;i=block/plug");

        }
        // application-layer
        app: DataCentricTestApp {
            parameters:
                @display("p=250,100;i=block/source");

        }
        // network layer
        net: DataCentricNetworkLayer {
            parameters:
                @display("p=250,200;i=block/network2");

        }
        // network interface
        //nic: Ieee802154Nic {
        nic: Ieee802154csmaNic {
                parameters:
                @display("p=250,300;q=queue;i=block/ifcard");

        }
    connections allowunconnected:
        radioIn --> nic.radioIn;
        nic.upperLayerOut --> net.lowerLayerIn;
        nic.upperLayerIn <-- net.lowerLayerOut;
        net.upperLayerOut --> app.lowerLayerIn;
        net.upperLayerIn <-- app.lowerLayerOut;
}
