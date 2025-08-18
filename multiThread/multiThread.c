//******************************* multiThread ***********************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : multiThread.c
// Summary : Create threads (Poller, Transport, Logger) send Request and give
//           Ackowledgment using message queue to turn LED ON using mutex and
//           conditional variables for synchronisation.
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
static bool multiThreadDestroyMutex(TASK_STATUS *pstTaskStatus);
static bool multiThreadDestroyCond(TASK_STATUS *pstTaskStatus);
static bool multiThreadInitMutex(TASK_STATUS *pstTaskStatus);
static bool multiThreadInitCond(TASK_STATUS *pstTaskStatus);
static bool multiThreadPoller(TASK_STATUS *pstTaskStatus);
static bool multiThreadTransport(TASK_STATUS *pstTaskStatus);
static bool multiThreadLogger(TASK_STATUS *pstTaskStatus);
static bool multiThreadMessageQUnlink();
static bool multiThreadmsgqOpen(TASK_STATUS *pstTaskStatus);
static bool multiThreadmsgqClose(TASK_STATUS *pstTaskStatus);

//**************************.multiThreadPoller.*********************************
// Purpose : Poller Thread - wait for key press (GPIO High), send Request 
//           message to transport through message queue. And wait for the 
//           acknowledgement from Transport.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadPoller(TASK_STATUS *pstTaskStatus)
{
    REQUEST stRequest = {0};
    ACK stRecievedAck = {0};
    static uint8 ucCount = 0;
    uint8 ucKey = 0;
    ucCount++;

    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stPollerToTransportMutex))
    {
        perror("pthread_mutex_lock");
    }

    printf("Enter any Key:\n");
    scanf("%c", &ucKey);

    while (NEWLINE_CHARACTER != getchar());

    if (SPACE_CHARACTER != ucKey)
    {
        stRequest.ucData = GPIO_ON;
        stRequest.ucCMD = CMD_SET;
    }
    else
    {
        stRequest.ucData = GPIO_OFF;
    }

    stRequest.ucUID = ucCount;

    if (true != multiThreadmsgqSend(&pstTaskStatus->RequestPoller, 
                                    (const char *)&stRequest))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        printf("UID : %d\n", stRequest.ucUID);

        pstTaskStatus->blRequestFlagPoller = true;

        if (GPIO_ON == stRequest.ucData)
        {
            printf("POLLER : GPIO is high.\n");
        }
        else
        {
            printf("POLLER : GPIO is Low.\n");
        }
    }

    pthread_cond_signal (&(pstTaskStatus)->stRequestFromPoller);

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stPollerToTransportMutex))
    {
        perror("pthread_mutex_unlock");
    }

    //wait for ack from transport.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stTransportToPollerMutex))
    {
        perror("pthread_mutex_lock");
    }

    while (true != pstTaskStatus->blRequestFlagTransportToPoller)
    {
        pthread_cond_wait (&(pstTaskStatus)->stAckFromTransport, 
                           &(pstTaskStatus)->stTransportToPollerMutex);
    }

    if (true != multiThreadmsgqRecieve(&pstTaskStatus->ackTransport, 
                                       (char *)&stRecievedAck))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("POLLER : Recieved Ack.\n");
    }

    if (STATE_ERROR != stRecievedAck.ucSTATE)
    {
        printf("UID : %d success\n\n", stRecievedAck.ucUID);
    }
    else
    {
        printf("UID : %d Failed\n\n", stRecievedAck.ucUID);
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stTransportToPollerMutex))
    {
        perror("pthread_mutex_unlock");
    }

    return true;
}

