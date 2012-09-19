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

simple Region
{
    parameters:
        int x; // x, y, z coordinates of starting corner of the region/room
        int y;
        int z;
        int w; // width of the room (x axis)
        int h; // length of the room front to back (y axis)
        int d; // height of the room (z axis)
        string context; // the datacentric context or hierarchical location id of the region 
    //submodules:
}





