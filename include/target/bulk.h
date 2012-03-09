#ifndef __BULK_H_INCLUDE__
#define __BULK_H_INCLUDE__

#include <target/generic.h>

#ifdef CONFIG_BULK_MAX_CHANS
#define NR_BULK_CHANS	CONFIG_BULK_MAX_CHANS
#else
#define NR_BULK_CHANS	1
#endif

/* bulk channel ID */
typedef uint8_t bulk_cid_t;
typedef uint16_t bulk_size_t;

typedef void (*bulk_open_cb)(size_t);
typedef void (*bulk_close_cb)(bulk_size_t);
typedef uint8_t (*bulk_read_cb)(void);
typedef void (*bulk_write_cb)(uint8_t);
typedef void (*bulk_xmit_cb)(uint8_t *, bulk_size_t);
typedef boolean (*bulk_space_cb)(void);

struct bulk_channel {
	uint8_t flags;
#define BULK_DIR_READ		0x80
#define BULK_XMIT_BURST		0x01
	bulk_size_t threshold;

	bulk_read_cb getchar;
	bulk_write_cb putchar;
	bulk_xmit_cb read;
	bulk_xmit_cb write;
};
__TEXT_TYPE__(const struct bulk_channel, bulk_channel_t);

/*=========================================================================
 * deprecated bulk APIs
 *=======================================================================*/
/* Both sides are CPU transfer */
void bulk_config_read_byte(bulk_cid_t bulk, bulk_size_t flush,
			   bulk_open_cb open,
			   bulk_read_cb read,
			   bulk_close_cb close);
void bulk_config_write_byte(bulk_cid_t bulk, bulk_size_t flush,
			    bulk_open_cb open,
			    bulk_write_cb write,
			    bulk_close_cb close);
void bulk_transfer_sync(uint8_t bulk, size_t size);

/*=========================================================================
 * bulk APIs
 *=======================================================================*/
bulk_cid_t bulk_alloc_fifo(uint8_t *buffer, bulk_size_t length);
void bulk_free_fifo(bulk_cid_t bulk);
void bulk_reset_fifo(bulk_cid_t bulk);

bulk_size_t bulk_write_buffer(bulk_cid_t bulk,
			      const uint8_t *buf, bulk_size_t len);
bulk_size_t bulk_write_byte(bulk_cid_t bulk, uint8_t c);

bulk_cid_t bulk_register_channel(bulk_channel_t *chan);

void bulk_handle_write_byte(bulk_cid_t bulk,
			    bulk_write_cb write,
			    bulk_size_t count);
void bulk_handle_read_byte(bulk_cid_t bulk,
			   bulk_read_cb read,
			   bulk_size_t count);
void bulk_handle_write_buffer(bulk_cid_t bulk,
			      bulk_xmit_cb write,
			      bulk_size_t count);
void bulk_handle_read_buffer(bulk_cid_t bulk,
			     bulk_xmit_cb read,
			     bulk_size_t count);

#endif /* __BULK_H_INCLUDE__ */
