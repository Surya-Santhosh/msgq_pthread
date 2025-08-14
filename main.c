//******************************** main ****************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : main.c
// Summary : Create threads (Poller, Transport, Logger) send Request and give
//           acknowledgement using message queue to turn LED ON using mutex and
//           conditional variables for synchronisation.
// Note    : None
// Author  : Surya Santhosh
// Day     : 04/Aug/2025
//******************************************************************************

//**************************** Include Files ***********************************
#include "common.h"
#include "multiThread.h"

//******************************* Local Types **********************************

//***************************** Local Constants ********************************

//**************************** Local Variables *********************************

//***************************** Local Functions ********************************

//*****************************.mainFunction.***********************************
// Purpose : Add two numbers.
// Inputs  : none
// Outputs : none
// Return  : 0
// Notes   : None
//******************************************************************************
int main()
{
    while (true)
    {
        if (true != multiThreadSetUp())
        {
            perror ("multiThreadSetUp");
        }
    }

    return 0;
}

// EOF