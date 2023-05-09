/******************************************************************************

 @file ttoad-server.c

 @brief.TI Openthread OAD server

 Group: CMCU, LPRF
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2023, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/inotify.h>

#include "coap_config.h"
#include "resource.h"
#include "coap.h"

#define COAP_RESOURCE_CHECK_TIME 2

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define OAD_TEST
#ifdef OAD_TEST
#include "coap_list.h"

#define OAD_BLOCK_SIZE  128
#define IMG_HDR_ADDR    0

#define OAD_IMG_ID_SIZE   8
#define OAD_SW_VER_SIZE   4
#define OAD_BIM_VER_SIZE  2

/* application types */
#define CLIFTD            1
#define CLIMTD            2
#define DOORLOCK          3
#define NCPFTD            4
#define SHADE             5
#define TEMPSENSOR        6
#define THERMOSTAT        7

/* platform types */
#define CC26x2R1          1
#define CC13x2R1          2
#define CC13x2P1          3
#define CC13x2P2          4
#define CC13x2P4          5
#define CC26x2RB          6
#define OAD_MAX_PLATFORMS 7

/* internal use for test */
#define DONT_CARE         0xFE
/* undefined type */
#define UNKNOWN           0xFF

/* platform type values deviating from CC26X2R1 and CC1352R1 */
#define CC2652RB_PLATFORM_VALUE                 4
#define CC1352P_2_PLATFORM_VALUE                14

#define MAX_FILE_NAME_SIZE                      256
#define MAX_DIR_NAME_SIZE                       4096
#define IPV6_ADDR_SIZE                          16
#define FWVERSTR_SIZE                           16
#define IMG_ID_SIZE                             8
#define SW_VER_SIZE                             4
#define MAX_CMD_STR_SIZE                        100
#define FILE_EVENT_BUF_SIZE                     1024
#define COAP_TEMP_STR_SIZE                      40
#define OAD_NTF_ACK_WAIT_TIME                   2 //2seconds

typedef struct __attribute__((__packed__))
{
  uint8_t   imgID[8];       //!< User-defined Image Identification bytes. */
  uint32_t  crc32;          //!< Image's crc32 value */
  uint8_t   bimVer;         //!< BIM version */
  uint8_t   metaVer;        //!< Metadata version */
  uint16_t  techType;       //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
  uint8_t   imgCpStat;      //!< Image copy status */
  uint8_t   crcStat;        //!< CRC status */
  uint8_t   imgType;        //!< Image Type */
  uint8_t   imgNo;          //!< Image number of 'image type' */
  uint32_t  imgVld;         //!< In indicates if the current image in valid 0xff - valid, 0x00 invalid image */
  uint32_t  len;            //!< Image length in bytes. */
  uint32_t  prgEntry;       //!< Program entry address */
  uint8_t   softVer[4];     //!< Software version of the image */
  uint32_t  imgEndAddr;     //!< Address of the last byte of a contiguous image */
  uint16_t  hdrLen;         //!< Total length of the image header */
  uint16_t  rfu;           //!< Reserved bytes */
} oad_imgHdr_t;

typedef struct __attribute__((__packed__))
{
  uint8_t img_id;
  uint16_t block_num;
  uint16_t total_blocks;
} oad_imgBlockReq_t;

typedef struct __attribute__((__packed__))
{
  uint8_t img_id;
  uint16_t block_num;
  uint8_t data[OAD_BLOCK_SIZE];
} oad_imgBlockRsp_t;

typedef struct __attribute__((__packed__))
{
  uint32_t swVer;
  uint8_t bimVer;
  uint8_t appType;
  uint8_t platformType;
  uint8_t imgId;
  uint16_t ntfId;
  uint16_t status;
  timer_t timerId;
  char addr[INET6_ADDRSTRLEN];
} oad_deviceInfo_t;

typedef struct __attribute__((__packed__))
{
  uint8_t   imgID[IMG_ID_SIZE];       //!< User-defined Image Identification bytes. */
  uint8_t   bimVer;         //!< BIM version */
  uint8_t   metaVer;        //!< Metadata version */
  uint8_t   imgCpStat;      //!< Image copy status bytes */
  uint8_t   crcStat;        //!< CRC status */
  uint8_t   imgType;        //!< Image Type */
  uint8_t   imgNo;          //!< Image number of 'image type' */
  uint32_t  len;            //!< Image length in octets
  uint8_t   softVer[4];     //!< Software version of the image */
} oad_imgIdentify_t;

typedef struct __attribute__((__packed__))
{
  uint8_t img_id;
  oad_imgIdentify_t img_iid;
} oad_imgNtfReq_t;

typedef struct __attribute__((__packed__))
{
  uint8_t ipAddr[IPV6_ADDR_SIZE];
  char fwVerStr[FWVERSTR_SIZE];
  uint8_t imgID[IMG_ID_SIZE];
  uint8_t platformType;
} oad_regMsg_t;

typedef struct
{
  char fileName[MAX_FILE_NAME_SIZE];
  uint8_t appType;
  uint8_t secType;
  uint8_t platformType;
  uint8_t compilerType;
  uint32_t swVer;
  uint8_t bimVer;
} oad_fileInfo_t;

typedef enum
{
  OAD_STATUS_SUCCESS  =   0,
  OAD_STATUS_ERROR    =  -1,
  OAD_STATUS_REREG    =  -2,
  OAD_STATUS_BUFFULL  =  -3,
  OAD_STATUS_NOREG    =  -4,
  OAD_STATUS_NONTF    =  -5,
  OAD_STATUS_NOMEM    =  -6  
} oad_status_t;

#define OAD_MAX_DEVICES     16
#define OAD_DEV_STATUS_NONE 0x00
#define OAD_DEV_STATUS_REG  (1U << 0)
#define OAD_DEV_STATUS_NTFG (1U << 1)
#define OAD_DEV_STATUS_NTFD (1U << 2)
#define OAD_DEV_STATUS_FWV  (1U << 3)
#define OAD_DEV_STATUS_PRGS (1U << 4)
#define OAD_DEV_STATUS_DONE (1U << 5)
#define OAD_DEV_STATUS_MASK 0xFF
static oad_deviceInfo_t oad_deviceList[OAD_MAX_DEVICES] = { 0, };

#define OAD_MAX_FILES       5
#define DIRNAME             "/home/debian/images/"
static oad_fileInfo_t oad_fileList[OAD_MAX_FILES] = {0, };
static char dirName[MAX_DIR_NAME_SIZE] = "";
static char cmd_str[MAX_CMD_STR_SIZE] = "";
typedef unsigned char method_t;
method_t method = 3;
unsigned char msgtype = COAP_MESSAGE_CON;
static str proxy = {0, NULL};
static coap_uri_t uri;
static coap_list_t *optlist = NULL;
static unsigned char _token_data[8];
str the_token = {0, _token_data };
int flags = 0;
coap_block_t block = { .num = 0, .m = 0, .szx = 6 };
unsigned int obs_seconds = 30;
coap_tick_t obs_wait = 0;
static FILE *file = NULL;
static str output_file = {0, NULL };
coap_tick_t max_wait;
unsigned int wait_seconds = 90;
static str payload = { 0, NULL };
static int ready = 0;
static coap_log_t oad_maxlog = LOG_INFO;
bool testMode = false;
const char oad_platformList[OAD_MAX_PLATFORMS][IMG_ID_SIZE + 1] = {"UNKNOWN","CC26x2R1","CC13x2R1","CC13x2P1","CC13x2P2","CC13x2P4","CC26x2RB"};

static int oad_start_timer(timer_t timerId, int expire, int interval);

#define FLAGS_BLOCK 0x01

static
void oad_log(coap_log_t level, const char *format, ...) {
  char timebuf[32];
  coap_tick_t now_ticks;
  time_t now_rt;
  struct tm *tmp;
  va_list ap;
  FILE *ofd = stdout;

  if (oad_maxlog < level)
    return;

  coap_ticks(&now_ticks);
  now_rt = coap_ticks_to_rt(now_ticks);
  tmp = localtime(&now_rt);
  memset(timebuf, 0, 32);
  strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tmp);
  fprintf(ofd, "%s ", timebuf);
  fprintf(ofd, "[OAD] ");

  va_start(ap, format);
  vfprintf(ofd, format, ap);
  va_end(ap);
  fflush(ofd);
}

static inline void
set_timeout(coap_tick_t *timer, const unsigned int seconds) {
      coap_ticks(timer);
        *timer += seconds * COAP_TICKS_PER_SECOND;
}

static int
append_to_output(const unsigned char *data, size_t len) {
  size_t written;

  if (!file) {
    if (!output_file.s || (output_file.length && output_file.s[0] == '-'))
      file = stdout;
    else {
      if (!(file = fopen((char *)output_file.s, "w"))) {
        perror("fopen");
        return -1;
      }
    }
  }

  do {
    written = fwrite(data, 1, len, file);
    len -= written;
    data += written;
  } while ( written && len );
  fflush(file);

  return 0;
}

inline static void oad_set_platform(uint8_t *type, char *platformType)
{
  uint8_t i;

  for(i = 1; i < OAD_MAX_PLATFORMS; i++)
  {
    if(!memcmp(platformType, &oad_platformList[i], IMG_ID_SIZE))
    {
      *type = i;
      return;
    }
  }
  *type = 0;
  return;
}

