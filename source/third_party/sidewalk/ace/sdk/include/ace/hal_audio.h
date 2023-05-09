/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
/**
 * @file hal_audio.h
 * @addtogroup ACE_HAL_AUDIO
 * @brief Audio HAL provides the interfaces to control the audio devices.
 * audio device state/control/open/close/read/write
 *
 * The normal working steps as following:
 * 1. aceAudDev_getDevice
 * 2. aceAudDev_open
 * 3. aceAudDev_write/aceAudDev_read
 * 4. aceAudDev_close
 * @{
 */
#ifndef ACE_SDK_HAL_AUDIO_H_
#define ACE_SDK_HAL_AUDIO_H_
#include <stdio.h>
#include <stdint.h>
#include <ace/ace_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ACE_HAL_AUDIO_DS Data Structures and Enums
 * @{
 * @ingroup ACE_HAL_AUDIO
 */

/**
 * @brief Audio Device IO handle type definition for playback or capture.
 */
typedef void* aceAud_ioHandle_t;

/**
 * @brief Audio device type definition.
 *  audio device can support playback or capture or both of them.
 *  Such as (ACE_AUD_DEV_PLAYBACK | ACE_AUD_DEV_CAPTURE)
 */
typedef enum {
    ACE_AUD_DEV_PLAYBACK = (1 << 0), /**< audio device support playback */
    ACE_AUD_DEV_CAPTURE = (1 << 1),  /**< audio device support capture */
    /** audio device support both playback and capture */
    ACE_AUD_DEV_PLAYBACK_CAPTURE = (ACE_AUD_DEV_PLAYBACK | ACE_AUD_DEV_CAPTURE),
} aceAud_devType_t;

/**
 * @brief audio device state type definition
 */
typedef enum {
    ACE_AUD_DEV_INVALID = 0, /**< device is invalid, can't be used now */
    ACE_AUD_DEV_VALID,       /**< device is valid, can be used now */
} aceAud_devState_t;

/**
 * @brief audio device control command type definition
 */
typedef enum {
    ACE_AUD_DEV_CMD_QUERY_SAMPLE_RATE = 0, /**< query sample rate */
    ACE_AUD_DEV_CMD_QUERY_FORMAT,          /**< query audio format */
    ACE_AUD_DEV_CMD_QUERY_CHANNEL_NUM,     /**< query channel number */
    ACE_AUD_DEV_CMD_QUERY_CHANNEL_LAYOUT,  /**< query channel layout */
    ACE_AUD_DEV_CMD_GET_LAST_ERROR,        /**< get last error code */
    ACE_AUD_DEV_CMD_GET_JACK_STATE,        /**< get jack state */
    ACE_AUD_DEV_CMD_SET_CALLBACK,          /**< set device callback */
} aceAud_devCmdID_t;

/**
 * @brief Headphone jack state type definition
 */
typedef enum {
    ACE_AUD_DEV_JACK_NONE = 0,  /**< No headphone/headset plug in */
    ACE_AUD_DEV_JACK_HEADPHONE, /**< Jack with headphone plug in */
    ACE_AUD_DEV_JACK_HEADSET,   /**< Jack with headset plug in */
} aceAud_devJackState_t;

/**
 * @brief Audio device callback event type definition
 */
typedef enum {
    /** Device State Change, args = ::aceAud_devState_t */
    ACE_AUD_DEV_CB_EVENT_DEV_STATE = 0,
    /** Jack State Change, args = ::aceAud_devJackState_t */
    ACE_AUD_DEV_CB_EVENT_JACK_STATE,
} aceAud_devCallbackEvent_t;

/**
 * @brief audio device cmd ack type definition
 */
typedef enum {
    ACE_AUD_DEV_ACK_OK = 0,         /**< Command ACK OK */
    ACE_AUD_DEV_ACK_ERR,            /**< Command ACK with error */
    ACE_AUD_DEV_ACK_UNSUPPORT_CMD,  /**< Unsupport command ACK */
    ACE_AUD_DEV_ACK_UNSUPPORT_TYPE, /**< Unsupport type ACK in query command */
} aceAud_devAck_t;

/**
 * @brief audio device I/O port type definition
 */
typedef enum {
    ACE_AUD_DEV_IOPORT_NONE = 0, /**< None audio I/O Port */
    ACE_AUD_DEV_IOPORT_DEFAULT,  /**< Default audio I/O Port */

    /** Basic audio I/O port */
    ACE_AUD_DEV_IOPORT_SPEAKER, /**< Build in Speaker, Output Only Port */
    ACE_AUD_DEV_IOPORT_MIC,     /**< Build in Mic Input Only Port */
    ACE_AUD_DEV_IOPORT_HEADSET, /**< Headset I/O Port */
    ACE_AUD_DEV_IOPORT_LINE,    /**< Line I/O I/O Port */
    ACE_AUD_DEV_IOPORT_BT,      /**< BT I/O Port */
    ACE_AUD_DEV_IOPORT_WIFI,    /**< WIFI I/O Port */
    ACE_AUD_DEV_IOPORT_SPDIF,   /**< SPDIF I/O Port */
    ACE_AUD_DEV_IOPORT_HDMI,    /**< HDMI I/O Port */
    ACE_AUD_DEV_IOPORT_ADSP,    /**< Audio DSP I/O Port */

    /** Extern general I2S I/O port number */
    ACE_AUD_DEV_IOPORT_I2S_1,
    ACE_AUD_DEV_IOPORT_I2S_2,
    ACE_AUD_DEV_IOPORT_I2S_3,
    ACE_AUD_DEV_IOPORT_I2S_4,

    /** Extern general build in audio DAC I/O port number. */
    ACE_AUD_DEV_IOPORT_DAC_1,
    ACE_AUD_DEV_IOPORT_DAC_2,
    ACE_AUD_DEV_IOPORT_DAC_3,
    ACE_AUD_DEV_IOPORT_DAC_4,

    /** Extern general build in audio ADC I/O port number. */
    ACE_AUD_DEV_IOPORT_ADC_1,
    ACE_AUD_DEV_IOPORT_ADC_2,
    ACE_AUD_DEV_IOPORT_ADC_3,
    ACE_AUD_DEV_IOPORT_ADC_4,

    /** Extern general IEC digital interface Rx/Tx I/O port number */
    ACE_AUD_DEV_IOPORT_IEC_1,
    ACE_AUD_DEV_IOPORT_IEC_2,
    ACE_AUD_DEV_IOPORT_IEC_3,
    ACE_AUD_DEV_IOPORT_IEC_4,

    /** More audio I/O port define @ here */

    /** Other Audio I/O Port Define */
    ACE_AUD_DEV_IOPORT_VIRTUAL = 0xE0, /**< Vitural I/O (Debug Only) */
    ACE_AUD_DEV_IOPORT_FILE,           /**< File I/O (Debug Only) */
    ACE_AUD_DEV_IOPORT_MAX = 0xFE,
    ACE_AUD_DEV_IOPORT_ALL = 0xFF
} aceAud_devIOPort_t;

/**
 * @brief audio device IO instance state type definition
 */
typedef enum {
    ACE_AUD_IO_INVALID = 0, /**< Audio device IO error */
    ACE_AUD_IO_OPENING,     /**< Audio device IO on opening */
    ACE_AUD_IO_READY,       /**< ready for read/write after open */
    ACE_AUD_IO_RUNNING,     /**< Audio device IO is running */
    ACE_AUD_IO_XRUN,        /**< under run or over run */
} aceAud_ioState_t;

/**
 * @brief Type audio device IO open mode definition.
 */
typedef enum {
    ACE_AUD_IO_PLAYBACK = 1, /**< Open IO as playback */
    ACE_AUD_IO_CAPTURE = 2,  /**< Open IO as capture */
} aceAud_ioMode_t;

/**
 * @brief audio IO control command type definition
 */
typedef enum {
    ACE_AUD_IO_CMD_GET_LATENCY = 1, /**< Get IO latency data in bytes */
    ACE_AUD_IO_CMD_GET_VALID,       /**< Get valid data size in bytes */
    ACE_AUD_IO_CMD_SET_CALLBACK,    /**< Set audio IO callback */
    ACE_AUD_IO_CMD_GET_BUFFER_SIZE, /**< Get audio IO buffer size iin bytes */
    ACE_AUD_IO_CMD_SET_THRESHOLD,   /**< Set audio IO threshold size in bytes */
    ACE_AUD_IO_CMD_SET_VOLUME,      /**< Set audio IO volume Fixed Val Q1.15 */
    ACE_AUD_IO_CMD_GET_VOLUME,      /**< Get audio IO volume Fixed Val Q1.15 */
} aceAud_ioCmdID_t;

/**
 * @brief audio IO control ACK definition
 */
typedef enum {
    ACE_AUD_IO_ACK_OK = 0,        /**< General IO ACK OK */
    ACE_AUD_IO_ACK_ERR,           /**< General IO command ACK ERROR */
    ACE_AUD_IO_ACK_UNSUPPORT_CMD, /**< Unsupport command by this device io */
} aceAud_ioAck_t;

/**
 * @brief Audio IO callback event type definition
 */
typedef enum {
    /** Audio IO State Change, args = ::aceAud_ioState_t */
    ACE_AUD_IO_CB_EVENT_IO_STATE = 0,
    /** Audio IO Buffer Change, args = valid buffer size */
    ACE_AUD_IO_CB_EVENT_VALID_BUFFER,
} aceAud_ioCallbackEvent_t;

/**
 * @brief audio IO date format type definition
 */
typedef enum {
    ACE_AUD_IO_FMT_PCM = 1,  /**< PCM */
    ACE_AUD_IO_FMT_MPEG,     /**< MPEG L I/II/III */
    ACE_AUD_IO_FMT_AAC,      /**< AAC (AAC LC, HE-AAC L1/L2) */
    ACE_AUD_IO_FMT_VORBIS,   /**< Vorbis */
    ACE_AUD_IO_FMT_OPUS,     /**< OPUS */
    ACE_AUD_IO_FMT_FLAC,     /**< FLAC */
    ACE_AUD_IO_FMT_APE,      /**< APE */
    ACE_AUD_IO_FMT_SBC,      /**< SBC (for BT) */
    ACE_AUD_IO_FMT_APTX,     /**< aptx (for BT) */
    ACE_AUD_IO_FMT_ALAC,     /**< ALAC */
    ACE_AUD_IO_FMT_WMA,      /**< WMA */
    ACE_AUD_IO_FMT_WMAPRO,   /**< WMA Pro */
    ACE_AUD_IO_FMT_COOK,     /**< Cook (rm) */
    ACE_AUD_IO_FMT_G729,     /**< G.729 (speech) */
    ACE_AUD_IO_FMT_G7231,    /**< G.723.1 (speech) */
    ACE_AUD_IO_FMT_AC3,      /**< AC3 */
    ACE_AUD_IO_FMT_EAC3,     /**< EAC3 (DD+) */
    ACE_AUD_IO_FMT_AMR_NB,   /**< AMR NB */
    ACE_AUD_IO_FMT_AMR_WB,   /**< AMW WB */
    ACE_AUD_IO_FMT_DTS,      /**< DTS */
    ACE_AUD_IO_FMT_DTSLBR,   /**< DTS Low bitrate */
    ACE_AUD_IO_FMT_TRUEHD,   /**< Dolby TrueHD */
    ACE_AUD_IO_FMT_DTSHD_MA, /**< DTS HD MA */
    ACE_AUD_IO_FMT_DSD,      /**< DSD (Direct Stream Digital) */
    ACE_AUD_IO_FMT_IEC60958, /**< IEC60958 packet */
    ACE_AUD_IO_FMT_IEC61937, /**< IEC61937 packet */
} aceAud_ioFmt_t;

/**
 * @brief Audio IO pcm sub types definition.
 *  all device have to support type ::ACE_AUD_IO_PCM_S16_LE
 */
typedef enum {
    ACE_AUD_IO_PCM_S16_LE = 0, /**< LPCM signed 16bits little endian */
    ACE_AUD_IO_PCM_S16_BE,     /**< LPCM signed 16bits big endian */
    ACE_AUD_IO_PCM_U16_LE,     /**< LPCM unsigned 16bits little endian */
    ACE_AUD_IO_PCM_U16_BE,     /**< LPCM unsigned 16bits big endian*/
    ACE_AUD_IO_PCM_S24_LE,     /**< LPCM signed packed 24bits little endian */
    ACE_AUD_IO_PCM_S24_BE,     /**< LPCM signed packed 24bits big endian */
    ACE_AUD_IO_PCM_U24_LE,     /**< LPCM unsigned packed 24bits LE */
    ACE_AUD_IO_PCM_U24_BE,     /**< LPCM unsigned packed 24bits big endian */
    ACE_AUD_IO_PCM_S32_LE,     /**< LPCM signed 32bits little endian */
    ACE_AUD_IO_PCM_S32_BE,     /**< LPCM signed 32bits big endian */
    ACE_AUD_IO_PCM_U32_LE,     /**< LPCM unsigned 32bits little endian */
    ACE_AUD_IO_PCM_U32_BE,     /**< LPCM unsigned 32bits big endian */
} aceAud_ioPcmSub_t;

/**
 * @brief Audio IO iec data type definition.
 *  the type value define follow IEC-61937 PC spec
 */
typedef enum {
    ACE_AUD_IO_IEC_AC3 = 1,
    ACE_AUD_IO_IEC_PAUSE = 3,
    ACE_AUD_IO_IEC_MPEG_LI = 4,
    ACE_AUD_IO_IEC_MPEG_LII_III = 5,
    ACE_AUD_IO_IEC_MPEG2_AAC = 7,
    ACE_AUD_IO_IEC_DTS_I = 11,
    ACE_AUD_IO_IEC_DTS_II = 12,
    ACE_AUD_IO_IEC_DTS_III = 13,
    ACE_AUD_IO_IEC_EAC3 = 21,
} aceAud_ioIECPc_t;

/**
 * @brief device IO sample rate type definition
 */
typedef enum {
    ACE_AUD_IO_8K_HZ = 8000,
    ACE_AUD_IO_11K_HZ = 11025,
    ACE_AUD_IO_12K_HZ = 12000,
    ACE_AUD_IO_16K_HZ = 16000,
    ACE_AUD_IO_22K_HZ = 22050,
    ACE_AUD_IO_24K_HZ = 24000,
    ACE_AUD_IO_32K_HZ = 32000,
    ACE_AUD_IO_44K_HZ = 44100,
    ACE_AUD_IO_48K_HZ = 48000,
    ACE_AUD_IO_64K_HZ = 64000,
    ACE_AUD_IO_88K_HZ = 88200,
    ACE_AUD_IO_96K_HZ = 96000,
    ACE_AUD_IO_128K_HZ = 128000,
    ACE_AUD_IO_176K_HZ = 176400,
    ACE_AUD_IO_192K_HZ = 192000,
    ACE_AUD_IO_256K_HZ = 256000,
    ACE_AUD_IO_352K_HZ = 352800,
    ACE_AUD_IO_384K_HZ = 384000,
} aceAud_ioRate_t;

/**
 * @brief Type PCM channel name type definition
 */
typedef enum {
    ACE_AUD_IO_CH_NONE = 0, /**< No channel */
    ACE_AUD_IO_CH_L = 1,    /**< Left channel */
    ACE_AUD_IO_CH_R = 2,    /**< Right channel */
    ACE_AUD_IO_CH_C = 3,    /**< Center Channel or mono channel */
    ACE_AUD_IO_CH_LFE = 4,  /**< LFE channel */
    ACE_AUD_IO_CH_S = 5,    /**< Surround channel */
    ACE_AUD_IO_CH_LS = 6,   /**< Left surround channel */
    ACE_AUD_IO_CH_RS = 7,   /**< Right surround channel */
    ACE_AUD_IO_CH_CS = 8,   /**< Center surround channel */
    ACE_AUD_IO_CH_LSR = 9,  /**< Left surround rear channel */
    ACE_AUD_IO_CH_RSR = 10, /**< Right surround rear channel */

    /** More channel name define @ here */

    ACE_AUD_IO_CH_MAX = 16, /**< channel name < max */
} aceAud_ioPcmChannelName_t;

#define ACE_AUD_IO_CH_BITS (4) /**< use 4bits for each channel name */

/**
 * @brief Type PCM channel layout definition
 */
typedef enum {
    /** Mono: PCM L,L,L,L,L,... */
    ACE_AUD_IO_PCM_MONO = ACE_AUD_IO_CH_C,
    /** Stereo: PCM L,R,L,R,L,R,... */
    ACE_AUD_IO_PCM_STEREO =
        (ACE_AUD_IO_CH_L | (ACE_AUD_IO_CH_R << ACE_AUD_IO_CH_BITS)),
    /** 2.1: PCM L,R,LFE,L,R,LFE,... */
    ACE_AUD_IO_PCM_2P1 = (ACE_AUD_IO_PCM_STEREO |
                          (ACE_AUD_IO_CH_LFE << (2 * ACE_AUD_IO_CH_BITS))),
    /** 5.1: PCM L,R,LFE,LS,RS,L,R,LFE,LS,RS,... */
    ACE_AUD_IO_PCM_5P1 =
        (ACE_AUD_IO_PCM_2P1 | (ACE_AUD_IO_CH_LS << (3 * ACE_AUD_IO_CH_BITS)) |
         (ACE_AUD_IO_CH_RS << (4 * ACE_AUD_IO_CH_BITS))),
    /** 7.1: PCM L,R,LFE,LS,RS,LSR,RSR,... */
    ACE_AUD_IO_PCM_7P1 =
        (ACE_AUD_IO_PCM_5P1 | (ACE_AUD_IO_CH_LSR << (5 * ACE_AUD_IO_CH_BITS)) |
         (ACE_AUD_IO_CH_RSR << (6 * ACE_AUD_IO_CH_BITS))),
} aceAud_ioPcmLayout_t;

/**
 * @brief Audio IO open attribute definition
 */
typedef struct {
    uint32_t rate; /**< sample rate ::aceAud_ioRate_t */
    uint16_t fmt;  /**< stream format ::aceAud_ioFmt_t */
    uint16_t mode; /**< IO open for playback/capture ::aceAud_ioMode_t */

    union {
        /** fmt == ACE_AUD_IO_FMT_PCM, parameters */
        struct {
            uint32_t layout;    /**< ::aceAud_ioPcmLayout_t */
            uint16_t sub_type;  /**< ::aceAud_ioPcmSub_t */
            uint8_t ch_num;     /**< PCM channel number */
            uint8_t frame_size; /**< = (ch_num * bitsPerSample) / 8 */
        } pcm;

        /** fmt == ACE_AUD_IO_FMT_IEC60958/IEC61937, parameters */
        struct {
            uint32_t ch_status; /**< 0~31bits channel status */
            uint16_t type;      /**< ::aceAud_ioIECPc_t */
            uint16_t sub_type;  /**< ::aceAud_ioPcmSub_t */
        } iec;

        /** [optional]: other compressed audio raw data format */
        struct {
            uint32_t args[2]; /**< raw data format extension args */
        } raw;
    } param;
} aceAud_ioAttr_t;

/**
 * @brief Device and IO control command and ack type definition.
 *  The device dev_ctrl and IO ctrl function will send cmd and get ack from
 *  device / IO.
 */
typedef struct {
    struct {
        /** The Device or IO command ID
         *  for dev_ctrl command ID refer to ::aceAud_devCmdID_t
         *  for device IO ctrl command ID refer to ::aceAud_ioCmdID_t */
        int32_t cmd;
        int32_t param; /**< command parameter */
        void* args[2]; /**< command extra arguments */
    } cmd;

    struct {
        /** The device or IO command ack.
         *  for dev_ctrl ack refer to ::aceAud_devAck_t
         *  for device IO ctrl ack refer to ::aceAud_ioAck_t */
        int32_t ack;
        int32_t param; /**< ack return parameters */
        void* args[2]; /**< ack return extra arguments */
    } ack;
} aceAud_devCmd_t;

/**
 * @brief Type audio device virtual function call definition
 */
typedef struct {
    /** Get device state, refer to ::aceAudDev_devState */
    int32_t (*dev_state)(void);
    /** Device command control, refer to ::aceAudDev_devCtrl */
    ace_status_t (*dev_ctrl)(aceAud_devCmd_t* cmd);

    /** Open device IO Port, refer to ::aceAudDev_open */
    aceAud_ioHandle_t (*open)(const aceAud_ioAttr_t* attr);
    /** Close device IO handle, refer to ::aceAudDev_close */
    ace_status_t (*close)(aceAud_ioHandle_t io);
    /** Write data to io, refer to ::aceAudDev_write */
    int32_t (*write)(aceAud_ioHandle_t io, void* data, int32_t size,
                     int32_t block);
    /** Read data from io, refer to ::aceAudDev_read */
    int32_t (*read)(aceAud_ioHandle_t io, void* data, int32_t size,
                    int32_t block);
    /** Get current IO State, refer to ::aceAudDev_state */
    int32_t (*state)(aceAud_ioHandle_t io);
    /** IO control command, refer to ::aceAudDev_ctrl */
    ace_status_t (*ctrl)(aceAud_ioHandle_t io, aceAud_devCmd_t* cmd);
} aceAud_devVfc_t;

/**
 * @brief Audio device instance structure type definition
 */
typedef struct {
    const char* name;           /**< device name */
    const aceAud_devVfc_t* vfc; /**< device virtual function call entry */
    uint16_t ioport;            /**< device IO port ::aceAud_devIOPort_t */
    uint16_t dtype;             /**< device support type ::aceAud_devType_t */
} aceAud_dev_t;

/**
 * @brief Audio device callback function type definition
 */
typedef ace_status_t (*aceAud_devCallback_t)(
    const aceAud_dev_t* dev, /**< audio device instance handle */
    int32_t event, /**< callback event type ::aceAud_devCallbackEvent_t */
    intptr_t args, /**< callback event arguments */
    void* puser    /**< callback user's data pointer */
    );

/**
 * @brief Audio IO callback function type definition
 */
typedef ace_status_t (*aceAud_ioCallback_t)(
    const aceAud_dev_t* dev, /**< audio device instance handle */
    aceAud_ioHandle_t io,    /**< audio io handle */
    int32_t event, /**< callback event type ::aceAud_ioCallbackEvent_t */
    intptr_t args, /**< callback event arguments */
    void* puser    /**< callback user's data pointer */
    );
/** @} */

/**
 * @defgroup ACE_HAL_AUDIO_API Public API
 * @{
 * @ingroup ACE_HAL_AUDIO
 */

/**
 * @brief Platform audio device global init.
 * Audio manager will call this function to init audio devices during startup.
 * @param args: [in] the arguments for audio device init on this platform.
 *      NULL means use default setting.
 *      !NULL platform defined args.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceAudDev_init(void* args);

/**
 * @brief Platform audio device global deinit function.
 *  The ace audio manager will call this function when audio manager exit.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceAudDev_deinit(void);

/**
 * @brief Get platform supported audio device list.
 * @param dev_num: [out] return supported audio device number.
 * @return Platform supported audio device list pointer.
 * @retval NULL This platform don't support any audio device.
 */
const aceAud_dev_t* const* aceAudDev_getDeviceList(int32_t* dev_num);

/**
 * @brief Get platform audio device instance function.
 *  The ace audio manager will call this function to get the specific
 *  audio device instance handle.
 * @param ioport [in] device io port type ::aceAud_devIOPort_t
 * @param dtype [in] device type ::aceAud_devType_t
 * @return The audio device instance handle.
 * @retval NULL get the specific audio device fail.
 * @retval !NULL get the audio device instance handle.
 */
const aceAud_dev_t* aceAudDev_getDevice(uint16_t ioport, uint16_t dtype);

/**
 * @brief  Get current device state ::aceAud_devState_t
 * @param dev: [in] audio device instance handle.
 * @return device state ::aceAud_devState_t
 */
int32_t aceAudDev_devState(const aceAud_dev_t* dev);

/**
 * @brief Device command control,
 * handle device control command and return ack.
 * @param dev: [in] audio device instance handle.
 * @param cmd: [in/out] the command and ack, refer to ::aceAud_devCmd_t.
 *      command id refer to ::aceAud_devCmdID_t.
 *      ack refer to ::aceAud_devAck_t.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceAudDev_devCtrl(const aceAud_dev_t* dev, aceAud_devCmd_t* cmd);

/**
 * @brief Set audio device callback
 * @param dev: [in] audio device instance handle.
 * @param callback [in] device callback function pointer
 * @param puser [in] callback user's data pointer
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceAudDev_setDevCallback(const aceAud_dev_t* dev,
                                      aceAud_devCallback_t callback,
                                      void* puser);

/**
 * @brief Get default audio attribute for IO open.
 * @param attr: [out] default attribte, 48KHz, 2ch, S16_LE, PCM
 */
void aceAudDev_getDefaultAttr(aceAud_ioAttr_t* attr);

/**
 * @brief  Device IO Open for playback or capture.
 * @param dev:  [in] audio device instance handle.
 * @param attr: [in] the IO attribute pointer for open.
 * @return IO handle ::aceAud_ioHandle_t
 * @retval !NULL open sucess.
 * @retval NULL open fail
 */
aceAud_ioHandle_t aceAudDev_open(const aceAud_dev_t* dev,
                                 const aceAud_ioAttr_t* attr);

/**
 * @brief  Device IO Close
 * @param dev: [in] audio device instance handle.
 * @param  io: [in] the device IO handle.
 * @return ace_status_t: Return zero if successful, or any non-zero error code.
 */
ace_status_t aceAudDev_close(const aceAud_dev_t* dev, aceAud_ioHandle_t io);

/**
 * @brief Device IO opened as playback, use write function write data to IO.
 * @param dev:  [in] audio device instance handle.
 * @param io:   [in] the device IO handle
 * @param data: [in] the data buffer to be write to IO
 * @param size: [in] the data size in bytes
 * @param block: [in] set the write process work in block or non-block mode.
 *          = 0 non-block mode, IO buffer full return real write data size.
 *          = 1 block mode, write until all data write to IO or error happens.
 * @return the real write data size in bytes or error code
 * @retval >= 0 real data size write to IO.
 *          In block mode if retval < size, means some error happens.
 * @retval <0   error code, refer to ace_status_t
 */
int32_t aceAudDev_write(const aceAud_dev_t* dev, aceAud_ioHandle_t io,
                        void* data, int32_t size, int32_t block);

/**
 * @brief Device IO opened as capture, use read function to read data from IO.
 * @param dev:  [in] audio device instance handle.
 * @param io:   [in] the device IO handle
 * @param data: [out] read data from IO to this buffer
 * @param size: [in] the valid data buffer size in bytes
 * @param block: [in] set the read process work in block or non-block mode.
 *          = 0 non-block mode, IO buffer empty return real read data size.
 *          = 1 block mode, read until all data read from IO or error happens.
 * @return the real read data size or error code ace_status_t
 * @retval >= 0 real data size read from IO.
 *          In block mode if retval < size, means some error happens.
 * @retval <0   error code, refer to ace_status_t
 */
int32_t aceAudDev_read(const aceAud_dev_t* dev, aceAud_ioHandle_t io,
                       void* data, int32_t size, int32_t block);

/**
 * @brief Get current device IO state ::
 * @param dev:  [in] audio device instance handle.
 * @param io:   [in] the device IO handle
 * @return  the IO state ::aceAud_ioState_t or error code ace_status_t
 * @retval  >= 0 IO state, refer to ::aceAud_ioState_t
 * @retval  < 0  error code, refer to ace_status_t
 */
int32_t aceAudDev_state(const aceAud_dev_t* dev, aceAud_ioHandle_t io);

/**
 * @brief Device IO control, handle io control command and return ack.
 * @param dev: [in] audio device instance handle.
 * @param io:  [in] the device IO handle
 * @param cmd: [in/out] the control command and ack, refer ::aceAud_devCmd_t.
 *          command id refer to ::aceAud_ioCmdID_t
 *          ack refer to ::aceAud_ioAck_t
 * @retval  < 0  error code, refer to ace_status_t
 */
ace_status_t aceAudDev_ctrl(const aceAud_dev_t* dev, aceAud_ioHandle_t io,
                            aceAud_devCmd_t* cmd);

/**
 * @brief Set audio IO callback function.
 * @param dev: [in] audio device instance handle.
 * @param io:  [in] the device IO handle
 * @param callback: [in] the IO callback function pointer
 * @param puser: [in] the callback user's data pointer.
 * @retval  < 0  error code, refer to ace_status_t
 */
ace_status_t aceAudDev_setIoCallback(const aceAud_dev_t* dev,
                                     aceAud_ioHandle_t io,
                                     aceAud_ioCallback_t callback, void* puser);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_AUDIO_H_ */
/** @} */
