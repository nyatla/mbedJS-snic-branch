#include "rpctbl.h"
#include "mimic.h"
#include "NyLPC_jsonrpc.h"
const struct NyLPC_TJsonRpcClassDef* RPCTBL[]=
{
    //mbed API
    &MiMic::MbedJsApi::RPC_MBED_DIGITAL_OUT,
    &MiMic::MbedJsApi::RPC_MBED_DIGITAL_IN,
    &MiMic::MbedJsApi::RPC_MBED_ANALOG_IN,
    &MiMic::MbedJsApi::RPC_MBED_ANALOG_OUT,
    &MiMic::MbedJsApi::RPC_MBED_BUS_IN,
    &MiMic::MbedJsApi::RPC_MBED_BUS_OUT,
    &MiMic::MbedJsApi::RPC_MBED_BUS_IN_OUT,
    &MiMic::MbedJsApi::RPC_MBED_PWM_OUT,
    &MiMic::MbedJsApi::RPC_MBED_PORT_OUT,
    &MiMic::MbedJsApi::RPC_MBED_PORT_IN,
    &MiMic::MbedJsApi::RPC_MBED_SPI,
    &MiMic::MbedJsApi::RPC_MBED_SPI_SLAVE,
    &MiMic::MbedJsApi::RPC_MBED_SERIAL,
    &MiMic::MbedJsApi::RPC_MBED_I2C,
    &MiMic::MbedJsApi::RPC_MBED_I2C_SLAVE,
    &MiMic::MbedJsApi::RPC_MBED_MCU,
	&NyLPC_cJsonRpcFunction_Memory,
    //end of table
    NULL
};
