//*********************************** msgq *************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved 
//******************************************************************************
//
// Summary : Send and Reeceive data via message queue.
// Note    : None
// 
//******************************************************************************
#ifndef MSGQ_H
#define MSGQ_H

//**************************** Include Files ***********************************
#include "common.h"

//*************************** Global Types ************************************* 

//************************* Global Constants *********************************** 

//************************* Global Variables *********************************** 

//************************* Forward Declarations ******************************* 
bool multiThreadmsgqSend(mqd_t *pmessage, const char * pcBuffer);
bool multiThreadmsgqRecieve(mqd_t *pmessage, char * pcBuffer);

//********************** Inline Method Implementations ************************* 

#endif 

// MSGQ_H 
// EOF