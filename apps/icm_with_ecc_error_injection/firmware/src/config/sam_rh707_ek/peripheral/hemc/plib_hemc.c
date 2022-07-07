/*****************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    plib_hemc.c

  Summary:
    HEMC PLIB Source file

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
#include <string.h>
#include "device.h"
#include "plib_hemc.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

static HEMC_OBJ hemcObj;

// *****************************************************************************
// *****************************************************************************
// Section: HEMC Implementation
// *****************************************************************************
// *****************************************************************************


void HSMC_Initialize( void )
{



    /* Chip Select CS1 Timings */
    /* Setup HSMC SETUP register */
    HSMC_REGS->HSMC_CS[1].HSMC_SETUP = HSMC_SETUP_NWE_SETUP(2) | HSMC_SETUP_NCS_WR_SETUP(2) | HSMC_SETUP_NRD_SETUP(2) | HSMC_SETUP_NCS_RD_SETUP(2);

    /* Setup HSMC CYCLE register */
    HSMC_REGS->HSMC_CS[1].HSMC_CYCLE = HSMC_CYCLE_NWE_CYCLE(10) | HSMC_CYCLE_NRD_CYCLE(10);

    /* Setup HSMC_PULSE register */
    HSMC_REGS->HSMC_CS[1].HSMC_PULSE = HSMC_PULSE_NWE_PULSE(6) | HSMC_PULSE_NCS_WR_PULSE(6) | HSMC_PULSE_NRD_PULSE(6) | HSMC_PULSE_NCS_RD_PULSE(6);

    /* Setup HSMC MODE register */
    HSMC_REGS->HSMC_CS[1].HSMC_MODE = HSMC_MODE_EXNW_MODE(0) | HSMC_MODE_DBW(0)   | HSMC_MODE_RMW_ENABLE_Msk;

}
void HEMC_Initialize( void )
{
    /* Read NCS0 Pin configuration for HECC */
    uint8_t eccEnableDefault = ( (HEMC_REGS->HEMC_CR_NCS0 & HEMC_CR_NCS0_ECC_ENABLE_Msk) >> HEMC_CR_NCS0_ECC_ENABLE_Pos);

    /* Enable NCS0 configuration modification through registers */
    HEMC_REGS->HEMC_CR_NCS0 |= HEMC_CR_NCS0_WRITE_ECC_CONF(1U);

    HEMC_REGS->HEMC_CR_NCS0 = HEMC_CR_NCS0_TYPE(0U) |
                              HEMC_CR_NCS0_ADDBASE(0x0U) |
                              HEMC_CR_NCS0_BANKSIZE(0x1FU) |
                              HEMC_CR_NCS0_WRITE_ECC_CONF(1U) |
                              HEMC_CR_NCS0_ECC_ENABLE((uint32_t)eccEnableDefault);

    HEMC_REGS->HEMC_CR_NCS1 = HEMC_CR_NCS1_TYPE(0) |
                              HEMC_CR_NCS1_ADDBASE(0x40) |
                              HEMC_CR_NCS1_BANKSIZE(0x6) |
                              HEMC_CR_NCS1_ECC_ENABLE(1);
    HSMC_Initialize();

    /* For RAM memories on NCS1, perform memory initialization of ECC check bit */
    memset((void*)(0x60100000), 0x00, 0x66660);
    if (DATA_CACHE_IS_ENABLED())
    {
        DCACHE_CLEAN_INVALIDATE_BY_ADDR((void*)(0x60100000), 0x66660);
    }

    /* Wait all memory is zeroized and clear previous interrupts when memory ECC wasn't initialized */
    __DSB();
    __ISB();
    HEMC_HECC_STATUS dummyStatus = HEMC_HeccGetStatus();
    /* Ignore the warning */
    (void)dummyStatus;

    // Enable interrupts
    HEMC_REGS->HEMC_HECC_IER = (HEMC_HECC_IER_MEM_FIX_Msk | HEMC_HECC_IER_MEM_NOFIX_Msk);

} /* HEMC_Initialize */

// *****************************************************************************
/* Function:
    void HEMC_HeccGetStatus(void)

   Summary:
    Get the HECC status of the HEMC peripheral.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    Current status of the HEMC peripheral.
*/
HEMC_HECC_STATUS HEMC_HeccGetStatus(void)
{
    return (HEMC_HECC_STATUS)(HEMC_REGS->HEMC_HECC_SR);
}

// *****************************************************************************
/* Function:
    uint32_t* HEMC_HeccGetFailAddress(void)

   Summary:
    Get the last fail address were ECC error occurs in a HEMC memory.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    Pointer of fail address were fixable or unfixable error occured in a HEMC memory.
*/
uint32_t* HEMC_HeccGetFailAddress(void)
{
    return (uint32_t*)(HEMC_REGS->HEMC_HECC_FAILAR);
}

