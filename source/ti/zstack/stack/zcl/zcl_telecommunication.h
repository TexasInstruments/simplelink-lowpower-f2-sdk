/**************************************************************************************************
  Filename:       zcl_telecommunication.h
  Revised:        $Date: 2018-22-11 10:12:34 -0700 (Thur, 22 Nov 2018) $
  Revision:       $Revision: 25678 $

  Description:    Zigbee Cluster Library - Telecommunication Cluster


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#ifndef ZCL_TEL_H
#define ZCL_TEL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"


/*********************************************************************
 * CONSTANTS
 */

/**********************************************/
/*** Information Clusters Attributes List ***/
/**********************************************/
// Node Information attribute set
#define ATTRID_TEL_INFORMATION_NODE_DESCRIPTOR         0x0000
#define ATTRID_TEL_INFORMATION_DELIVERY_ENABLE         0x0001
#define ATTRID_TEL_INFORMATION_PUSH_INFORMATION_TIMER  0x0002
#define ATTRID_TEL_INFORMATION_ENABLE_SECURE_CONFIG    0x0003

// Content Information attribute set
#define ATTRID_TEL_INFORMATION_NUMBER_OF_CONTENTS      0x0010
#define ATTRID_TEL_INFORMATION_CONTENT_ROOT_ID         0x0011

/**********************************************/
/*** Chatting Clusters Attributes List      ***/
/**********************************************/
// User Related attribute set
#define ATTRID_TEL_CHATTING_U_ID                       0x0000
#define ATTRID_TEL_CHATTING_NICKNAME                   0x0001

// Chat Session Related attribute set
#define ATTRID_TEL_CHATTING_C_ID                       0x0010
#define ATTRID_TEL_CHATTING_NAME                       0x0011
#define ATTRID_TEL_CHATTING_ENABLE_ADD_CHAT            0x0012


/**********************************************/
/*** Voice Clusters Attributes List         ***/
/**********************************************/
// Voice Information attribute set
#define ATTRID_TEL_VOICE_CODEC_TYPE                    0x0000
#define ATTRID_TEL_VOICE_SAMPLING_FREQUENCY            0x0001
#define ATTRID_TEL_VOICE_CODEC_RATE                    0x0002
#define ATTRID_TEL_VOICE_ESTABLISHMENT_TIMEOUT         0x0003
#define ATTRID_TEL_VOICE_CODEC_TYPE_SUB1               0x0004
#define ATTRID_TEL_VOICE_CODEC_TYPE_SUB2               0x0005
#define ATTRID_TEL_VOICE_CODEC_TYPE_SUB3               0x0006
#define ATTRID_TEL_VOICE_COMPRESSION_TYPE              0x0007
#define ATTRID_TEL_VOICE_COMPRESSION_RATE              0x0008
#define ATTRID_TEL_VOICE_OPTION_FLAGS                  0x0009
#define ATTRID_TEL_VOICE_THRESHOLD                     0x000A


/***********************************/
/***Information Cluster Commands ***/
/***********************************/

// Commands Received by Information Cluster Server
#define COMMAND_TEL_INFORMATION_REQ_INFO                0x0000
#define COMMAND_TEL_INFORMATION_PUSH_INFO_RSP           0x0001
#define COMMAND_TEL_INFORMATION_SEND_PREF               0x0002
#define COMMAND_TEL_INFORMATION_REQ_PREF_RSP            0x0003
#define COMMAND_TEL_INFORMATION_UPDATE                  0x0004
#define COMMAND_TEL_INFORMATION_DELETE                  0x0005
#define COMMAND_TEL_INFORMATION_CONF_NODE_DESC          0x0006
#define COMMAND_TEL_INFORMATION_CONF_DELIVERY_ENABLE    0x0007
#define COMMAND_TEL_INFORMATION_CONF_PUSH_INFO_TIMER    0x0008
#define COMMAND_TEL_INFORMATION_CONF_SET_ROOT_ID        0x0009

// Commands generated by Information Cluster Server
#define COMMAND_TEL_INFORMATION_REQ_INFO_RSP            0x0000
#define COMMAND_TEL_INFORMATION_PUSH_INFO               0x0001
#define COMMAND_TEL_INFORMATION_SEND_PREF_RSP           0x0002
#define COMMAND_TEL_INFORMATION_SERVER_REQ_PREF         0x0003
#define COMMAND_TEL_INFORMATION_REQ_PREF_CONFIR         0x0004
#define COMMAND_TEL_INFORMATION_UPDATE_RSP              0x0005
#define COMMAND_TEL_INFORMATION_DELETE_RSP              0x0006

/***********************************/
/***  Chatting Cluster Commands  ***/
/***********************************/

