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
#include "NyLPC_config.h"
#if NyLPC_MCU==NyLPC_MCU_K64F
#include "K64F_IAP.h"
//For K64F
#   include "MK64F12.h"
#   define USE_ProgramPhrase 1
#   define FTFA                        FTFE
#   define FTFA_FSTAT_FPVIOL_MASK      FTFE_FSTAT_FPVIOL_MASK 
#   define FTFA_FSTAT_ACCERR_MASK      FTFE_FSTAT_ACCERR_MASK
#   define FTFA_FSTAT_RDCOLERR_MASK    FTFE_FSTAT_RDCOLERR_MASK
#   define FTFA_FSTAT_CCIF_MASK        FTFE_FSTAT_CCIF_MASK
#   define FTFA_FSTAT_MGSTAT0_MASK     FTFE_FSTAT_MGSTAT0_MASK

enum FCMD {
    Read1s = 0x01,
    ProgramCheck = 0x02,
    ReadResource = 0x03,
    ProgramLongword = 0x06,
    ProgramPhrase = 0x07,    
    EraseSector = 0x09,
    Read1sBlock = 0x40,
    ReadOnce = 0x41,
    ProgramOnce = 0x43,
    EraseAll = 0x44,
    VerifyBackdoor = 0x45
    };


#define INT_FALSE (0!=0)
#define INT_TRUE (0==0)

static inline void run_command(void);
//static int check_boundary(int address, unsigned int length);
static int check_align(int address);

static K64F_IAP_TIAPCode verify_erased(int address, unsigned int length);
static K64F_IAP_TIAPCode check_error(void);
static K64F_IAP_TIAPCode program_word(int address, char *data);
    
K64F_IAP_TIAPCode K64F_IAP_erase_sector(int address) {
    #ifdef IAPDEBUG
    printf("IAP: Erasing at %x\r\n", address);
    #endif
    if (check_align(address))
        return K64F_IAP_TIAPCode_AlignError;
    
    //Setup command
    FTFA->FCCOB0 = EraseSector;
    FTFA->FCCOB1 = (address >> 16) & 0xFF;
    FTFA->FCCOB2 = (address >> 8) & 0xFF;
    FTFA->FCCOB3 = address & 0xFF;
    
    run_command();
    
    return check_error();
}

K64F_IAP_TIAPCode K64F_IAP_program_flash(int address, char *data, unsigned int length) {
    #ifdef IAPDEBUG
    printf("IAP: Programming flash at %x with length %d\r\n", address, length);
    #endif
    if (check_align(address))
        return K64F_IAP_TIAPCode_AlignError;
        
    K64F_IAP_TIAPCode eraseCheck = verify_erased(address, length);
    if (eraseCheck != K64F_IAP_TIAPCode_Success)
        return eraseCheck;
    
    K64F_IAP_TIAPCode progResult;
    for (int i = 0; i < length; i+=8) {
        progResult = program_word(address + i, data + i);
        if (progResult != K64F_IAP_TIAPCode_Success)
            return progResult;
    }
    return K64F_IAP_TIAPCode_Success;
}

unsigned int K64F_IAP_flash_size(void) {
    unsigned int retval = (SIM->FCFG2 & 0x7F000000u) >> (24-13);
    if (SIM->FCFG2 & (1<<23))           //Possible second flash bank
        retval += (SIM->FCFG2 & 0x007F0000u) >> (16-13);
    return retval;
}