inline static bool oad_is_matched(oad_fileInfo_t *info, oad_deviceInfo_t *device)
{
  return ((info->appType == device->appType)
  && (info->platformType == device->platformType)
  && (info->bimVer == device->bimVer)
  && (testMode || (info->swVer > device->swVer)));
}

static uint16_t oad_get_matched_device(uint8_t fid)
{
  uint16_t i;
  uint16_t devid = OAD_MAX_DEVICES;

  for(i = 0; i < OAD_MAX_DEVICES; i++)
  {
    if(oad_is_matched(&oad_fileList[fid], &oad_deviceList[i]))
    {
      devid = i;
      break;
    }
  }

  return devid;
}

static const char *oad_parse_platform_type(char *fn)
{
  char *token = strtok(fn, "_.");
  while(token != NULL)
  {
    if(!strncmp(token, "CC1352", 6))
    {
      if(!strncmp(token, "CC1352R1", 8))
      {
        return oad_platformList[CC13x2R1];
      }
      else if(!strncmp(token, "CC1352P1", 8))
      {
        return oad_platformList[CC13x2P1];
      }
      else if(!strncmp(token, "CC1352P", 7))
      {
        token = strtok(NULL, "_.");
        if(!strncmp(token, "2", 1))
        {
          return oad_platformList[CC13x2P2];
        }
        else if(!strncmp(token, "4", 1))
        {
          return oad_platformList[CC13x2P4];
        }
        else
        {
          //do nothing
        }
      }
      else
      {
        //do nothing
      }
    }
    else if(!strncmp(token, "CC26X2R1", 8))
    {
      return oad_platformList[CC26x2R1];
    }
    else if(!strncmp(token, "CC2652RB", 8))
    {
      return oad_platformList[CC26x2RB];
    }
	else
	{
		//do nothing
	}
    token = strtok(NULL, "_.");
  }
  return NULL;
}

static bool oad_get_file_header(char *name, oad_imgHdr_t *hdr)
{
  FILE *fp;
  char fname[MAX_FILE_NAME_SIZE] = "";
  size_t len;

  if(!strchr(name, '/'))
  {
    strcat(fname, dirName);
  }

  strcat(fname, name);

  if((fp = fopen(fname, "r")) == NULL)
  {
    oad_log(LOG_ERR, "cannot open file %s\n", name);
    return false;
  }

  fseek(fp, 0, SEEK_SET);

  len = fread(hdr, 1, sizeof(oad_imgHdr_t), fp);

  fclose(fp);

  if(len < sizeof(oad_imgHdr_t))
  {
    oad_log(LOG_DEBUG, "file %s has wrong\n", name);
    return false;
  }

  return true;
}

static bool oad_get_file_info(char *name, oad_fileInfo_t *info)
{
  oad_imgHdr_t hdr;
  char platformType[IMG_ID_SIZE];
  char swVer[SW_VER_SIZE+1];
  const char *pType = NULL;
  char fn[MAX_FILE_NAME_SIZE] = "";

  if(!name)
  {
    return false;
  }

  if(!oad_get_file_header(name, &hdr))
  {
    return false;
  }
  
  /* need to get detailed platform type from file name */
  strcpy(fn, name);
  pType = oad_parse_platform_type(fn);  
  if(!pType)
  {
    return false;
  }
  oad_log(LOG_DEBUG, "platform redefined as %s\n", pType);
  memcpy(hdr.imgID, pType, IMG_ID_SIZE);
  
  memset(platformType, 0, sizeof(platformType));
  memset(swVer, 0, sizeof(swVer));

  strcpy(info->fileName, name);
  memcpy(platformType, hdr.imgID, IMG_ID_SIZE);
  info->bimVer = hdr.bimVer;
  memcpy(swVer, hdr.softVer, SW_VER_SIZE);
  info->swVer = atoi(swVer);
  /* we have only one application type now */
  info->appType = DOORLOCK;
  oad_set_platform(&info->platformType, platformType);
  if(info->platformType == UNKNOWN)
  {
    return false;
  }

  oad_log(LOG_DEBUG, "%-60s | platform : %s | bim ver : %d |sw ver : %d\n", info->fileName, &oad_platformList[info->platformType], info->bimVer, info->swVer);
  return true;
}

static bool oad_insert_file_info(oad_fileInfo_t *info)
{
  uint16_t i;
  uint16_t devid;
  uint16_t di = OAD_MAX_FILES;
  uint16_t ei = OAD_MAX_FILES;

  if(!info)
  {
    return false;
  }

  if(!strlen(info->fileName))
  {
    return false;
  }

  /* search if already exists */
  for(i = 0; i < OAD_MAX_FILES; i++)
  {
    if((oad_fileList[i].appType == info->appType)
    && (oad_fileList[i].platformType == info->platformType))
    {
      di = i;
    }
    if(!strlen(oad_fileList[i].fileName) && (ei == OAD_MAX_FILES))
    {
      ei = i;
    }
    if((di < OAD_MAX_FILES) && (ei < OAD_MAX_FILES))
    {
      break;
    }
  }
  if (di < OAD_MAX_FILES)
  {
    oad_log(LOG_DEBUG, " %s already exists\n", info->fileName);
    if(oad_fileList[di].swVer < info->swVer)
    {
      memset(oad_fileList[di].fileName, 0, sizeof(oad_fileList[di].fileName));
      strcpy(oad_fileList[di].fileName, info->fileName);
      oad_fileList[di].bimVer = info->bimVer;
      oad_fileList[di].swVer = info->swVer;
    }
  }
  else
  {
    oad_log(LOG_DEBUG, "%s is new file\n", info->fileName);
    if(ei < OAD_MAX_FILES)
    {
      memcpy(&oad_fileList[ei], info, sizeof(oad_fileInfo_t));
      devid = oad_get_matched_device(ei);
      if(devid == OAD_MAX_DEVICES)
      {
        oad_log(LOG_DEBUG, "%s does not have matched device\n", info->fileName);
      }
      else
      {
        if(oad_deviceList[devid].imgId != OAD_MAX_FILES)
        {
          oad_log(LOG_ERR, "%s has matched device %d, but the device has already matched file\n", info->fileName, devid);
        }
        else
        {
          oad_deviceList[devid].imgId = ei;
          oad_log(LOG_DEBUG, "%s has matched device %d\n", info->fileName, devid);
        }
      }
    }
    else
    {
      oad_log(LOG_WARNING, " %s not added due to memory full\n", info->fileName);
      return false;
    }
  }
  return true;
}

static bool oad_delete_file_info(oad_fileInfo_t *info)
{
  uint8_t i;
  uint16_t devid;

  if(!info)
  {
    return false;
  }

  if(!strlen(info->fileName))
  {
    return false;
  }

  /* search if exists */
  for(i = 0; i < OAD_MAX_FILES; i++)
  {
    if(!strcmp(oad_fileList[i].fileName, info->fileName))
    { 
      devid = oad_get_matched_device(i);
      oad_deviceList[devid].imgId = OAD_MAX_FILES;
      oad_deviceList[devid].status = OAD_DEV_STATUS_REG;
      memset(&oad_fileList[i], 0, sizeof(oad_fileInfo_t));
      oad_log(LOG_DEBUG, "%s deleted\n", info->fileName);
      return true;
    }
  }
  oad_log(LOG_DEBUG, "%s not found\n", info->fileName);
  return false;
}

static void oad_show_file_list(void)
{
  uint16_t i;
  oad_fileInfo_t *info;

  for(i = 0; i < OAD_MAX_FILES; i++)
  {
    oad_log(LOG_INFO, "[IMG] %d : ", i);
    info = &oad_fileList[i];
    if(!strlen(info->fileName))
    {
      oad_log(LOG_INFO, "none!\n");
    }
    else
    {
      oad_log(LOG_INFO, "%-60s | platform : %s | bim ver : %d | sw ver : %d\n",info->fileName, &oad_platformList[info->platformType], info->bimVer, info->swVer);
    }
  }
}

static bool oad_read_directory(char *dirname)
{
  DIR *d;
  struct dirent *dir;
  oad_fileInfo_t info;

  d = opendir(dirname);
  if(!d)
  {
    return false;
  }
  while((dir = readdir(d)) != NULL)
  {
    if(strcmp("..", dir->d_name) && strcmp(".", dir->d_name))
    {
      memset(&info, 0, sizeof(oad_fileInfo_t));
      if(oad_get_file_info(dir->d_name, &info))
      {
        oad_insert_file_info(&info);
      }
    }
  }
  closedir(d);
  oad_show_file_list();
  return true;
}

static bool oad_is_temp_event(char *name)
{
  char *token1, *token2;
  token1 = strchr(name, '.');
  if(token1 == name)
  {
    oad_log(LOG_DEBUG, "temp file %s\n",name);
    return true;
  }
  token1 = strrchr(name, '.');
  token2 = strrchr(name,'~');
  if(token2 != NULL && token2 - token1 == 4)
  {
    oad_log(LOG_DEBUG, "temp file %s\n",name);
    return true;
  }
  token1 = strrchr(name, '.');
  if(!strcmp(token1+1, "filepart"))
  {
    oad_log(LOG_DEBUG, "temp file %s\n",name);
    return true;
  }
  token1 = strrchr(name, '.');
  if(strcmp(token1+1, "bin"))
  {
    oad_log(LOG_DEBUG, "temp file %s\n",name);
    return true;
  }
  return false;
}

