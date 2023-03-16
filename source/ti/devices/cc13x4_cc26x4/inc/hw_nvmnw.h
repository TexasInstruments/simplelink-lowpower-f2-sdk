/******************************************************************************
*  Filename:       hw_nvmnw_h
*
* Copyright (c) 2015 - 2017, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3) Neither the name of the ORGANIZATION nor the names of its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __HW_NVMNW_H__
#define __HW_NVMNW_H__

//*****************************************************************************
//
// This section defines the register offsets of
// NVMNW component
//
//*****************************************************************************
// Internal
#define NVMNW_O_IIDX                                                0x00000020

// Internal
#define NVMNW_O_IMASK                                               0x00000028

// Internal
#define NVMNW_O_RIS                                                 0x00000030

// Internal
#define NVMNW_O_MIS                                                 0x00000038

// Internal
#define NVMNW_O_ISET                                                0x00000040

// Internal
#define NVMNW_O_ICLR                                                0x00000048

// Internal
#define NVMNW_O_EVT_MODE                                            0x000000E0

// Internal
#define NVMNW_O_DESC                                                0x000000FC

// Internal
#define NVMNW_O_CMDEXEC                                             0x00000100

// Internal
#define NVMNW_O_CMDTYPE                                             0x00000104

// Internal
#define NVMNW_O_CMDCTL                                              0x00000108

// Internal
#define NVMNW_O_CMDADDR                                             0x00000120

// Internal
#define NVMNW_O_CMDBYTEN                                            0x00000124

// Internal
#define NVMNW_O_CMDDATAINDEX                                        0x0000012C

// Internal
#define NVMNW_O_CMDDATA0                                            0x00000130

// Internal
#define NVMNW_O_CMDDATA1                                            0x00000134

// Internal
#define NVMNW_O_CMDDATA2                                            0x00000138

// Internal
#define NVMNW_O_CMDDATA3                                            0x0000013C

// Internal
#define NVMNW_O_CMDDATA4                                            0x00000140

// Internal
#define NVMNW_O_CMDDATA5                                            0x00000144

// Internal
#define NVMNW_O_CMDDATA6                                            0x00000148

// Internal
#define NVMNW_O_CMDDATA7                                            0x0000014C

// Internal
#define NVMNW_O_CMDDATA8                                            0x00000150

// Internal
#define NVMNW_O_CMDDATA9                                            0x00000154

// Internal
#define NVMNW_O_CMDDATA10                                           0x00000158

// Internal
#define NVMNW_O_CMDDATA11                                           0x0000015C

// Internal
#define NVMNW_O_CMDDATA12                                           0x00000160

// Internal
#define NVMNW_O_CMDDATA13                                           0x00000164

// Internal
#define NVMNW_O_CMDDATA14                                           0x00000168

// Internal
#define NVMNW_O_CMDDATA15                                           0x0000016C

// Internal
#define NVMNW_O_CMDWEPROTA                                          0x000001D0

// Internal
#define NVMNW_O_CMDWEPROTB                                          0x000001D4

// Internal
#define NVMNW_O_CMDWEPROTNM                                         0x00000210

// Internal
#define NVMNW_O_CMDWEPROTTR                                         0x00000214

// Internal
#define NVMNW_O_CMDWEPROTEN                                         0x00000218

// Internal
#define NVMNW_O_CFGCMD                                              0x000003B0

// Internal
#define NVMNW_O_CFGPCNT                                             0x000003B4

// Internal
#define NVMNW_O_STATCMD                                             0x000003D0

// Internal
#define NVMNW_O_STATADDR                                            0x000003D4

// Internal
#define NVMNW_O_STATPCNT                                            0x000003D8

// Internal
#define NVMNW_O_STATMODE                                            0x000003DC

// Internal
#define NVMNW_O_GBLINFO0                                            0x000003F0

// Internal
#define NVMNW_O_GBLINFO1                                            0x000003F4

// Internal
#define NVMNW_O_GBLINFO2                                            0x000003F8

// Internal
#define NVMNW_O_BANK0INFO0                                          0x00000400

// Internal
#define NVMNW_O_BANK0INFO1                                          0x00000404

// Internal
#define NVMNW_O_BANK1INFO0                                          0x00000410

// Internal
#define NVMNW_O_BANK1INFO1                                          0x00000414

//*****************************************************************************
//
// Register: NVMNW_O_IIDX
//
//*****************************************************************************
// Field:     [0] STAT
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// DONE                     Internal. Only to be used through TI provided API.
// NO_INTR                  Internal. Only to be used through TI provided API.
#define NVMNW_IIDX_STAT                                             0x00000001
#define NVMNW_IIDX_STAT_BITN                                                 0
#define NVMNW_IIDX_STAT_M                                           0x00000001
#define NVMNW_IIDX_STAT_S                                                    0
#define NVMNW_IIDX_STAT_DONE                                        0x00000001
#define NVMNW_IIDX_STAT_NO_INTR                                     0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_IMASK
//
//*****************************************************************************
// Field:     [0] DONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENABLED                  Internal. Only to be used through TI provided API.
// DISABLED                 Internal. Only to be used through TI provided API.
#define NVMNW_IMASK_DONE                                            0x00000001
#define NVMNW_IMASK_DONE_BITN                                                0
#define NVMNW_IMASK_DONE_M                                          0x00000001
#define NVMNW_IMASK_DONE_S                                                   0
#define NVMNW_IMASK_DONE_ENABLED                                    0x00000001
#define NVMNW_IMASK_DONE_DISABLED                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_RIS
//
//*****************************************************************************
// Field:     [0] DONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// SET                      Internal. Only to be used through TI provided API.
// CLR                      Internal. Only to be used through TI provided API.
#define NVMNW_RIS_DONE                                              0x00000001
#define NVMNW_RIS_DONE_BITN                                                  0
#define NVMNW_RIS_DONE_M                                            0x00000001
#define NVMNW_RIS_DONE_S                                                     0
#define NVMNW_RIS_DONE_SET                                          0x00000001
#define NVMNW_RIS_DONE_CLR                                          0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_MIS
//
//*****************************************************************************
// Field:     [0] DONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// SET                      Internal. Only to be used through TI provided API.
// CLR                      Internal. Only to be used through TI provided API.
#define NVMNW_MIS_DONE                                              0x00000001
#define NVMNW_MIS_DONE_BITN                                                  0
#define NVMNW_MIS_DONE_M                                            0x00000001
#define NVMNW_MIS_DONE_S                                                     0
#define NVMNW_MIS_DONE_SET                                          0x00000001
#define NVMNW_MIS_DONE_CLR                                          0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_ISET
//
//*****************************************************************************
// Field:     [0] DONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// SET                      Internal. Only to be used through TI provided API.
// NO_EFFECT                Internal. Only to be used through TI provided API.
#define NVMNW_ISET_DONE                                             0x00000001
#define NVMNW_ISET_DONE_BITN                                                 0
#define NVMNW_ISET_DONE_M                                           0x00000001
#define NVMNW_ISET_DONE_S                                                    0
#define NVMNW_ISET_DONE_SET                                         0x00000001
#define NVMNW_ISET_DONE_NO_EFFECT                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_ICLR
//
//*****************************************************************************
// Field:     [0] DONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// CLR                      Internal. Only to be used through TI provided API.
// NO_EFFECT                Internal. Only to be used through TI provided API.
#define NVMNW_ICLR_DONE                                             0x00000001
#define NVMNW_ICLR_DONE_BITN                                                 0
#define NVMNW_ICLR_DONE_M                                           0x00000001
#define NVMNW_ICLR_DONE_S                                                    0
#define NVMNW_ICLR_DONE_CLR                                         0x00000001
#define NVMNW_ICLR_DONE_NO_EFFECT                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_EVT_MODE
//
//*****************************************************************************
// Field:   [1:0] INT0_CFG
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// HARDWARE                 Internal. Only to be used through TI provided API.
// SOFTWARE                 Internal. Only to be used through TI provided API.
// DISABLE                  Internal. Only to be used through TI provided API.
#define NVMNW_EVT_MODE_INT0_CFG_W                                            2
#define NVMNW_EVT_MODE_INT0_CFG_M                                   0x00000003
#define NVMNW_EVT_MODE_INT0_CFG_S                                            0
#define NVMNW_EVT_MODE_INT0_CFG_HARDWARE                            0x00000002
#define NVMNW_EVT_MODE_INT0_CFG_SOFTWARE                            0x00000001
#define NVMNW_EVT_MODE_INT0_CFG_DISABLE                             0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_DESC
//
//*****************************************************************************
// Field: [31:16] MODULEID
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_DESC_MODULEID_W                                               16
#define NVMNW_DESC_MODULEID_M                                       0xFFFF0000
#define NVMNW_DESC_MODULEID_S                                               16
#define NVMNW_DESC_MODULEID_MAXIMUM                                 0xFFFF0000
#define NVMNW_DESC_MODULEID_MINIMUM                                 0x00000000

// Field: [15:12] FEATUREVER
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_DESC_FEATUREVER_W                                              4
#define NVMNW_DESC_FEATUREVER_M                                     0x0000F000
#define NVMNW_DESC_FEATUREVER_S                                             12
#define NVMNW_DESC_FEATUREVER_MAXIMUM                               0x0000F000
#define NVMNW_DESC_FEATUREVER_MINIMUM                               0x00000000

// Field:  [11:8] INSTNUM
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_DESC_INSTNUM_W                                                 4
#define NVMNW_DESC_INSTNUM_M                                        0x00000F00
#define NVMNW_DESC_INSTNUM_S                                                 8
#define NVMNW_DESC_INSTNUM_MAXIMUM                                  0x00000F00
#define NVMNW_DESC_INSTNUM_MINIMUM                                  0x00000000

// Field:   [7:4] MAJREV
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_DESC_MAJREV_W                                                  4
#define NVMNW_DESC_MAJREV_M                                         0x000000F0
#define NVMNW_DESC_MAJREV_S                                                  4
#define NVMNW_DESC_MAJREV_MAXIMUM                                   0x000000F0
#define NVMNW_DESC_MAJREV_MINIMUM                                   0x00000000

// Field:   [3:0] MINREV
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_DESC_MINREV_W                                                  4
#define NVMNW_DESC_MINREV_M                                         0x0000000F
#define NVMNW_DESC_MINREV_S                                                  0
#define NVMNW_DESC_MINREV_MAXIMUM                                   0x0000000F
#define NVMNW_DESC_MINREV_MINIMUM                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDEXEC
//
//*****************************************************************************
// Field:     [0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// EXECUTE                  Internal. Only to be used through TI provided API.
// NOEXECUTE                Internal. Only to be used through TI provided API.
#define NVMNW_CMDEXEC_VAL                                           0x00000001
#define NVMNW_CMDEXEC_VAL_BITN                                               0
#define NVMNW_CMDEXEC_VAL_M                                         0x00000001
#define NVMNW_CMDEXEC_VAL_S                                                  0
#define NVMNW_CMDEXEC_VAL_EXECUTE                                   0x00000001
#define NVMNW_CMDEXEC_VAL_NOEXECUTE                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDTYPE
//
//*****************************************************************************
// Field:   [6:4] SIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// BANK                     Internal. Only to be used through TI provided API.
// SECTOR                   Internal. Only to be used through TI provided API.
// EIGHTWORD                Internal. Only to be used through TI provided API.
// FOURWORD                 Internal. Only to be used through TI provided API.
// TWOWORD                  Internal. Only to be used through TI provided API.
// ONEWORD                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDTYPE_SIZE_W                                                 3
#define NVMNW_CMDTYPE_SIZE_M                                        0x00000070
#define NVMNW_CMDTYPE_SIZE_S                                                 4
#define NVMNW_CMDTYPE_SIZE_BANK                                     0x00000050
#define NVMNW_CMDTYPE_SIZE_SECTOR                                   0x00000040
#define NVMNW_CMDTYPE_SIZE_EIGHTWORD                                0x00000030
#define NVMNW_CMDTYPE_SIZE_FOURWORD                                 0x00000020
#define NVMNW_CMDTYPE_SIZE_TWOWORD                                  0x00000010
#define NVMNW_CMDTYPE_SIZE_ONEWORD                                  0x00000000

// Field:   [2:0] COMMAND
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// BLANKVERIFY              Internal. Only to be used through TI provided API.
// CLEARSTATUS              Internal. Only to be used through TI provided API.
// MODECHANGE               Internal. Only to be used through TI provided API.
// ERASE                    Internal. Only to be used through TI provided API.
// PROGRAM                  Internal. Only to be used through TI provided API.
// NOOP                     Internal. Only to be used through TI provided API.
#define NVMNW_CMDTYPE_COMMAND_W                                              3
#define NVMNW_CMDTYPE_COMMAND_M                                     0x00000007
#define NVMNW_CMDTYPE_COMMAND_S                                              0
#define NVMNW_CMDTYPE_COMMAND_BLANKVERIFY                           0x00000006
#define NVMNW_CMDTYPE_COMMAND_CLEARSTATUS                           0x00000005
#define NVMNW_CMDTYPE_COMMAND_MODECHANGE                            0x00000004
#define NVMNW_CMDTYPE_COMMAND_ERASE                                 0x00000002
#define NVMNW_CMDTYPE_COMMAND_PROGRAM                               0x00000001
#define NVMNW_CMDTYPE_COMMAND_NOOP                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDCTL
//
//*****************************************************************************
// Field:    [21] DATAVEREN
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENABLE                   Internal. Only to be used through TI provided API.
// DISABLE                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_DATAVEREN                                      0x00200000
#define NVMNW_CMDCTL_DATAVEREN_BITN                                         21
#define NVMNW_CMDCTL_DATAVEREN_M                                    0x00200000
#define NVMNW_CMDCTL_DATAVEREN_S                                            21
#define NVMNW_CMDCTL_DATAVEREN_ENABLE                               0x00200000
#define NVMNW_CMDCTL_DATAVEREN_DISABLE                              0x00000000

// Field:    [20] SSERASEDIS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// DISABLE                  Internal. Only to be used through TI provided API.
// ENABLE                   Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_SSERASEDIS                                     0x00100000
#define NVMNW_CMDCTL_SSERASEDIS_BITN                                        20
#define NVMNW_CMDCTL_SSERASEDIS_M                                   0x00100000
#define NVMNW_CMDCTL_SSERASEDIS_S                                           20
#define NVMNW_CMDCTL_SSERASEDIS_DISABLE                             0x00100000
#define NVMNW_CMDCTL_SSERASEDIS_ENABLE                              0x00000000

// Field:    [19] ERASEMASKDIS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// DISABLE                  Internal. Only to be used through TI provided API.
// ENABLE                   Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_ERASEMASKDIS                                   0x00080000
#define NVMNW_CMDCTL_ERASEMASKDIS_BITN                                      19
#define NVMNW_CMDCTL_ERASEMASKDIS_M                                 0x00080000
#define NVMNW_CMDCTL_ERASEMASKDIS_S                                         19
#define NVMNW_CMDCTL_ERASEMASKDIS_DISABLE                           0x00080000
#define NVMNW_CMDCTL_ERASEMASKDIS_ENABLE                            0x00000000

// Field:    [18] PROGMASKDIS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// DISABLE                  Internal. Only to be used through TI provided API.
// ENABLE                   Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_PROGMASKDIS                                    0x00040000
#define NVMNW_CMDCTL_PROGMASKDIS_BITN                                       18
#define NVMNW_CMDCTL_PROGMASKDIS_M                                  0x00040000
#define NVMNW_CMDCTL_PROGMASKDIS_S                                          18
#define NVMNW_CMDCTL_PROGMASKDIS_DISABLE                            0x00040000
#define NVMNW_CMDCTL_PROGMASKDIS_ENABLE                             0x00000000

// Field:    [16] ADDRXLATEOVR
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// OVERRIDE                 Internal. Only to be used through TI provided API.
// NOOVERRIDE               Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_ADDRXLATEOVR                                   0x00010000
#define NVMNW_CMDCTL_ADDRXLATEOVR_BITN                                      16
#define NVMNW_CMDCTL_ADDRXLATEOVR_M                                 0x00010000
#define NVMNW_CMDCTL_ADDRXLATEOVR_S                                         16
#define NVMNW_CMDCTL_ADDRXLATEOVR_OVERRIDE                          0x00010000
#define NVMNW_CMDCTL_ADDRXLATEOVR_NOOVERRIDE                        0x00000000

// Field:    [15] POSTVEREN
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENABLE                   Internal. Only to be used through TI provided API.
// DISABLE                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_POSTVEREN                                      0x00008000
#define NVMNW_CMDCTL_POSTVEREN_BITN                                         15
#define NVMNW_CMDCTL_POSTVEREN_M                                    0x00008000
#define NVMNW_CMDCTL_POSTVEREN_S                                            15
#define NVMNW_CMDCTL_POSTVEREN_ENABLE                               0x00008000
#define NVMNW_CMDCTL_POSTVEREN_DISABLE                              0x00000000

// Field:    [14] PREVEREN
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENABLE                   Internal. Only to be used through TI provided API.
// DISABLE                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_PREVEREN                                       0x00004000
#define NVMNW_CMDCTL_PREVEREN_BITN                                          14
#define NVMNW_CMDCTL_PREVEREN_M                                     0x00004000
#define NVMNW_CMDCTL_PREVEREN_S                                             14
#define NVMNW_CMDCTL_PREVEREN_ENABLE                                0x00004000
#define NVMNW_CMDCTL_PREVEREN_DISABLE                               0x00000000

// Field:  [12:9] REGIONSEL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENGR                     Internal. Only to be used through TI provided API.
// TRIM                     Internal. Only to be used through TI provided API.
// NONMAIN                  Internal. Only to be used through TI provided API.
// MAIN                     Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_REGIONSEL_W                                             4
#define NVMNW_CMDCTL_REGIONSEL_M                                    0x00001E00
#define NVMNW_CMDCTL_REGIONSEL_S                                             9
#define NVMNW_CMDCTL_REGIONSEL_ENGR                                 0x00001000
#define NVMNW_CMDCTL_REGIONSEL_TRIM                                 0x00000800
#define NVMNW_CMDCTL_REGIONSEL_NONMAIN                              0x00000400
#define NVMNW_CMDCTL_REGIONSEL_MAIN                                 0x00000200

// Field:     [4] BANKSEL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// BANK4                    Internal. Only to be used through TI provided API.
// BANK3                    Internal. Only to be used through TI provided API.
// BANK2                    Internal. Only to be used through TI provided API.
// BANK1                    Internal. Only to be used through TI provided API.
// BANK0                    Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_BANKSEL                                        0x00000010
#define NVMNW_CMDCTL_BANKSEL_BITN                                            4
#define NVMNW_CMDCTL_BANKSEL_M                                      0x00000010
#define NVMNW_CMDCTL_BANKSEL_S                                               4
#define NVMNW_CMDCTL_BANKSEL_BANK4                                  0x00000100
#define NVMNW_CMDCTL_BANKSEL_BANK3                                  0x00000080
#define NVMNW_CMDCTL_BANKSEL_BANK2                                  0x00000040
#define NVMNW_CMDCTL_BANKSEL_BANK1                                  0x00000020
#define NVMNW_CMDCTL_BANKSEL_BANK0                                  0x00000010

// Field:   [3:0] MODESEL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ERASEBNK                 Internal. Only to be used through TI provided API.
// PGMMW                    Internal. Only to be used through TI provided API.
// ERASESECT                Internal. Only to be used through TI provided API.
// ERASEVER                 Internal. Only to be used through TI provided API.
// PGMSW                    Internal. Only to be used through TI provided API.
// PGMVER                   Internal. Only to be used through TI provided API.
// RDMARG1B                 Internal. Only to be used through TI provided API.
// RDMARG0B                 Internal. Only to be used through TI provided API.
// RDMARG1                  Internal. Only to be used through TI provided API.
// RDMARG0                  Internal. Only to be used through TI provided API.
// READ                     Internal. Only to be used through TI provided API.
#define NVMNW_CMDCTL_MODESEL_W                                               4
#define NVMNW_CMDCTL_MODESEL_M                                      0x0000000F
#define NVMNW_CMDCTL_MODESEL_S                                               0
#define NVMNW_CMDCTL_MODESEL_ERASEBNK                               0x0000000F
#define NVMNW_CMDCTL_MODESEL_PGMMW                                  0x0000000E
#define NVMNW_CMDCTL_MODESEL_ERASESECT                              0x0000000C
#define NVMNW_CMDCTL_MODESEL_ERASEVER                               0x0000000B
#define NVMNW_CMDCTL_MODESEL_PGMSW                                  0x0000000A
#define NVMNW_CMDCTL_MODESEL_PGMVER                                 0x00000009
#define NVMNW_CMDCTL_MODESEL_RDMARG1B                               0x00000007
#define NVMNW_CMDCTL_MODESEL_RDMARG0B                               0x00000006
#define NVMNW_CMDCTL_MODESEL_RDMARG1                                0x00000004
#define NVMNW_CMDCTL_MODESEL_RDMARG0                                0x00000002
#define NVMNW_CMDCTL_MODESEL_READ                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDADDR
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDADDR_VAL_W                                                 32
#define NVMNW_CMDADDR_VAL_M                                         0xFFFFFFFF
#define NVMNW_CMDADDR_VAL_S                                                  0
#define NVMNW_CMDADDR_VAL_MAXIMUM                                   0xFFFFFFFF
#define NVMNW_CMDADDR_VAL_MINIMUM                                   0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDBYTEN
//
//*****************************************************************************
// Field:  [15:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDBYTEN_VAL_W                                                16
#define NVMNW_CMDBYTEN_VAL_M                                        0x0000FFFF
#define NVMNW_CMDBYTEN_VAL_S                                                 0
#define NVMNW_CMDBYTEN_VAL_MAXIMUM                                  0x0000FFFF
#define NVMNW_CMDBYTEN_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATAINDEX
//
//*****************************************************************************
// Field:   [1:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATAINDEX_VAL_W                                             2
#define NVMNW_CMDDATAINDEX_VAL_M                                    0x00000003
#define NVMNW_CMDDATAINDEX_VAL_S                                             0
#define NVMNW_CMDDATAINDEX_VAL_MAXIMUM                              0x00000003
#define NVMNW_CMDDATAINDEX_VAL_MINIMUM                              0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA0
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA0_VAL_W                                                32
#define NVMNW_CMDDATA0_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA0_VAL_S                                                 0
#define NVMNW_CMDDATA0_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA0_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA1
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA1_VAL_W                                                32
#define NVMNW_CMDDATA1_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA1_VAL_S                                                 0
#define NVMNW_CMDDATA1_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA1_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA2
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA2_VAL_W                                                32
#define NVMNW_CMDDATA2_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA2_VAL_S                                                 0
#define NVMNW_CMDDATA2_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA2_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA3
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA3_VAL_W                                                32
#define NVMNW_CMDDATA3_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA3_VAL_S                                                 0
#define NVMNW_CMDDATA3_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA3_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA4
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA4_VAL_W                                                32
#define NVMNW_CMDDATA4_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA4_VAL_S                                                 0
#define NVMNW_CMDDATA4_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA4_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA5
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA5_VAL_W                                                32
#define NVMNW_CMDDATA5_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA5_VAL_S                                                 0
#define NVMNW_CMDDATA5_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA5_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA6
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA6_VAL_W                                                32
#define NVMNW_CMDDATA6_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA6_VAL_S                                                 0
#define NVMNW_CMDDATA6_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA6_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA7
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA7_VAL_W                                                32
#define NVMNW_CMDDATA7_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA7_VAL_S                                                 0
#define NVMNW_CMDDATA7_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA7_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA8
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA8_VAL_W                                                32
#define NVMNW_CMDDATA8_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA8_VAL_S                                                 0
#define NVMNW_CMDDATA8_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA8_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA9
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA9_VAL_W                                                32
#define NVMNW_CMDDATA9_VAL_M                                        0xFFFFFFFF
#define NVMNW_CMDDATA9_VAL_S                                                 0
#define NVMNW_CMDDATA9_VAL_MAXIMUM                                  0xFFFFFFFF
#define NVMNW_CMDDATA9_VAL_MINIMUM                                  0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA10
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA10_VAL_W                                               32
#define NVMNW_CMDDATA10_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA10_VAL_S                                                0
#define NVMNW_CMDDATA10_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA10_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA11
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA11_VAL_W                                               32
#define NVMNW_CMDDATA11_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA11_VAL_S                                                0
#define NVMNW_CMDDATA11_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA11_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA12
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA12_VAL_W                                               32
#define NVMNW_CMDDATA12_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA12_VAL_S                                                0
#define NVMNW_CMDDATA12_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA12_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA13
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA13_VAL_W                                               32
#define NVMNW_CMDDATA13_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA13_VAL_S                                                0
#define NVMNW_CMDDATA13_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA13_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA14
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA14_VAL_W                                               32
#define NVMNW_CMDDATA14_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA14_VAL_S                                                0
#define NVMNW_CMDDATA14_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA14_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDDATA15
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDDATA15_VAL_W                                               32
#define NVMNW_CMDDATA15_VAL_M                                       0xFFFFFFFF
#define NVMNW_CMDDATA15_VAL_S                                                0
#define NVMNW_CMDDATA15_VAL_MAXIMUM                                 0xFFFFFFFF
#define NVMNW_CMDDATA15_VAL_MINIMUM                                 0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDWEPROTA
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDWEPROTA_VAL_W                                              32
#define NVMNW_CMDWEPROTA_VAL_M                                      0xFFFFFFFF
#define NVMNW_CMDWEPROTA_VAL_S                                               0
#define NVMNW_CMDWEPROTA_VAL_MAXIMUM                                0xFFFFFFFF
#define NVMNW_CMDWEPROTA_VAL_MINIMUM                                0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDWEPROTB
//
//*****************************************************************************
// Field:  [31:0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDWEPROTB_VAL_W                                              32
#define NVMNW_CMDWEPROTB_VAL_M                                      0xFFFFFFFF
#define NVMNW_CMDWEPROTB_VAL_S                                               0
#define NVMNW_CMDWEPROTB_VAL_MAXIMUM                                0xFFFFFFFF
#define NVMNW_CMDWEPROTB_VAL_MINIMUM                                0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDWEPROTNM
//
//*****************************************************************************
// Field:     [0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDWEPROTNM_VAL                                       0x00000001
#define NVMNW_CMDWEPROTNM_VAL_BITN                                           0
#define NVMNW_CMDWEPROTNM_VAL_M                                     0x00000001
#define NVMNW_CMDWEPROTNM_VAL_S                                              0
#define NVMNW_CMDWEPROTNM_VAL_MAXIMUM                               0x00000001
#define NVMNW_CMDWEPROTNM_VAL_MINIMUM                               0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDWEPROTTR
//
//*****************************************************************************
// Field:     [0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDWEPROTTR_VAL                                       0x00000001
#define NVMNW_CMDWEPROTTR_VAL_BITN                                           0
#define NVMNW_CMDWEPROTTR_VAL_M                                     0x00000001
#define NVMNW_CMDWEPROTTR_VAL_S                                              0
#define NVMNW_CMDWEPROTTR_VAL_MAXIMUM                               0x00000001
#define NVMNW_CMDWEPROTTR_VAL_MINIMUM                               0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CMDWEPROTEN
//
//*****************************************************************************
// Field:     [0] VAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CMDWEPROTEN_VAL                                       0x00000001
#define NVMNW_CMDWEPROTEN_VAL_BITN                                           0
#define NVMNW_CMDWEPROTEN_VAL_M                                     0x00000001
#define NVMNW_CMDWEPROTEN_VAL_S                                              0
#define NVMNW_CMDWEPROTEN_VAL_MAXIMUM                               0x00000001
#define NVMNW_CMDWEPROTEN_VAL_MINIMUM                               0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CFGCMD
//
//*****************************************************************************
// Field:   [3:0] WAITSTATE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CFGCMD_WAITSTATE_W                                             4
#define NVMNW_CFGCMD_WAITSTATE_M                                    0x0000000F
#define NVMNW_CFGCMD_WAITSTATE_S                                             0
#define NVMNW_CFGCMD_WAITSTATE_MAXIMUM                              0x0000000F
#define NVMNW_CFGCMD_WAITSTATE_MINIMUM                              0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_CFGPCNT
//
//*****************************************************************************
// Field:  [11:4] MAXPCNTVAL
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_CFGPCNT_MAXPCNTVAL_W                                           8
#define NVMNW_CFGPCNT_MAXPCNTVAL_M                                  0x00000FF0
#define NVMNW_CFGPCNT_MAXPCNTVAL_S                                           4
#define NVMNW_CFGPCNT_MAXPCNTVAL_MAXIMUM                            0x00000FF0
#define NVMNW_CFGPCNT_MAXPCNTVAL_MINIMUM                            0x00000000

// Field:     [0] MAXPCNTOVR
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// OVERRIDE                 Internal. Only to be used through TI provided API.
// DEFAULT                  Internal. Only to be used through TI provided API.
#define NVMNW_CFGPCNT_MAXPCNTOVR                                    0x00000001
#define NVMNW_CFGPCNT_MAXPCNTOVR_BITN                                        0
#define NVMNW_CFGPCNT_MAXPCNTOVR_M                                  0x00000001
#define NVMNW_CFGPCNT_MAXPCNTOVR_S                                           0
#define NVMNW_CFGPCNT_MAXPCNTOVR_OVERRIDE                           0x00000001
#define NVMNW_CFGPCNT_MAXPCNTOVR_DEFAULT                            0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_STATCMD
//
//*****************************************************************************
// Field:    [12] FAILMISC
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILMISC                                      0x00001000
#define NVMNW_STATCMD_FAILMISC_BITN                                         12
#define NVMNW_STATCMD_FAILMISC_M                                    0x00001000
#define NVMNW_STATCMD_FAILMISC_S                                            12
#define NVMNW_STATCMD_FAILMISC_STATFAIL                             0x00001000
#define NVMNW_STATCMD_FAILMISC_STATNOFAIL                           0x00000000

// Field:     [8] FAILINVDATA
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILINVDATA                                   0x00000100
#define NVMNW_STATCMD_FAILINVDATA_BITN                                       8
#define NVMNW_STATCMD_FAILINVDATA_M                                 0x00000100
#define NVMNW_STATCMD_FAILINVDATA_S                                          8
#define NVMNW_STATCMD_FAILINVDATA_STATFAIL                          0x00000100
#define NVMNW_STATCMD_FAILINVDATA_STATNOFAIL                        0x00000000

// Field:     [7] FAILMODE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILMODE                                      0x00000080
#define NVMNW_STATCMD_FAILMODE_BITN                                          7
#define NVMNW_STATCMD_FAILMODE_M                                    0x00000080
#define NVMNW_STATCMD_FAILMODE_S                                             7
#define NVMNW_STATCMD_FAILMODE_STATFAIL                             0x00000080
#define NVMNW_STATCMD_FAILMODE_STATNOFAIL                           0x00000000

// Field:     [6] FAILILLADDR
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILILLADDR                                   0x00000040
#define NVMNW_STATCMD_FAILILLADDR_BITN                                       6
#define NVMNW_STATCMD_FAILILLADDR_M                                 0x00000040
#define NVMNW_STATCMD_FAILILLADDR_S                                          6
#define NVMNW_STATCMD_FAILILLADDR_STATFAIL                          0x00000040
#define NVMNW_STATCMD_FAILILLADDR_STATNOFAIL                        0x00000000

// Field:     [5] FAILVERIFY
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILVERIFY                                    0x00000020
#define NVMNW_STATCMD_FAILVERIFY_BITN                                        5
#define NVMNW_STATCMD_FAILVERIFY_M                                  0x00000020
#define NVMNW_STATCMD_FAILVERIFY_S                                           5
#define NVMNW_STATCMD_FAILVERIFY_STATFAIL                           0x00000020
#define NVMNW_STATCMD_FAILVERIFY_STATNOFAIL                         0x00000000

// Field:     [4] FAILWEPROT
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATFAIL                 Internal. Only to be used through TI provided API.
// STATNOFAIL               Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_FAILWEPROT                                    0x00000010
#define NVMNW_STATCMD_FAILWEPROT_BITN                                        4
#define NVMNW_STATCMD_FAILWEPROT_M                                  0x00000010
#define NVMNW_STATCMD_FAILWEPROT_S                                           4
#define NVMNW_STATCMD_FAILWEPROT_STATFAIL                           0x00000010
#define NVMNW_STATCMD_FAILWEPROT_STATNOFAIL                         0x00000000

// Field:     [2] CMDINPROGRESS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATINPROGRESS           Internal. Only to be used through TI provided API.
// STATCOMPLETE             Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_CMDINPROGRESS                                 0x00000004
#define NVMNW_STATCMD_CMDINPROGRESS_BITN                                     2
#define NVMNW_STATCMD_CMDINPROGRESS_M                               0x00000004
#define NVMNW_STATCMD_CMDINPROGRESS_S                                        2
#define NVMNW_STATCMD_CMDINPROGRESS_STATINPROGRESS                  0x00000004
#define NVMNW_STATCMD_CMDINPROGRESS_STATCOMPLETE                    0x00000000

// Field:     [1] CMDPASS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATPASS                 Internal. Only to be used through TI provided API.
// STATFAIL                 Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_CMDPASS                                       0x00000002
#define NVMNW_STATCMD_CMDPASS_BITN                                           1
#define NVMNW_STATCMD_CMDPASS_M                                     0x00000002
#define NVMNW_STATCMD_CMDPASS_S                                              1
#define NVMNW_STATCMD_CMDPASS_STATPASS                              0x00000002
#define NVMNW_STATCMD_CMDPASS_STATFAIL                              0x00000000

// Field:     [0] CMDDONE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// STATDONE                 Internal. Only to be used through TI provided API.
// STATNOTDONE              Internal. Only to be used through TI provided API.
#define NVMNW_STATCMD_CMDDONE                                       0x00000001
#define NVMNW_STATCMD_CMDDONE_BITN                                           0
#define NVMNW_STATCMD_CMDDONE_M                                     0x00000001
#define NVMNW_STATCMD_CMDDONE_S                                              0
#define NVMNW_STATCMD_CMDDONE_STATDONE                              0x00000001
#define NVMNW_STATCMD_CMDDONE_STATNOTDONE                           0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_STATADDR
//
//*****************************************************************************
// Field: [25:21] BANKID
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// BANK4                    Internal. Only to be used through TI provided API.
// BANK3                    Internal. Only to be used through TI provided API.
// BANK2                    Internal. Only to be used through TI provided API.
// BANK1                    Internal. Only to be used through TI provided API.
// BANK0                    Internal. Only to be used through TI provided API.
#define NVMNW_STATADDR_BANKID_W                                              5
#define NVMNW_STATADDR_BANKID_M                                     0x03E00000
#define NVMNW_STATADDR_BANKID_S                                             21
#define NVMNW_STATADDR_BANKID_BANK4                                 0x02000000
#define NVMNW_STATADDR_BANKID_BANK3                                 0x01000000
#define NVMNW_STATADDR_BANKID_BANK2                                 0x00800000
#define NVMNW_STATADDR_BANKID_BANK1                                 0x00400000
#define NVMNW_STATADDR_BANKID_BANK0                                 0x00200000

// Field: [20:16] REGIONID
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ENGR                     Internal. Only to be used through TI provided API.
// TRIM                     Internal. Only to be used through TI provided API.
// NONMAIN                  Internal. Only to be used through TI provided API.
// MAIN                     Internal. Only to be used through TI provided API.
#define NVMNW_STATADDR_REGIONID_W                                            5
#define NVMNW_STATADDR_REGIONID_M                                   0x001F0000
#define NVMNW_STATADDR_REGIONID_S                                           16
#define NVMNW_STATADDR_REGIONID_ENGR                                0x00080000
#define NVMNW_STATADDR_REGIONID_TRIM                                0x00040000
#define NVMNW_STATADDR_REGIONID_NONMAIN                             0x00020000
#define NVMNW_STATADDR_REGIONID_MAIN                                0x00010000

// Field:  [15:0] BANKADDR
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_STATADDR_BANKADDR_W                                           16
#define NVMNW_STATADDR_BANKADDR_M                                   0x0000FFFF
#define NVMNW_STATADDR_BANKADDR_S                                            0
#define NVMNW_STATADDR_BANKADDR_MAXIMUM                             0x0000FFFF
#define NVMNW_STATADDR_BANKADDR_MINIMUM                             0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_STATPCNT
//
//*****************************************************************************
// Field:  [11:0] PULSECNT
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_STATPCNT_PULSECNT_W                                           12
#define NVMNW_STATPCNT_PULSECNT_M                                   0x00000FFF
#define NVMNW_STATPCNT_PULSECNT_S                                            0
#define NVMNW_STATPCNT_PULSECNT_MAXIMUM                             0x00000FFF
#define NVMNW_STATPCNT_PULSECNT_MINIMUM                             0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_STATMODE
//
//*****************************************************************************
// Field:    [17] BANK1TRDY
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// TRUE                     Internal. Only to be used through TI provided API.
// FALSE                    Internal. Only to be used through TI provided API.
#define NVMNW_STATMODE_BANK1TRDY                                    0x00020000
#define NVMNW_STATMODE_BANK1TRDY_BITN                                       17
#define NVMNW_STATMODE_BANK1TRDY_M                                  0x00020000
#define NVMNW_STATMODE_BANK1TRDY_S                                          17
#define NVMNW_STATMODE_BANK1TRDY_TRUE                               0x00020000
#define NVMNW_STATMODE_BANK1TRDY_FALSE                              0x00000000

// Field:    [16] BANK2TRDY
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// TRUE                     Internal. Only to be used through TI provided API.
// FALSE                    Internal. Only to be used through TI provided API.
#define NVMNW_STATMODE_BANK2TRDY                                    0x00010000
#define NVMNW_STATMODE_BANK2TRDY_BITN                                       16
#define NVMNW_STATMODE_BANK2TRDY_M                                  0x00010000
#define NVMNW_STATMODE_BANK2TRDY_S                                          16
#define NVMNW_STATMODE_BANK2TRDY_TRUE                               0x00010000
#define NVMNW_STATMODE_BANK2TRDY_FALSE                              0x00000000

// Field:  [11:8] BANKMODE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// ERASEBNK                 Internal. Only to be used through TI provided API.
// PGMMW                    Internal. Only to be used through TI provided API.
// ERASESECT                Internal. Only to be used through TI provided API.
// ERASEVER                 Internal. Only to be used through TI provided API.
// PGMSW                    Internal. Only to be used through TI provided API.
// PGMVER                   Internal. Only to be used through TI provided API.
// RDMARG1B                 Internal. Only to be used through TI provided API.
// RDMARG0B                 Internal. Only to be used through TI provided API.
// RDMARG1                  Internal. Only to be used through TI provided API.
// RDMARG0                  Internal. Only to be used through TI provided API.
// READ                     Internal. Only to be used through TI provided API.
#define NVMNW_STATMODE_BANKMODE_W                                            4
#define NVMNW_STATMODE_BANKMODE_M                                   0x00000F00
#define NVMNW_STATMODE_BANKMODE_S                                            8
#define NVMNW_STATMODE_BANKMODE_ERASEBNK                            0x00000F00
#define NVMNW_STATMODE_BANKMODE_PGMMW                               0x00000E00
#define NVMNW_STATMODE_BANKMODE_ERASESECT                           0x00000C00
#define NVMNW_STATMODE_BANKMODE_ERASEVER                            0x00000B00
#define NVMNW_STATMODE_BANKMODE_PGMSW                               0x00000A00
#define NVMNW_STATMODE_BANKMODE_PGMVER                              0x00000900
#define NVMNW_STATMODE_BANKMODE_RDMARG1B                            0x00000700
#define NVMNW_STATMODE_BANKMODE_RDMARG0B                            0x00000600
#define NVMNW_STATMODE_BANKMODE_RDMARG1                             0x00000400
#define NVMNW_STATMODE_BANKMODE_RDMARG0                             0x00000200
#define NVMNW_STATMODE_BANKMODE_READ                                0x00000000

// Field:     [0] BANKNOTINRD
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// BANK4                    Internal. Only to be used through TI provided API.
// BANK3                    Internal. Only to be used through TI provided API.
// BANK2                    Internal. Only to be used through TI provided API.
// BANK1                    Internal. Only to be used through TI provided API.
// BANK0                    Internal. Only to be used through TI provided API.
#define NVMNW_STATMODE_BANKNOTINRD                                  0x00000001
#define NVMNW_STATMODE_BANKNOTINRD_BITN                                      0
#define NVMNW_STATMODE_BANKNOTINRD_M                                0x00000001
#define NVMNW_STATMODE_BANKNOTINRD_S                                         0
#define NVMNW_STATMODE_BANKNOTINRD_BANK4                            0x00000010
#define NVMNW_STATMODE_BANKNOTINRD_BANK3                            0x00000008
#define NVMNW_STATMODE_BANKNOTINRD_BANK2                            0x00000004
#define NVMNW_STATMODE_BANKNOTINRD_BANK1                            0x00000002
#define NVMNW_STATMODE_BANKNOTINRD_BANK0                            0x00000001

//*****************************************************************************
//
// Register: NVMNW_O_GBLINFO0
//
//*****************************************************************************
// Field: [18:16] NUMBANKS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO0_NUMBANKS_W                                            3
#define NVMNW_GBLINFO0_NUMBANKS_M                                   0x00070000
#define NVMNW_GBLINFO0_NUMBANKS_S                                           16
#define NVMNW_GBLINFO0_NUMBANKS_MAXIMUM                             0x00050000
#define NVMNW_GBLINFO0_NUMBANKS_MINIMUM                             0x00010000

// Field:  [15:0] SECTORSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// TWOKB                    Internal. Only to be used through TI provided API.
// ONEKB                    Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO0_SECTORSIZE_W                                         16
#define NVMNW_GBLINFO0_SECTORSIZE_M                                 0x0000FFFF
#define NVMNW_GBLINFO0_SECTORSIZE_S                                          0
#define NVMNW_GBLINFO0_SECTORSIZE_TWOKB                             0x00000800
#define NVMNW_GBLINFO0_SECTORSIZE_ONEKB                             0x00000400

//*****************************************************************************
//
// Register: NVMNW_O_GBLINFO1
//
//*****************************************************************************
// Field: [18:16] REDWIDTH
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// W4BIT                    Internal. Only to be used through TI provided API.
// W2BIT                    Internal. Only to be used through TI provided API.
// W0BIT                    Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO1_REDWIDTH_W                                            3
#define NVMNW_GBLINFO1_REDWIDTH_M                                   0x00070000
#define NVMNW_GBLINFO1_REDWIDTH_S                                           16
#define NVMNW_GBLINFO1_REDWIDTH_W4BIT                               0x00040000
#define NVMNW_GBLINFO1_REDWIDTH_W2BIT                               0x00020000
#define NVMNW_GBLINFO1_REDWIDTH_W0BIT                               0x00000000

// Field:  [12:8] ECCWIDTH
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// W16BIT                   Internal. Only to be used through TI provided API.
// W8BIT                    Internal. Only to be used through TI provided API.
// W0BIT                    Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO1_ECCWIDTH_W                                            5
#define NVMNW_GBLINFO1_ECCWIDTH_M                                   0x00001F00
#define NVMNW_GBLINFO1_ECCWIDTH_S                                            8
#define NVMNW_GBLINFO1_ECCWIDTH_W16BIT                              0x00001000
#define NVMNW_GBLINFO1_ECCWIDTH_W8BIT                               0x00000800
#define NVMNW_GBLINFO1_ECCWIDTH_W0BIT                               0x00000000

// Field:   [7:0] DATAWIDTH
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// W128BIT                  Internal. Only to be used through TI provided API.
// W64BIT                   Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO1_DATAWIDTH_W                                           8
#define NVMNW_GBLINFO1_DATAWIDTH_M                                  0x000000FF
#define NVMNW_GBLINFO1_DATAWIDTH_S                                           0
#define NVMNW_GBLINFO1_DATAWIDTH_W128BIT                            0x00000080
#define NVMNW_GBLINFO1_DATAWIDTH_W64BIT                             0x00000040

//*****************************************************************************
//
// Register: NVMNW_O_GBLINFO2
//
//*****************************************************************************
// Field:   [3:0] DATAREGISTERS
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXIMUM                  Internal. Only to be used through TI provided API.
// MINIMUM                  Internal. Only to be used through TI provided API.
#define NVMNW_GBLINFO2_DATAREGISTERS_W                                       4
#define NVMNW_GBLINFO2_DATAREGISTERS_M                              0x0000000F
#define NVMNW_GBLINFO2_DATAREGISTERS_S                                       0
#define NVMNW_GBLINFO2_DATAREGISTERS_MAXIMUM                        0x00000008
#define NVMNW_GBLINFO2_DATAREGISTERS_MINIMUM                        0x00000001

//*****************************************************************************
//
// Register: NVMNW_O_BANK0INFO0
//
//*****************************************************************************
// Field:  [11:0] MAINSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK0INFO0_MAINSIZE_W                                         12
#define NVMNW_BANK0INFO0_MAINSIZE_M                                 0x00000FFF
#define NVMNW_BANK0INFO0_MAINSIZE_S                                          0
#define NVMNW_BANK0INFO0_MAINSIZE_MAXSECTORS                        0x00000200
#define NVMNW_BANK0INFO0_MAINSIZE_MINSECTORS                        0x00000008

//*****************************************************************************
//
// Register: NVMNW_O_BANK0INFO1
//
//*****************************************************************************
// Field: [23:16] ENGRSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK0INFO1_ENGRSIZE_W                                          8
#define NVMNW_BANK0INFO1_ENGRSIZE_M                                 0x00FF0000
#define NVMNW_BANK0INFO1_ENGRSIZE_S                                         16
#define NVMNW_BANK0INFO1_ENGRSIZE_MAXSECTORS                        0x00200000
#define NVMNW_BANK0INFO1_ENGRSIZE_MINSECTORS                        0x00000000

// Field:  [15:8] TRIMSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK0INFO1_TRIMSIZE_W                                          8
#define NVMNW_BANK0INFO1_TRIMSIZE_M                                 0x0000FF00
#define NVMNW_BANK0INFO1_TRIMSIZE_S                                          8
#define NVMNW_BANK0INFO1_TRIMSIZE_MAXSECTORS                        0x00002000
#define NVMNW_BANK0INFO1_TRIMSIZE_MINSECTORS                        0x00000000

// Field:   [7:0] NONMAINSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK0INFO1_NONMAINSIZE_W                                       8
#define NVMNW_BANK0INFO1_NONMAINSIZE_M                              0x000000FF
#define NVMNW_BANK0INFO1_NONMAINSIZE_S                                       0
#define NVMNW_BANK0INFO1_NONMAINSIZE_MAXSECTORS                     0x00000020
#define NVMNW_BANK0INFO1_NONMAINSIZE_MINSECTORS                     0x00000000

//*****************************************************************************
//
// Register: NVMNW_O_BANK1INFO0
//
//*****************************************************************************
// Field:  [11:0] MAINSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK1INFO0_MAINSIZE_W                                         12
#define NVMNW_BANK1INFO0_MAINSIZE_M                                 0x00000FFF
#define NVMNW_BANK1INFO0_MAINSIZE_S                                          0
#define NVMNW_BANK1INFO0_MAINSIZE_MAXSECTORS                        0x00000200
#define NVMNW_BANK1INFO0_MAINSIZE_MINSECTORS                        0x00000008

//*****************************************************************************
//
// Register: NVMNW_O_BANK1INFO1
//
//*****************************************************************************
// Field: [23:16] ENGRSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK1INFO1_ENGRSIZE_W                                          8
#define NVMNW_BANK1INFO1_ENGRSIZE_M                                 0x00FF0000
#define NVMNW_BANK1INFO1_ENGRSIZE_S                                         16
#define NVMNW_BANK1INFO1_ENGRSIZE_MAXSECTORS                        0x00200000
#define NVMNW_BANK1INFO1_ENGRSIZE_MINSECTORS                        0x00000000

// Field:  [15:8] TRIMSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK1INFO1_TRIMSIZE_W                                          8
#define NVMNW_BANK1INFO1_TRIMSIZE_M                                 0x0000FF00
#define NVMNW_BANK1INFO1_TRIMSIZE_S                                          8
#define NVMNW_BANK1INFO1_TRIMSIZE_MAXSECTORS                        0x00002000
#define NVMNW_BANK1INFO1_TRIMSIZE_MINSECTORS                        0x00000000

// Field:   [7:0] NONMAINSIZE
//
// Internal. Only to be used through TI provided API.
// ENUMs:
// MAXSECTORS               Internal. Only to be used through TI provided API.
// MINSECTORS               Internal. Only to be used through TI provided API.
#define NVMNW_BANK1INFO1_NONMAINSIZE_W                                       8
#define NVMNW_BANK1INFO1_NONMAINSIZE_M                              0x000000FF
#define NVMNW_BANK1INFO1_NONMAINSIZE_S                                       0
#define NVMNW_BANK1INFO1_NONMAINSIZE_MAXSECTORS                     0x00000020
#define NVMNW_BANK1INFO1_NONMAINSIZE_MINSECTORS                     0x00000000


#endif // __NVMNW__
