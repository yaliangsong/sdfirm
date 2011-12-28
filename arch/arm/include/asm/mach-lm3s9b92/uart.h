#ifndef __UART_LM3S9B92_H_INCLUDE__
#define __UART_LM3S9B92_H_INCLUDE__

#include <target/config.h>
#include <target/generic.h>
#include <asm/reg.h>
#include <asm/mach/clk.h>
#include <asm/mach/pm.h>
#include <asm/mach/gpio.h>

#ifdef CONFIG_UART_LM3S9B92
#ifndef ARCH_HAVE_UART
#define ARCH_HAVE_UART		1
#else
#error "Multiple UART controller defined"
#endif
#endif

#define UART_BASE	0x4000C000
#define UART(offset)	(UART_BASE + offset)

#define UARTDR(n)	UART(0x##n##000)
#define DRFE		8
#define DRPE		9
#define DRBE		10
#define DROE		11
#define UARTRSR(n)	UART(0x##n##004)
#define UARTECR(n)	UART(0x##n##004)
#define SRFE		0
#define SRPE		1
#define SRBE		2
#define SROE		3
#define UARTFR(n)	UART(0x##n##018)
#define RI		8
#define TXFE		7
#define RXFF		6
#define TXFF		5
#define RXFE		4
#define BUSY		3
#define DCD		2
#define DSR		1
#define CTS		0
#define UARTILPR(n)	UART(0x##n##020)
#define UARTIBRD(n)	UART(0x##n##024)
#define UARTFBRD(n)	UART(0x##n##028)
#define UARTLCRH(n)	UART(0x##n##02C)
#define SPS		7
#define WLEN		5 /* width = 2bits */
#define FEN		4
#define STP2		3
#define EPS		2
#define PEN		1
#define BRK		0
#define UARTCTL(n)	UART(0x##n##030)
#define CTSEN		15
#define RTSEN		14
#define RTS		11
#define DTR		10
#define RXE		9
#define TXE		8
#define LBE		7
#define LIN		6
#define HSE		5
#define EOT		4
#define SMART		3
#define SIRLP		2
#define SIREN		1
#define UARTEN		0
#define UARTIFLS(n)	UART(0x##n##034)
#define UARTIM(n)	UART(0x##n##038)
#define UARTRIS(n)	UART(0x##n##03C)
#define UARTMIS(n)	UART(0x##n##040)
#define UARTICR(n)	UART(0x##n##044)
#define UARTDMACTL(n)	UART(0x##n##048)

#define __UART_HW_SMART_PARAM	(8 | UART_PARITY_EVEN | UART_STOPB_TWO)

/* Since fractional part is based on the 64 (2^6) times of the BRD,
 * 6 is define as FBRD offset here.
 */
/* compute the baud rate divisor
 * BRD = IBRD + FBRD = SysClk / (ClkDiv * baudrate)
 *
 * we will compute 64*BRD+0.5 to get DIVINT & DIVFRAC value
 * DIVINT = (64*BRD+0.5) / 64 = BRD
 * DIVFRAC = (64*BRD+0.5) % 64
 *
 * 64*BRD+0.5 = ((64 * SysClk) / (ClkDiv * baudrate)) + 0.5
 *            = ((CLK_SYS * 64000) / (16 * baudrate)) + 0.5
 *            = ((CLK_SYS * 4000) / (baudrate)) + 0.5
 *            = (((CLK_SYS * 20) / (baudrate/400)) + 1) / 2
 */
#define __UART_HW_FBRD_OFFSET	6
#define __UART_HW_FBRD_MASK	((1<<__UART_HW_FBRD_OFFSET)-1)

