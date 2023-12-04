/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <string.h>

#include "fu-ep963x-common.h"
#include "fu-ep963x-firmware.h"
#include "fu-ep963x-struct.h"

struct _FuEp963xFirmware {
	FuFirmware parent_instance;
};

G_DEFINE_TYPE(FuEp963xFirmware, fu_ep963x_firmware, FU_TYPE_FIRMWARE)

static gboolean
fu_ep963x_firmware_validate(FuFirmware *firmware,
			    GInputStream *stream,
			    gsize offset,
			    GError **error)
{
	return fu_struct_ep963x_firmware_hdr_validate_stream(stream, offset, error);
}

static gboolean
fu_ep963x_firmware_parse(FuFirmware *firmware,
			 GBytes *fw,
			 gsize offset,
			 FwupdInstallFlags flags,
			 GError **error)
{
	gsize len = g_bytes_get_size(fw);

	/* check size */
	if (len != FU_EP963_FIRMWARE_SIZE) {
		g_set_error(error,
			    FWUPD_ERROR,
			    FWUPD_ERROR_INVALID_FILE,
			    "firmware size expected 0x%x, got 0x%x",
			    (guint)FU_EP963_FIRMWARE_SIZE,
			    (guint)len);
		return FALSE;
	}

	/* success */
	return TRUE;
}

static void
fu_ep963x_firmware_init(FuEp963xFirmware *self)
{
}

static void
fu_ep963x_firmware_class_init(FuEp963xFirmwareClass *klass)
{
	FuFirmwareClass *klass_firmware = FU_FIRMWARE_CLASS(klass);
	klass_firmware->validate = fu_ep963x_firmware_validate;
	klass_firmware->parse = fu_ep963x_firmware_parse;
}
