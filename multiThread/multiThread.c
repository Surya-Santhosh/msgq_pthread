//******************************* multiThread ***********************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : add.c
// Summary : Add two numbers.
// Note    : None
// Author  : Surya Santhosh
// Day     : 04/Aug/2025
//******************************************************************************

//**************************** Include Files ***********************************
#include "multiThread.h"

//******************************* Local Types **********************************

//***************************** Local Constants ********************************

//**************************** Local Variables *********************************

//***************************** Local Functions ********************************
static bool multiThreadDestroy(TASK_STATUS *stTaskStatus);
static bool multiThreadInit(TASK_STATUS *stTaskStatus);
static bool multiThreadPoller(TASK_STATUS *stTaskStatus);
static bool multiThreadTransport(TASK_STATUS *stTaskStatus);
static bool multiThreadLogger(TASK_STATUS *stTaskStatus);
static bool multiThreadMessageQUnlink();
static bool multiThreadmsgqSend(mqd_t *message, const char* pcBuffer, 
                                char *pcMsgqFileName, uint32 pcMsgSize);
static bool multiThreadmsgqRecieve(mqd_t *message, const char* pcBuffer, 
                                   char *pcMsgqFileName, uint32 pcMsgSize);

//**************************.multiThreadPoller.*********************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadPoller(TASK_STATUS *stTaskStatus)
{
    REQUEST stRequest = {0};
    ACK stRecievedAck = {0};

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    printf("Enter any Key:\n");

    if (-1 != getchar())
    {
        stRequest.ucCMD = CMD_SET;
    }

    if (true != multiThreadmsgqSend(&stTaskStatus->msgPoller, 
                                    (const char *)&stRequest, 
                                     MSGQ_POLLER_TO_TRANSPORT, sizeof(REQUEST)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        stTaskStatus->blSendFlagPoller = true;

        printf("GPIO is high.\n");
    }

    pthread_cond_signal (&(stTaskStatus)->stMsgFromPoller);

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    //wait for ack from transport.
    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    while (true != stTaskStatus->blSendFlagTransportToPoller)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->ackTransport, 
                                       (const char *)&stRecievedAck, 
                                       MSGQ_TRANSPORT_TO_POLLER, 
                                       sizeof(ACK)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("Recieved acknowledgment from Transport.\n");
    }

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    return true;
}

//***************************.multiThreadTransport.*****************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadTransport(TASK_STATUS *stTaskStatus)
{
    ACK stRecievedAck = {0};
    REQUEST stRecievedRequest = {0};

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for message from poller.
    while (true != stTaskStatus->blSendFlagPoller)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromPoller, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->msgPoller, 
                                       (const char *)&stRecievedRequest, 
                                       MSGQ_POLLER_TO_TRANSPORT, 
                                       sizeof(REQUEST)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        stTaskStatus->blReceiveFlagTransport = true;

        printf ("Recieved message from Poller.\n");
    }

    // Send msg to logger.
    // stTaskHandler.blTransport = true;

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    if (true != multiThreadmsgqSend(&stTaskStatus->msgTransport, 
                                    (const char *)&stRecievedRequest, 
                                     MSGQ_TRANSPORT_TO_LOGGER, sizeof(REQUEST)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        stTaskStatus->blSendFlagTransportToLogger = true;

        printf("Send message from Transport to Logger.\n");
    }

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    //wait for ack from logger.
    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }
    while (true != stTaskStatus->blSendFlagLogger)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromLogger, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->ackLogger, 
                                       (const char *)&stRecievedAck, 
                                       MSGQ_LOGGER_TO_TRANSPORT, sizeof(ACK)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("Recieved acknowledgment from Logger.\n");
    }

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    // Send ack to poller.
    pthread_cond_signal (&(stTaskStatus)->stAckFromTransport);

    if (true != multiThreadmsgqSend(&stTaskStatus->ackTransport, 
                                    (const char *)&stRecievedAck, 
                                     MSGQ_TRANSPORT_TO_POLLER, sizeof(ACK)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        stTaskStatus->blSendFlagTransportToPoller = true;

        printf("Send acknowledgment from Transport to poller.\n");
    }
    
    return true;
}

