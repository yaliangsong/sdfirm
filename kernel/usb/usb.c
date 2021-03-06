/* USB HCD/OTG A and DEV/OTG B dispatcher */

#include <target/usb.h>
#include <target/bh.h>
#include <target/irq.h>

#ifdef CONFIG_USB_DEV
extern void usbd_init(void);
extern void usbd_switch(void);
extern void usbd_handler(uint8_t event);
#else
#define usbd_init()
#define usbd_switch()
#define usbd_handler(event)	do {} while (0)
#endif

#ifdef CONFIG_USB_HCD
extern void hcd_init(void);
extern void hcd_switch(void);
extern void hcd_handler(uint8_t event);
#else
#define hcd_init()
#define hcd_switch()
#define hcd_handler(event)	do {} while (0)
#endif

__near__ bh_t usb_bh = INVALID_BH;

void usb_wakeup_state(void)
{
	bh_resume(usb_bh);
}

#if defined(CONFIG_USB_DEV) && defined(CONFIG_USB_HCD)
uint8_t usb_mode = USB_MODE_DEF;

uint8_t usb_get_mode(void)
{
	return usb_mode;
}

void usb_set_mode(uint8_t mode)
{
	usb_mode = mode;
}
#endif

#ifdef SYS_REALTIME
#define usb_irq_init()		irq_register_poller(usb_bh)
#define usb_irq_poll(event)	usb_hw_irq_poll();
#else
#define usb_irq_init()		usb_hw_irq_init()
#define usb_irq_poll(event)	do { } while (0)
#endif

void usb_handler(uint8_t event)
{

	if (event == BH_POLLIRQ)
		usb_irq_poll(event);
	else {
		if (usb_get_mode() == USB_MODE_DEVICE)
			usbd_handler(event);
		else
			hcd_handler(event);
	}
}

uint8_t usb_endpoint_size_type(uint16_t size)
{
	return __ffs16((uint16_t)(size >> 3));
}

void usb_switch_mode(uint8_t mode)
{
	usb_set_mode(mode);
	if (mode == USB_MODE_DEVICE) {
		usbd_switch();
	} else {
		hcd_switch();
	}
}

void usb_init(void)
{
	DEVICE_INTF(DEVICE_INTF_USB);
	usb_bh = bh_register_handler(usb_handler);
	usb_hw_ctrl_init();
	usb_irq_init();
	usbd_init();
	hcd_init();
	usb_switch_mode(USB_MODE_DEF);
}
