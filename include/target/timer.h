#ifndef __TIMER_H_INCLUDE__
#define __TIMER_H_INCLUDE__

/* TODO:
 * 1. promptly timer TIMER_IRQ.
 */
#include <target/gpt.h>

typedef uint8_t tid_t;

/* The max timers that can be registered */
#ifdef CONFIG_MAX_TIMERS
#define NR_TIMERS		CONFIG_MAX_TIMERS
#else
#define NR_TIMERS		0
#endif
#define INVALID_TID		NR_TIMERS

/* TIMER_BH: Background running timers, which should be delayed to the
 *           process context.  No interrupt masking is required for such
 *           timers.  This is also known as generic timers.
 * TIMER_IRQ: Foreground running timers, which should be handled in the
 *            interrupt context without delay to ensure that it can be
 *            handled in time.  This is also known as realtime timers.
 */
#define TIMER_BH		0x00
#ifdef CONFIG_TICK
#define TIMER_IRQ		TIMER_BH
#else
#define TIMER_IRQ		0x01
#endif

struct timer_desc {
	uint8_t type;
	void (*handler)(void);
};
__TEXT_TYPE__(const struct timer_desc, timer_desc_t);

#ifdef CONFIG_TIMER
void timer_init(void);
tid_t timer_register(timer_desc_t *timer);
void timer_unregister(tid_t tid);
void timer_schedule_shot(tid_t tid, timeout_t tout_ms);
#else
#define timer_init()				do { } while (0)
#define timer_register(timer)			INVALID_TID
#define timer_unregister(tid)			do { } while (0)
#define timer_schedule_shot(tid, tout_ms)	do { } while (0)
#endif

#endif /* __TIMER_H_INCLUDE__ */