//**************************.multiThreadLogger.*********************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadLogger(TASK_STATUS *stTaskStatus)
{
    ACK stAck = {0};
    REQUEST stRecievedRequest = {0};

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for Transport variable.
    while (true != stTaskStatus->blSendFlagTransportToLogger)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    if (true != multiThreadmsgqRecieve(&stTaskStatus->msgTransport, 
                                       (const char *)&stRecievedRequest, 
                                       MSGQ_TRANSPORT_TO_LOGGER, 
                                       sizeof(REQUEST)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        stTaskStatus->blReceiveFlagLogger = true;

        printf("Recieved message from Transport.\n");
    }

    // Send ack to Transport.
    // stTaskHandler.blLoggerAck = true;
    if (GPIO_ON == stRecievedRequest.ucData)
    {
        printf("LED ON\n");

        stAck.ucCMD = CMD_ACK;
        stAck.ucSTATE = STATE_OK;
        stAck.ucData = GPIO_ON;
    }
    else
    {
        stAck.ucCMD = CMD_ACK;
        stAck.ucSTATE = STATE_ERROR;
        stAck.ucData = GPIO_OFF;
    }

    if (true != multiThreadmsgqSend(&stTaskStatus->ackLogger, 
                                    (const char *)&stAck, 
                                     MSGQ_LOGGER_TO_TRANSPORT, sizeof(ACK)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        stTaskStatus->blSendFlagLogger = true;

        printf("Send acknowledgment from Logger to transport.\n");
    }

    pthread_cond_signal(&(stTaskStatus)->stAckFromLogger);

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    return true;
}

//****************************.multiThreadSetUp.********************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
bool multiThreadSetUp()
{
    pthread_t ulPoller = 0;
    pthread_t ulTransport = 0;
    pthread_t ulLogger = 0;
    TASK_STATUS stTaskStatus = {0};

    if (0 != pthread_mutex_init (&stTaskStatus.stMutex, NULL))
    {
        perror ("pthread_mutex_init");
    }

    if (true != multiThreadInit(&stTaskStatus))
    {
        perror("multiThreadInit");
    }

    pthread_create (&ulPoller, NULL, (void *)multiThreadPoller, &stTaskStatus);
    pthread_create (&ulTransport, NULL, (void *)multiThreadTransport, 
                    &stTaskStatus);
    pthread_create (&ulLogger, NULL, (void *)multiThreadLogger, &stTaskStatus);

    pthread_join (ulPoller, NULL);
    pthread_join (ulTransport, NULL);
    pthread_join (ulLogger, NULL);

    if (true != multiThreadDestroy(&stTaskStatus))
    {
        perror("multiThreadDestroy");
    }

    if(true != multiThreadMessageQUnlink())
    {
        perror ("multiThreadMessageQUnlink");
    }

    if (0 != pthread_mutex_destroy (&stTaskStatus.stMutex))
    {
        perror ("pthread_mutex_destroy");
    }

    return true;
}

//****************************.multiThreadInit.*********************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadInit(TASK_STATUS *stTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_cond_init(&(stTaskStatus)->stMsgFromPoller, NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(stTaskStatus)->stMsgFromTransport, NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(stTaskStatus)->stAckFromTransport, NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(stTaskStatus)->stAckFromLogger, NULL)) 
        {
            perror("pthread_cond_init");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//***************************.multiThreadDestroy.*******************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadDestroy(TASK_STATUS *stTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_cond_destroy (&(stTaskStatus)->stMsgFromPoller))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_cond_destroy (&(stTaskStatus)->stMsgFromTransport))
        {
            perror ("pthread_mutex_destroy");
        }

       if (0 != pthread_cond_destroy (&(stTaskStatus)->stAckFromTransport))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_cond_destroy (&(stTaskStatus)->stAckFromLogger))
        {
            perror ("pthread_mutex_destroy");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;

}

//*************************.multiThreadMessageQUnlink.**************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadMessageQUnlink()
{
    bool blResult = false;

    do 
    {
        if (0 != mq_unlink(MSGQ_POLLER_TO_TRANSPORT))
        {
            perror ("mq_unlink");
        }
        
        if (0 != mq_unlink(MSGQ_TRANSPORT_TO_LOGGER))
        {
            perror ("mq_unlink");
        }

        if (0 != mq_unlink(MSGQ_TRANSPORT_TO_POLLER))
        {
            perror ("mq_unlink");
        }

        if (0 != mq_unlink(MSGQ_LOGGER_TO_TRANSPORT))
        {
            perror ("mq_unlink");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//*************************.multiThreadMessageQUnlink.**************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqSend(mqd_t *message, const char* pcBuffer, 
                                char *pcMsgqFileName, uint32 pcMsgSize)
{
    bool blResult = false;
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE;
    attr.mq_msgsize = pcMsgSize;
    attr.mq_curmsgs = 0;

    do
    {
        *message = mq_open(pcMsgqFileName, O_CREAT | O_RDWR, 0644, &attr);

        if (-1 == *message)
        {
            perror("mq_open");
        }

        if (-1 == mq_send(*message, pcBuffer, pcMsgSize, 0))
        {
            perror ("mq_send");
        }

        if (0 != mq_close(*message))
        {
            perror ("mq_close.");
        }

        blResult = true;
        
    }while (true != blResult);
    
    return blResult;
}

//*************************.multiThreadMessageQUnlink.**************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqRecieve(mqd_t *message, const char* pcBuffer, 
                                   char *pcMsgqFileName, uint32 pcMsgSize)
{
    bool blResult = false;
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE;
    attr.mq_msgsize = pcMsgSize;
    attr.mq_curmsgs = 0;

    do
    {
        *message = mq_open(pcMsgqFileName, O_CREAT | O_RDWR, 0644, &attr);
                                         
        if (-1 == *message)
        {                           
            perror("mq_open");
        }

        if (-1 == mq_receive(*message, (char *)&pcBuffer, pcMsgSize, NULL))
        {
            perror ("mq_receive.");
        }
        else
        {
            printf("Recieved acknowledgment from Transport.\n");
        }

        if (0 != mq_close(*message))
        {
            perror ("mq_close.");
        }

        blResult = true;
        
    }while (true != blResult);
    
    return blResult;
}


// EOF