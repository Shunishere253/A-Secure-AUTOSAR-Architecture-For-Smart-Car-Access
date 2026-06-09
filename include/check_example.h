/*
 * Copyright 2020 NXP
 *
 * NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly accepting such terms
 * or by downloading, installing, activating and/or otherwise using the software, you are agreeing
 * that you have read, and that you agree to comply with and are bound by, such license terms. If
 * you do not agree to be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 *
 * This file contains sample code only. It is not part of the production code deliverables.
 */

#ifndef _CHECK_EXAMPLE_H_
#define _CHECK_EXAMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "Std_Types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
static inline void Exit_Example(boolean Result)
{
    volatile uint8 *testResultBasePtr = (volatile uint8 *)VV_RESULT_ADDRESS;

    if (TRUE == Result)
    {
        *testResultBasePtr = 0x5AU;
    }
    else
    {
        *testResultBasePtr = 0x33U;
    }
}

static inline void EX_ASSERT(boolean Result)
{
    volatile uint8 *testResultBasePtr = (volatile uint8 *)VV_RESULT_ADDRESS;

    if (TRUE == Result)
    {
        if (*testResultBasePtr != 0x33U)
        {
            *testResultBasePtr = 0x5AU;
        }
    }
    else
    {
        *testResultBasePtr = 0x33U;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _CHECK_EXAMPLE_H_ */