// Commands Received by Chatting Cluster Server
#define COMMAND_TEL_CHATTING_JOIN_CHAT_REQ              0x0000
#define COMMAND_TEL_CHATTING_LEAVE_CHAT_REQ             0x0001
#define COMMAND_TEL_CHATTING_SEARCH_CHAT_REQ            0x0002
#define COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_RSP        0x0003
#define COMMAND_TEL_CHATTING_START_CHAT_REQ             0x0004
#define COMMAND_TEL_CHATTING_CHAT_MESSAGE               0x0005
#define COMMAND_TEL_CHATTING_GET_NODE_INFO_REQ          0x0006

// Commands generated by Chatting Cluster Server
#define COMMAND_TEL_CHATTING_START_CHAT_RSP                 0x0000
#define COMMAND_TEL_CHATTING_JOIN_CHAT_RSP                  0x0001
#define COMMAND_TEL_CHATTING_USER_LEFT                      0x0002
#define COMMAND_TEL_CHATTING_USER_JOINED                    0x0003
#define COMMAND_TEL_CHATTING_SEARCH_CHAT_RSP                0x0004
#define COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_REQ            0x0005
#define COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_CONFIRM        0x0006
#define COMMAND_TEL_CHATTING_SWITCH_CHAIRMAN_NOTIFICATION   0x0007
#define COMMAND_TEL_CHATTING_GET_NODE_INFO_RSP              0x0008

/***********************************/
/***    Voice Cluster Commands   ***/
/***********************************/

// Commands Received by Voice Over ZigBee Cluster Server
#define COMMAND_TEL_VOICE_ESTABLISHMENT_REQ              0x0000
#define COMMAND_TEL_VOICE_VOICE_TRANSMISSION             0x0001
#define COMMAND_TEL_VOICE_VOICE_TRANSMISSION_COMPLETION  0x0002
#define COMMAND_TEL_VOICE_CONTROL_RSP                    0x0003

// Commands generated by Voice Over ZigBee Cluster Server
#define COMMAND_TEL_VOICE_VOICE_TRANSMISSION_RSP         0x0000
#define COMMAND_TEL_VOICE_ESTABLISHMENT_RSP              0x0001
#define COMMAND_TEL_VOICE_CONTROL                        0x0002


/******************************************************************/
/***        Other Constants                                     ***/
/******************************************************************/




/********************************************************************
 * MACROS
 */

typedef enum
{
  TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_CONTENT_ID,
  TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_MULTIPLE_ID,
  TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_ALL,
  TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_DEPTH
}Telecommunications_Information_Inquiry_t;



#define TELECOMMUNICATIONS_INFORMATION_DATA_TYPE_TITLE       0x01
#define TELECOMMUNICATIONS_INFORMATION_DATA_TYPE_OCTECT_STR  0x02
#define TELECOMMUNICATIONS_INFORMATION_DATA_TYPE_CHAR_STR    0x04
#define TELECOMMUNICATIONS_INFORMATION_DATA_TYPE_RSS_FEED    0x08


#define TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_CONTENT_ID  0x0000
#define TELECOMMUNICATIONS_INFORMATION_PREFERENCE_TYPE_MULTIPLE_OCTECT_STR  0x0001


#define TELECOMMUNICATIONS_INFORMATION_UPDATE_ACCESS_CTRL_FREE_TO_ACCESS      0x00
#define TELECOMMUNICATIONS_INFORMATION_UPDATE_ACCESS_CTRL_LINK_KEY_EST_BASED  0x01
#define TELECOMMUNICATIONS_INFORMATION_UPDATE_ACCESS_CTRL_BILLING_BASED       0x02

#define TELECOMMUNICATIONS_INFORMATION_UPDATE_OPT_FORWARD_BIT                 0x01
#define TELECOMMUNICATIONS_INFORMATION_UPDATE_OPT_REDIRECT_BIT                0x02
#define TELECOMMUNICATIONS_INFORMATION_UPDATE_OPT_OVERWRITE_UPDATE_BIT        0x04

#define TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S1_BIT                            0x01
#define TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S2_BIT                            0x02
#define TELECOMMUNICATIONS_VOICE_CODEC_TYPE_S3_BIT                            0x04
#define TELECOMMUNICATIONS_VOICE_COMPRESSION_BIT                              0x08
/*********************************************************************
 * TYPEDEFS
 */

/*** Structures used for callback functions ***/


/**
 * @brief Information Cluster - Request Information payload format when Inquiry ID is TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_MULTIPLE_ID
 */
