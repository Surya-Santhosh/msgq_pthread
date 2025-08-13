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
TASK_HANDLER stTaskHandler = {0};

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
    TASK_HANDLER stRecievedStatus = {0};
    // struct mq_attr attr = {0};
    // attr.mq_flags = 0;
    // attr.mq_maxmsg = MAX_MESSAGE;
    // attr.mq_msgsize = sizeof(TASK_HANDLER);
    // attr.mq_curmsgs = 0;

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    printf("Enter any Key:\n");

    if (-1 != getchar())
    {
        stTaskHandler.blPoller = true;
    }

    if (true != multiThreadmsgqSend(&stTaskStatus->msgPoller, 
                                    (const char *)&stTaskHandler, 
                                     MSGQ_POLLER_TO_TRANSPORT, 
                                     sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        printf("GPIO is high.\n");
    }

    // stTaskStatus->msgPoller = mq_open(MSGQ_POLLER_TO_TRANSPORT, 
    //                                   O_CREAT | O_RDWR, 0644, &attr);

    // if (-1 == stTaskStatus->msgPoller)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_send(stTaskStatus->msgPoller, 
    //                  (const char*) &stTaskHandler, 
    //                  sizeof(TASK_HANDLER), 0))
    // {
    //     perror ("mq_send");
    // }
    // else
    // {
    //     printf("GPIO is high.\n");
    // }

    // if (0 != mq_close(stTaskStatus->msgPoller))
    // {
    //     perror ("mq_close.");
    // }

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

    while (true != stTaskHandler.blTransportAck)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->ackTransport, 
                                       (const char *)&stRecievedStatus, 
                                       MSGQ_TRANSPORT_TO_POLLER, 
                                       sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("Recieved acknowledgment from Transport.\n");
    }

    // stTaskStatus->ackTransport = mq_open(MSGQ_TRANSPORT_TO_POLLER, 
    //                                      O_CREAT | O_RDWR, 0644, &attr);
                                         
    // if (-1 == stTaskStatus->ackTransport)
    // {                           
    //     perror("mq_open");
    // }

    // if (-1 == mq_receive(stTaskStatus->ackTransport, (char *)&stRecievedStatus, 
    //                      sizeof(TASK_HANDLER), NULL))
    // {
    //     perror ("mq_receive.");
    // }
    // else
    // {
    //     printf("Recieved acknowledgment from Transport.\n");
    // }

    // if (0 != mq_close(stTaskStatus->ackTransport))
    // {
    //     perror ("mq_close.");
    // }

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
    TASK_HANDLER stRecievedStatus = {0};
    // struct mq_attr attr = {0};
    // attr.mq_flags = 0;
    // attr.mq_maxmsg = MAX_MESSAGE;
    // attr.mq_msgsize = sizeof(TASK_HANDLER);
    // attr.mq_curmsgs = 0;

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for message from poller.
    while (true != stTaskHandler.blPoller)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromPoller, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->msgPoller, 
                                       (const char *)&stRecievedStatus, 
                                       MSGQ_POLLER_TO_TRANSPORT, 
                                       sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf ("Recieved message from Poller.\n");
    }

    // stTaskStatus->msgPoller = mq_open(MSGQ_POLLER_TO_TRANSPORT, 
    //                                   O_CREAT | O_RDWR, 0644, &attr);

    // if (-1 == stTaskStatus->msgPoller)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_receive(stTaskStatus->msgPoller, (char *)&stRecievedStatus, 
    //                      sizeof(TASK_HANDLER), NULL))
    // {
    //     perror ("mq_receive.");
    // }
    // else
    // {
    //     printf ("Recieved message from Poller.\n");
    // }

    // if (0 != mq_close(stTaskStatus->msgPoller))
    // {
    //     perror ("mq_close.");
    // }

    // Send ack to poller.
    stTaskHandler.blTransportAck = true;

    pthread_cond_signal (&(stTaskStatus)->stAckFromTransport);

    if (true != multiThreadmsgqSend(&stTaskStatus->ackTransport, 
                                    (const char *)&stTaskHandler, 
                                     MSGQ_TRANSPORT_TO_POLLER, 
                                     sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        printf("Send acknowledgment from Transport to poller.\n");
    }

    // stTaskStatus->ackTransport = mq_open(MSGQ_TRANSPORT_TO_POLLER, 
    //                                      O_CREAT | O_RDWR, 0644, &attr);
                                         
    // if (-1 == stTaskStatus->ackTransport)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_send(stTaskStatus->ackTransport, 
    //                   (const char*)&stTaskHandler,
    //                   sizeof(TASK_HANDLER), 0))
    // {
    //     perror ("mq_send");
    // }
    // else
    // {
    //     printf("Send acknowledgment from Transport to poller.\n");
    // }

    // if (0 != mq_close(stTaskStatus->ackTransport))
    // {
    //     perror ("mq_close.");
    // }

    // Send msg to logger.
    stTaskHandler.blTransport = true;

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    if (true != multiThreadmsgqSend(&stTaskStatus->msgTransport, 
                                    (const char *)&stTaskHandler, 
                                     MSGQ_TRANSPORT_TO_LOGGER, 
                                     sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        printf("Send message from Transport to Logger.\n");
    }

    // stTaskStatus->msgTransport = mq_open(MSGQ_TRANSPORT_TO_LOGGER, 
    //                                      O_CREAT | O_RDWR, 0644, &attr);

    // if (-1 == stTaskStatus->msgTransport)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_send(stTaskStatus->msgTransport, 
    //                   (const char*) &stTaskHandler,
    //                   sizeof(TASK_HANDLER), 0))
    // {
    //     perror ("mq_send");
    // }
    // else
    // {
    //     printf("Send message from Transport to Logger.\n");
    // }

    // if (0 != mq_close(stTaskStatus->msgTransport))
    // {
    //     perror ("mq_close.");
    // }

    if (0 != pthread_mutex_unlock(&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_unlock");
    }

    //wait for ack from logger.
    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }
    while (true != stTaskHandler.blLoggerAck)
    {
        pthread_cond_wait (&(stTaskStatus)->stAckFromLogger, 
                           &(stTaskStatus)->stMutex);
    }

    if (true != multiThreadmsgqRecieve(&stTaskStatus->ackLogger, 
                                       (const char *)&stRecievedStatus, 
                                       MSGQ_LOGGER_TO_TRANSPORT, 
                                       sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("Recieved acknowledgment from Logger.\n");
    }

    // stTaskStatus->ackLogger = mq_open(MSGQ_LOGGER_TO_TRANSPORT, 
    //                                   O_CREAT | O_RDWR, 0644, &attr);
    // if (-1 == stTaskStatus->ackLogger)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_receive(stTaskStatus->ackLogger, (char *)&stRecievedStatus, 
    //                      sizeof(TASK_HANDLER), NULL))
    // {
    //     perror ("mq_receive.");
    // }
    // else
    // {
    //     printf("Recieved acknowledgment from Logger.\n");
    // }

    // if (0 != mq_close(stTaskStatus->ackLogger))
    // {
    //     perror ("mq_close.");
    // }

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
    TASK_HANDLER stRecievedStatus = {0};
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGE;
    attr.mq_msgsize = sizeof(TASK_HANDLER);
    attr.mq_curmsgs = 0;

    if (0 != pthread_mutex_lock (&(stTaskStatus)->stMutex))
    {
        perror("pthread_mutex_lock");
    }

    //wait for Transport variable.
    while (true != stTaskHandler.blTransport)
    {
        pthread_cond_wait (&(stTaskStatus)->stMsgFromTransport, 
                           &(stTaskStatus)->stMutex);
    }

    pthread_cond_signal (&(stTaskStatus)->stMsgFromTransport);

    if (true != multiThreadmsgqRecieve(&stTaskStatus->msgTransport, 
                                       (const char *)&stRecievedStatus, 
                                       MSGQ_TRANSPORT_TO_LOGGER, 
                                       sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqRecieve");
    }
    else
    {
        printf("Recieved message from Transport.\n");
    }

    // stTaskStatus->msgTransport = mq_open(MSGQ_TRANSPORT_TO_LOGGER, 
    //                                      O_CREAT | O_RDWR, 0644, &attr);
    // if (-1 == stTaskStatus->msgTransport)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_receive(stTaskStatus->msgTransport, (char *)&stRecievedStatus, 
    //                      sizeof(TASK_HANDLER), NULL))
    // {
    //     perror ("mq_receive.");
    // }
    // else
    // {
    //     printf("Recieved message from Transport.\n");
    // }

    // if (0 != mq_close(stTaskStatus->msgTransport))
    // {
    //     perror ("mq_close.");
    // }

    printf("LED ON\n");

    // Send ack to Transport.
    stTaskHandler.blLoggerAck = true;

    if (true != multiThreadmsgqSend(&stTaskStatus->ackLogger, 
                                    (const char *)&stTaskHandler, 
                                     MSGQ_LOGGER_TO_TRANSPORT, 
                                     sizeof(TASK_HANDLER)))
    {
        perror ("multiThreadmsgqSend");
    }
    else
    {
        printf("Send acknowledgment from Logger to transport.\n");
    }

    // stTaskStatus->ackLogger = mq_open(MSGQ_LOGGER_TO_TRANSPORT, 
    //                                   O_CREAT | O_RDWR, 0644, &attr);
    // if (-1 == stTaskStatus->ackLogger)
    // {
    //     perror("mq_open");
    // }

    // if (-1 == mq_send(stTaskStatus->ackLogger, 
    //                   (const char*) &stTaskHandler,
    //                   sizeof(TASK_HANDLER), 0))
    // {
    //     perror ("mq_send");
    // }
    // else
    // {
    //     printf("Send acknowledgment from Logger to transport.\n");
    // }

    // if (0 != mq_close(stTaskStatus->ackLogger))
    // {
    //     perror ("mq_close.");
    // }

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

        if (-1 == mq_send(*message, pcBuffer, sizeof(TASK_HANDLER), 0))
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

        if (-1 == mq_receive(*message, (char *)&pcBuffer, sizeof(TASK_HANDLER), 
                             NULL))
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