static void oad_directory_event_handler(struct inotify_event *event)
{
  oad_fileInfo_t info;

  if(event->mask & IN_CREATE)
  {
    if(event->mask & IN_ISDIR)
    {
      oad_log(LOG_DEBUG, "new directory %s created\n", event->name);
    }
    else
    {
      oad_log(LOG_DEBUG, "new file %s created\n", event->name);
      memset(&info, 0, sizeof(oad_fileInfo_t));
      if(oad_get_file_info(event->name, &info))
      {
        oad_insert_file_info(&info);
      }
    }
  }
  if(event->mask & IN_MODIFY)
  {
    if(event->mask & IN_ISDIR)
    {
      oad_log(LOG_DEBUG, "directory %s modified\n", event->name);
    }
    else
    {
      oad_log(LOG_DEBUG, "new file %s modified\n", event->name);

      memset(&info, 0, sizeof(oad_fileInfo_t));
      if(oad_get_file_info(event->name, &info))
      {
        oad_insert_file_info(&info);
      }

    }
  }
  if(event->mask & IN_DELETE)
  {
    if(event->mask & IN_ISDIR)
    {
      oad_log(LOG_DEBUG, "directory %s deleted\n", event->name);
    }
    else
    {
      oad_log(LOG_DEBUG, "old file %s deleted\n", event->name);
      memset(&info, 0, sizeof(oad_fileInfo_t));
      strcpy(info.fileName, event->name);
      oad_delete_file_info(&info);
    }
  }
  if(event->mask & IN_CLOSE_WRITE)
  {
    if(event->mask & IN_ISDIR)
    {
      oad_log(LOG_DEBUG, "directory %s closed\n", event->name);
    }
    else
    {
      oad_log(LOG_DEBUG, "new file %s closed\n", event->name);
      memset(&info, 0, sizeof(oad_fileInfo_t));
      if(oad_get_file_info(event->name, &info))
      {
        oad_insert_file_info(&info);
      }
    }
  }
  if(event->mask & IN_MOVE)
  {
    if(event->mask & IN_ISDIR)
    {
      oad_log(LOG_DEBUG, "directory %s moved\n", event->name);
    }
    else
    {
      if(event->mask & IN_MOVED_FROM)
      {
        oad_log(LOG_DEBUG, "old file %s moved out\n", event->name);
        memset(&info, 0, sizeof(oad_fileInfo_t));
        strcpy(info.fileName, event->name);
        oad_delete_file_info(&info);
      }
      if(event->mask & IN_MOVED_TO)
      {
        oad_log(LOG_DEBUG, "new file %s moved in\n", event->name);
        memset(&info, 0, sizeof(oad_fileInfo_t));
        if(oad_get_file_info(event->name, &info))
        {
          oad_insert_file_info(&info);
        }
      }
    }
  }
  oad_show_file_list();
}

static bool oad_read_image(int fd)
{
  int length;
  char *pb;
  struct inotify_event *event;
  char buf[FILE_EVENT_BUF_SIZE];

  length = read(fd, buf, sizeof(buf));

  if(!length)
  {
          return false;
  }

  pb = buf;

  while(length)
  {
    event = (struct inotify_event *)pb;
    if (event->len && !oad_is_temp_event(event->name))
    {
      oad_log(LOG_DEBUG, "%x %x %x %x %s\n",event->wd, event->mask, event->cookie, event->len, event->name);
      oad_directory_event_handler(event);
    }
    length -= (16 + event->len);
    pb += (16 + event->len);
  }

  return true;
}

static uint8_t oad_get_matched_file(uint16_t devid)
{
  uint8_t i;
  uint8_t fid;
  uint8_t oad_max_files = OAD_MAX_FILES;

  fid = oad_max_files;
  for(i = 0; i < oad_max_files; i++)
  {
    if(oad_is_matched(&oad_fileList[i], &oad_deviceList[devid]))
    {
      fid = i;
      break;
    }
  }
 
  if(fid == oad_max_files)
  {
    fid = OAD_MAX_FILES;
  } 
  return fid;
}

static bool oad_build_ntf(uint16_t devid, oad_imgNtfReq_t *oad_ntf_pdu)
{
  uint8_t fid;
  oad_imgHdr_t img_hdr;

  fid = oad_deviceList[devid].imgId;

  /* could not find */
  if(fid == OAD_MAX_FILES)
  {
    return false;
  }

  if(!oad_get_file_header(oad_fileList[fid].fileName, &img_hdr))
  {
    return false;
  }

  oad_ntf_pdu->img_id = fid;
  memcpy(oad_ntf_pdu->img_iid.imgID, img_hdr.imgID, 8);
  oad_ntf_pdu->img_iid.bimVer = img_hdr.bimVer;
  oad_ntf_pdu->img_iid.metaVer = img_hdr.metaVer;
  oad_ntf_pdu->img_iid.imgCpStat = img_hdr.imgCpStat;
  oad_ntf_pdu->img_iid.crcStat = img_hdr.crcStat;
  oad_ntf_pdu->img_iid.imgType = img_hdr.imgType;
  oad_ntf_pdu->img_iid.imgNo = img_hdr.imgNo;
  oad_ntf_pdu->img_iid.len = img_hdr.len;
  memcpy(oad_ntf_pdu->img_iid.softVer, img_hdr.softVer, 4);  
  return true;
}

static void oad_update_ntf(char *addr, uint16_t id)
{
  uint16_t i;

  for(i = 0; i < OAD_MAX_DEVICES; i++)
  {
    oad_log(LOG_DEBUG, "[NTF] devid = %d %s %s 0x%x 0x%x\n",i, oad_deviceList[i].addr, addr, oad_deviceList[i].ntfId, id);
    if(!strcmp(oad_deviceList[i].addr, addr) && (oad_deviceList[i].ntfId == id))
    {
      oad_deviceList[i].status &= ~OAD_DEV_STATUS_NTFG;
      oad_deviceList[i].status |= OAD_DEV_STATUS_NTFD;
      oad_start_timer(oad_deviceList[i].timerId, 0, 0);
      oad_log(LOG_NOTICE, "[NTF] notification to %s(0x%x) has been ACKnowledged\n", addr, oad_deviceList[i].status);
      return;
    }
  }
}

static int oad_get_block(char *fn, uint32_t blk_no, uint32_t blk_size, uint8_t *blk_buf)
{
  size_t  bytes_read;
  FILE *ifp;
  char fname[MAX_FILE_NAME_SIZE] = "";

  if(!strchr(fn, '/'))
  {
    strcat(fname, dirName);
  }

  strcat(fname, fn);
  
  ifp = fopen(fname, "r");
  if (!ifp)
  {
    oad_log(LOG_ERR, "cannot open file %s\n", fn);
    return -1;
  }
  fseek(ifp, blk_no * blk_size, SEEK_SET);
  bytes_read = (int) fread(blk_buf, 1, blk_size, ifp);
  fclose(ifp);
  return bytes_read;
}

static oad_status_t oad_update_list(char *addr, oad_regMsg_t *reg_req)
{
  uint16_t i;
  char platformType[OAD_IMG_ID_SIZE] = {0, };
  char swVer[OAD_SW_VER_SIZE + 1] = {0, };
  char bimVer[OAD_BIM_VER_SIZE + 1] = {0, };

  oad_deviceInfo_t *pDevice;

  for (i = 0; i < OAD_MAX_DEVICES; i++)
  {
    if(!strcmp(oad_deviceList[i].addr, addr) && oad_deviceList[i].status != OAD_DEV_STATUS_NONE)
    {
      oad_log(LOG_NOTICE, "[REG] registeration again from %s(0x%x)\n", addr, oad_deviceList[i].status);
      return OAD_STATUS_REREG;
    }
  }

  for (i = 0; i < OAD_MAX_DEVICES; i++)
  {
    pDevice = &oad_deviceList[i];
    if(pDevice->status == OAD_DEV_STATUS_NONE)
    {
      strcpy(pDevice->addr, addr);
      pDevice->status = OAD_DEV_STATUS_REG;
      /* we have only one app type now */
      pDevice->appType = DOORLOCK;
      if(reg_req)
      {
        snprintf(swVer, OAD_SW_VER_SIZE + 1, "%s", &reg_req->fwVerStr[3]);
        snprintf(bimVer, OAD_BIM_VER_SIZE + 1, "%s", &reg_req->fwVerStr[11]);
    if(reg_req->platformType == CC2652RB_PLATFORM_VALUE)
    {
      memcpy(platformType, oad_platformList[CC26x2RB], OAD_IMG_ID_SIZE);
    }
	else if(reg_req->platformType == CC1352P_2_PLATFORM_VALUE)
    {
      memcpy(platformType, oad_platformList[CC13x2P2], OAD_IMG_ID_SIZE);
    }
    else
    {
      memcpy(platformType,  reg_req->imgID, OAD_IMG_ID_SIZE);
    }
        oad_set_platform(&pDevice->platformType, platformType);
        pDevice->bimVer = atoi(bimVer);
        pDevice->swVer = atoi(swVer);
      }
      else
      {
        /* This is only for internal use by test team */
        pDevice->platformType = DONT_CARE;
        pDevice->bimVer = DONT_CARE;
        pDevice->swVer = DONT_CARE;
      }

      pDevice->imgId = oad_get_matched_file(i);

      oad_log(LOG_NOTICE, "[REG] registration from %s with platform : %s | bim ver : %d | sw ver : %d\n",addr, &oad_platformList[pDevice->platformType], pDevice->bimVer, pDevice->swVer);
      return OAD_STATUS_SUCCESS;
     } 
  } 
  oad_log(LOG_WARNING, "device list is full\n");
  return OAD_STATUS_BUFFULL;
}
#endif