// *****************************************************************************
/* Function:
    uint32_t HEMC_HeccGetFailData(void)

   Summary:
    Get the last fail data were ECC error occurs in HEMC memory.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    Data were fixable or unfixable error occurred in HEMC memory.
*/
uint32_t HEMC_HeccGetFailData(void)
{
    return (uint32_t)(HEMC_REGS->HEMC_HECC_FAILDR);
}

// *****************************************************************************
/* Function:
    void HEMC_HeccResetCounters(void)

   Summary:
    Reset Fix and NoFix error counters of the HEMC peripheral.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None
*/
void HEMC_HeccResetCounters(void)
{
    HEMC_REGS->HEMC_HECC_CR |= (HEMC_HECC_CR_RST_FIX_CPT_Msk | HEMC_HECC_CR_RST_NOFIX_CPT_Msk);
}

// *****************************************************************************
/* Function:
    void HEMC_FixCallbackRegister(HEMC_CALLBACK callback,
                                                              uintptr_t context)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given HEMC's ECC Fixable Error interrupt events occur.

  Description:
    This function sets the pointer to a client function to be called "back" when
    the given HEMC's interrupt events occur. It also passes a context value
    (usually a pointer to a context structure) that is passed into the function
    when it is called. The specified callback function will be called from the
    peripheral interrupt context.

  Precondition:
    HEMC_Initialize must have been called for the associated
    HEMC instance.

  Parameters:
    callback      - A pointer to a function with a calling signature defined by
                    the HEMC_CALLBACK data type. Setting this to NULL
                    disables the callback feature.

    contextHandle - A value (usually a pointer) passed (unused) into the
                    function identified by the callback parameter.

  Returns:
    None.

  Example:
    <code>
        // Refer to the description of the HEMC_CALLBACK data type for
        // example usage.
    </code>

  Remarks:
    None.
*/
void HEMC_FixCallbackRegister(HEMC_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    hemcObj.fix_callback = callback;
    hemcObj.fix_context = contextHandle;
}

// *****************************************************************************
/* Function:
    void HEMC_NoFixCallbackRegister(HEMC_CALLBACK callback,
                                                              uintptr_t context)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given HEMC's ECC Not Fixable Error interrupt events occur.

  Description:
    This function sets the pointer to a client function to be called "back" when
    the given HEMC's interrupt events occur. It also passes a context value
    (usually a pointer to a context structure) that is passed into the function
    when it is called. The specified callback function will be called from the
    peripheral interrupt context.

  Precondition:
    HEMC_Initialize must have been called for the associated
    HEMC instance.

  Parameters:
    callback      - A pointer to a function with a calling signature defined by
                    the HEMC_CALLBACK data type. Setting this to NULL
                    disables the callback feature.

    contextHandle - A value (usually a pointer) passed (unused) into the
                    function identified by the callback parameter.

  Returns:
    None.

  Example:
    <code>
        // Refer to the description of the HEMC_CALLBACK data type for
        // example usage.
    </code>

  Remarks:
    None.
*/
void HEMC_NoFixCallbackRegister(HEMC_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback == NULL)
    {
        return;
    }

    hemcObj.nofix_callback = callback;
    hemcObj.nofix_context = contextHandle;
}


// *****************************************************************************
/* Function:
    void HEMC_FIX_InterruptHandler(void)

   Summary:
    HEMC Peripheral Interrupt Handler.

   Description:
    This function is HEMC Peripheral Interrupt Handler and will
    called on every HEMC ECC Fixable error interrupt.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None.

   Remarks:
    The function is called as peripheral instance's interrupt handler if the
    instance interrupt is enabled. If peripheral instance's interrupt is not
    enabled user need to call it from the main while loop of the application.
*/
void HEMC_INTFIX_InterruptHandler(void)
{

    if (hemcObj.fix_callback != NULL)
    {
        hemcObj.fix_callback(hemcObj.fix_context);
    }
}

// *****************************************************************************
/* Function:
    void HEMC_NOFIX_InterruptHandler(void)

   Summary:
    HEMC Peripheral Interrupt Handler.

   Description:
    This function is HEMC Peripheral Interrupt Handler and will
    called on every HEMC ECC Not Fixable error interrupt.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None.

   Remarks:
    The function is called as peripheral instance's interrupt handler if the
    instance interrupt is enabled. If peripheral instance's interrupt is not
    enabled user need to call it from the main while loop of the application.
*/
void HEMC_INTNOFIX_InterruptHandler(void)
{

    if (hemcObj.nofix_callback != NULL)
    {
        hemcObj.nofix_callback(hemcObj.nofix_context);
    }
}

/*******************************************************************************
 End of File
*/