#define LM3S9B92_UART(n)						\
static inline uint32_t __uart##n##_hw_config_param(uint8_t params)	\
{									\
	uint32_t cfg;							\
	cfg = (uart_bits(params)-5) << WLEN;				\
	switch (uart_parity(params)) {					\
	case UART_PARITY_EVEN:						\
		cfg |= _BV(EPS);					\
	case UART_PARITY_ODD:						\
		cfg |= _BV(PEN);					\
		break;							\
	}								\
	if (uart_stopb(params)) {					\
		cfg |= _BV(STP2);					\
	}								\
	return cfg;							\
}									\
static inline void __uart##n##_hw_ctrl_disable(void)			\
{									\
	while (__raw_readl(UARTFR(n)) & _BV(BUSY));			\
	/* disable the UART */						\
	__raw_clearl(_BV(UARTEN) | _BV(TXE) | _BV(RXE), UARTCTL(n));	\
}									\
static inline void __uart##n##_hw_ctrl_enable(void)			\
{									\
	/* disable the FIFO and BRK */					\
	__raw_writel(__uart##n##_hw_config_param(UART_PARAMS),		\
		     UARTLCRH(n));					\
	/* enable RX, TX, and the UART */				\
	__raw_writel(_BV(UARTEN) | _BV(TXE) | _BV(RXE), UARTCTL(n));	\
	/* clear the flags register */					\
	/* __raw_writel(0, UART0FR); */					\
}									\
static void __uart##n##_hw_config_baudrate(uint32_t baudrate)		\
{									\
	unsigned long cfg;						\
	/* is the required baud rate greater than the maximum rate */	\
	/* supported without the use of high speed mode? */		\
	if (baudrate > mul16u((uint16_t)div32u(CLK_SYS, 16), 1000)) {	\
		/* enable high speed mode */				\
		__raw_setl_atomic(HSE, UARTCTL(0));			\
		baudrate = div32u(baudrate, 2);				\
	} else {							\
		/* disable high speed mode */				\
		__raw_clearl_atomic(HSE, UARTCTL(0));			\
	}								\
	cfg = div32u(div32u((uint32_t)(mul32u(CLK_SYS, 20)),		\
			    div32u(baudrate, 400)) + 1, 2);		\
	/* set the baud rate */						\
	__raw_writel(cfg >> __UART_HW_FBRD_OFFSET, UARTIBRD(0));	\
	__raw_writel(cfg & __UART_HW_FBRD_MASK, UARTFBRD(0));		\
}									\
static inline void __uart##n##_hw_ctrl_config(uint8_t params,		\
					      uint32_t baudrate)	\
{									\
	__uart##n##_hw_config_baudrate(baudrate);			\
	/* UARTLCRH write must follows UART(I|F)BRD writes */		\
	__raw_writel_mask(__uart##n##_hw_config_param(params), 0xEE,	\
			  UARTLCRH(n));					\
}									\
static inline boolean __uart##n##_hw_status_error(void)			\
{									\
	return __raw_readl(UARTRSR(n));					\
}									\
static inline void __uart##n##_hw_write_byte(uint8_t byte)		\
{									\
	while (__raw_readl(UARTFR(n)) & _BV(TXFF));			\
	__raw_writeb(byte, UARTDR(n));					\
}									\
static inline uint8_t __uart##n##_hw_read_byte(void)			\
{									\
	while (__raw_readl(UARTFR(n)) & _BV(RXFE));			\
	return __raw_readb(UARTDR(n));					\
}

#define LM3S9B92_UART_SMART(n)						\
LM3S9B92_UART(n)							\
static inline void __uart##n##_hw_smart_start(void)			\
{									\
	__raw_writel(__uart##n##_hw_config_param(__UART_HW_SMART_PARAM),\
						 UARTLCRH(n));		\
	__raw_writel(_BV(UARTEN) | _BV(SMART), UARTCTL(n));		\
}									\
static inline void __uart##n##_hw_smart_stop(void)			\
{									\
	while (__raw_readl(UARTFR(n)) & _BV(BUSY));			\
	__raw_clearl(_BV(UARTEN) | _BV(SMART), UARTCTL(n));		\
}									\
static inline boolean __uart##n##_hw_smart_empty(void)			\
{									\
	return __raw_readl(UARTFR(n)) & _BV(RXFE);			\
}									\
static inline uint8_t __uart##n##_hw_smart_read(void)			\
{									\
	return __raw_readb(UARTDR(n));					\
}

void uart_hw_ctrl_init(void);
void uart_hw_ctrl_start(void);
void uart_hw_ctrl_stop(void);
void uart_hw_set_params(uint8_t params, uint32_t baudrate);
void uart_hw_write_byte(uint8_t byte);
uint8_t uart_hw_read_byte(void);

#endif /* __UART_LM3S9B92_H_INCLUDE__ */