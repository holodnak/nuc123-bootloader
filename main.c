/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 15/07/02 11:18a $
 * @brief    Software Development Template.
 * @note
 * Copyright (C) 2014~2015 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NUC123.h"
#include "version.h"

#define HCLK_CLOCK           72000000

void update_init(void)
{
	int i;
	uint32_t u32Data = 0, u32Data2 = 0;

    FMC_Open();


	for(i=0;i<(0x8000 - 4);i+=4) {
		u32Data ^= FMC_Read(0x10000 - 8);		//identification		
	}
	
	u32Data2 = FMC_Read(0x10000 - 4);		//checksum

	if(u32Data != u32Data2) {
		printf("checksum doesnt match\n");
		return;
	}

	//read the firmware header
	u32Data = FMC_Read(0x10000 - 8);		//identification

	if(u32Data != 0xDEADBEEF) {
		printf("id doesnt match\n");
		return;
	}

	if(u32Data == 0xDEADBEEF) {
		FMC_EnableAPUpdate();

		//erase lower 32k
		for(i=0;i<0x8000;i+=512) {
			FMC_Erase(i);
		}

		//copy upper flash to lower flash
		for(i=0;i<0x8000;i+=4) {
			u32Data = FMC_Read(i + 0x8000);
			FMC_Write(i, u32Data);
		}

		//erase upper 32k
		for(i=0;i<0x8000;i+=512) {
			FMC_Erase(i + 0x8000);
		}

		FMC_DisableAPUpdate();
		printf("update finished\n");
	}
	FMC_Close();

}

int main()
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable XT1_OUT(PF0) and XT1_IN(PF1) */
    SYS->GPF_MFP |= SYS_GPF_MFP_PF0_XT1_OUT | SYS_GPF_MFP_PF1_XT1_IN;

    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as HCLK_CLOCK */
    CLK_SetCoreClock(HCLK_CLOCK);

    /* Enable module clocks */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select HCLK as the clock source of SPI0 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));

    /* Select UART module clock source */

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Set GPB multi-function pins for UART0 RXD(PB.0) and TXD(PB.1) */
    SYS->GPC_MFP &= ~(SYS_GPC_MFP_PC4_Msk | SYS_GPC_MFP_PC5_Msk);
    SYS->GPC_MFP = SYS_GPC_MFP_PC4_UART0_RXD | SYS_GPC_MFP_PC5_UART0_TXD;
	SYS->ALT_MFP = SYS_ALT_MFP_PC4_UART0_RXD | SYS_ALT_MFP_PC5_UART0_TXD;

    /* Setup SPI0 multi-function pins */
//    SYS->GPC_MFP |= SYS_GPC_MFP_PC0_SPI0_SS0 | SYS_GPC_MFP_PC1_SPI0_CLK | SYS_GPC_MFP_PC2_SPI0_MISO0 | SYS_GPC_MFP_PC3_SPI0_MOSI0;
//    SYS->ALT_MFP |= SYS_ALT_MFP_PC0_SPI0_SS0 | SYS_ALT_MFP_PC1_SPI0_CLK | SYS_ALT_MFP_PC2_SPI0_MISO0 | SYS_ALT_MFP_PC3_SPI0_MOSI0;

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and cyclesPerUs automatically. */
    SystemCoreClockUpdate();
	
//    CLK_EnableModuleClock(TMR2_MODULE);
//    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2_S_HCLK, 0);

	//disk flip button
    GPIO_SetMode(PA, BIT10, GPIO_PMD_INPUT);

	//led
//    GPIO_SetMode(PB, BIT8, GPIO_PMD_OUTPUT);
	

    /* Init UART0 to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    printf("\nnuc123-bootloader v%x.%02x\n",VERSION_HI,VERSION_LO);

	update_init();

	//check if button is held down
	if(PA10 != 0) {
		printf("entering firmware update mode.\n");
//		update_init();
	}
	
    /* Lock protected registers */
    SYS_LockReg();

	//boot to aprom
    SYS_UnlockReg();
	FMC->ISPCON = 0;
	SYS->IPRSTC1 = 2;
	while(1);
}

/*** (C) COPYRIGHT 2014~2015 Nuvoton Technology Corp. ***/