//**************************.multiThreadTransport.******************************
// Purpose : Transport Thread - wait for message from Poller, forward to Logger  
//           and wait for the acknowledgement from Logger, forward to Poller. 
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadTransport(TASK_STATUS *pstTaskStatus)
{
    ACK stRecievedAck = {0};
    REQUEST stRecievedRequest = {0};

    //wait for message from poller.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stPollerToTransportMutex))
    {
        perror("pthread_mutex_lock");
    }

    while (true != pstTaskStatus->blRequestFlagPoller)
    {
        pthread_cond_wait (&(pstTaskStatus)->stRequestFromPoller, 
                           &(pstTaskStatus)->stPollerToTransportMutex);
    }

    if (true != multiThreadmsgqRecieve(&pstTaskStatus->RequestPoller, 
                                       (char *)&stRecievedRequest))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        pstTaskStatus->blAckFlagTransport = true;

        printf ("TRANSPORT : Recieved message.\n");
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stPollerToTransportMutex))
    {
        perror("pthread_mutex_unlock");
    }

    // Send msg to logger.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stTransportToLoggerMutex))
    {
        perror("pthread_mutex_lock");
    }

    pthread_cond_signal (&(pstTaskStatus)->stReqestFromTransport);

    if (true != multiThreadmsgqSend(&pstTaskStatus->RequestTransport, 
                                    (const char *)&stRecievedRequest))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        pstTaskStatus->blRequestFlagTransportToLogger = true;

        printf("TRANSPORT : Send message.\n");
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stTransportToLoggerMutex))
    {
        perror("pthread_mutex_unlock");
    }

    //wait for ack from logger.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stLoggerToTransportMutex))
    {
        perror("pthread_mutex_lock");
    }

    while (true != pstTaskStatus->blRequestFlagLogger)
    {
        pthread_cond_wait (&(pstTaskStatus)->stAckFromLogger, 
                           &(pstTaskStatus)->stLoggerToTransportMutex);
    }

    if (true != multiThreadmsgqRecieve(&pstTaskStatus->ackLogger, 
                                       (char *)&stRecievedAck))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("TRANSPORT : Recieved Ack.\n");
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stLoggerToTransportMutex))
    {
        perror("pthread_mutex_unlock");
    }

    // Send ack to poller.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stTransportToPollerMutex))
    {
        perror("pthread_mutex_lock");
    }

    pthread_cond_signal (&(pstTaskStatus)->stAckFromTransport);

    if (true != multiThreadmsgqSend(&pstTaskStatus->ackTransport, 
                                    (const char *)&stRecievedAck))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        pstTaskStatus->blRequestFlagTransportToPoller = true;

        printf("TRANSPORT : Send Ack.\n");
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stTransportToPollerMutex))
    {
        perror("pthread_mutex_unlock");
    }
    
    return true;
}

//****************************.multiThreadLogger.*******************************
// Purpose : Logger Thread - wait for message from Transport, turns LED ON/OFF    
//           send acknowledgement to Transpot. 
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadLogger(TASK_STATUS *pstTaskStatus)
{
    ACK stAck = {0};
    REQUEST stRecievedRequest = {0};

    //wait for Transport variable.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stTransportToLoggerMutex))
    {
        perror("pthread_mutex_lock");
    }

    while (true != pstTaskStatus->blRequestFlagTransportToLogger)
    {
        pthread_cond_wait (&(pstTaskStatus)->stReqestFromTransport, 
                           &(pstTaskStatus)->stTransportToLoggerMutex);
    }

    if (true != multiThreadmsgqRecieve(&pstTaskStatus->RequestTransport, 
                                       (char *)&stRecievedRequest))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        pstTaskStatus->blAckFlagLogger = true;

        printf("LOGGER : Recieved message.\n");
    }

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stTransportToLoggerMutex))
    {
        perror("pthread_mutex_unlock");
    }

    // Send ack to Transport.
    if (0 != pthread_mutex_lock (&(pstTaskStatus)->stLoggerToTransportMutex))
    {
        perror("pthread_mutex_lock");
    }

    if (CMD_SET == stRecievedRequest.ucCMD)
    {
        printf("LED ON\n");
        printf("Data : %04x\n", stRecievedRequest.ucData);

        stAck.ucSTATE = STATE_OK;
        stAck.ucData = GPIO_ON;
    }
    else
    {
        printf("LED OFF\n");
        printf("Data : %04x\n", stRecievedRequest.ucData);
        stAck.ucSTATE = STATE_ERROR;
        stAck.ucData = GPIO_OFF;
    }

    stAck.ucCMD = CMD_ACK;
    stAck.ucUID = stRecievedRequest.ucUID;

    if (true != multiThreadmsgqSend(&pstTaskStatus->ackLogger, 
                                    (const char *)&stAck))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        pstTaskStatus->blRequestFlagLogger = true;

        printf("LOGGER : Send Ack.\n");
    }

    pthread_cond_signal(&(pstTaskStatus)->stAckFromLogger);

    if (0 != pthread_mutex_unlock(&(pstTaskStatus)->stLoggerToTransportMutex))
    {
        perror("pthread_mutex_unlock");
    }

    return true;
}

