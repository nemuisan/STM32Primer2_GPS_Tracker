/********************************************************************************/
/*!
	@file			main.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        88.00
    @date           2019.11.29

    @section HISTORY
		2019.11.29	V88.00	See Whatnew.txt

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
volatile int TaskStat = NO_SELECTED; /* Default State */

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void (* volatile xTask)(void);

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*!
    @brief  Main Program.
	@param  None.
    @retval None.
*/
/**************************************************************************/
int main(void)
{
	/* Set Basis System For STM32 Primer2 */
	Set_System();

	/* Set SysTickCounter for _delay_ms(); / _delay_us(); */
	SysTickInit(INTERVAL);

	/* Select GNSS-LOGGER/USB-MSC/USB-CDC Mode */
	/* No Key input,then goes GNSS Logger */
	for(int mscnt=0; mscnt<3; mscnt++){
		_delay_ms(1000);
		if(!(GPIO_ReadInputDataBit(GPIOE, KEY_R) | GPIO_ReadInputDataBit(GPIOE, KEY_L))){
			TaskStat = GNSS_LOGGING;
			break;
		}
	}

	/* Select USB-MSC/USB-CDC Mode */
	if(TaskStat == NO_SELECTED) {
		if     (GPIO_ReadInputDataBit(GPIOE, KEY_L)) TaskStat = USB_VCOM;
		else if(GPIO_ReadInputDataBit(GPIOE, KEY_R)) TaskStat = USB_MSC;
	}

	/* Enable IWDG */
	IWDG_Enable();

	/* Install Main GNSS tracker Function */
	if(TaskStat == GNSS_LOGGING){
		xTask 				= gps_task;
		xUART_IRQ			= conio_IRQ;
		xEP1_IN_Callback  	= NOP_Process;
		xEP2_OUT_Callback 	= NOP_Process;
		xEP3_OUT_Callback 	= NOP_Process;
		xSOF_Callback     	= NOP_Process;
		xUSB_Istr	      	= NOP_Process;
	}
	/* Install USB-CDC VirtualCOM Function */
	else if(TaskStat == USB_VCOM){
		xTask 				= cdc_task;
		xUART_IRQ			= CDC_IRQ;
		xEP1_IN_Callback  	= CDC_EP1_IN_Callback;
		xEP2_OUT_Callback 	= NOP_Process;
		xEP3_OUT_Callback 	= CDC_EP3_OUT_Callback;
		xSOF_Callback     	= CDC_SOF_Callback;
		xUSB_Istr	      	= CDC_USB_Istr;
		CDC_SetStructure();
	}
	/* Install USB-MSC Function */
	else if(TaskStat == USB_MSC){
		xTask 				= msc_task;
		xUART_IRQ			= conio_IRQ;
		xEP1_IN_Callback  	= MSC_EP1_IN_Callback;
		xEP2_OUT_Callback 	= MSC_EP2_OUT_Callback;
		xEP3_OUT_Callback 	= NOP_Process;
		xSOF_Callback     	= NOP_Process;
		xUSB_Istr	      	= MSC_USB_Istr;
		MSC_SetStructure();
	}
	/* Fool Proof Anyway GNSS-Logger */
	else{
		TaskStat 			= GNSS_LOGGING;
		xTask 				= gps_task;
		xUART_IRQ			= conio_IRQ;
		xEP1_IN_Callback  	= NOP_Process;
		xEP2_OUT_Callback 	= NOP_Process;
		xEP3_OUT_Callback 	= NOP_Process;
		xSOF_Callback     	= NOP_Process;
		xUSB_Istr	      	= NOP_Process;
	}

	/* Main Loop */
	while (1)
	{
		xTask();
	}
}

/* End Of File ---------------------------------------------------------------*/
