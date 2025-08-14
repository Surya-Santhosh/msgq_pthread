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
typedef char int8;
typedef unsigned short uint16;
typedef unsigned long uint32;

// Request data.
typedef struct REQUEST
{
    uint32 ucUID;
    int8 ucCMD;
    uint32 ucData;
}REQUEST;

// Acknowledgment data.
typedef struct ACK
{
    uint32 ucUID;
    int8 ucCMD;
    int8 ucSTATE;
    uint32 ucData;
}ACK;

typedef struct TASK_STATUS
{
    // Conditional variables.
    pthread_mutex_t stMutex;
    pthread_cond_t stMsgFromPoller;
    pthread_cond_t stMsgFromTransport;
    pthread_cond_t stAckFromTransport;
    pthread_cond_t stAckFromLogger;

    // Message queue
    mqd_t msgPoller;
    mqd_t ackTransport;
    mqd_t msgTransport;
    mqd_t ackLogger;

    bool blSendFlagPoller;
    bool blSendFlagTransportToLogger;
    bool blSendFlagTransportToPoller;
    bool blSendFlagLogger;

    bool blReceiveFlagTransport;
    bool blReceiveFlagLogger;
}TASK_STATUS;

//************************* Global Constants ***********************************
#define MAX_ITREATION             (5)
#define INCREMENT_COUNT           (20)
#define DECREMENT_COUNT           (30)
#define MAX_SIZE                  (50)
#define MAX_ARRAYSIZE             (5)
#define MAX_MESSAGE               (10)                
#define MSGQ_POLLER_TO_TRANSPORT  ("/Poller_to_Transport")
#define MSGQ_TRANSPORT_TO_LOGGER  ("/Transport_to_logger")
#define MSGQ_TRANSPORT_TO_POLLER  ("/Transport_to_poller")
#define MSGQ_LOGGER_TO_TRANSPORT  ("/logger_to_transport")
#define CMD_ACK                   (0)
#define CMD_GET                   (1)
#define CMD_SET                   (2)
#define STATE_OK                  (0)
#define STATE_ERROR               (1)
#define GPIO_ON                   (1)
#define GPIO_OFF                  (0)

//************************* Global Variables *********************************** 

//************************* Forward Declarations *******************************  

//************************ Inline Method Implementations *********************** 

#endif 

// COMMON_H 
// EOF