static K64F_IAP_TIAPCode program_word(int address, char *data) {
    #ifdef IAPDEBUG
    printf("IAP: Programming word at %x, %d - %d - %d - %d - %d - %d - %d - %d\r\n", address, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
    #endif
    if (check_align(address)){
        return K64F_IAP_TIAPCode_AlignError;
    }
    FTFA->FCCOB0 = ProgramPhrase;
    FTFA->FCCOB1 = (address >> 16) & 0xFF;
    FTFA->FCCOB2 = (address >> 8) & 0xFF;
    FTFA->FCCOB3 = address & 0xFF;
    FTFA->FCCOB4 = data[3];
    FTFA->FCCOB5 = data[2];
    FTFA->FCCOB6 = data[1];
    FTFA->FCCOB7 = data[0];
    FTFA->FCCOB8 = data[7];
    FTFA->FCCOB9 = data[6];
    FTFA->FCCOBA = data[5];
    FTFA->FCCOBB = data[4];    
    run_command();    
    return check_error();
}

/* Clear possible flags which are set, run command, wait until done */
static inline void run_command(void) {
    //Clear possible old errors, start command, wait until done
    __disable_irq();            //Disable IRQs, preventing IRQ routines from trying to access flash (thanks to https://mbed.org/users/mjr/)
    FTFA->FSTAT = FTFA_FSTAT_FPVIOL_MASK | FTFA_FSTAT_ACCERR_MASK | FTFA_FSTAT_RDCOLERR_MASK;
    FTFA->FSTAT = FTFA_FSTAT_CCIF_MASK;
    while (!(FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK));
    __enable_irq();
}    
    
    

/* Check if no flash boundary is violated
   Returns true on violation *//*
static int check_boundary(int address, unsigned int length) {
    int temp = (address+length - 1) / K64F_IAP_SECTOR_SIZE;
    address /= K64F_IAP_SECTOR_SIZE;
    int retval = (address != temp);
    #ifdef IAPDEBUG
    if (retval)
        printf("IAP: Boundary violation\r\n");
    #endif
    return retval;
}*/

/* Check if address is correctly aligned
   Returns true on violation */
static int check_align(int address) {
    int retval = address & 0x03;
    #ifdef IAPDEBUG
    if (retval)
        printf("IAP: Alignment violation\r\n");
    #endif
    return retval;
}

/* Check if an area of flash memory is erased
   Returns error code or Success (in case of fully erased) */
static K64F_IAP_TIAPCode verify_erased(int address, unsigned int length) {
    #ifdef IAPDEBUG
    printf("IAP: Verify erased at %x with length %d\r\n", address, length);
    #endif
    
    if (check_align(address)){
        return K64F_IAP_TIAPCode_AlignError;
    }
    
    //Setup command
    FTFA->FCCOB0 = Read1s;
    FTFA->FCCOB1 = (address >> 16) & 0xFF;
    FTFA->FCCOB2 = (address >> 8) & 0xFF;
    FTFA->FCCOB3 = address & 0xFF;
    FTFA->FCCOB4 = (length >> 10) & 0xFF;
    FTFA->FCCOB5 = (length >> 2) & 0xFF;
    FTFA->FCCOB6 = 0;
    
    run_command();
    
    K64F_IAP_TIAPCode retval = check_error();
    if (retval == K64F_IAP_TIAPCode_RuntimeError) {
        #ifdef IAPDEBUG
        printf("IAP: Flash was not erased\r\n");
        #endif
        return K64F_IAP_TIAPCode_EraseError;
    }
    return retval;        
}

/* Check if an error occured 
   Returns error code or Success*/
static K64F_IAP_TIAPCode check_error(void) {
    if (FTFA->FSTAT & FTFA_FSTAT_FPVIOL_MASK) {
        #ifdef IAPDEBUG
        printf("IAP: Protection violation\r\n");
        #endif
        return K64F_IAP_TIAPCode_ProtectionError;
    }
    if (FTFA->FSTAT & FTFA_FSTAT_ACCERR_MASK) {
        #ifdef IAPDEBUG
        printf("IAP: Flash access error\r\n");
        #endif
        return K64F_IAP_TIAPCode_AccessError;
    }
    if (FTFA->FSTAT & FTFA_FSTAT_RDCOLERR_MASK) {
        #ifdef IAPDEBUG
        printf("IAP: Collision error\r\n");
        #endif
        return K64F_IAP_TIAPCode_CollisionError;
    }
    if (FTFA->FSTAT & FTFA_FSTAT_MGSTAT0_MASK) {
        #ifdef IAPDEBUG
        printf("IAP: Runtime error\r\n");
        #endif
        return K64F_IAP_TIAPCode_RuntimeError;
    }
    #ifdef IAPDEBUG
    printf("IAP: No error reported\r\n");
    #endif
    return K64F_IAP_TIAPCode_Success;
}
#endif
