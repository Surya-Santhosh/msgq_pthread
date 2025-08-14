//******************************* multiThread **********************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved 
//******************************************************************************
//
// Summary : Create threads (Poller, Transport, Logger) send Request and give
//           Ackowledgment using message queue to turn LED ON using mutex and
//           conditional variables for synchronisation.
// Note    : None
// 
//******************************************************************************
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

//**************************** Include Files ***********************************
#include "common.h"
#include "msgq.h"

//*************************** Global Types ************************************* 

//************************* Global Constants *********************************** 

//************************* Global Variables *********************************** 

//************************* Forward Declarations ******************************* 
bool multiThreadSetUp();

//********************** Inline Method Implementations ************************* 

#endif 

// MULTITHREAD_H 
// EOF