/* temporary storage for dynamic resource representations */
static int quit = 0;

/* changeable clock base (see handle_put_time()) */
static time_t clock_offset;
static time_t my_clock_base = 0;

struct coap_resource_t *time_resource = NULL;

#ifndef WITHOUT_ASYNC
/* This variable is used to mimic long-running tasks that require
 * asynchronous responses. */
static coap_async_state_t *async = NULL;
#endif /* WITHOUT_ASYNC */

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */

/* SIGINT handler: set quit to 1 for graceful termination */
static void
handle_sigint(int signum UNUSED_PARAM) {
  quit = 1;
}

#define INDEX "This is a test server made with libcoap (see https://libcoap.net)\n" \
              "Copyright (C) 2010--2016 Olaf Bergmann <bergmann@tzi.org>\n\n"

static void
hnd_get_index(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request UNUSED_PARAM,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  coap_add_data(response, strlen(INDEX), (unsigned char *)INDEX);
}

#define OAD_REGACK "OK\n"

static void
hnd_put_oad_reg(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  unsigned char buf[3];
  void *addrptr = NULL;
  char addr[INET6_ADDRSTRLEN];
  size_t size;
  unsigned char *data;
  oad_regMsg_t reg_req;

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  addrptr = &peer->addr.sin6.sin6_addr;
  inet_ntop(peer->addr.sa.sa_family, addrptr, addr, sizeof(addr));
  (void)coap_get_data(request, &size, &data);

  if(size != sizeof(oad_regMsg_t))
  {
    oad_log(LOG_WARNING, "[REG] payload is wrong\n");
    response->hdr->code = COAP_RESPONSE_CODE(408);
  }
  else
  {   
    memcpy(&reg_req, data, size);
    oad_update_list(addr, &reg_req);
    response->hdr->code = COAP_RESPONSE_CODE(205);
    coap_add_data(response, strlen(OAD_REGACK), (unsigned char *)OAD_REGACK);
  }
}

#define OAD_IMAGEID "0x55\n\n"

static void
hnd_get_oad_fwv(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request UNUSED_PARAM,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  unsigned char buf[3];

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  coap_add_data(response, strlen(OAD_IMAGEID), (unsigned char *)OAD_IMAGEID);
}

#define OAD_IMAGEBLOCK "Block :  \n\n"

static void
hnd_get_oad_blk(coap_context_t *ctx UNUSED_PARAM,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer UNUSED_PARAM,
              coap_pdu_t *request,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  unsigned char buf[3];
  char str[COAP_TEMP_STR_SIZE] = {0, };
  size_t size;
  unsigned char *data;
  void *addrptr = NULL;
  char addr[INET6_ADDRSTRLEN];
  uint16_t devid;
  //unsigned char block_buf[OAD_BLOCK_SIZE];
  oad_imgBlockRsp_t block_buf;
  uint32_t block_num = 0;
  uint32_t bytes_read = 0;
  uint8_t fid;
  oad_imgBlockReq_t block_req;

  response->hdr->code = COAP_RESPONSE_CODE(205);

  coap_add_option(response,
                  COAP_OPTION_CONTENT_TYPE,
                  coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x2ffff), buf);

  //check if the device is in the list
  addrptr = &peer->addr.sin6.sin6_addr;
  inet_ntop(peer->addr.sa.sa_family, addrptr, addr, sizeof(addr));

  for(devid = 0; devid < OAD_MAX_DEVICES; devid++)
  {
    if(!strcmp(addr, oad_deviceList[devid].addr))
    {
      break;
    }
  }
  if (devid == OAD_MAX_DEVICES)
  {
    oad_log(LOG_WARNING, "[BLK] device with %s cannot be found\n", addr);  
    response->hdr->code = COAP_RESPONSE_CODE(404);
    return;
  }
  memset(str, 0, sizeof(str));

  if (!(oad_deviceList[devid].status & OAD_DEV_STATUS_REG) 
   || !(oad_deviceList[devid].status & OAD_DEV_STATUS_NTFD))
  {
    oad_log(LOG_WARNING, "[BLK] device with %s(0x%x) is not registered or notified\n", addr, oad_deviceList[devid].status);  
    response->hdr->code = COAP_RESPONSE_CODE(401);
    return;
  }

  (void)coap_get_data(request, &size, &data);

  if (!size)
  {
    oad_log(LOG_WARNING, "[BLK] device with %s requested without payload\n", addr);  
    response->hdr->code = COAP_RESPONSE_CODE(408);
  }
  else
  {
    if (size != sizeof(oad_imgBlockReq_t))
    {
      memcpy(str, data, size);
      block_num = atoi(str);
      fid = oad_get_matched_file(devid);
      if(fid == OAD_MAX_FILES)
      {
        oad_log(LOG_WARNING, "[BLK] device with %s requested unavailable image : %d-%d\n", addr, oad_deviceList[devid].imgId, fid);
        response->hdr->code = COAP_RESPONSE_CODE(406);
        return;
      }
    }
    else
    {
      memset(&block_req, 0, sizeof(oad_imgBlockReq_t));
      memcpy(&block_req, data, size);
      block_num = block_req.block_num;
      fid = oad_get_matched_file(devid);
      if(fid == OAD_MAX_FILES)
      {
        oad_log(LOG_WARNING, "[BLK] device with %s requested unavailable image : %d-%d\n", addr, oad_deviceList[devid].imgId, fid);
        response->hdr->code = COAP_RESPONSE_CODE(406);
        return;
      }
      if(fid != block_req.img_id)
      {
        oad_log(LOG_WARNING, "[BLK] device with %s requested wrong image : %d-%d\n", addr, oad_deviceList[devid].imgId, block_req.img_id);
        response->hdr->code = COAP_RESPONSE_CODE(406);
        return;
      }
    }
    bytes_read = oad_get_block(oad_fileList[fid].fileName, block_num, OAD_BLOCK_SIZE, block_buf.data);
    if (bytes_read > 0)
    {
      block_buf.img_id = fid;
      block_buf.block_num = block_num;
      oad_log(LOG_INFO, "[BLK] block %4d : %3d bytes sent to device %d %s with %s\n", block_num, bytes_read + 3, devid, &oad_platformList[oad_deviceList[devid].platformType], addr);
      oad_deviceList[devid].status |= OAD_DEV_STATUS_PRGS;
      response->hdr->code = COAP_RESPONSE_CODE(205);
      coap_add_data(response, bytes_read + 3, (unsigned char *)&block_buf);
    }
    else
    {
      oad_log(LOG_INFO, "[BLK] device with %s already reached end of image\n", addr);
      oad_deviceList[devid].status |= OAD_DEV_STATUS_DONE;
      response->hdr->code = COAP_RESPONSE_CODE(406);
    }
  }
}

static void
hnd_get_time(coap_context_t  *ctx,
             struct coap_resource_t *resource,
             const coap_endpoint_t *local_interface UNUSED_PARAM,
             coap_address_t *peer,
             coap_pdu_t *request,
             str *token,
             coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned char buf[40];
  size_t len;
  time_t now;
  coap_tick_t t;

  /* FIXME: return time, e.g. in human-readable by default and ticks
   * when query ?ticks is given. */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code =
    my_clock_base ? COAP_RESPONSE_CODE(205) : COAP_RESPONSE_CODE(404);

  if (coap_find_observer(resource, peer, token)) {
    /* FIXME: need to check for resource->dirty? */
    coap_add_option(response,
                    COAP_OPTION_OBSERVE,
                    coap_encode_var_bytes(buf, ctx->observe), buf);
  }

  if (my_clock_base)
    coap_add_option(response,
                    COAP_OPTION_CONTENT_FORMAT,
                    coap_encode_var_bytes(buf, COAP_MEDIATYPE_TEXT_PLAIN), buf);

  coap_add_option(response,
                  COAP_OPTION_MAXAGE,
                  coap_encode_var_bytes(buf, 0x01), buf);

  if (my_clock_base) {

    /* calculate current time */
    coap_ticks(&t);
    now = my_clock_base + (t / COAP_TICKS_PER_SECOND);

    if (request != NULL
        && (option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter))
        && memcmp(COAP_OPT_VALUE(option), "ticks",
        min(5, COAP_OPT_LENGTH(option))) == 0) {
          /* output ticks */
          len = snprintf((char *)buf,
                         min(sizeof(buf),
                             response->max_size - response->length),
                             "%u", (unsigned int)now);
          coap_add_data(response, len, buf);

    } else {      /* output human-readable time */
      struct tm *tmp;
      tmp = gmtime(&now);
      len = strftime((char *)buf,
                     min(sizeof(buf),
                     response->max_size - response->length),
                     "%b %d %H:%M:%S", tmp);
      coap_add_data(response, len, buf);
    }
  }
}

