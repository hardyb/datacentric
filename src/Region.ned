//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package datacentric;

module Region
{
    parameters:
        int numRegionHosts = default(125);
        double x = default(0); // x, y, z coordinates of starting corner of the region/room
        double y = default(0);
        double z = default(0);
        double w = default(0); // width of the room (x axis)
        double h = default(0); // length of the room front to back (y axis)
        double d = default(0); // height of the room (z axis)
        string context = default(""); // the datacentric context or hierarchical location id of the region 
        @display("p=-10,40");
    submodules:
        regionHost[numRegionHosts]: HostReference;
    
    connections allowunconnected:
        
}





