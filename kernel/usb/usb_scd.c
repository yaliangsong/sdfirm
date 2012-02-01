#include <target/usb_scd.h>

#ifdef CONFIG_SCD_BULK
/*=========================================================================
 * bulk endpoint
 *=======================================================================*/
__near__ uint8_t scd_states[NR_SCD_QUEUES];
__near__ struct scd_cmd scd_cmds[NR_SCD_QUEUES];
__near__ struct scd_resp scd_resps[NR_SCD_QUEUES];
scd_data_t scd_cmd_data;

#if NR_SCD_QUEUES > 1
__near__ scd_qid_t scd_qid = INVALID_SCD_QID;

void scd_qid_restore(scd_qid_t qid)
{
	scd_qid = qid;
}

scd_qid_t scd_qid_save(scd_qid_t id)
{
	scd_qid_t scid;

	scid = scd_qid;
	scd_qid_restore(id);
	return scid;
}
#endif

boolean scd_is_cmd_status(uint8_t status)
{
	return ((scd_resps[scd_qid].bStatus & SCD_CMD_STATUS_MASK) == status);
}

void __scd_queue_reset(scd_qid_t qid)
{
	scd_states[qid] = SCD_SLOT_STATE_PC2RDR;
	scd_resps[qid].bStatus &= ~SCD_CMD_STATUS_MASK;
}

/*=========================================================================
 * bulk-out endpoint
 *=======================================================================*/
void scd_CmdHeader_out(void)
{
	USBD_OUTB(scd_cmds[scd_qid].bMessageType);
	USBD_OUTL(scd_cmds[scd_qid].dwLength);
	USBD_OUTB(scd_cmds[scd_qid].bSlot);
	USBD_OUTB(scd_cmds[scd_qid].bSeq);
	USBD_OUTB(scd_cmds[scd_qid].abRFU[0]);
	USBD_OUTB(scd_cmds[scd_qid].abRFU[1]);
	USBD_OUTB(scd_cmds[scd_qid].abRFU[2]);
}

void __scd_CmdSuccess_out(void)
{
	scd_resps[scd_qid].bStatus = scd_slot_status();
	scd_resps[scd_qid].bError = 0;
}

void __scd_CmdFailure_out(uint8_t error, uint8_t status)
{
	scd_resps[scd_qid].bStatus = SCD_CMD_STATUS_FAIL |
				     status;
	scd_resps[scd_qid].bError = error;
	scd_resps[scd_qid].abRFU3 = 0;
	scd_resps[scd_qid].dwLength = 0;
}

void scd_SlotStatus_out(void)
{
	__scd_CmdSuccess_out();
	scd_resps[scd_qid].abRFU3 = 0;
	scd_resps[scd_qid].dwLength = 0;
}

void scd_DataBlock_out(void)
{
	scs_size_t nr = scd_xchg_avail();
	scs_size_t ne = SCD_XB_NE;

	__scd_CmdSuccess_out();
	scd_resps[scd_qid].abRFU3 = 0;
	scd_resps[scd_qid].dwLength = ne ? (nr < ne ? nr : ne) : nr;
}

void __scd_XfrBlock_out(scs_size_t hdr_size, scs_size_t blk_size)
{
	scs_off_t i;
	uint8_t byte = 0;

	if (usbd_request_handled() == hdr_size) {
		/* reset SCS error */
		SCD_XB_ERR = SCS_ERR_SUCCESS;
		/* reset Nc */
		SCD_XB_NC = 0;
		/* Ne is from wLevelParameter
		 * TODO: extended APDU level
		 * more efforts
		 */
		SCD_XB_NE = (scd_cmds[scd_qid].abRFU[1] << 8) |
			     scd_cmds[scd_qid].abRFU[2];
		/* reset WI */
		SCD_XB_WI = scd_cmds[scd_qid].abRFU[0];
	}
	/* force USB reap on error */
	if (!scd_slot_success(SCD_XB_ERR)) {
		return;
	}

	usbd_iter_accel();
	for (i = SCD_XB_NC; i < blk_size; i++) {
		/* XXX: USBD_OUTB May Fake Reads 'byte'
		 * Following codes are enabled only when USBD_OUTB actually
		 * gets something into the 'byte', which happens when:
		 * handled-1 == NC+hdr_size.
		 */
		USBD_OUT_BEGIN(byte) {
			/* Now byte contains non-fake value. */
			SCD_XB_ERR = scd_write_byte(SCD_XB_NC, byte);
			if (!scd_slot_success(SCD_XB_ERR)) {
				return;
			}
			SCD_XB_NC++;
		} USBD_OUT_END
	}
}

