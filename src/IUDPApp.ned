// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package datacentric;

//
// Template for UDP applications. It shows what gates a UDP app
// needs, to be able to be used in ~StandardHost etc.
//
moduleinterface IUDPApp
{
    parameters:
        @display("i=block/app");
    gates:
        input udpIn;// @labels(UDPControlInfo/up);
        output udpOut;// @labels(UDPControlInfo/down);
        input datacentricAppIn;
        output datacentricAppOut;
}
