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
bool multiThreadmsgqOpen(TASK_STATUS *pstTaskStatus);
bool multiThreadmsgqClose(TASK_STATUS *pstTaskStatus);

//********************** Inline Method Implementations ************************* 

#endif 

// MSGQ_H 
// EOF