void scd_IccPowerOn_out(void)
{
	if (usbd_request_handled() == SCD_HEADER_SIZE) {
		/* reset SCD error */
		SCD_XB_ERR = SCS_ERR_SUCCESS;
		/* reset Nc */
		SCD_XB_NC = 0;
		/* Ne should be 32 (ATR) + 1 (TS) */
		SCD_XB_NE = SCS_ATR_MAX;
		/* reset WI */
		SCD_XB_WI = 0;
	}
}

void scd_SlotNotExist_cmp(void)
{
	__scd_CmdFailure_out(5, SCD_SLOT_STATUS_NOTPRESENT);
	scd_CmdResponse_cmp();
}

void scd_CmdOffset_cmp(uint8_t offset)
{
	scd_CmdFailure_out(offset);
	scd_CmdResponse_cmp();
}

void scd_SlotStatus_cmp(void)
{
	scd_SlotStatus_out();
	scd_CmdResponse_cmp();
}

void scd_XfrBlock_cmp(void)
{
	scs_err_t err = SCD_XB_ERR;

	/* TODO: wLevelParameter check when XCHG_CHAR or XCHG_APDU_EXT */
	if (scd_slot_success(err) && SCD_XB_NC != 0) {
		err = scd_xchg_block(SCD_XB_NC, SCD_XB_NE);
	}
	scd_ScsSequence_cmp(err, true);
}

void scd_IccPowerOn_cmp(void)
{
	scs_err_t err;
	uint8_t bPowerSelect = scd_cmds[scd_qid].abRFU[0];
	err = scd_power_on(bPowerSelect);
	scd_ScsSequence_cmp(err, true);
}

void scd_IccPowerOff_cmp(void)
{
	scs_err_t err;
	err = scd_power_off();
	scd_ScsSequence_cmp(err, false);
}

void scd_ScsSequence_cmp(scs_err_t err, boolean block)
{
	if (!scd_slot_progress(err)) {
		if (!scd_slot_success(err)) {
			scd_CmdFailure_out(scd_slot_error(err));
		} else {
			if (block) {
				scd_DataBlock_out();
			} else {
				scd_SlotStatus_out();
			}
		}
		scd_CmdResponse_cmp();
	} else {
		scd_slot_enter(SCD_SLOT_STATE_ISO7816);
	}
}

/*=========================================================================
 * bulk-in endpoint
 *=======================================================================*/
void scd_RespHeader_in(scs_size_t length)
{
	USBD_INB(scd_resp_message());
	USBD_INL(length);
	USBD_INB(scd_cmds[scd_qid].bSlot);
	USBD_INB(scd_cmds[scd_qid].bSeq);
	USBD_INB(scd_resps[scd_qid].bStatus);
	USBD_INB(scd_resps[scd_qid].bError);
	USBD_INB(scd_resps[scd_qid].abRFU3);
}

void scd_DataBlock_in(void)
{
	scs_off_t i;

	scd_RespHeader_in(scd_resps[scd_qid].dwLength);

	usbd_iter_accel();

	for (i = usbd_request_handled()-SCD_HEADER_SIZE;
	     i < scd_resps[scd_qid].dwLength; i++) {
		USBD_INB(scd_read_byte(i));
	}

#if 0
	if (usbd_request_handled() ==
	    scd_resps[scd_qid].dwLength + SCD_HEADER_SIZE) {
		/* allow next XfrBlock fetching next bytes */
		scd_resps[scd_qid].dwLength;
	}
#endif

	BUG_ON(usbd_request_handled() >
	       scd_resps[scd_qid].dwLength + SCD_HEADER_SIZE);
}

/*=========================================================================
 * bulk endpoint entrance
 *=======================================================================*/