typedef struct
{
uint8_t   NumContent;    //!<Number of contents
uint16_t* ContentIDs;    //!<Pointer to memory with the contents to be requested (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_MultipleContent_t;

/**
 * @brief Information Cluster - Request Information payload format when Inquiry ID is TELECOMMUNICATIONS_INFORMATION_INQUIRY_ID_REQUEST_BY_DEPTH
 */
typedef struct
{
uint16_t  StartId;    //!<Holds content ID for starting point to retrieve structured contents
uint8_t   Depth;      //!<Depth field holds how many levels to request from Start ID tracing child
                    //!<information. If a depth equals to 0x00, the requested content should
                    //!<be single content of Start ID itself.
}zcl_Tel_Info_ReqByDepth_t;


/**
 * @brief Information Cluster - Request Information payload format
 */
typedef struct
{
uint8_t  InquiryId;                //!<Inquiry Id, see #Telecommunications_Information_Inquiry_t
uint8_t  DataType;                 //!<Data Type ID indicates what type of contents the response command requires.
  union {
    uint16_t                          ReqAContentByContentId; //!<Content Id to be requested
    zcl_Tel_Info_MultipleContent_t  MultipleContent; //!<MultipleContent parameters
    uint8_t                           ReqAll;          //!<Placeholder, not needed when InquiryId is ReqAll
    zcl_Tel_Info_ReqByDepth_t       ReqByDepth;      //!<Request By Depth parameters
  } RequestInfoPayload;
}zcl_Tel_Info_RequestInfo_t;




/**
 * @brief Information Cluster - Request Information payload format
 */
typedef struct
{
uint16_t ContentId;           //!<Content ID
uint8_t  StatusFeedback;      //!<Status Freedback
}zcl_Tel_Info_Notification_t;

/**
 * @brief Information Cluster - Push Notification Response payload format
 */
typedef struct
{
uint8_t                         NumNotifications;   //!<Number of notifications
zcl_Tel_Info_Notification_t*  Notifications;      //!<Pointer to the to memory with the notifications (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_PushInformationRsp_t;


/**
 * @brief Information Cluster - Send Preference payload format
 */
typedef struct
{
uint16_t         PreferenceType;   //!<The Preference Type determines the format of the preference Payload. All devices must support Preference Type of 0x0000.
uint8_t          PreferenceCount;
  union {
    uint16_t*                         PrefMultipleContentId; //!<Pointer to the to memory with the content IDs (this memory must be release by app after calling sending cmd API)
    uint8_t*                          PrefMultipleOctectStr; //!<Pointer to the to memory with the octect strings (this memory must be release by app after calling sending cmd API)
  } PreferencePayload;
}zcl_Tel_Info_SendPreference_t;



/**
 * @brief Information Cluster - Request Preference Response payload format
 */
typedef struct
{
uint8_t          StatusFeedback;   //!<Status Freedback
uint16_t         PreferenceType;   //!<The Preference Type determines the format of the preference Payload. All devices must support Preference Type of 0x0000.
uint8_t          PreferenceCount;
  union {
    uint16_t*                         PrefMultipleContentId; //!<Pointer to the to memory with the content IDs (this memory must be release by app after calling sending cmd API)
    uint8_t*                          PrefMultipleOctectStr; //!<Pointer to the to memory with the octect strings (this memory must be release by app after calling sending cmd API)
  } PreferencePayload;
}zcl_Tel_Info_ReqPreferenceRsp_t;


typedef struct
{
uint16_t  ContentID;         //<! Content ID corresponds to the content.
uint8_t   DataTypeID;        //<! Data Type indicates the supported data types of content
uint8_t*  TitleString;       //<! Pointer to the to memory with long Character String data type defined in ZCL (this memory must be release by app after calling sending cmd API)
uint8_t*  ContentString;
uint8_t   NumberOfChildren;  //<! Indicates how many links to child-contents this content has
uint16_t* ChildContentId;    //<! Pointer to the to memory with the list of child conent (this memory must be release by app after calling sending cmd API)
}SingleContent_t;


typedef struct
{
uint8_t                    NumberSingleContent;  //<! Number field holds a number of single contents
SingleContent_t*         SingleContent;        //<! List of content
}MultipleContent_t;

/**
 * @brief Information Cluster - Update command payload format
 */
typedef struct
{
uint8_t                     AccessControl;   //<! Access Control Field is 8-bit enumeration and is used to indicate security level for the validation to access the contents which are carried by the Update command
uint8_t                     OptionField;     //<! ption Field is used for advanced indication while updating contents: Forward Flag,Redirection, Overwrite update.
MultipleContent_t         MultipleContent;
}zcl_Tel_Info_Update_t;

/**
 * @brief Information Cluster - Delete command payload format
 */
typedef struct
{
uint8_t   DeletionOption;   //<! Deteletion options. If Recursive (bit1) is set, all subtree content is deleted, otherwise only content stated is deleted.
uint8_t   NumberContentIDs; //<! Number of entries in the ContentIDs parameter.
uint16_t* ContentIDs;       //<! Pointer to the to memory with the list of conent to be erased (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_Delete_t;


/**
 * @brief Information Cluster - Request Information Response payload format
 */
typedef struct
{
uint8_t                    NumberSingleContent;  //<! Number field holds a number of single contents
uint8_t                    StatusFeedback;       //<! carries a message as a response to the previous 'Request Information' command sent by the client
SingleContent_t          SingleContent;
}zcl_Tel_Info_ReqInfoRsp_t;

/**
 * @brief Information Cluster - Push Information payload format
 */
typedef struct
{
MultipleContent_t         MultipleContent;
}zcl_Tel_Info_PushInfo_t;

/**
 * @brief Information Cluster - Send Preference Response payload format
 */
typedef struct
{
uint8_t    NumberStatus;   //<! Number field holds a number of single contents
uint8_t*   StatusFeedback; //<! Pointer to the to memory with the List of status Feedback carries a message as a response to the previous command 'Send Preference' command from the client (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_PreferenceRsp_t;

/**
 * @brief Information Cluster - Send Preference Confirm payload format
 */
typedef struct
{
uint8_t    NumberStatus;   //<! Number field holds a number of single contents
uint8_t*   StatusFeedback; //<! Pointer to the to memory with the List of status Feedback carries a message as a response to the previous command 'Send Preference' command from the client (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_PreferenceConfirm_t;


/**
 * @brief Information Cluster - Update Response payload format
 */
typedef struct
{
uint8_t                         NumNotifications;   //!<Number of notifications
zcl_Tel_Info_Notification_t*  Notifications;      //!<Pointer to the to memory with the notifications (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_UpdateRsp_t;

/**
 * @brief Information Cluster - Update Response payload format
 */
typedef struct
{
uint8_t                         NumNotifications;   //!<Number of notifications
zcl_Tel_Info_Notification_t*  Notifications;      //!<Pointer to the to memory with the notifications (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Info_DeleteRsp_t;

/**
 * @brief Chatting Cluster - Join Request payload format
 */
typedef struct
{
uint16_t  U_ID;     //<!Indicates unique identification of the user in the chat room
uint8_t*  Nickname; //<!This is type of character string which is a unique display name of the user while talking in the public chat room.
uint16_t  C_ID;     //<!The C_ID field is unique identification of a chat room. It indicates the ID of the chat room which the client wants to join
}zcl_Tel_Chat_JoinReq_t;

/**
 * @brief Chatting Cluster - Leave Chat Request payload format
 */
typedef struct
{
uint16_t  C_ID;     //<!The C_ID field is unique identification of a chat room. It indicates the ID of the chat room which the client wants to join
uint16_t  U_ID;     //<!Indicates unique identification of the user in the chat room
}zcl_Tel_Chat_LeaveChatReq_t;

/**
 * @brief Chatting Cluster - Switch Chairman Rsp payload format
 */
typedef struct
{
uint16_t  C_ID;     //<!The C_ID field is unique identification of a chat room. It indicates the ID of the chat room which the client wants to join
uint16_t  U_ID;     //<!Indicates unique identification of the user in the chat room
}zcl_Tel_Chat_SwitchChairmanRsp_t;


/**
 * @brief Chatting Cluster - Start Chat Request payload format
 */
typedef struct
{
uint8_t*  Name;     //<!The Name field indicates the topic of the chat room.
uint16_t  U_ID;     //<!Indicates unique identification of the user in the chat room
uint8_t*  Nickname; //<!The Nickname field indicates the Nickname set by the requester.
}zcl_Tel_Chat_StartChatReq_t;


/**
 * @brief Chatting Cluster - Chat Message payload format
 */
typedef struct
{
uint16_t  Dest_U_ID; //<! Indicates the destination node's U_ID
uint16_t  Src_U_ID;  //<! Indicates the source node's U_ID
uint16_t  C_ID;      //<! ID of the chat room which the sender belongs to
uint8_t*  Nickname;  //<! Indicates the sender's Nickname, which shall be in Character string data type
uint8_t*  Message;   //<! Message to be send in Character String data type.
}zcl_Tel_Chat_ChatMessage_t;

/**
 * @brief Chatting Cluster - Get Node Information Request payload format
 */
typedef struct
{
uint16_t  U_ID;  //<! indicates the U_ID of the node to be investigated.
uint16_t  C_ID;  //<! Indicates the ID of the chat room which the investigated node belongs to.
}zcl_Tel_Chat_GetNodeInfoReq_t;


/**
 * @brief Chatting Cluster - Start Chat Response payload format
 */
typedef struct
{
uint8_t   status; //<! Status field indicates the status of the previous request. If success, C_ID must be provided.
uint16_t  C_ID;   //<! Indicates the ID of the chat room which the investigated node belongs to.
}zcl_Tel_Chat_StartChatRsp_t;


typedef struct
{
uint16_t  U_ID;        //<! User ID
uint8_t*  Nickname;    //<! Pointer to memory with the nickname of the user (this memory must be release by app after calling sending cmd API)
}ChatUser_t;

/**
 * @brief Chatting Cluster - Start Join Response payload format
 */
typedef struct
{
uint8_t   status; //<! Status field indicates the status of the previous request. If success, C_ID must be provided.
uint16_t  C_ID;   //<! Indicates the ID of the chat room joined if successful.
uint8_t   numberChatUsers;  //<! Indicate the number of users in the chat list provided.
ChatUser_t *ChatUsers;    //<! Pointer to the to memory with the list of users in the chat (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Chat_JoinChatRsp_t;

/**
 * @brief Chatting Cluster - Start User Left payload format
 */
typedef struct
{
uint8_t      didUserLeft; //<! Set to true to send UserLeft Cmd, false to send UserJoin Cmd
uint16_t     C_ID;        //<! Indicates the ID of the chat room which the investigated node belongs to.
ChatUser_t ChatUser;    //<! Chat User information
}zcl_Tel_Chat_UserLeftJoined_t;


typedef struct
{
uint16_t   C_ID;     //<! Chat ID
uint8_t*   Name;     //<! Name of the chat
}ChatRoom_t;


/**
 * @brief Chatting Cluster - Switch Chairman Request payload format
 */
typedef struct
{
uint8_t   options;          //<! Bit 0 of the Options field indicates whether the server permits other users to add new chat rooms in it. The value 0b0 means permit while 0b1 means not permit
uint8_t   numberChatRooms;  //<! Indicate the number of chat rooms in the list provided.
ChatRoom_t *ChatRoom;     //<! Pointer to the to memory with the list of chat rooms in the chat (this memory must be release by app after calling sending cmd API)
}zcl_Tel_Chat_SearchChatRsp_t;


typedef struct
{
uint16_t   U_ID;       //<! Unique ID of the node in the chat
uint16_t   ShortAddr;  //<! Short address of the node
uint8_t    endpoint;   //<! Endpoint of the node
uint8_t*   Nickname;   //<! Nickname of the node in the chat
}NodeInfo_t;

/**
 * @brief Chatting Cluster - Switch Chairman Confirm payload format
 */
typedef struct
{
uint16_t       C_ID;           //<! ID of the chat room
uint8_t        numberNodeInfo; //<! Number of nodes participating in the chat
NodeInfo_t*  NodeInfo;       //<! Information of the nodes participating in the chat.
}zcl_Tel_Chat_SwitchChairmanCnf_t;


/**
 * @brief Chatting Cluster - Switch Chairman Notification payload format
 */
typedef struct
{
uint16_t  C_ID;      //<! The C_ID field is the ID of the chat room which the chairman manages
uint16_t  U_ID;      //<! The U_ID field is the unique ID of the node which is the new chairman of the chat room
uint16_t  ShortAddr; //<! Short address of the new chairman
uint8_t   endpoint;  //<! Endpoint of the new chairman
}zcl_Tel_Chat_SwitchChairmanNotification_t;



/**
 * @brief Chatting Cluster - Get Node Information Response payload format
 */
typedef struct
{
uint8_t   status;    //<! Indicates the status of the previous request
uint16_t  C_ID;      //<! ID of the chat room in which the node is
uint16_t  U_ID;      //<! Unique ID of the node
uint16_t  shortAddr; //<! Short address of the node
uint8_t   endpoint;  //<! Endpoint of the node
uint8_t*  Nickname;  //<! Nickname of the node in the chat
}zcl_Tel_Chat_GetNodeInfoRsp_t;

/**
 * @brief Voice Cluster - Establishment Request payload format
 */
typedef struct
{
uint8_t   Flag;  //<! BitMap to indicate the presence of the following fields. Bit0 - CodecTypeS1, Bit1 - CodecTypeS2, Bit2 - CodecTypeS3, Bit3 - Compresion fields
uint8_t   CodecType;
uint8_t   SampleFreq;
uint8_t   CodecRate;
uint8_t   ServiceType;
uint8_t   CodecTypeS1;
uint8_t   CodecTypeS2;
uint8_t   CodecTypeS3;
uint8_t   CompType;
uint8_t   CompRate;
}zcl_Tel_Voice_GetNodeInfoRsp_t;


/*********************************************************************
 * CALLBACKS
 */

/*********************************************/
/***Information Cluster Commands Callbacks ***/
/*********************************************/

/// This callback is called to process an incomming Request Information Command
typedef ZStatus_t (*zclTel_RequestInformation_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Request Information Response Command
typedef ZStatus_t (*zclTel_RequestInformationRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Push Information Command
typedef ZStatus_t (*zclTel_PushInformation_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Push Information Response Command
typedef ZStatus_t (*zclTel_PushInformationRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Send Preference Command
typedef ZStatus_t (*zclTel_SendPreference_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Send Preference Response Command
typedef ZStatus_t (*zclTel_SendPreferenceRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Request Preference Confirmation Command
typedef ZStatus_t (*zclTel_RequestPreferenceConfirmation_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Server Request Preference Command
typedef ZStatus_t (*zclTel_ServerRequestPreference_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Request Preference Response Command
typedef ZStatus_t (*zclTel_RequestPreferenceRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Update Command
typedef ZStatus_t (*zclTel_Update_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Update Response Command
typedef ZStatus_t (*zclTel_UpdateRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Delete Command
typedef ZStatus_t (*zclTel_Delete_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Delete Response Command
typedef ZStatus_t (*zclTel_DeleteRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Configure Node Descriptor Command
typedef ZStatus_t (*zclTel_ConfigureNodeDescription_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Configure Delivery Enable Command
typedef ZStatus_t (*zclTel_ConfigureDeliveryEnable_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Configure Push Information Timer Command
typedef ZStatus_t (*zclTel_ConfigurePushInformationTimer_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Configure Set Root ID Command
typedef ZStatus_t (*zclTel_ConfigureSetRootID_t)( zclIncoming_t *pInMsg );

/******************************************/
/***Chatting Cluster Commands Callbacks ***/
/******************************************/

/// This callback is called to process an incomming Join Chat Command
typedef ZStatus_t (*zclTel_JoinChatReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Join Chat Response Command
typedef ZStatus_t (*zclTel_JoinChatRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Leave Chat Command
typedef ZStatus_t (*zclTel_LeaveChatReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Search Chat Request Command
typedef ZStatus_t (*zclTel_SearchChatReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Search Chat Response Command
typedef ZStatus_t (*zclTel_SearchChatRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Switch Chairman Request Command
typedef ZStatus_t (*zclTel_SwitchChairmanReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Switch Chairman Response Command
typedef ZStatus_t (*zclTel_SwitchChairmanRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Switch Chairman Confirmation Command
typedef ZStatus_t (*zclTel_SwitchChairmanConfirm_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Switch Chairman Notification Command
typedef ZStatus_t (*zclTel_SwitchChairmanNotification_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Start Chat Request Command
typedef ZStatus_t (*zclTel_StartChatReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Start Chat Response Command
typedef ZStatus_t (*zclTel_StartChatRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Chat Message Command
typedef ZStatus_t (*zclTel_ChatMessage_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Get Node Information Request Command
typedef ZStatus_t (*zclTel_GetNodeInformationReq_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Get Node Information Response Command
typedef ZStatus_t (*zclTel_GetNodeInformationRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming User Left Command
typedef ZStatus_t (*zclTel_UserLeft_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming User Joined Command
typedef ZStatus_t (*zclTel_UserJoined_t)( zclIncoming_t *pInMsg );

/******************************************/
/***Voice Cluster Commands Callbacks ***/
/******************************************/

/// This callback is called to process an incomming Establishment Request Command
typedef ZStatus_t (*zclTel_EstablishmentRequest_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Establishment Response Command
typedef ZStatus_t (*zclTel_EstablishmentRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Voice Transmission Command
typedef ZStatus_t (*zclTel_VoiceTransmission_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Voice Transmission Response Command
typedef ZStatus_t (*zclTel_VoiceTransmissionRsp_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Voice Transmission Completion
typedef ZStatus_t (*zclTel_VoiceTransmissionCompletion_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Control Command
typedef ZStatus_t (*zclTel_Control_t)( zclIncoming_t *pInMsg );

/// This callback is called to process an incomming Control Response Command
typedef ZStatus_t (*zclTel_ControlRsp_t)( zclIncoming_t *pInMsg );


// Register Callbacks table entry - enter function pointers for callbacks that
// the application would like to receive


/**
 * @brief Register Callbacks Information Cluster table entry - enter function pointers for callbacks that
 * the application would like to receive
 */
typedef struct
{
  zclTel_RequestInformation_t               pfnRequestInformation;
  zclTel_RequestInformationRsp_t            pfnRequestInformationRsp;
  zclTel_PushInformation_t                  pfnPushInformation;
  zclTel_PushInformationRsp_t               pfnPushInformationRsp;
  zclTel_SendPreference_t                   pfnSendPreference;
  zclTel_SendPreferenceRsp_t                pfnSendPreferenceRsp;
  zclTel_RequestPreferenceConfirmation_t    pfnRequestPreferenceConfirmation;
  zclTel_ServerRequestPreference_t          pfnServerRequestPreference;
  zclTel_RequestPreferenceRsp_t             pfnRequestPreferenceRsp;
  zclTel_Update_t                           pfnUpdate;
  zclTel_UpdateRsp_t                        pfnUpdateRsp;
  zclTel_Delete_t                           pfnDelete;
  zclTel_DeleteRsp_t                        pfnDeleteRsp;
  zclTel_ConfigureNodeDescription_t         pfnConfigureNodeDescription;
  zclTel_ConfigureDeliveryEnable_t          pfnConfigureDeliveryEnable;
  zclTel_ConfigurePushInformationTimer_t    pfnConfigurePushInformationTimer;
  zclTel_ConfigureSetRootID_t               pfnConfigureSetRootID;
} zclTel_InformationAppCallbacks_t;

/**
 * @brief Register Callbacks Chatting Cluster table entry - enter function pointers for callbacks that
 * the application would like to receive
 */
typedef struct
{
  zclTel_JoinChatReq_t                pfnJoinChatReq;
  zclTel_JoinChatRsp_t                pfnJoinChatRsp;
  zclTel_LeaveChatReq_t               pfnLeaveChatReq;
  zclTel_SearchChatReq_t              pfnSearchChatReq;
  zclTel_SearchChatRsp_t              pfnSearchChatRsp;
  zclTel_SwitchChairmanReq_t          pfnSwitchChairmanReq;
  zclTel_SwitchChairmanRsp_t          pfnSwitchChairmanRsp;
  zclTel_SwitchChairmanConfirm_t      pfnSwitchChairmanConfirm;
  zclTel_SwitchChairmanNotification_t pfnSwitchChairmanNotification;
  zclTel_StartChatReq_t               pfnStartChatReq;
  zclTel_StartChatRsp_t               pfnStartChatRsp;
  zclTel_ChatMessage_t                pfnChatMessage;
  zclTel_GetNodeInformationReq_t      pfnGetNodeInformationReq;
  zclTel_GetNodeInformationRsp_t      pfnGetNodeInformationRsp;
  zclTel_UserLeft_t                   pfnUserLeft;
  zclTel_UserJoined_t                 pfnUserJoined;
} zclTel_ChattingAppCallbacks_t;

/**
 * @brief Register Voice Over Zigbee Cluster table entry - enter function pointers for callbacks that
 * the application would like to receive
 */
typedef struct
{
  zclTel_EstablishmentRequest_t         pfnEstablishmentRequest;
  zclTel_EstablishmentRsp_t             pfnEstablishmentRsp;
  zclTel_VoiceTransmission_t            pfnVoiceTransmission;
  zclTel_VoiceTransmissionRsp_t         pfnVoiceTransmissionRsp;
  zclTel_VoiceTransmissionCompletion_t  pfnVoiceTransmissionCompletion;
  zclTel_Control_t                      pfnControl;
  zclTel_ControlRsp_t                   pfnControlRsp;
} zclTel_VoiceAppCallbacks_t;

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTION MACROS
 */


/*********************************************************************
 * FUNCTIONS
 */

#ifdef ZCL_INFORMATION
/*
 * Register for callbacks for Information cluster library
 */
extern ZStatus_t zclTel_RegisterInformationCmdCallbacks( uint8_t endpoint, zclTel_InformationAppCallbacks_t *callbacks );

/*
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Information structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendRequestInformation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_RequestInfo_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Push Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendPushInformationRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PushInformationRsp_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Push Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendPreference( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_SendPreference_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Preference Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendReqPreferenceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_ReqPreferenceRsp_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendUpdate( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_Update_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Delete command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pDescription - Description to update the node descriptor attribute of remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendConfigureNodeDescription( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t *pDescription,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Configure Delivery Enable command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   EnableFlag - Flag to set the Delivery Enable attribute on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendConfigureDeliveryEnable( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t EnableFlag, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Configure Push Information Timer command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   Timer - Time to set the Push Information Timer attribute on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendConfigurePushInformationTimer( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint32_t Timer, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Configure Set Root ID command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   RootID - RootID attribute to set on the remote device.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendConfigureSetRootID( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint16_t RootID, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Request Information Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Request Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendRequestInformationResponse( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_ReqInfoRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Push Information command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to push Information structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendPushInformation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PushInfo_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum );

/*
 * @brief   Call to send out a Send Preference Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Send Preference Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendPreferenceRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PreferenceRsp_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum );

/*
 * @brief   Call to send out a Send Server Request Preference command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendServerRequestPreference( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Preference Confirmation command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Send Preference Confirmation structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendPreferenceConfirmation( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_PreferenceConfirm_t* pCmd, uint8_t disableDefaultRsp,
                                    uint8_t seqNum );

/*
 * @brief   Call to send out a Send Delete Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Update Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendUpdateRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_UpdateRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*********************************************************************
 * @fn      zclTel_InfoSendDelete
 *
 * @brief   Call to send out a Send Delete command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to delete structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

ZStatus_t zclTel_InfoSendDelete( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_Delete_t *pCmd,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Delete Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Delete Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_InfoSendDeleteRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Info_DeleteRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

#endif

#ifdef ZCL_CHATTING
/*
 * Register for callbacks for Chatting cluster library
 */
extern ZStatus_t zclTel_RegisterChattingCmdCallbacks( uint8_t endpoint, zclTel_ChattingAppCallbacks_t *callbacks );

/*
 * @brief   Call to send out a Send Join Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Join Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatJoinChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_JoinReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Leave Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Leave Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatLeaveChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_LeaveChatReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Search Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Leave Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSearchChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                   uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Switch Chairman Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSwitchChairmanRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Start Chat Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */

extern ZStatus_t zclTel_ChatStartChatReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_StartChatReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Chat Message command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Chat Message structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatChatMessage( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_ChatMessage_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Get Node Information Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Get Node Information Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatGetNodeInfoReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_GetNodeInfoReq_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Start Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatStartChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_StartChatRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Join Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Start Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatJoinChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_JoinChatRsp_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send User Left or User Joined command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to User Left or Join structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatUserLeftJoined( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_UserLeftJoined_t* pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Search Chat Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Search Chat Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSearchChatRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SearchChatRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Switch Chairman Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   C_ID - ID of the chat room where the chairman is requested to be changed
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSwitchChairmanReq( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint16_t C_ID, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Switch Chairman Confirm command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Confirm structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSwitchChairmanCnf( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanCnf_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Switch Chairman Notification command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Switch Chairman Notification structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatSwitchChairmanNotification( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_SwitchChairmanNotification_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Get Node Information Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Get Node Information Response structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_ChatGetNodeInfoRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Chat_GetNodeInfoRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );
#endif

#ifdef ZCL_VOICE
/*
 * Register for callbacks for Voice cluster library
 */
extern ZStatus_t zclTel_RegisterVoiceCmdCallbacks( uint8_t endpoint, zclTel_VoiceAppCallbacks_t *callbacks );

/*
 * @brief   Call to send out a Send Establishment Request command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pCmd - pointer to Establishment Request structure
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceEstablishmentRequest( uint8_t srcEP, afAddrType_t *dstAddr,
                                    zcl_Tel_Voice_GetNodeInfoRsp_t *pCmd, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Voice Transmission command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   pVoiceData - pointer voice data
 * @param   voiceDataLen - Length of the voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceTransmission( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t* pVoiceData, uint16_t voiceDataLen, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Voice Transmission Completion command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceTransmissionCompletion( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send ControlRsp command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ACK_NACK - pointer voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceControlRsp( uint8_t srcEP, afAddrType_t *dstAddr, uint8_t ACK_NACK,
                                    uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Voice Transmission Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   zclSeqNumFailed - ZCL Sequence Number of the frame failed.
 * @param   ErrorFlag - Error reason: 0x00 Failure to decode voice data, 0x01 Wrong order of voice data
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceTransmissionRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t zclSeqNumFailed, uint8_t ErrorFlag, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Voice Establishment Response command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ACK_NACK - ACK=0x01 NAK=0x00. Set to 0, if codec type not supported and provide codectype supported in field CodecType.
 * @param   CodecType - Codec Type supported. Only used if ACK_NACK is set to 0.
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceEstablishmentRsp( uint8_t srcEP, afAddrType_t *dstAddr,
                                    uint8_t ACK_NACK, uint8_t CodecType, uint8_t disableDefaultRsp, uint8_t seqNum );

/*
 * @brief   Call to send out a Send Control command
 *
 * @param   srcEP - Sending application's endpoint
 * @param   dstAddr - where you want the message to go
 * @param   ControlType -  indicates the control options, including the play operation (0x01), the stop operation (0x02), and the disconnection operation (0x03)
 * @param   disableDefaultRsp - disable default response
 * @param   seqNum - ZCL sequence number
 *
 * @return  ZStatus_t
 */
extern ZStatus_t zclTel_VoiceControl( uint8_t srcEP, afAddrType_t *dstAddr,
                                     uint8_t ControlType, uint8_t disableDefaultRsp, uint8_t seqNum );

#endif


#ifdef __cplusplus
}
#endif

#endif /* ZCL_TEL_H */
