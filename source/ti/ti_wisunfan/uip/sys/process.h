/*
 * process.h
 *
 *  Created on: Mar 13, 2015
 *      Author: x0224946
 *
 *      This process object will be the representation of any process running (tcpip process, TCP socket process, CoAP process, etc)
 *      It has a callback function that needs to be implemented to switch contexts for that process. That function will be called when it is necessary inside the uIP Stack
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include "uip-conf.h"

typedef void (*process_post_func_t)(process_event_t event, process_data_t data);

typedef struct process_obj
{
    process_post_func_t process_post;
} process_obj_t;

/* We define this function as a macro for performance purposes
 *
 void process_obj_init(process_obj_t *process_obj, process_post_func_t process_post_func);
 */
#define process_obj_init(process_obj,process_post_func)	((process_obj)->process_post=process_post_func)

#endif /* PROCESS_H_ */
