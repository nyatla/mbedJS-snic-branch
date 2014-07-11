#pragma once
#include "NyLPC_http.h"
#include "mbed.h"
namespace MiMic
{
    class MbedJsApi{
    public:
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_DIGITAL_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_DIGITAL_IN;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_ANALOG_IN;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_ANALOG_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_BUS_IN;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_BUS_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_BUS_IN_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_PWM_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_PORT_OUT;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_PORT_IN;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_SPI;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_SPI_SLAVE;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_SERIAL;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_I2C;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_I2C_SLAVE;
        const static struct NyLPC_TJsonRpcClassDef RPC_MBED_MCU;
    };
}
