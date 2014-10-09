/**
 * Copyright 2014 MiMicProject
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * This file based on https://mbed.org/users/Sissors/code/FreescaleIAP/
 */
#ifndef K64F_IAP_h
#define K64F_IAP_h
#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_K64F
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#define K64F_IAP_SECTOR_SIZE    4096

typedef int K64F_IAP_TIAPCode;

#define K64F_IAP_TIAPCode_BoundaryError     -99 //Commands may not span several sectors
#define K64F_IAP_TIAPCode_AlignError        -98 //Data must be aligned on longword (two LSBs zero)
#define K64F_IAP_TIAPCode_ProtectionError   -97 //Flash sector is protected
#define K64F_IAP_TIAPCode_AccessError       -96 //Something went wrong
#define K64F_IAP_TIAPCode_CollisionError    -95 //During writing something tried to flash which was written to
#define K64F_IAP_TIAPCode_LengthError       -94 //The length must be multiples of 4
#define K64F_IAP_TIAPCode_RuntimeError      -93        
#define K64F_IAP_TIAPCode_EraseError        -92 //The flash was not erased before writing to it
#define K64F_IAP_TIAPCode_Success           0


/** Erase a flash sector
 *
 * The size erased depends on the used device
 *
 * @param address address in the sector which needs to be erased
 * @param return Success if no errors were encountered, otherwise one of the error states
 */
K64F_IAP_TIAPCode K64F_IAP_erase_sector(int address);

/** Program flash
 *
 * Before programming the used area needs to be erased. The erase state is checked
 * before programming, and will return an error if not erased.
 *
 * @param address starting address where the data needs to be programmed (must be longword alligned: two LSBs must be zero)
 * @param data pointer to array with the data to program
 * @param length number of bytes to program (must be a multiple of 4. must be a multiple of 8 when K64F)
 * @param return Success if no errors were encountered, otherwise one of the error states
 */
K64F_IAP_TIAPCode K64F_IAP_program_flash(int address, char *data, unsigned int length);

/**
 * Returns size of flash memory
 * 
 * This is the first address which is not flash
 *
 * @param return length of flash memory in bytes
 */
unsigned int K64F_IAP_flash_size(void);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
#endif