//*****************************.multiThreadSetUp.*******************************
// Purpose : Initialize 3 threads (Poller, Transport, and Logger) and joins 
//           them, Initialize and destroy mutex and conditional variables,
//           Unlink message queue.
// Inputs  : None
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
bool multiThreadSetUp()
{
    bool blResult = false;
    pthread_t ulPoller = 0;
    pthread_t ulTransport = 0;
    pthread_t ulLogger = 0;
    TASK_STATUS pstTaskStatus = {0};

    do 
    {
        if (true != multiThreadInitMutex(&pstTaskStatus))
        {
            perror("multiThreadInit");
        }

        if (true != multiThreadInitCond(&pstTaskStatus))
        {
            perror("multiThreadInit");
        }

        if (true != multiThreadmsgqOpen(&pstTaskStatus))
        {
            perror ("multiThreadmsgqOpen");
        }

        pthread_create (&ulPoller, NULL, (void *)multiThreadPoller, 
                        &pstTaskStatus);
        pthread_create (&ulTransport, NULL, (void *)multiThreadTransport, 
                        &pstTaskStatus);
        pthread_create (&ulLogger, NULL, (void *)multiThreadLogger, 
                        &pstTaskStatus);

        pthread_join (ulPoller, NULL);
        pthread_join (ulTransport, NULL);
        pthread_join (ulLogger, NULL);

        if (true != multiThreadDestroyCond(&pstTaskStatus))
        {
            perror("multiThreadDestroy");
        }

        if (true != multiThreadmsgqClose(&pstTaskStatus))
        {
            perror ("multiThreadmsgqClose");
        }

        if(true != multiThreadMessageQUnlink())
        {
            perror ("multiThreadMessageQUnlink");
        }

        if (true != multiThreadDestroyMutex(&pstTaskStatus))
        {
            perror("multiThreadDestroy");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//**************************.multiThreadInitCond.*******************************
// Purpose : Initialize conditional variables.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadInitCond(TASK_STATUS *pstTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_cond_init(&(pstTaskStatus)->stRequestFromPoller, NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(pstTaskStatus)->stReqestFromTransport, 
                                   NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(pstTaskStatus)->stAckFromTransport, NULL)) 
        {
            perror("pthread_cond_init");
        }

        if (0 != pthread_cond_init(&(pstTaskStatus)->stAckFromLogger, NULL)) 
        {
            perror("pthread_cond_init");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//**************************.multiThreadInitMutex.******************************
// Purpose : Initialize Mutex.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadInitMutex(TASK_STATUS *pstTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_mutex_init (&(pstTaskStatus)->stPollerToTransportMutex, 
                                     NULL))
        {
            perror ("pthread_mutex_init");
        }

        if (0 != pthread_mutex_init (&(pstTaskStatus)->stTransportToLoggerMutex, 
                                     NULL))
        {
            perror ("pthread_mutex_init");
        }

        if (0 != pthread_mutex_init (&(pstTaskStatus)->stLoggerToTransportMutex, 
                                     NULL))
        {
            perror ("pthread_mutex_init");
        }

        if (0 != pthread_mutex_init (&(pstTaskStatus)->stTransportToPollerMutex, 
                                     NULL))
        {
            perror ("pthread_mutex_init");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//*************************.multiThreadDestroyCond.*****************************
// Purpose : Destroy conditional variables.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadDestroyCond(TASK_STATUS *pstTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_cond_destroy (&(pstTaskStatus)->stRequestFromPoller))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_cond_destroy (&(pstTaskStatus)->stReqestFromTransport))
        {
            perror ("pthread_mutex_destroy");
        }

       if (0 != pthread_cond_destroy (&(pstTaskStatus)->stAckFromTransport))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_cond_destroy (&(pstTaskStatus)->stAckFromLogger))
        {
            perror ("pthread_mutex_destroy");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//*************************.multiThreadDestroyMutex.****************************
// Purpose : Destroy conditional variables.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadDestroyMutex(TASK_STATUS *pstTaskStatus)
{ 
    bool blResult = false;

    do 
    {
        if (0 != pthread_mutex_destroy
                 (&(pstTaskStatus)->stPollerToTransportMutex))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_mutex_destroy 
                 (&(pstTaskStatus)->stTransportToLoggerMutex))
        {
            perror ("pthread_mutex_destroy");
        }

       if (0 != pthread_mutex_destroy 
                (&(pstTaskStatus)->stLoggerToTransportMutex))
        {
            perror ("pthread_mutex_destroy");
        }

        if (0 != pthread_mutex_destroy 
                 (&(pstTaskStatus)->stTransportToPollerMutex))
        {
            perror ("pthread_mutex_destroy");
        }

        blResult = true;

    }while (true != blResult);

    return blResult;
}