void __scd_handle_command(scd_qid_t qid)
{
	scd_sid_t sid;

	scd_qid_select(qid);
	scd_debug(SCD_DEBUG_SLOT, scd_qid);

	BUG_ON(scd_states[scd_qid] != SCD_SLOT_STATE_PC2RDR &&
	       scd_states[scd_qid] != SCD_SLOT_STATE_SANITY);

	scd_slot_enter(SCD_SLOT_STATE_SANITY);

	/* CCID message header */
	scd_CmdHeader_out();

	if (usbd_request_handled() < SCD_HEADER_SIZE)
		return;

	if (usbd_request_handled() == SCD_HEADER_SIZE) {
		usbd_request_commit(SCD_HEADER_SIZE +
				    scd_cmds[scd_qid].dwLength);
		scd_debug(SCD_DEBUG_PC2RDR, scd_cmds[scd_qid].bMessageType);
	}

	sid = scd_cmds[scd_qid].bSlot;
	if (sid >= NR_SCD_USB_SLOTS) {
		return;
	}
	/* XXX: care should be taken on BULK ABORT */
	if (scd_abort_requested())
		return;

	/* slot ID determined */
	scd_sid_select(sid);
	scd_debug(SCD_DEBUG_SLOT, scd_sid);

	if (scd_cmds[scd_qid].bMessageType == SCD_PC2RDR_GETSLOTSTATUS) {
		return;
	}
	if (scd_slot_status() == SCD_SLOT_STATUS_NOTPRESENT) {
		scd_CmdFailure_out(SCD_ERROR_ICC_MUTE);
		return;
	}
#if 0
	/* FIXME: check auto sequence */
	if (scd_slot_status() != SCD_SLOT_STATUS_ACTIVE) {
		scd_CmdFailure_out(CCID_ERROR_BUSY_AUTO_SEQ);
		return;
	}
#endif

	switch (scd_cmds[scd_qid].bMessageType) {
	case SCD_PC2RDR_ICCPOWERON:
		scd_IccPowerOn_out();
		break;
	case SCD_PC2RDR_ICCPOWEROFF:
	case SCD_PC2RDR_GETPARAMETERS:
		/* nothing to do */
		break;
	case SCD_PC2RDR_XFRBLOCK:
		scd_XfrBlock_out();
		break;
	case SCD_PC2RDR_ESCAPE:
		scd_Escape_out();
		break;
	default:
		scd_handle_bulk_pc2rdr();
		break;
	}
}

void __scd_complete_command(scd_qid_t qid)
{
	scd_sid_t sid;

	scd_qid_select(qid);
	scd_debug(SCD_DEBUG_SLOT, scd_qid);

	BUG_ON(scd_states[scd_qid] != SCD_SLOT_STATE_PC2RDR &&
	       scd_states[scd_qid] != SCD_SLOT_STATE_SANITY);

	sid = scd_cmds[scd_qid].bSlot;
	if (usbd_request_handled() < SCD_HEADER_SIZE ||
	    sid >= NR_SCD_USB_SLOTS) {
		scd_SlotNotExist_cmp();
		return;
	}

	/* XXX: care should be taken on BULK ABORT */
	if (scd_abort_completed())
		return;
	if (scd_is_cmd_status(SCD_CMD_STATUS_FAIL)) {
		/* CMD_ABORTED and SLOT_BUSY error */
		scd_CmdResponse_cmp();
		return;
	}

	/* SANITY state completed */
	/* CCID_QID_OUT should be idle */
	scd_slot_enter(SCD_SLOT_STATE_PC2RDR);

	/* now we are able to handle slot specific request */
	scd_sid_select(sid);
	scd_debug(SCD_DEBUG_SLOT, scd_sid);

	if (usbd_request_handled() !=
	    (scd_cmds[scd_qid].dwLength + SCD_HEADER_SIZE)) {
		scd_CmdOffset_cmp(1);
		return;
	}
	if (scd_cmds[scd_qid].bMessageType == SCD_PC2RDR_GETSLOTSTATUS) {
		scd_SlotStatus_cmp();
		return;
	}
	if (scd_is_cmd_status(SCD_CMD_STATUS_FAIL)) {
		scd_CmdResponse_cmp();
		return;
	}

	switch (scd_cmds[scd_qid].bMessageType) {
	case SCD_PC2RDR_ICCPOWERON:
		scd_IccPowerOn_cmp();
		break;
	case SCD_PC2RDR_ICCPOWEROFF:
		scd_IccPowerOff_cmp();
		break;
	case SCD_PC2RDR_XFRBLOCK:
		scd_XfrBlock_cmp();
		break;
	case SCD_PC2RDR_ESCAPE:
		scd_Escape_cmp();
		break;
	default:
		scd_complete_bulk_pc2rdr();
		break;
	}
}

void __scd_complete_response(scd_qid_t qid)
{
	scd_qid_select(qid);
	scd_debug(SCD_DEBUG_SLOT, scd_qid);
	scd_slot_enter(SCD_SLOT_STATE_PC2RDR);
}

