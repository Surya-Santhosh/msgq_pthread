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

//**************************.multiThreadPoller.*********************************
// Purpose : Add two numbers
// Inputs  : None
// Outputs : None
// Return  : true
// Notes   : None
//******************************************************************************
static bool multiThreadPoller(TASK_STATUS *stTaskStatus)
{
    TASK_HANDLER stRecievedStatus = {0};
    stTaskStatus->stTaskHandler.blPoller = false;
    struct mq_attr attr = {0, MAX_MESSAGE, sizeof(TASK_HANDLER), 0};

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    printf("Enter any Key:\n");

    if (-1 != getchar())
    {
        stTaskStatus->stTaskHandler.blPoller = true;
    }

    stTaskStatus->msgPoller = mq_open(MSGQ_POLLER_TO_TRANSPORT, 
                                      O_CREAT | O_RDWR, 0644, &attr);

    if (-1 == stTaskStatus->msgPoller)
    {
        perror("mq_open");
    }

    if (-1 == mq_send(stTaskStatus->msgPoller, 
                     (const char*) &(stTaskStatus)->stTaskHandler, 
                     sizeof(TASK_HANDLER), 0))
    {
        perror ("mq_send");
    }
    else
    {
        printf("GPIO is high.\n");
    }

    if (0 != mq_close(stTaskStatus->msgPoller))
    {
        perror ("mq_close.");
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

    while (true != stTaskStatus->stTaskHandler.blTransportAck)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    stTaskStatus->ackTransport = mq_open(MSGQ_TRANSPORT_TO_POLLER, 
                                         O_CREAT | O_RDWR, 0644, &attr);
                                         
    if (-1 == stTaskStatus->ackTransport)
    {                           
        perror("mq_open");
    }

    if (-1 == mq_receive(stTaskStatus->ackTransport, (char *)&stRecievedStatus, 
                         sizeof(TASK_HANDLER), NULL))
    {
        perror ("mq_receive.");
    }
    else
    {
        printf("Recieved acknowledgment from Transport.\n");
    }

    if (0 != mq_close(stTaskStatus->ackTransport))
    {
        perror ("mq_close.");
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
    TASK_STATUS stRecievedStatus = {0};
    struct mq_attr attr = {0, MAX_MESSAGE, sizeof(TASK_HANDLER), 0};
    stTaskStatus->stTaskHandler.blTransport = false;
    stTaskStatus->stTaskHandler.blTransportAck = false;

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for message from poller.
    while (true != stTaskStatus->stTaskHandler.blPoller)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromPoller, 
                           &(stTaskStatus)->stMutex);
    }

    stTaskStatus->msgPoller = mq_open(MSGQ_POLLER_TO_TRANSPORT, 
                                      O_CREAT | O_RDWR, 0644, &attr);

    if (-1 == stTaskStatus->msgPoller)
    {
        perror("mq_open");
    }

    if (-1 == mq_receive(stTaskStatus->msgPoller, (char *)&stRecievedStatus, 
                         sizeof(TASK_HANDLER), NULL))
    {
        perror ("mq_receive.");
    }
    else
    {
        printf ("Recieved message from Poller.\n");
    }

    if (0 != mq_close(stTaskStatus->msgPoller))
    {
        perror ("mq_close.");
    }

    // Send ack to poller.
    stTaskStatus->stTaskHandler.blTransportAck = true;

    pthread_cond_signal (&(stTaskStatus)->stAckFromTransport);

    stTaskStatus->ackTransport = mq_open(MSGQ_TRANSPORT_TO_POLLER, 
                                         O_CREAT | O_RDWR, 0644, &attr);
                                         
    if (-1 == stTaskStatus->ackTransport)
    {
        perror("mq_open");
    }

    if (-1 == mq_send(stTaskStatus->ackTransport, 
                      (const char*)&(stTaskStatus)->stTaskHandler,
                      sizeof(TASK_HANDLER), 0))
    {
        perror ("mq_send");
    }
    else
    {
        printf("Send acknowledgment from Transport to poller.\n");
    }

    if (0 != mq_close(stTaskStatus->ackTransport))
    {
        perror ("mq_close.");
    }

    // Send msg to logger.
    stTaskStatus->stTaskHandler.blTransport = true;

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    stTaskStatus->msgTransport = mq_open(MSGQ_TRANSPORT_TO_LOGGER, 
                                         O_CREAT | O_RDWR, 0644, &attr);

    if (-1 == stTaskStatus->msgTransport)
    {
        perror("mq_open");
    }

    if (-1 == mq_send(stTaskStatus->msgTransport, 
                      (const char*) &(stTaskStatus)->stTaskHandler,
                      sizeof(TASK_HANDLER), 0))
    {
        perror ("mq_send");
    }
    else
    {
        printf("Send message from Transport to Logger.\n");
    }

    if (0 != mq_close(stTaskStatus->msgTransport))
    {
        perror ("mq_close.");
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
    while (true != stTaskStatus->stTaskHandler.blLoggerAck)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromLogger, 
                           &(stTaskStatus)->stMutex);
    }

    stTaskStatus->ackLogger = mq_open(MSGQ_LOGGER_TO_TRANSPORT, 
                                      O_CREAT | O_RDWR, 0644, &attr);
    if (-1 == stTaskStatus->ackLogger)
    {
        perror("mq_open");
    }

    if (-1 == mq_receive(stTaskStatus->ackLogger, (char *)&stRecievedStatus, 
                         sizeof(TASK_HANDLER), NULL))
    {
        perror ("mq_receive.");
    }
    else
    {
        printf("Recieved acknowledgment from Logger.\n");
    }

    if (0 != mq_close(stTaskStatus->ackLogger))
    {
        perror ("mq_close.");
    }

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
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
    struct mq_attr attr = {0, MAX_MESSAGE, sizeof(TASK_HANDLER), 0};
    TASK_STATUS stRecievedStatus = {0};
    stTaskStatus->stTaskHandler.blLoggerAck = false;

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for Transport variable.
    while (true != stTaskStatus->stTaskHandler.blTransport)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    stTaskStatus->msgTransport = mq_open(MSGQ_TRANSPORT_TO_LOGGER, 
                                         O_CREAT | O_RDWR, 0644, &attr);
    if (-1 == stTaskStatus->msgTransport)
    {
        perror("mq_open");
    }

    if (-1 == mq_receive(stTaskStatus->msgTransport, (char *)&stRecievedStatus, 
                         sizeof(TASK_HANDLER), NULL))
    {
        perror ("mq_receive.");
    }
    else
    {
        printf("Recieved message from Transport.\n");
    }

    if (0 != mq_close(stTaskStatus->msgTransport))
    {
        perror ("mq_close.");
    }

    printf("LED ON\n");

    // Send ack to Transport.
    stTaskStatus->stTaskHandler.blLoggerAck = true;

    stTaskStatus->ackLogger = mq_open(MSGQ_LOGGER_TO_TRANSPORT, 
                                      O_CREAT | O_RDWR, 0644, &attr);
    if (-1 == stTaskStatus->ackLogger)
    {
        perror("mq_open");
    }

    if (-1 == mq_send(stTaskStatus->ackLogger, 
                      (const char*) &(stTaskStatus)->stTaskHandler,
                      sizeof(TASK_HANDLER), 0))
    {
        perror ("mq_send");
    }
    else
    {
        printf("Send acknowledgment from Logger to transport.\n");
    }

    if (0 != mq_close(stTaskStatus->ackLogger))
    {
        perror ("mq_close.");
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
    uint8 ucIndex = 0;
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

// EOF