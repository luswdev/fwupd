/*
 * Copyright (C) 2023 Adam.Chen <Adam.Chen@genesyslogic.com.tw>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "fu-genesys-common.h"
#include "fu-genesys-usbhub-firmware.h"
#include "fu-genesys-usbhub-pd-firmware.h"
#include "fu-genesys-usbhub-struct.h"

struct _FuGenesysUsbhubPdFirmware {
	FuFirmwareClass parent_instance;
};

G_DEFINE_TYPE(FuGenesysUsbhubPdFirmware, fu_genesys_usbhub_pd_firmware, FU_TYPE_FIRMWARE)

static gboolean
fu_genesys_usbhub_pd_firmware_validate(FuFirmware *firmware,
				       GInputStream *stream,
				       gsize offset,
				       GError **error)
{
	return fu_struct_genesys_pd_firmware_hdr_validate_stream(stream, offset, error);
}

static gboolean
fu_genesys_usbhub_pd_firmware_parse(FuFirmware *firmware,
				    GBytes *fw,
				    gsize offset,
				    FwupdInstallFlags flags,
				    GError **error)
{
	gsize code_size = 0;
	g_autoptr(GBytes) fw_trunc = NULL;

	fu_firmware_set_id(firmware, fu_genesys_fw_type_to_string(FU_GENESYS_FW_TYPE_PD));
	fu_firmware_set_idx(firmware, FU_GENESYS_FW_TYPE_PD);
	fu_firmware_set_alignment(firmware, FU_FIRMWARE_ALIGNMENT_1K);

	/* truncate to correct size */
	if (!fu_genesys_usbhub_firmware_calculate_size(fw, offset, &code_size, error)) {
		g_prefix_error(error, "not valid for pd: ");
		return FALSE;
	}
	fw_trunc = fu_bytes_new_offset(fw, offset, code_size, error);
	if (fw_trunc == NULL)
		return FALSE;
	fu_firmware_set_bytes(firmware, fw_trunc);

	/* calculate checksum */
	if ((flags & FWUPD_INSTALL_FLAG_IGNORE_CHECKSUM) == 0) {
		if (!fu_genesys_usbhub_firmware_verify_checksum(fw_trunc, error)) {
			g_prefix_error(error, "not valid for pd: ");
			return FALSE;
		}
	}

	/* get firmware version */
	if (!fu_genesys_usbhub_firmware_ensure_version(firmware, error)) {
		g_prefix_error(error, "not valid for pd: ");
		return FALSE;
	}

	/* success */
	return TRUE;
}

static void
fu_genesys_usbhub_pd_firmware_init(FuGenesysUsbhubPdFirmware *self)
{
	fu_firmware_add_flag(FU_FIRMWARE(self), FU_FIRMWARE_FLAG_HAS_CHECKSUM);
}

static void
fu_genesys_usbhub_pd_firmware_class_init(FuGenesysUsbhubPdFirmwareClass *klass)
{
	FuFirmwareClass *klass_firmware = FU_FIRMWARE_CLASS(klass);
	klass_firmware->validate = fu_genesys_usbhub_pd_firmware_validate;
	klass_firmware->parse = fu_genesys_usbhub_pd_firmware_parse;
}