static void
hnd_put_time(coap_context_t *ctx UNUSED_PARAM,
             struct coap_resource_t *resource UNUSED_PARAM,
             const coap_endpoint_t *local_interface UNUSED_PARAM,
             coap_address_t *peer UNUSED_PARAM,
             coap_pdu_t *request,
             str *token UNUSED_PARAM,
             coap_pdu_t *response) {
  coap_tick_t t;
  size_t size;
  unsigned char *data;

  /* FIXME: re-set my_clock_base to clock_offset if my_clock_base == 0
   * and request is empty. When not empty, set to value in request payload
   * (insist on query ?ticks). Return Created or Ok.
   */

  /* if my_clock_base was deleted, we pretend to have no such resource */
  response->hdr->code =
    my_clock_base ? COAP_RESPONSE_CODE(204) : COAP_RESPONSE_CODE(201);

  resource->dirty = 1;

  /* coap_get_data() sets size to 0 on error */
  (void)coap_get_data(request, &size, &data);

  if (size == 0)        /* re-init */
    my_clock_base = clock_offset;
  else {
    my_clock_base = 0;
    coap_ticks(&t);
    while(size--)
      my_clock_base = my_clock_base * 10 + *data++;
    my_clock_base -= t / COAP_TICKS_PER_SECOND;
  }
}

static void
hnd_delete_time(coap_context_t *ctx UNUSED_PARAM,
                struct coap_resource_t *resource UNUSED_PARAM,
                const coap_endpoint_t *local_interface UNUSED_PARAM,
                coap_address_t *peer UNUSED_PARAM,
                coap_pdu_t *request UNUSED_PARAM,
                str *token UNUSED_PARAM,
                coap_pdu_t *response UNUSED_PARAM) {
  my_clock_base = 0;    /* mark clock as "deleted" */

  /* type = request->hdr->type == COAP_MESSAGE_CON  */
  /*   ? COAP_MESSAGE_ACK : COAP_MESSAGE_NON; */
}

#ifndef WITHOUT_ASYNC
static void
hnd_get_async(coap_context_t *ctx,
              struct coap_resource_t *resource UNUSED_PARAM,
              const coap_endpoint_t *local_interface UNUSED_PARAM,
              coap_address_t *peer,
              coap_pdu_t *request,
              str *token UNUSED_PARAM,
              coap_pdu_t *response) {
  coap_opt_iterator_t opt_iter;
  coap_opt_t *option;
  unsigned long delay = 5;
  size_t size;

  if (async) {
    if (async->id != request->hdr->id) {
      coap_opt_filter_t f;
      coap_option_filter_clear(f);
      response->hdr->code = COAP_RESPONSE_CODE(503);
    }
    return;
  }

  option = coap_check_option(request, COAP_OPTION_URI_QUERY, &opt_iter);
  if (option) {
    unsigned char *p = COAP_OPT_VALUE(option);

    delay = 0;
    for (size = COAP_OPT_LENGTH(option); size; --size, ++p)
      delay = delay * 10 + (*p - '0');
  }

  async = coap_register_async(ctx,
                              peer,
                              request,
                              COAP_ASYNC_SEPARATE | COAP_ASYNC_CONFIRM,
                              (void *)(COAP_TICKS_PER_SECOND * delay));
}

static void
check_async(coap_context_t *ctx,
            const coap_endpoint_t *local_if,
            coap_tick_t now) {
  coap_pdu_t *response;
  coap_async_state_t *tmp;

  size_t size = sizeof(coap_hdr_t) + 13;

  if (!async || now < async->created + (unsigned long)async->appdata)
    return;

  response = coap_pdu_init(async->flags & COAP_ASYNC_CONFIRM
             ? COAP_MESSAGE_CON
             : COAP_MESSAGE_NON,
             COAP_RESPONSE_CODE(205), 0, size);
  if (!response) {
    debug("check_async: insufficient memory, we'll try later\n");
    async->appdata =
      (void *)((unsigned long)async->appdata + 15 * COAP_TICKS_PER_SECOND);
    return;
  }

  response->hdr->id = coap_new_message_id(ctx);

  if (async->tokenlen)
    coap_add_token(response, async->tokenlen, async->token);

  coap_add_data(response, 4, (unsigned char *)"done");

  if (coap_send(ctx, local_if, &async->peer, response) == COAP_INVALID_TID) {
    debug("check_async: cannot send response for message %d\n",
    response->hdr->id);
  }
  coap_delete_pdu(response);
  coap_remove_async(ctx, async->id, &tmp);
  coap_free_async(async);
  async = NULL;
}
#endif /* WITHOUT_ASYNC */

static void
init_resources(coap_context_t *ctx) {
  coap_resource_t *r;

  r = coap_resource_init(NULL, 0, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_index);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"General Info\"", 14, 0);
  coap_add_resource(ctx, r);

  /* Thread OAD Concept Test : Get Version */
  r = coap_resource_init((unsigned char *)"oad/reg", 7, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_oad_reg);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"OAD : Client Register\"", 23, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Firmware\"", 10, 0);
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"file\"", 6, 0);
  coap_add_resource(ctx, r);

  /* Thread OAD Concept Test : Get Image Identity */
  r = coap_resource_init(NULL, 0, 0);
  r = coap_resource_init((unsigned char *)"oad/fwv", 7, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_oad_fwv);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"OAD : FW Version\"", 18, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Firmware\"", 10, 0);
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"file\"", 6, 0);
  coap_add_resource(ctx, r);

  /* Thread OAD Concept Test : Get Image Block */
  r = coap_resource_init(NULL, 0, 0);
  r = coap_resource_init((unsigned char *)"oad/img", 7, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_oad_blk);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"OAD : FW Image Blocks\"", 23, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Firmware\"", 10, 0);
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"file\"", 6, 0);
  coap_add_resource(ctx, r);

  /* store clock base to use in /time */
  my_clock_base = clock_offset;

  r = coap_resource_init((unsigned char *)"time", 4, COAP_RESOURCE_FLAGS_NOTIFY_CON);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_time);
  coap_register_handler(r, COAP_REQUEST_PUT, hnd_put_time);
  coap_register_handler(r, COAP_REQUEST_DELETE, hnd_delete_time);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_attr(r, (unsigned char *)"title", 5, (unsigned char *)"\"Internal Clock\"", 16, 0);
  coap_add_attr(r, (unsigned char *)"rt", 2, (unsigned char *)"\"Ticks\"", 7, 0);
  r->observable = 1;
  coap_add_attr(r, (unsigned char *)"if", 2, (unsigned char *)"\"clock\"", 7, 0);

  coap_add_resource(ctx, r);
  time_resource = r;

#ifndef WITHOUT_ASYNC
  r = coap_resource_init((unsigned char *)"async", 5, 0);
  coap_register_handler(r, COAP_REQUEST_GET, hnd_get_async);

  coap_add_attr(r, (unsigned char *)"ct", 2, (unsigned char *)"0", 1, 0);
  coap_add_resource(ctx, r);
#endif /* WITHOUT_ASYNC */
}

static void
usage( const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf( stderr, "%s v%s -- a small CoAP implementation\n"
     "(c) 2010,2011,2015 Olaf Bergmann <bergmann@tzi.org>\n\n"
     "usage: %s [-A address] [-p port]\n\n"
     "\t-A address\t\tinterface address to bind to\n"
     "\t-g group\t\tjoin the given multicast group\n"
     "\t-p port\t\t\tlisten on specified port\n"
     "\t-v num\t\t\tverbosity level (default : 3)\n"
     "\t-d directory_name\tdirectory to search images (default : /home/debian/images/)\n"
     "\t-t test_mode\t\tno version check (default : 0)\n"
     "\t-e num\t\t\tnumber of devices to test before termination (default : 0)\n",
     program, version, program );
}

static coap_context_t *
get_context(const char *node, const char *port) {
  coap_context_t *ctx = NULL;
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
#ifdef OAD_TEST
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;
#else
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
#endif

  s = getaddrinfo(node, port, &hints, &result);
  if ( s != 0 ) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return NULL;
  }

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      ctx = coap_new_context(&addr);
      if (ctx) {
        /* TODO: output address:port for successful binding */
        goto finish;
      }
    }
  }

  fprintf(stderr, "no context available for interface '%s'\n", node);

  finish:
  freeaddrinfo(result);
  return ctx;
}

