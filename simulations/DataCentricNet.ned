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

//package inet.examples.wpan.StarNet;
//package datacentric;

import datacentric.DataCentricNode;
import datacentric.DataCentricNetworkMan;
import inet.world.radio.ChannelControl;
import datacentric.Region;



network DataCentricNet
{
    parameters:
        int numRegions = default(4);
        double numHosts;
        double nodeSeparation @unit("m")=default (3m);
        int iVar1 = default(0);
        int iVar2 = default(0);
        int iVar3 = default(0);
        int iVar4 = default(0);
        string nodeParsUsed = default("");
    submodules:
        region[numRegions]: Region;
        host[numHosts]: DataCentricNode {
            parameters:
                  //y =  (floor(index/sqrt(numHosts))*nodeSeparation)+(nodeSeparation);
                  //x = ((index%floor(sqrt(numHosts)))*nodeSeparation)+(nodeSeparation);
                  
                  
                  //ss = 15 + (floor(fmod(index,4)/3)*20);//(fmod(index,4)*10)+10;
                  //@display("b=$ss;i=device/pocketpc_s;r=0,,,0");//;r=,,#707070");
                  @display("i=device/pocketpc_s;r=,,#707070");
                  //@display("i=device/pocketpc_s;r=,,#707070;p=84,57");
        }
        dataCentricNetworkMan : DataCentricNetworkMan{
            parameters:
                @display("p=-10,30");
        }
        channelControl: ChannelControl {
            parameters:
                @display("p=-10,20;i=misc/sun");
        }
        connections allowunconnected:
}

