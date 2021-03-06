#ifndef __MEM_VEXPRESSA9_H_INCLUDE__
#define __MEM_VEXPRESSA9_H_INCLUDE__

#include <target/config.h>
#include <asm/reg.h>

/* DUI0447J 4.2.1 ARM Legacy Memory Map */

/* Chip select 7 */
#define UATX_SYSREG	UATX_PERIPH1(CS7, 0)
#define UATX_SYSCTL	UATX_PERIPH1(CS7, 1)
#define UATX_PCIE	UATX_PERIPH1(CS7, 2)

#define UATX_AACI	UATX_PERIPH1(CS7, 4)
#define UATX_MMCI	UATX_PERIPH1(CS7, 5)
#define UATX_KMI0	UATX_PERIPH1(CS7, 6)
#define UATX_KMI1	UATX_PERIPH1(CS7, 7)

#define UATX_UART0	UATX_PERIPH1(CS7, 9)
#define UATX_UART1	UATX_PERIPH1(CS7, 10)
#define UATX_UART2	UATX_PERIPH1(CS7, 11)
#define UATX_UART3	UATX_PERIPH1(CS7, 12)

#define UATX_WDT	UATX_PERIPH1(CS7, 15)

#define UATX_TIMER01	UATX_PERIPH1(CS7, 17)
#define UATX_TIMER23	UATX_PERIPH1(CS7, 18)

#define UATX_DVI	UATX_PERIPH1(CS7, 22)
#define UATX_RTC	UATX_PERIPH1(CS7, 23)

#define UATX_CF		UATX_PERIPH1(CS7, 26)

#define UATX_CLCD	UATX_PERIPH1(CS7, 31)

/* Chip select 3 */
#define UATX_VRAM	UATX_PERIPH2(CS3, 0)
#define UATX_ETH	UATX_PERIPH2(CS3, 2)
#define UATX_USB	UATX_PERIPH2(CS3, 3)

#endif /* __MEM_VEXPRESSA9_H_INCLUDE__ */