static int
join(coap_context_t *ctx, char *group_name){
  struct ipv6_mreq mreq;
  struct addrinfo   *reslocal = NULL, *resmulti = NULL, hints, *ainfo;
  int result = -1;

  /* we have to resolve the link-local interface to get the interface id */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;

  result = getaddrinfo("::", NULL, &hints, &reslocal);
  if (result < 0) {
    fprintf(stderr, "join: cannot resolve link-local interface: %s\n",
            gai_strerror(result));
    goto finish;
  }

  /* get the first suitable interface identifier */
  for (ainfo = reslocal; ainfo != NULL; ainfo = ainfo->ai_next) {
    if (ainfo->ai_family == AF_INET6) {
      mreq.ipv6mr_interface =
                ((struct sockaddr_in6 *)ainfo->ai_addr)->sin6_scope_id;
      break;
    }
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_DGRAM;

  /* resolve the multicast group address */
  result = getaddrinfo(group_name, NULL, &hints, &resmulti);

  if (result < 0) {
    fprintf(stderr, "join: cannot resolve multicast address: %s\n",
            gai_strerror(result));
    goto finish;
  }

  for (ainfo = resmulti; ainfo != NULL; ainfo = ainfo->ai_next) {
    if (ainfo->ai_family == AF_INET6) {
      mreq.ipv6mr_multiaddr =
                ((struct sockaddr_in6 *)ainfo->ai_addr)->sin6_addr;
      break;
    }
  }

  result = setsockopt(ctx->sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
          (char *)&mreq, sizeof(mreq));
  if (result < 0)
    perror("join: setsockopt");

 finish:
  freeaddrinfo(resmulti);
  freeaddrinfo(reslocal);

  return result;
}

#ifdef OAD_TEST
static coap_list_t *
new_option_node(unsigned short key, size_t length, unsigned char *data) {
  coap_list_t *node;

  node = coap_malloc(sizeof(coap_list_t) + sizeof(coap_option) + length);

  if (node) {
    coap_option *option;
    option = (coap_option *)(node->data);
    COAP_OPTION_KEY(*option) = key;
    COAP_OPTION_LENGTH(*option) = length;
    memcpy(COAP_OPTION_DATA(*option), data, length);
  } else {
    coap_log(LOG_DEBUG, "new_option_node: malloc\n");
  }

  return node;
}

static int
order_opts(void *a, void *b) {
  coap_option *o1, *o2;

  if (!a || !b)
    return a < b ? -1 : 1;

  o1 = (coap_option *)(((coap_list_t *)a)->data);
  o2 = (coap_option *)(((coap_list_t *)b)->data);

  return (COAP_OPTION_KEY(*o1) < COAP_OPTION_KEY(*o2))
    ? -1
    : (COAP_OPTION_KEY(*o1) != COAP_OPTION_KEY(*o2));
}

/**
 * Sets global URI options according to the URI passed as @p arg.
 * This function returns 0 on success or -1 on error.
 *
 * @param arg             The URI string.
 * @param create_uri_opts Flags that indicate whether Uri-Host and
 *                        Uri-Port should be suppressed.
 * @return 0 on success, -1 otherwise
 */
static int
cmdline_uri(char *arg, int create_uri_opts) {
  unsigned char portbuf[2];
#define BUFSIZE 40
  unsigned char _buf[BUFSIZE];
  unsigned char *buf = _buf;
  size_t buflen;
  int res;

  if (proxy.length) {   /* create Proxy-Uri from argument */
    size_t len = strlen(arg);
    while (len > 270) {
      coap_insert(&optlist,
                  new_option_node(COAP_OPTION_PROXY_URI,
                  270,
                  (unsigned char *)arg));

      len -= 270;
      arg += 270;
    }

    coap_insert(&optlist,
                new_option_node(COAP_OPTION_PROXY_URI,
                len,
                (unsigned char *)arg));

  } else {      /* split arg into Uri-* options */
    if (coap_split_uri((unsigned char *)arg, strlen(arg), &uri) < 0) {
      return -1;
    }

    if (uri.port != COAP_DEFAULT_PORT && create_uri_opts) {
      coap_insert(&optlist,
                  new_option_node(COAP_OPTION_URI_PORT,
                  coap_encode_var_bytes(portbuf, uri.port),
                  portbuf));
    }

    if (uri.path.length) {
      buflen = BUFSIZE;
      res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);

      while (res--) {
        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_PATH,
                    COAP_OPT_LENGTH(buf),
                    COAP_OPT_VALUE(buf)));

        buf += COAP_OPT_SIZE(buf);
      }
    }

    if (uri.query.length) {
      buflen = BUFSIZE;
      buf = _buf;
      res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);

      while (res--) {
        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_QUERY,
                    COAP_OPT_LENGTH(buf),
                    COAP_OPT_VALUE(buf)));

        buf += COAP_OPT_SIZE(buf);
      }
    }
  }

  return 0;
}

static int
resolve_address(const str *server, struct sockaddr *dst) {

  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  static char addrstr[256];
  int error, len=-1;

  memset(addrstr, 0, sizeof(addrstr));
  if (server->length)
    memcpy(addrstr, server->s, server->length);
  else
    memcpy(addrstr, "localhost", 9);

  memset ((char *)&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;

  error = getaddrinfo(addrstr, NULL, &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return error;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    switch (ainfo->ai_family) {
    case AF_INET6:
    case AF_INET:
      len = ainfo->ai_addrlen;
      memcpy(dst, ainfo->ai_addr, len);
      goto finish;
    default:
      ;
    }
  }

 finish:
  freeaddrinfo(res);
  return len;
}

static coap_pdu_t *
coap_new_request(coap_context_t *ctx,
                 method_t m,
                 coap_list_t **options,
                 unsigned char *data,
                 size_t length) {
  coap_pdu_t *pdu;
  coap_list_t *opt;

  if ( ! ( pdu = coap_new_pdu() ) )
    return NULL;

  pdu->hdr->type = msgtype;
  pdu->hdr->id = coap_new_message_id(ctx);
  pdu->hdr->code = m;

  pdu->hdr->token_length = the_token.length;
  if ( !coap_add_token(pdu, the_token.length, the_token.s)) {
    debug("cannot add token to request\n");
  }

  coap_show_pdu(pdu);

  if (options) {
    /* sort options for delta encoding */
    LL_SORT((*options), order_opts);

    LL_FOREACH((*options), opt) {
      coap_option *o = (coap_option *)(opt->data);
      coap_add_option(pdu,
                      COAP_OPTION_KEY(*o),
                      COAP_OPTION_LENGTH(*o),
                      COAP_OPTION_DATA(*o));
    }
  }

  if (length) {
    if ((flags & FLAGS_BLOCK) == 0)
      coap_add_data(pdu, length, data);
    else
      coap_add_block(pdu, length, data, block.num, block.szx);
  }

  return pdu;
}

static inline int
check_token(coap_pdu_t *received) {
  return received->hdr->token_length == the_token.length &&
    memcmp(received->hdr->token, the_token.s, the_token.length) == 0;
}

static void
message_handler(struct coap_context_t *ctx,
                const coap_endpoint_t *local_interface,
                const coap_address_t *remote,
                coap_pdu_t *sent,
                coap_pdu_t *received,
                const coap_tid_t id UNUSED_PARAM) {

  coap_pdu_t *pdu = NULL;
  coap_opt_t *block_opt;
  coap_opt_iterator_t opt_iter;
  unsigned char buf[4];
  coap_list_t *option;
  size_t len;
  unsigned char *databuf;
  coap_tid_t tid;
#ifdef OAD_TEST
  char addr[INET6_ADDRSTRLEN];
#endif

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("** process incoming %d.%02d response:\n",
          (received->hdr->code >> 5), received->hdr->code & 0x1F);
    coap_show_pdu(received);
  }
#endif

  /* check if this is a response to our original request */
  if (!check_token(received)) {
    /* drop if this was just some message, or send RST in case of notification */
    if (!sent && (received->hdr->type == COAP_MESSAGE_CON ||
                  received->hdr->type == COAP_MESSAGE_NON))
      coap_send_rst(ctx, local_interface, remote, received);
    return;
  }

  if (received->hdr->type == COAP_MESSAGE_RST) {
    info("got RST\n");
    return;
  }

  /* output the received data, if any */
  if (COAP_RESPONSE_CLASS(received->hdr->code) == 2) {

    /* set obs timer if we have successfully subscribed a resource */
    if (sent && coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter)) {
      debug("observation relationship established, set timeout to %d\n", obs_seconds);
      set_timeout(&obs_wait, obs_seconds);
    }

    /* Got some data, check if block option is set. Behavior is undefined if
     * both, Block1 and Block2 are present. */
    block_opt = coap_check_option(received, COAP_OPTION_BLOCK2, &opt_iter);
    if (block_opt) { /* handle Block2 */
      unsigned short blktype = opt_iter.type;

      /* TODO: check if we are looking at the correct block number */
      if (coap_get_data(received, &len, &databuf))
        append_to_output(databuf, len);

      if(COAP_OPT_BLOCK_MORE(block_opt)) {
        /* more bit is set */
        debug("found the M bit, block size is %u, block nr. %u\n",
              COAP_OPT_BLOCK_SZX(block_opt),
              coap_opt_block_num(block_opt));

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if ( pdu ) {
          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
                ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          debug("query block %d\n", (coap_opt_block_num(block_opt) + 1));
          coap_add_option(pdu,
                          blktype,
                          coap_encode_var_bytes(buf,
                                 ((coap_opt_block_num(block_opt) + 1) << 4) |
                                  COAP_OPT_BLOCK_SZX(block_opt)), buf);

          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      }
    } else { /* no Block2 option */
      block_opt = coap_check_option(received, COAP_OPTION_BLOCK1, &opt_iter);

      if (block_opt) { /* handle Block1 */
        unsigned int szx = COAP_OPT_BLOCK_SZX(block_opt);
        unsigned int num = coap_opt_block_num(block_opt);
        debug("found Block1 option, block size is %u, block nr. %u\n", szx, num);
        if (szx != block.szx) {
          unsigned int bytes_sent = ((block.num + 1) << (block.szx + 4));
          if (bytes_sent % (1 << (szx + 4)) == 0) {
            /* Recompute the block number of the previous packet given the new block size */
            block.num = (bytes_sent >> (szx + 4)) - 1;
            block.szx = szx;
            debug("new Block1 size is %u, block number %u completed\n", (1 << (block.szx + 4)), block.num);
          } else {
            debug("ignoring request to increase Block1 size, "
            "next block is not aligned on requested block size boundary. "
            "(%u x %u mod %u = %u != 0)\n",
                  block.num + 1, (1 << (block.szx + 4)), (1 << (szx + 4)),
                  bytes_sent % (1 << (szx + 4)));
          }
        }

        if (payload.length <= (block.num+1) * (1 << (block.szx + 4))) {
          debug("upload ready\n");
          ready = 1;
          return;
        }

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if (pdu) {

          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_CONTENT_FORMAT :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
              ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          block.num++;
          block.m = ((block.num+1) * (1 << (block.szx + 4)) < payload.length);

          debug("send block %d\n", block.num);
          coap_add_option(pdu,
                          COAP_OPTION_BLOCK1,
                          coap_encode_var_bytes(buf,
                          (block.num << 4) | (block.m << 3) | block.szx), buf);

          coap_add_block(pdu,
                         payload.length,
                         payload.s,
                         block.num,
                         block.szx);
          coap_show_pdu(pdu);
          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      } else {
        /* There is no block option set, just read the data and we are done. */
#ifdef OAD_TEST
        inet_ntop(remote->addr.sa.sa_family, &remote->addr.sin6.sin6_addr, addr, sizeof(addr));
        oad_log(LOG_DEBUG, "[NTF] rsp of %s = code %d id 0x%x\n", addr, received->hdr->code, received->hdr->id);
        oad_update_ntf(addr, received->hdr->id);
#endif
        if (coap_get_data(received, &len, &databuf))
        append_to_output(databuf, len);
      }
    }
  } else {      /* no 2.05 */

    /* check if an error was signaled and output payload if so */
    if (COAP_RESPONSE_CLASS(received->hdr->code) >= 4) {
      fprintf(stderr, "%d.%02d",
              (received->hdr->code >> 5), received->hdr->code & 0x1F);
      if (coap_get_data(received, &len, &databuf)) {
        fprintf(stderr, " ");
        while(len--)
        fprintf(stderr, "%c", *databuf++);
      }
      fprintf(stderr, "\n");
    }

  }

  /* finally send new request, if needed */
  if (pdu && coap_send(ctx, local_interface, remote, pdu) == COAP_INVALID_TID) {
    debug("message_handler: error sending response");
  }
  coap_delete_pdu(pdu);

  /* our job is done, we can exit at any time */
  ready = coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter) == NULL;
}