void __scd_handle_response(scd_qid_t qid)
{
	scd_sid_t sid;

	scd_qid_select(qid);
	scd_debug(SCD_DEBUG_SLOT, scd_qid);

	BUG_ON(scd_states[scd_qid] != SCD_SLOT_STATE_RDR2PC);

	if (scd_abort_responded())
		return;
	if (scd_is_cmd_status(SCD_CMD_STATUS_FAIL)) {
		scd_SlotStatus_in();
		return;
	}

	sid = scd_cmds[scd_qid].bSlot;
	scd_sid_select(sid);
	scd_debug(SCD_DEBUG_SLOT, scd_sid);

	switch (scd_cmds[scd_qid].bMessageType) {
	case SCD_PC2RDR_ICCPOWERON:
	case SCD_PC2RDR_XFRBLOCK:
		scd_DataBlock_in();
		break;
	case SCD_PC2RDR_ESCAPE:
		scd_Escape_in();
		break;
	case SCD_PC2RDR_ICCPOWEROFF:
	case SCD_PC2RDR_GETSLOTSTATUS:
		scd_SlotStatus_in();
		break;
	default:
		scd_handle_bulk_rdr2pc();
		break;
	}
}

usbd_endpoint_t scd_endpoint_out = {
	USBD_ENDP_BULK_OUT,
	SCD_ENDP_INTERVAL_OUT,
	scd_submit_command,
	scd_handle_command,
	scd_complete_command,
};

usbd_endpoint_t scd_endpoint_in = {
	USBD_ENDP_BULK_IN,
	SCD_ENDP_INTERVAL_IN,
	scd_submit_response,
	scd_handle_response,
	scd_complete_response,
};
#endif

/*=========================================================================
 * control endpoint
 *=======================================================================*/
#define SCD_STRING_FIRST	0x10
#define SCD_STRING_INTERFACE	SCD_STRING_FIRST+0
#define SCD_STRING_LAST		SCD_STRING_INTERFACE

static uint16_t scd_config_length(void)
{
	return SCD_DT_SCD_SIZE;
}

static void scd_get_config_desc(void)
{
	usbd_input_interface_desc(USB_INTERFACE_CLASS_SCD,
				  USB_DEVICE_SUBCLASS_NONE,
				  USB_INTERFACE_PROTOCOL_SCD,
				  SCD_STRING_INTERFACE);
	scd_ctrl_get_desc();
}

static void scd_get_string_desc(void)
{
	uint8_t id = LOBYTE(usbd_control_request_value());

	switch (id) {
	case SCD_STRING_INTERFACE:
		usbd_input_device_name();
		break;
	default:
		USBD_INB(0x00);
		break;
	}
}

static void scd_get_descriptor(void)
{
	uint8_t desc;
	
	desc = HIBYTE(usbd_control_request_value());

	switch (desc) {
	case USB_DT_CONFIG:
		scd_get_config_desc();
		break;
	case USB_DT_STRING:
		scd_get_string_desc();
		break;
	default:
		usbd_endpoint_halt();
		break;
	}
}

#define scd_set_descriptor()		usbd_endpoint_halt()

static void scd_handle_standard_request(void)
{
	uint8_t req = usbd_control_request_type();

	switch (req) {
	case USB_REQ_GET_DESCRIPTOR:
		scd_get_descriptor();
		break;
	case USB_REQ_SET_DESCRIPTOR:
		scd_set_descriptor();
		break;
	default:
		usbd_endpoint_halt();
	}
}

static void scd_handle_ctrl_data(void)
{
	uint8_t type = usbd_control_setup_type();
	uint8_t recp = usbd_control_setup_recp();

	switch (recp) {
	case USB_RECP_DEVICE:
		switch (type) {
		case USB_TYPE_STANDARD:
			scd_handle_standard_request();
			return;
		}
		break;
	case USB_RECP_INTERFACE:
		switch (type) {
		case USB_TYPE_STANDARD:
			scd_handle_standard_request();
			return;
		case USB_TYPE_CLASS:
			scd_handle_ctrl_class();
			return;
		}
		break;
	}
	usbd_endpoint_halt();
}

usbd_interface_t usb_scd_interface = {
	SCD_STRING_FIRST,
	SCD_STRING_LAST,
	NR_SCD_ENDPS,
	scd_config_length,
	scd_handle_ctrl_data,
};
