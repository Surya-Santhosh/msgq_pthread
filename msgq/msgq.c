//*********************************** msgq *************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : msgq.c
// Summary : Send and Reeceive data via message queue.
// Note    : None
// Author  : Surya Santhosh
// Day     : 04/Aug/2025
//******************************************************************************

//**************************** Include Files ***********************************
#include "msgq.h"

//******************************* Local Types **********************************

//***************************** Local Constants ********************************

//**************************** Local Variables *********************************

//***************************** Local Functions ********************************

//*****************************.multiThreadmsgqSend.****************************
// Purpose : send data to message queue. 
// Inputs  : message - pointer to message descriptor.
//           pcBuffer - pointer to buffer containig meesage.
//           pcMsgSize - size of message.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqSend(mqd_t *pmessage, const char* pcBuffer, 
                                uint32 pcMsgSize)
{
    bool blResult = false;

    if (-1 == mq_send(*pmessage, pcBuffer, pcMsgSize, 0))
    {
        perror ("mq_send");
    }
    else
    {
        blResult = true;
    }

    return blResult;
}

//**************************.multiThreadmsgqRecieve.****************************
// Purpose : Recieve data from message queue. 
// Inputs  : message - pointer to message descriptor.
//           pcBuffer - pointer to buffer where recieve data will be stored.
//           pcMsgSize - size of message.
// Outputs : None
// Return  : blResult
// Notes   : None
//******************************************************************************
static bool multiThreadmsgqRecieve(mqd_t *pmessage, char* pcBuffer, 
                                   uint32 pcMsgSize)
{
    bool blResult = false;

    if (-1 == mq_receive(*pmessage, (char *)pcBuffer, pcMsgSize, NULL))
    {
        perror ("mq_receive.");
    }
    else
    {
        blResult = true;
    }
    
    return blResult;
}

// EOF