//******************************** main ****************************************
// Copyright (c) 2025 Trenser Technology Solutions
// All Rights Reserved
//******************************************************************************
// File    : main.c
// Summary : Store and update device information using a linked list and a 
//           binary file.
// Note    : None
// Author  : Surya Santhosh
// Day     : 04/Aug/2025
//******************************************************************************

//**************************** Include Files ***********************************
#include "common.h"
#include "multiTread/multiThread.h"

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
    if (true == multiThreadSetUp())
    {
        printf("Success\n");
    }

    return 0;
}

// EOF