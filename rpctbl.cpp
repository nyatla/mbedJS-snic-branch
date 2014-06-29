#pragma once

#include "rpctbl.h"
#include "mimic.h"

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


    //end of table
    NULL
};
