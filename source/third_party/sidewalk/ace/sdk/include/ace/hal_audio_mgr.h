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
 * @file hal_audio_mgr.h
 * @addtogroup ACE_HAL_AUDIO_MANAGER
 * @brief Audio Manager HAL provides the APIs to control the audio manager.
 * @{
 */
#ifndef ACE_SDK_HAL_AUDIO_MGR_H_
#define ACE_SDK_HAL_AUDIO_MGR_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ace/ace_status.h"
#include "ace/hal_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ACE_HAL_AUDIO_MANAGER_DS Data Structures and Enums
 * @{
 * @ingroup ACE_HAL_AUDIO_MANAGER
 */

/**
 * @brief ACE Audio common handle type definition
 */
typedef void* aceAud_handle_t;

/**
 * @brief ACE Audio stream handle
 */
typedef aceAud_handle_t aceAud_stream_t;

/**
 * @brief ACE audio message command ID definition
 * ::aceAud_msg_t.cmd_id = ::aceAudCmd_id_t
 */
typedef enum {
    ACE_AUD_CMD_CTRL_SYS = 0,  /**< ctrl system command */
    ACE_AUD_CMD_CTRL_PROC,     /**< ctrl audio proc command */
    ACE_AUD_CMD_CTRL_EXT_BASE, /**< ctrl extend cmd base */
    ACE_AUD_CMD_MAX = 0xFF
} aceAudCmd_id_t;

/**
 * @brief System Command bparam type definition.
 * if ::aceAud_msg_t.cmd_id == ACE_AUD_CMD_CTRL_SYS
 * the ::aceAud_msg_t.bparam value definition
 */
typedef enum {
    /** ACE Audio Client (AC) app used ctrl system command */
    ACE_AUD_CTRL_SYS_TEST_ACK = 0, /**< audio mgr test ack command */
    ACE_AUD_CTRL_SYS_EXIT,         /**< audio mgr exit command */
    ACE_AUD_CTRL_SYS_PROC_NEW,     /**< audio mgr new a proc */
    ACE_AUD_CTRL_SYS_PROC_DEL,     /**< audio mgr delete a proc */
    ACE_AUD_CTRL_SYS_PROC_LINK,    /**< audio mgr add proc to link  */
    ACE_AUD_CTRL_SYS_PROC_UNLINK,  /**< audio mgr del proc from link */
    ACE_AUD_CTRL_SYS_EXT_MAX,      /**< the end ctrl id from AC to AM */
    /** TODO: Other extern definition base */
    ACE_AUD_CTRL_SYS_MAX = 0xFF
} aceAudCmd_ctrlSysBParam_t;

/**
 * @brief Proc command bparam type definition.
 * if ::aceAud_msg_t.cmd_id = ACE_AUD_CMD_CTRL_PROC
 * the ::aceAud_msg_t.bparam value definition.
 * All of the ctrl command are optional for Proc to implement.
 */
typedef enum {
    ACE_AUD_CTRL_PROC_TEST_ACK = 0, /**< proc test ack command */
    ACE_AUD_CTRL_PROC_INIT,         /**< proc init command, optional */
    ACE_AUD_CTRL_PROC_START,        /**< proc start command, optional */
    ACE_AUD_CTRL_PROC_STOP,         /**< proc stop command, optional */
    ACE_AUD_CTRL_PROC_PAUSE,        /**< proc pause command, optional */
    ACE_AUD_CTRL_PROC_RESUME,       /**< proc resume command, optional */
    ACE_AUD_CTRL_PROC_CTRL,         /**< proc control command, optional */
    ACE_AUD_CTRL_PROC_DATA_LINK,    /**< proc data link, optional */
    ACE_AUD_CTRL_PROC_DATA_UNLINK,  /**< proc data unlink, optional */
    ACE_AUD_CTRL_PROC_EXTERN_BASE,  /**< proc extern cmd base, optional */
    ACE_AUD_CTRL_PROC_MAX = 0xFF
} aceAudCmd_ctrlProcBParam_t;

/**
 * @brief ACE audio message type definition.
 */
typedef struct {
    intptr_t pparam; /**< int pointer parameters */
    uint32_t lparam; /**< long parameters */
    uint16_t sparam; /**< short parameter */
    uint16_t src_id; /**< source id (AC ctrl id or AM proc id) */
    uint16_t dst_id; /**< target id (AC ctrl id or AM proc id) */
    uint8_t bparam;  /**< byte parameter */
    uint8_t cmd_id;  /**< message command ::aceAudCmd_id_t */
} aceAud_msg_t;

/**
 * @brief The note pool position type definition.
 * For ::aceAudCtrl_linkProc APIs used.
 */
typedef enum {
    ACE_AUD_NOTE_POOL_POS_SET = 0,          /**< the head of the link */
    ACE_AUD_NOTE_POOL_POS_AHEAD = 1,        /**< add ahead of the target */
    ACE_AUD_NOTE_POOL_POS_AFTER = 2,        /**< add after of the target */
    ACE_AUD_NOTE_POOL_POS_END = UINT16_MAX, /**< the tail of the link */
} aceAudNotePool_position_t;

/**
 * @brief audio stream playback or capture mode type define.
 */
typedef enum {
    ACE_AUD_PLAYBACK = (1 << 0),
    ACE_AUD_CAPTURE = (1 << 1),
} aceAud_mode_t;

/**
 * @brief ACE audio app user type definition
 */
typedef enum {
    ACE_AUD_USER_MEDIA = 0,    /**< audio for media player */
    ACE_AUD_USER_NOTIFICATION, /**< audio for notification sound */
    ACE_AUD_USER_RING,         /**< audio for call ring */
    ACE_AUD_USER_ALARM,        /**< audio for alarm */
    ACE_AUD_USER_CALL,         /**< audio for call */
    ACE_AUD_USER_EXCLUSIVE,    /**< audio for exclusive sound (e.g. Alexa) */
    /** TODO: Other User Type */
    ACE_AUD_USER_NUM, /**< audio user types number */
    ACE_AUD_USER_TYPE_MAX = 0xFF
} aceAud_user_t;

/**
 * @brief ACE audio stream content type definition
 */
typedef enum {
    ACE_AUD_CONTENT_MUSIC = 0, /**< audio content is music */
    ACE_AUD_CONTENT_MOVIE,     /**< audio content is movie */
    ACE_AUD_CONTENT_VOICE,     /**< audio content is voice (call) */
    ACE_AUD_CONTENT_TTS,       /**< audio content is TTS (Alexa response) */
    ACE_AUD_CONTENT_NEWS,      /**< audio content is news */
    /** TODO: Other audio content type define */
    ACE_AUD_CONTENT_NUM, /**< audio content types number */
    ACE_AUD_CONTENT_TYPE_MAX = 0xFF
} aceAud_content_t;

/**
 * @brief ACE Audio stream open attribute type definition
 */
typedef struct {
    uint32_t note_id;     /**< the stream proc note id (0: use default) */
    uint32_t app_id;      /**< The APP id (0: as default) */
    uint32_t bsize;       /**< set bit stream buffer size (0: use default) */
    uint32_t bps;         /**< stream bits per second (default 0 invalid) */
    aceAud_ioFmt_t ftype; /**< the stream format type */
    aceAud_ioRate_t rate; /**< stream sample rate */
    aceAud_ioPcmLayout_t layout; /**< audio stream layout*/
    uint32_t param[3];      /**< stream format ext parameters, sub format/... */
    aceAud_user_t utype;    /**< stream user type */
    aceAud_content_t ctype; /**< audio content type */
    aceAud_devIOPort_t io_port; /**< stream io port */
    bool data_header;           /**< stream data with header or not */
} aceAudStream_attr_t;

/**
 * @brief Audio stream data type.
 */
typedef enum {
    ACE_AUD_DATA_RAW = 0, /**< audio bit stream raw data type */
    ACE_AUD_DATA_META,    /**< audio stream metadata type */
    ACE_AUD_DATA_PTS,     /**< audio PTS in msec */
    /** TODO: More data type */
    ACE_AUD_DATA_MAX = UINT16_MAX
} aceAudStream_dataType_t;

/**
 * @brief Audio stream data header type definition.
 * If set ::aceAudStream_attr_t.data_header = true.
 * The audio stream data have to add this header for each audio data chunk.
 * |Header|Data|Header|Data|....
 */
#define ACE_AUD_DATA_SYNC_WORD (0xCAFE)
typedef struct {
    uint16_t sync; /**< the sync word = ::ACE_AUD_DATA_SYNC_WORD */
    uint16_t type; /**< the data type ::aceAudStream_dataType_t */
    uint32_t size; /**< following audio data size */
} aceAudStream_dataHeader_t;

/**
 * @brief audio steam state type
 *  State change:
 *  1. New proc as ACE_AUD_STREAM_INVALID
 *  2. Init proc done change to ACE_AUD_STREAM_READY
 *  3. Start proc done change to ACE_AUD_STREAM_PLAYING
 *  3.1. Pause proc done change to ACE_AUD_STREAM_PAUSED
 *  3.1.1. Stop proc from pause change to ACE_AUD_STREAM_STOPPED
 *  3.1.2. Resume proc from pause change to ACE_AUD_STREAM_PLAYING
 *  3.2. Stop proc from playing change to ACE_AUD_STREAM_STOPPED
 *  ACE_AUD_STREAM_PAUSING / ACE_AUD_STREAM_STOPPING are middle state.
 */
typedef enum {
    ACE_AUD_STREAM_INVALID = 0, /**< this stream is invalid, need init */
    ACE_AUD_STREAM_READY,       /**< stream is ready to start */
    ACE_AUD_STREAM_PLAYING,     /**< stream is playing */
    ACE_AUD_STREAM_PAUSING,     /**< stream is pausing */
    ACE_AUD_STREAM_STOPPING,    /**< stream is stopping */
    ACE_AUD_STREAM_PAUSED,      /**< stream is paused */
    ACE_AUD_STREAM_STOPPED,     /**< stream is stopped */
} aceAudStream_state_t;

/**
 * @brief audio stream valid bit stream buffer type
 */
typedef struct {
    void* data;    /**< the valid data buffer pointer */
    uint32_t size; /**< the valid data buffer size */
} aceAudStream_buf_t;

/**
 * @brief audio stream control command id define
 */
typedef enum {
    /** Set stream input/output IO port ::aceAud_devIOPort_t
     *  The default audio stream io port is ::ACE_AUD_DEV_IOPORT_DEFAULT */
    ACE_AUD_STREAM_CMD_SET_IO_PORT = ACE_AUD_CMD_CTRL_EXT_BASE,
    ACE_AUD_STREAM_CMD_GET_IO_PORT, /**< get stream io port. */
    ACE_AUD_STREAM_CMD_GET_LATENCY, /**< get stream latency */
    ACE_AUD_STREAM_CMD_GET_PTS,     /**< get stream PTS */
    ACE_AUD_STREAM_CMD_SET_SPEED,   /**< set stream speed (1.0x) */
    ACE_AUD_STREAM_CMD_GET_SPEED,   /**< get stream speed (1.0x) */
    ACE_AUD_STREAM_CMD_EXT_BASE,    /**< stream cmd extend base */
    /* TODO: other stream ctrl command */
    ACE_AUD_STREAM_CMD_MAX = 0xFF
} aceAudStream_cmdID_t;

/**
 * @brief ACE audio stream control command type definiton
 */
typedef struct {
    intptr_t pparam; /**< int pointer parameters */
    uint32_t lparam; /**< long parameters */
    uint16_t sparam; /**< short parameters */
    uint8_t bparam;  /**< byte parameter */
    uint8_t cmd_id;  /**< stream cmd id ::aceAudStream_cmdID_t */
} aceAudStream_ctrlCmd_t;

/**
 * @brief ACE audio stream control ack type definiton
 */
typedef struct {
    intptr_t pparam; /**< int pointer value */
    uint32_t lparam; /**< long value */
    uint16_t sparam; /**< short value */
    uint8_t bparam;  /**< byte value */
    int8_t ack;      /**< return ack state ace_status_t */
} aceAudStream_ctrlAck_t;

/**
 * @brief audio stream proc sync from audio client to audio stream proc
 */
typedef struct {
    uint32_t rate;       /**< sample rate ::aceAud_ioRate_t */
    uint32_t layout;     /**< layout ::aceAud_ioPcmLayout_t */
    uint32_t bps;        /**< stream bits per second */
    uint32_t param[3];   /**< stream extend parameters */
    uint16_t vol;        /**< stream vol gain, fixed point Q4.12 */
    uint16_t speed;      /**< stream present speed, Q4.12 */
    uint8_t ftype;       /**< stream format ::aceAud_ioFmt_t */
    uint8_t ctype;       /**< stream content type ::aceAud_content_t */
    uint8_t utype;       /**< stream user type ::aceAud_user_t */
    uint8_t ioport;      /**< stream io port */
    uint8_t data_header; /**< 0: without header, 1: with header */
    uint8_t sync_cnt;    /**< sync count */
    uint8_t mute;        /**< 0: un-mute, >= 1 mute */
    uint8_t eos;         /**< 0: not end, 1: end of stream */
} aceAudStream_syncC2P_t;

/**
 * @brief ACE audio stream AM to AC Sync basic type definition
 */
typedef struct {
    uint32_t rate;       /**< sample rate ::aceAud_ioRate_t */
    uint32_t layout;     /**< channel layout ::aceAud_ioPcmLayout_t */
    uint32_t bps;        /**< bits per second */
    uint32_t latency;    /**< latency in msec */
    uint32_t pts;        /**< presentation time stamp in msec */
    uint32_t samples;    /**< stream present samples */
    uint32_t bs_size;    /**< bit stream buffer size */
    uint8_t state;       /**< stream state ::aceAudStream_state_t */
    uint8_t sync_cnt;    /**< sync count */
    uint8_t reserved[2]; /**< reserved for 4bytes align */
} aceAudStream_syncP2C_t;

/**
 * @brief ace audio ipc user type definition
 */
typedef enum {
    ACE_AUD_IPC_INVALID_USER = 0,

    ACE_AUD_IPC_CLIENT = 1,      /**< It's client 1 user (MCU - default) */
    ACE_AUD_IPC_CLIENT_2,        /**< It's client 2 user (MCU 2) */
    ACE_AUD_IPC_CLIENT_3,        /**< It's client 3 user (MCU 3) */
    ACE_AUD_IPC_CLIENT_4,        /**< It's client 4 user (MCU 4) */
    ACE_AUD_IPC_CLIENT_MAX = 32, /**< Support upto 32 audio client user*/

    ACE_AUD_IPC_AUDMGR,   /**< It's AM user (MCU or DSP 1 - default) */
    ACE_AUD_IPC_AUDMGR_2, /**< It's AM 2 user (DSP 2) */
    ACE_AUD_IPC_AUDMGR_3, /**< It's AM 3 user (DSP 3) */
    ACE_AUD_IPC_AUDMGR_4, /**< It's AM 4 user (DSP 4) */
} aceAudIPC_user_t;

/** ACE Audio Proc ID type definition */
typedef enum {
    ACE_AUD_PROC_ID_INVALID = 0, /**< 0 is invalid proc id */

    /** 1 ~ 32767 reserved for user processor */
    ACE_AUD_PROC_ID_USER_BASE = 1,    /**< Proc user base id */
    ACE_AUD_PROC_ID_USER_MAX = 32767, /**< Proc user max id */

    /** 32768 ~ 65534 reserved for AC & AM IPC used only */
    ACE_AUD_PROC_ID_CORE_BASE = 32768, /**< Core reserved proc id */
    ACE_AUD_PROC_ID_CLIENT = ACE_AUD_PROC_ID_CORE_BASE + ACE_AUD_IPC_CLIENT,
    ACE_AUD_PROC_ID_AUDMGR = ACE_AUD_PROC_ID_CORE_BASE + ACE_AUD_IPC_AUDMGR,
    ACE_AUD_PROC_ID_ALL = UINT16_MAX /**< proc id for all proc */
} aceAudProc_id_t;

/**
 * @brief ACE audio buffer type definition
 */
typedef struct {
    intptr_t base; /**< the buffer base address */
    uint32_t size; /**< the buffer size */
} aceAud_buf_t;

/**
 * @brief ACE audio wait time type definition
 */
typedef enum {
    ACE_AUD_TIME_NO_WAIT = 0,          /**< no time wait */
    ACE_AUD_TIME_INFINITE = UINT32_MAX /**< wait done or error happen */
} aceAudTime_type_t;

/**
 * @brief ACE AM open attribute type definition
 */
typedef struct {
    aceAudIPC_user_t ac_user; /**< Audio Client user */
    aceAudIPC_user_t am_user; /**< Audio manager user */
} aceAudMgr_attr_t;

/**
 * @brief AM IPC open attribute type definition
 */
typedef struct {
    aceAudIPC_user_t src_user; /**< source user */
    aceAudIPC_user_t dst_user; /**< destination user */
    uint16_t cid;              /**< AC cid */
    uint16_t proc_id;          /**< AM Proc Id */
    uint16_t rid_val;          /**< rid_val, default is 0 */
} aceAudMgrIPC_attr_t;

#define ACE_AUD_VOL_Q (12) /**< stream vol gain fixed fmt Q4.12 */
#define ACE_AUD_VOL_0DB (1 << ACE_AUD_VOL_Q)    /**< 0dB volume */
#define ACE_AUD_SPEED_Q (12)                    /**< stream speed value Q4.12 */
#define ACE_AUD_SPEED_1X (1 << ACE_AUD_SPEED_Q) /**< 1.0 speed */

/**
 * @brief audio manager test proc plugin note type.
 * This plugin implement for basic AM unit test used.
 * md5("ACE_AUD_NOTE_PROC_TEST") = c324b325d32a0bcf7a3635e2466e635f
 */
#define ACE_AUD_NOTE_PROC_TEST (0xc324b325)

/**
 * @brief audio stream playback and capture plugin note type.
 * This plugin implement for audio stream playback and capture used.
 * md5("ACE_AUD_NOTE_PROC_TEST_STREAM")     = 8042568f9c65cdfcf704f2c79dee0cd0
 * md5("ACE_AUD_NOTE_PROC_STREAM_PLAYBACK") = 7be0640e478916b3c06c404e72fda107
 * md5("ACE_AUD_NOTE_PROC_STREAM_CAPTURE")  = 4cc58c01a8125d54dd1cb10254c03c6a
 */
#define ACE_AUD_NOTE_PROC_TEST_STREAM (0x8042568f)
#define ACE_AUD_NOTE_PROC_STREAM_PLAYBACK (0x7be0640e)
#define ACE_AUD_NOTE_PROC_STREAM_CAPTURE (0x4cc58c01)
/** @} */

/**
 * @defgroup ACE_HAL_AUDIO_MANAGER_API Public API
 * @{
 * @ingroup ACE_HAL_AUDIO_MANAGER
 */
/**
 * @brief ACE audio manager global init.
 * It's for platform init audio manager used, not for media apps usage.
 * Platform call this function before any audio apps.
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgr_init(void);

/**
 * @brief ACE audio manager global deinit.
 * It's for platform init audio manager used, not for media apps usage.
 * Platform call this function to exist audio manager.
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgr_deinit(void);

/**
 * @brief Open new AM handle for AC used.
 * @param attr [in] AM open attribute ::aceAudMgr_attr_t
 * @return the handle of AM
 * @retval NULL Open AM Fail.
 * @retval !NULL Open AM Success.
 */
aceAud_handle_t aceAudMgr_open(const aceAudMgr_attr_t* attr);

/**
 * @brief Close AM handle
 * @param hmgr [in] AM handle
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgr_close(aceAud_handle_t hmgr);

/**
 * @brief Open new msgQ between AC and AM Proc.
 * @param hmgr [in] AM handle
 * @param attr [in] IPC attribute
 * @return the handle of AM MsgQ IPC
 * @retval NULL fail.
 */
aceAud_handle_t aceAudMgrMsgQ_open(aceAud_handle_t hmgr,
                                   const aceAudMgrIPC_attr_t* attr);

/**
 * @brief Close AM MsgQ IPC handle
 * @param hmgr [in] AM handle
 * @param hmsg [in] MsgQ IPC handle
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrMsgQ_close(aceAud_handle_t hmgr, aceAud_handle_t hmsg);

/**
 * @brief Get MsgQ state
 * @param hmgr [in] AM handle
 * @param hmsg [in] MsgQ IPC handle
 * @param state [out] the state of MsgQ.
 *      = 0 : empty
 *      = UINT32_MAX : full
 *      = other value : message number
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrMsgQ_getState(aceAud_handle_t hmgr, aceAud_handle_t hmsg,
                                    uint32_t* state);

/**
 * @brief Post msg to AM Proc
 * @param hmgr [in] AM handle
 * @param hmsg [in] MsgQ IPC handle
 * @param pmsg [in] msg to be posted
 * @param time [in] time out in msec, or ::aceAudTime_type_t
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrMsgQ_post(aceAud_handle_t hmgr, aceAud_handle_t hmsg,
                                const void* pmsg, uint32_t time);

/**
 * @brief Take msg from AM Proc
 * @param hmgr [in] AM handle
 * @param hmsg [in] MsgQ IPC handle
 * @param pmsg [out] msg from AM Proc
 * @param time [in] time out in msec, or ::aceAudTime_type_t
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrMsgQ_take(aceAud_handle_t hmgr, aceAud_handle_t hmsg,
                                void* pmsg, uint32_t time);

/**
 * @brief Try touch msg from AM Proc
 * @param hmgr [in] AM handle
 * @param hmsg [in] MsgQ IPC handle
 * @param pmsg [out] msg from AM Proc
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrMsgQ_touch(aceAud_handle_t hmgr, aceAud_handle_t hmsg,
                                 void* pmsg);

/**
 * @brief Open new sync IPC between AC and AM Proc
 * @param hmgr [in] AM handle
 * @param attr [in] Sync IPC open attribute
 * @return The handle of sync IPC between AC and AM Proc
 * @retval !NULL, success.
 * @retval NULL, fail.
 */
aceAud_handle_t aceAudMgrSync_open(aceAud_handle_t hmgr,
                                   const aceAudMgrIPC_attr_t* attr);

/**
 * @brief Close Sync IPC
 * @param hmgr [in] AM handle
 * @param hsync [in] The sync IPC handle to be closed
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrSync_close(aceAud_handle_t hmgr, aceAud_handle_t hsync);

/**
 * @brief Get Sync buffer
 * @param hmgr [in] AM handle
 * @param hsync [in] The sync IPC handle
 * @param buf [out] the sync buffer
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrSync_getBuf(aceAud_handle_t hmgr, aceAud_handle_t hsync,
                                  aceAud_buf_t* buf);

/**
 * @brief Update Sync buffer
 * @param hmgr [in] AM handle
 * @param hsync [in] The sync IPC handle
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrSync_update(aceAud_handle_t hmgr, aceAud_handle_t hsync);

/**
 * @brief Open new data IPC between AC and AM Proc
 * @param hmgr [in] AM handle
 * @param attr [in] Data IPC open attribute
 * @return The handle of data IPC between AC and AM Proc
 * @retval !NULL, success.
 * @retval NULL, fail.
 */
aceAud_handle_t aceAudMgrData_open(aceAud_handle_t hmgr,
                                   const aceAudMgrIPC_attr_t* attr);

/**
 * @brief Close data IPC
 * @param hmgr [in] AM handle
 * @param hdata [in] The data IPC handle to be closed
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrData_close(aceAud_handle_t hmgr, aceAud_handle_t hdata);

/**
 * @brief Get current data valid size in bytes.
 * @param hmgr [in] AM handle
 * @param hdata [in] The data IPC handle
 * @return The valid data size. or error code.
 * @retval >=0, the valid data size.
 * @retval <0, error code ace_status_t
 */
int32_t aceAudMgrData_getValidSize(aceAud_handle_t hmgr, aceAud_handle_t hdata);

/**
 * @brief Get valid buffer
 * @param hmgr [in] AM handle
 * @param hdata [in] The data IPC handle
 * @param buf [out] the valid data buffer
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrData_getValidBuf(aceAud_handle_t hmgr,
                                       aceAud_handle_t hdata,
                                       aceAud_buf_t* buf);

/**
 * @brief Set buffer as used
 * @param hmgr [in] AM handle
 * @param hdata [in] The data IPC handle
 * @param buf [in] set the valid data buffer as used.
 * @return ace_status_t, return 0 if successful, or any < 0 is error code.
 */
ace_status_t aceAudMgrData_setUsedBuf(aceAud_handle_t hmgr,
                                      aceAud_handle_t hdata, aceAud_buf_t* buf);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ACE_SDK_HAL_AUDIO_MGR_H_ */
/** @} */
