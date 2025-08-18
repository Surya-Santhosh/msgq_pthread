//******************************* common ***************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved 
//****************************************************************************** 
// 
// Summary : Typedefs are included.
// Note    : None
// 
//******************************************************************************
#ifndef COMMON_H
#define COMMON_H 

//**************************** Include Files ***********************************
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>

//**************************** Global Types ************************************
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef struct TASK_STATUS
{
    // Conditional variables.
    pthread_mutex_t stPollerToTransportMutex;
    pthread_mutex_t stTransportToLoggerMutex;
    pthread_mutex_t stLoggerToTransportMutex;
    pthread_mutex_t stTransportToPollerMutex;
    pthread_cond_t stRequestFromPoller;
    pthread_cond_t stReqestFromTransport;
    pthread_cond_t stAckFromTransport;
    pthread_cond_t stAckFromLogger;

    // Message queue
    mqd_t RequestPoller;
    mqd_t ackTransport;
    mqd_t RequestTransport;
    mqd_t ackLogger;

    bool blRequestFlagPoller;
    bool blRequestFlagTransportToLogger;
    bool blRequestFlagTransportToPoller;
    bool blRequestFlagLogger;

    bool blAckFlagTransport;
    bool blAckFlagLogger;
}TASK_STATUS;

//************************* Global Constants ***********************************               
#define MSGQ_POLLER_TO_TRANSPORT  ("/Poller_to_Transport")
#define MSGQ_TRANSPORT_TO_LOGGER  ("/Transport_to_logger")
#define MSGQ_TRANSPORT_TO_POLLER  ("/Transport_to_poller")
#define MSGQ_LOGGER_TO_TRANSPORT  ("/logger_to_transport")
#define MAX_MESSAGE               (10) 
#define CMD_ACK                   (0)
#define CMD_GET                   (1)
#define CMD_SET                   (2)
#define STATE_OK                  (0)
#define STATE_ERROR               (1)
#define GPIO_ON                   (1)
#define GPIO_OFF                  (0)
#define FAILURE_CASE              (-1)
#define NEWLINE_CHARACTER         ('\n')
#define SPACE_CHARACTER           (' ')

//************************* Global Variables *********************************** 

//************************* Forward Declarations *******************************  

//************************ Inline Method Implementations *********************** 

#endif 

// COMMON_H 
// EOF