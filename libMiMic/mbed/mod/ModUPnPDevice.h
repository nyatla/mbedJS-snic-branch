#include "NyLPC_net.h"
#include "ModBaseClass.h"
#include "HttpdConnection.h"
#include "Httpd.h"
#include "Net.h"

namespace MiMic
{
    class HttpdConnection;

    /**
     * This class is UPnPDevice module.
     * The class provides 3 services.
     * <ul>
     * <li>d.xml - a device description.</li>
     * <li>control/xx - soap handler</li>
     * <li>event/xx -event handler.</li>
     * </ul>
     */
    class ModUPnPDevice:ModBaseClass
    {
    private:
        const Net* _ref_net;
    public:
        ModUPnPDevice();
        ModUPnPDevice(const Net& i_ref_upnp);
        virtual ~ModUPnPDevice();
        void setParam(const Net& i_ref_upnp);
        bool execute(HttpdConnection& i_connection);
    };
}