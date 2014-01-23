//
// Copyright (C) 2008 Feng Chen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//package inet.examples.wpan.csma802154;
//package datacentric;

import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.world.radio.ChannelControl;
import datacentric.nodecsma802154;
import datacentric.DataCentricNetworkMan;
//import datacentric.
import datacentric.Region;


network csma802154net
{
    parameters:
        int numRegions = default(4);
        int numFixHosts;
        int iVar3 = default(0);
        string nodeParsUsed = default("");
        double nodeSeparation @unit("m")=default (3m); // 3m for home network
        int numMainsNodes = default(0);
    submodules:
        region[numRegions]: Region;
        fixhost[numFixHosts]: nodecsma802154 {
            parameters:
              //y =  (floor(index/sqrt(numFixHosts))*nodeSeparation)+(nodeSeparation);
              //x = ((index%floor(sqrt(numFixHosts)))*nodeSeparation)+(nodeSeparation);
              @display("i=device/pocketpc_s;r=,,#707070");
        }
        dataCentricNetworkMan : DataCentricNetworkMan{
            parameters:
                @display("p=-10,30");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                networkAddress = "145.236.0.0";
                netmask = "255.255.0.0";
                @display("p=140,50;i=block/cogwheel_s");
        }
        //for i=0..numFixHosts {
        //    fixhost[i].app.sinkFor = "xx";
        //}
        
    connections allowunconnected:
}


