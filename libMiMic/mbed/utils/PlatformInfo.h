#pragma once

namespace MiMic
{
    #define PlatformInfo_DETECTION_MODE_AUTO 1
    #define PlatformInfo_DETECTION_MODE_MBED 2
    #define PlatformInfo_DETECTION_MODE_LPCXPRESSO 3
    #define PlatformInfo_DETECTION_MODE PlatformInfo_DETECTION_MODE_AUTO

    class PlatformInfo
    {
    public:
        const static int PF_UNKNOWN=0;
        const static int PF_MBED=1;
        const static int PF_LPCXPRESSO=2;
        /**
         * This function returns platform type value.
         */
        static int getPlatformType();
    private:
        static void check();
        static int _pftype;
        PlatformInfo(){};
    };
}