//*************************.multiThreadMessageQUnlink.**************************
// Purpose : Unlink message queue.
// Inputs  : None
// Outputs : None
// Return  : blResult
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

//****************************.multiThreadmsgqOpen.*****************************
// Purpose : Open message queue. 
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqOpen(TASK_STATUS *pstTaskStatus)
{
    bool blResult = false;
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE;
    attr.mq_curmsgs = 0;

    do
    {
        attr.mq_msgsize = sizeof(ACK); 

        pstTaskStatus->RequestPoller = mq_open(MSGQ_POLLER_TO_TRANSPORT, 
                                           O_CREAT | O_RDWR, 0644, &attr);                            
        if (FAILURE_CASE == pstTaskStatus->RequestPoller)
        {                           
            perror("mq_open");
        }

        pstTaskStatus->ackTransport = mq_open(MSGQ_TRANSPORT_TO_LOGGER,
                                              O_CREAT | O_RDWR, 0644, &attr); 
        if (FAILURE_CASE == pstTaskStatus->ackTransport)
        {                           
            perror("mq_open");
        }

        attr.mq_msgsize = sizeof(REQUEST); 

        pstTaskStatus->RequestTransport = mq_open(MSGQ_LOGGER_TO_TRANSPORT, 
                                              O_CREAT | O_RDWR, 0644, &attr); 
        if (FAILURE_CASE == pstTaskStatus->RequestTransport)
        {                           
            perror("mq_open");
        }

        pstTaskStatus->ackLogger = mq_open(MSGQ_TRANSPORT_TO_POLLER, 
                                           O_CREAT | O_RDWR, 0644, &attr); 
        if (FAILURE_CASE == pstTaskStatus->ackLogger)
        {                           
            perror("mq_open");
        }

        blResult = true;
        
    }while (true != blResult);
    
    return blResult;
}

//***************************.multiThreadmsgqClose.*****************************
// Purpose : Close message queue.
// Inputs  : pstTaskStatus - Pointer to TASK_STATUS struct containg message
//           queue descriptor, mutex, conditional variables and status flags.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqClose(TASK_STATUS *pstTaskStatus)
{
    bool blResult = false;

    do
    {
        if (0 != mq_close(pstTaskStatus->RequestPoller))
        {
            perror ("mq_close.");
        }

        if (0 != mq_close( pstTaskStatus->ackTransport))
        {
            perror ("mq_close.");
        }

        if (0 != mq_close(pstTaskStatus->RequestTransport))
        {
            perror ("mq_close.");
        }

        if (0 != mq_close(pstTaskStatus->ackLogger))
        {
            perror ("mq_close.");
        }

        blResult = true;
        
    }while (true != blResult);
    
    return blResult;
}

// EOF