static
bool oad_need_terminated(uint32_t testDevices)
{
   uint16_t i;
   uint32_t devices;

   if (!testDevices)
      return false;

   devices = 0;   
   for (i = 0; i < testDevices; i++)
   {
      if (oad_deviceList[i].status & OAD_DEV_STATUS_DONE)
         devices++;
   }
   if (devices == testDevices)
      return true;
   else
      return false;
}

static bool oad_need_fwv(uint16_t devid)
{
  return (oad_deviceList[devid].status == OAD_DEV_STATUS_REG);
}

#define OAD_FWV "FWV"
static oad_status_t oad_send_fwv(coap_context_t *ctx, uint16_t devid)
{
  char addr[INET6_ADDRSTRLEN];
  void *addrptr = NULL;
  coap_address_t dst;
  coap_pdu_t *pdu;
  str server;
  unsigned short port = COAP_DEFAULT_PORT;
  coap_tid_t tid = COAP_INVALID_TID;
  int create_uri_opts = 1;
  int res;

  if(!(oad_deviceList[devid].status & OAD_DEV_STATUS_REG))
      return OAD_STATUS_NOREG;

  memset(cmd_str, 0, sizeof(cmd_str));
  sprintf(cmd_str, "coap://[%s]/oad/fwv",oad_deviceList[devid].addr);    
  /* this call allocates memory */
  cmdline_uri(cmd_str, create_uri_opts);

  payload.length = strlen(OAD_FWV);
  payload.s = (unsigned char *)OAD_FWV;

  server = uri.host;
  port = uri.port;

  /* resolve destination address where server should be sent */
  res = resolve_address(&server, &dst.addr.sa);
  
  if (res < 0) {
    fprintf(stderr, "failed to resolve address\n");
    /* free the allocated memory */
    coap_delete_list(optlist);
    optlist = NULL;
    exit(-1);
  }

  dst.size = res;
  dst.addr.sin.sin_port = htons(port);
  
  addrptr = &dst.addr.sin6.sin6_addr;

  /* construct CoAP message */
  if (!proxy.length && addrptr
      && (inet_ntop(dst.addr.sa.sa_family, addrptr, addr, sizeof(addr)) != 0)
      && (strlen(addr) != uri.host.length
      || memcmp(addr, uri.host.s, uri.host.length) != 0)
      && create_uri_opts) {
        /* add Uri-Host */

        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_HOST,
                    uri.host.length,
                    uri.host.s));
  }
  
  if (! (pdu = coap_new_request(ctx, method, &optlist, payload.s, payload.length)))
  {
    /* free the allocated memory */
    coap_delete_list(optlist);
    optlist = NULL;
    return OAD_STATUS_NOMEM;
  }

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("sending CoAP request:\n");
    coap_show_pdu(pdu);
  }
#endif

  oad_log(LOG_NOTICE, "[FWV] sending fwver req to %s\n", oad_deviceList[devid].addr);

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, ctx->endpoint, &dst, pdu);
  else
    tid = coap_send(ctx, ctx->endpoint, &dst, pdu);

  if (pdu->hdr->type != COAP_MESSAGE_CON || tid == COAP_INVALID_TID)
    coap_delete_pdu(pdu);
  
  oad_deviceList[devid].status |= OAD_DEV_STATUS_FWV;

  /* free the alloated memory */
  coap_delete_list(optlist);
  optlist = NULL;

  return OAD_STATUS_SUCCESS;
}

static void oad_timer_handler(int sig, siginfo_t *sigInfo, void *uc)
{
  uint16_t devid;
  timer_t *tidp = sigInfo->si_value.sival_ptr;
  (void) sig;
  (void) uc;

  for(devid = 0; devid < OAD_MAX_DEVICES; devid++)
  {
    if(*tidp == oad_deviceList[devid].timerId)
    {
      oad_deviceList[devid].status &= ~OAD_DEV_STATUS_NTFG;
      oad_log(LOG_ERR, "timer %d expired\n", devid);
      return;
    }
  }
}

static int oad_create_timer(int sigNo, timer_t *pTimerId, void (*handler)(int, siginfo_t *, void *))
{
  struct sigevent sev;
  struct sigaction sa;

  /* establish handler for timer siganl */
  oad_log(LOG_DEBUG, "establishing handler for signal %d\n", sigNo);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(sigNo, &sa, NULL) == -1)
  {
    oad_log(LOG_ERR, "sigacton failed\n");
    return OAD_STATUS_ERROR;
  }

  /* create the timer */
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = sigNo;
  sev.sigev_value.sival_ptr = pTimerId;
  if (timer_create(CLOCK_REALTIME, &sev, pTimerId) == -1)
  {
    oad_log(LOG_ERR, "timer_create failed\n");
    return OAD_STATUS_ERROR;
  }
  oad_log(LOG_DEBUG, "timer id is 0x%lx\n", (long)*pTimerId);

  return OAD_STATUS_SUCCESS;
}

static int oad_start_timer(timer_t timerId, int expire, int interval)
{
  struct itimerspec its;

  /* start the timer */
  its.it_value.tv_sec = expire;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = interval;
  its.it_interval.tv_nsec = 0;

  if (timer_settime(timerId, 0, &its, NULL) == -1)
  {
    oad_log(LOG_ERR, "timer settime failed\n");
    return OAD_STATUS_ERROR;
  }

  return OAD_STATUS_SUCCESS;
}

static bool oad_need_ntf(uint16_t devid)
{
  return (oad_deviceList[devid].status == (OAD_DEV_STATUS_REG));
}

