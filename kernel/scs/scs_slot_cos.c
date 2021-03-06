#include <target/scs_slot.h>
#include <target/cos.h>

scs_sid_t cos_sid;

static scs_err_t cos_slot_error(scs_err_t err)
{
	switch (err) {
	case SCS_ERR_SUCCESS:
	case SCS_ERR_PROGRESS:
		return err;
	default:
		return SCS_ERR_HW_ERROR;
	}
}

static void cos_slot_select(void)
{
	/* Shouldn't be multiple COSes. */
}

static scs_err_t cos_slot_activate(void)
{
	scs_err_t err = cos_power_on();
	return cos_slot_error(err);
}

static scs_err_t cos_slot_deactivate(void)
{
	return SCS_ERR_SUCCESS;
}

static scs_err_t cos_slot_xchg_block(scs_size_t nc, scs_size_t ne)
{
	/* TODO: execute COS commands */
	return SCS_ERR_SUCCESS;
}

static scs_size_t cos_slot_xchg_avail(void)
{
	return cos_xchg_avail();
}

static scs_err_t cos_slot_xchg_write(scs_off_t index, uint8_t byte)
{
	cos_xchg_write(index, byte);
	return SCS_ERR_SUCCESS;
}

static uint8_t cos_slot_xchg_read(scs_off_t index)
{
	return cos_xchg_read(index);
}

static uint8_t cos_slot_status(void)
{
	scs_slot_select(cos_sid);
	return cos_get_status();
}

static uint8_t cos_slot_get_error(void)
{
	return cos_slot_error(SCS_ERR_SUCCESS);
}

static void cos_slot_complete_slot(void)
{
	scs_slot_select(cos_sid);
	scs_complete_slot();
}

scs_slot_driver_t cos_slot = {
	cos_slot_get_error,
	cos_slot_status,
	cos_slot_select,
	cos_slot_activate,
	cos_slot_deactivate,
	cos_slot_xchg_block,
	cos_slot_xchg_avail,
	cos_slot_xchg_write,
	cos_slot_xchg_read,
};

void cos_slot_init(void)
{
	cos_sid = scs_register_slot(&cos_slot);
	cos_register_completion(cos_slot_complete_slot);
}