static oad_status_t oad_send_ntf(coap_context_t *ctx, uint16_t devid, oad_imgNtfReq_t *oad_ntf_pdu)
{
  char addr[INET6_ADDRSTRLEN];
  void *addrptr = NULL;
  coap_address_t dst;
  coap_pdu_t *pdu;
  str server;
  unsigned short port = COAP_DEFAULT_PORT;
  coap_tid_t tid = COAP_INVALID_TID;
  int create_uri_opts = 1;
  int res;
  char imgId[OAD_IMG_ID_SIZE + 1]={0,};

  if(!(oad_deviceList[devid].status & OAD_DEV_STATUS_REG))
      return OAD_STATUS_NOREG;
  if((oad_deviceList[devid].status & OAD_DEV_STATUS_NTFG))
      return OAD_STATUS_NONTF;
  if((oad_deviceList[devid].status & OAD_DEV_STATUS_NTFD))
      return OAD_STATUS_NONTF;

  memset(cmd_str, 0, sizeof(cmd_str));
  sprintf(cmd_str, "coap://[%s]/oad/ntf",oad_deviceList[devid].addr);    
  /* this call allocates memory */
  cmdline_uri(cmd_str, create_uri_opts);

  payload.length = sizeof(oad_imgNtfReq_t);
  payload.s = (unsigned char *)oad_ntf_pdu;

  server = uri.host;
  port = uri.port;

  /* resolve destination address where server should be sent */
  res = resolve_address(&server, &dst.addr.sa);
  
  if (res < 0) {
    fprintf(stderr, "failed to resolve address\n");
    /* free the allocated memory */
    coap_delete_list(optlist);
    optlist = NULL;
    exit(-1);
  }

  dst.size = res;
  dst.addr.sin.sin_port = htons(port);
  
  addrptr = &dst.addr.sin6.sin6_addr;

  /* construct CoAP message */
  if (!proxy.length && addrptr
      && (inet_ntop(dst.addr.sa.sa_family, addrptr, addr, sizeof(addr)) != 0)
      && (strlen(addr) != uri.host.length
      || memcmp(addr, uri.host.s, uri.host.length) != 0)
      && create_uri_opts) {
        /* add Uri-Host */

        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_HOST,
                    uri.host.length,
                    uri.host.s));
  }
  
  if (! (pdu = coap_new_request(ctx, method, &optlist, payload.s, payload.length)))
  {
    /* free the allocated memory */
    coap_delete_list(optlist);
    optlist = NULL;
    return OAD_STATUS_NOMEM;
  }

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("sending CoAP request:\n");
    coap_show_pdu(pdu);
  }
#endif

  strncpy(imgId, (char *)oad_ntf_pdu->img_iid.imgID, 8);
  oad_log(LOG_NOTICE, "[NTF] sending notification id 0x%x of %s to %s\n", pdu->hdr->id, imgId, oad_deviceList[devid].addr);

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, ctx->endpoint, &dst, pdu);
  else
    tid = coap_send(ctx, ctx->endpoint, &dst, pdu);

  if (pdu->hdr->type != COAP_MESSAGE_CON || tid == COAP_INVALID_TID)
    coap_delete_pdu(pdu);
  
  oad_deviceList[devid].ntfId = pdu->hdr->id;
  if(!oad_deviceList[devid].timerId)
  {
    oad_create_timer(SIGALRM, &oad_deviceList[devid].timerId, oad_timer_handler);
  }
  oad_start_timer(oad_deviceList[devid].timerId, OAD_NTF_ACK_WAIT_TIME, 0);
  oad_deviceList[devid].status |= OAD_DEV_STATUS_NTFG;

  /* free the alloated memory */
  coap_delete_list(optlist);
  optlist = NULL;

  return OAD_STATUS_SUCCESS;
}

#endif

int
main(int argc, char **argv) {
  coap_context_t  *ctx;
  char *group = NULL;
  fd_set readfds;
  struct timeval tv, *timeout;
  int result;
  coap_tick_t now;
  coap_queue_t *nextpdu;
  char addr_str[NI_MAXHOST] = "::";
  char port_str[NI_MAXSERV] = "5683";
  char client_str[INET6_ADDRSTRLEN] = "";
  int opt;
#ifdef OAD_TEST
  oad_imgNtfReq_t oad_ntf_pdu[OAD_MAX_DEVICES];
  uint16_t i;
  uint32_t testDevices = 0;
  int fd;
  char *token;
  bool oad_fwv_control = false;
#endif

  coap_log_t log_level = LOG_WARNING;

  clock_offset = time(NULL);

  while ((opt = getopt(argc, argv, "A:g:p:v:c:d:e:t:")) != -1) {
    switch (opt) {
    case 'A' :
      strncpy(addr_str, optarg, NI_MAXHOST-1);
      addr_str[NI_MAXHOST - 1] = '\0';
      break;
    case 'g' :
      group = optarg;
      break;
    case 'p' :
      strncpy(port_str, optarg, NI_MAXSERV-1);
      port_str[NI_MAXSERV - 1] = '\0';
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    case 'c' :
      strncpy(client_str, optarg, INET6_ADDRSTRLEN-1);
      client_str[INET6_ADDRSTRLEN - 1] = '\0';
      break;
    case 'd' :
      strncpy(dirName, optarg, MAX_DIR_NAME_SIZE-1);
      dirName[MAX_DIR_NAME_SIZE - 1] = '\0';
      break;
    case 'e' :
      testDevices = strtol(optarg, NULL, 10);
      break;
    case 't' :
      testMode = strtol(optarg, NULL, 10);
      break;
    default:
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
    }
  }

  coap_set_log_level(log_level);

  ctx = get_context(addr_str, port_str);
  if (!ctx)
    return -1;

  init_resources(ctx);

  /* join multicast group if requested at command line */
  if (group)
    join(ctx, group);

  signal(SIGINT, handle_sigint);
  
#ifdef OAD_TEST
  if(!strlen(dirName))
  {
    strncpy(dirName, DIRNAME, MAX_DIR_NAME_SIZE-1);
    dirName[MAX_DIR_NAME_SIZE - 1] = '\0';
  }
  token = strrchr(dirName, '/');
  if(strlen(token + 1))
  {
    strcat(dirName, "/");
  }
    
  if(testMode)
  {
    oad_log(LOG_INFO, "test mode enabled\n");
  }

  memset(oad_fileList, 0, OAD_MAX_FILES * sizeof(oad_fileInfo_t));
  if(strlen(client_str))
  {
      oad_update_list(client_str, NULL);
  }

  oad_read_directory(dirName);
  fd = inotify_init();
  /* IN_CREATE|IN_DELETE|IN_MODIFY|IN_CLOSE_WRITE|IN_MOVE */
  inotify_add_watch(fd, dirName, IN_DELETE|IN_CREATE|IN_CLOSE_WRITE|IN_MOVE);

  coap_register_option(ctx, COAP_OPTION_BLOCK2);
  coap_register_response_handler(ctx, message_handler);
#endif

  while ( !quit ) {
#ifdef OAD_TEST
    for(i = 0; i < OAD_MAX_DEVICES; i++)
    {
      if (oad_fwv_control && oad_need_fwv(i))
      {
        oad_send_fwv(ctx, i);
      }
    }

    for(i = 0; i < OAD_MAX_DEVICES; i++)
    {
      if (oad_need_ntf(i) && oad_build_ntf(i, &oad_ntf_pdu[i]))
      {
        oad_send_ntf(ctx, i, &oad_ntf_pdu[i]);
      }
    }

    if (oad_need_terminated(testDevices))
    {
      quit = 1;
      continue;
    }
#endif
    
    FD_ZERO(&readfds);
    FD_SET( ctx->sockfd, &readfds );
#ifdef OAD_TEST
    FD_SET( fd, &readfds);
#endif
    nextpdu = coap_peek_next( ctx );

    coap_ticks(&now);
    while (nextpdu && nextpdu->t <= now - ctx->sendqueue_basetime) {
      coap_retransmit( ctx, coap_pop_next( ctx ) );
      nextpdu = coap_peek_next( ctx );
    }

    if (nextpdu && nextpdu->t <= (COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND)) {
      /* set timeout if there is a pdu to send before our automatic timeout occurs */
      tv.tv_usec = ((nextpdu->t) % COAP_TICKS_PER_SECOND) * 1000000 / COAP_TICKS_PER_SECOND;
      tv.tv_sec = (nextpdu->t) / COAP_TICKS_PER_SECOND;
      timeout = &tv;
    } else {
      tv.tv_usec = 0;
      tv.tv_sec = COAP_RESOURCE_CHECK_TIME;
      timeout = &tv;
    }
    result = select( FD_SETSIZE, &readfds, 0, 0, timeout );

    if ( result < 0 ) {         /* error */
      if (errno != EINTR)
        perror("select");
    } else if ( result > 0 ) {  /* read from socket */
      if ( FD_ISSET( ctx->sockfd, &readfds ) ) {
        coap_read( ctx );       /* read received data */
        /* coap_dispatch( ctx );  /\* and dispatch PDUs from receivequeue *\/ */
      }
#ifdef OAD_TEST      
      else if (FD_ISSET( fd, &readfds ))
      {
        oad_read_image(fd);
      }
      else
      {
        /* do command line processing in the future purpose */
      }
#endif      
    } else {      /* timeout */
      if (time_resource) {
        time_resource->dirty = 1;
      }
    }

#ifndef WITHOUT_ASYNC
    /* check if we have to send asynchronous responses */
    check_async(ctx, ctx->endpoint, now);
#endif /* WITHOUT_ASYNC */

#ifndef WITHOUT_OBSERVE
    /* check if we have to send observe notifications */
    coap_check_notify(ctx);
#endif /* WITHOUT_OBSERVE */
  }

  coap_free_context(ctx);

  return 0;
}
