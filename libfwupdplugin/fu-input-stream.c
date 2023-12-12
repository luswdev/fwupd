/*
 * Copyright (C) 2023 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define G_LOG_DOMAIN "FuInputStream"

#include "config.h"

#include "fu-chunk-array.h"
#include "fu-input-stream.h"
#include "fu-mem-private.h"
#include "fu-sum.h"

/**
 * fu_input_stream_from_path:
 * @path: a filename
 * @error: (nullable): optional return location for an error
 *
 * Opens the file as n input stream.
 *
 * Returns: (transfer full): a #GInputStream, or %NULL on error
 *
 * Since: 2.0.0
 **/
GInputStream *
fu_input_stream_from_path(const gchar *path, GError **error)
{
	g_autoptr(GFile) file = NULL;
	g_autoptr(GFileInputStream) stream = NULL;

	g_return_val_if_fail(path != NULL, NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	file = g_file_new_for_path(path);
	stream = g_file_read(file, NULL, error);
	if (stream == NULL)
		return NULL;
	return G_INPUT_STREAM(g_steal_pointer(&stream));
}

/**
 * fu_input_stream_read_safe:
 * @stream: a #GInputStream
 * @buf: a buffer to read data into
 * @bufsz: size of @buf
 * @offset: offset in bytes into @buf to copy from
 * @seek_set: given offset to seek to
 * @count: the number of bytes that will be read from the stream
 * @error: (nullable): optional return location for an error
 *
 * Tries to read count bytes from the stream into the buffer starting at @buf.
 *
 * Returns: %TRUE for success
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_safe(GInputStream *stream,
			  guint8 *buf,
			  gsize bufsz,
			  gsize offset,
			  gsize seek_set,
			  gsize count,
			  GError **error)
{
	gssize rc;

	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(buf != NULL, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	if (!fu_memchk_write(bufsz, offset, count, error))
		return FALSE;
	if (!g_seekable_seek(G_SEEKABLE(stream), seek_set, G_SEEK_SET, NULL, error)) {
		g_prefix_error(error, "seek to 0x%x: ", (guint)seek_set);
		return FALSE;
	}
	rc = g_input_stream_read(stream, buf + offset, count, NULL, error);
	if (rc == -1) {
		g_prefix_error(error, "failed read of 0x%x: ", (guint)count);
		return FALSE;
	}
	if ((gsize)rc != count) {
		g_set_error(error,
			    G_IO_ERROR,
			    G_IO_ERROR_PARTIAL_INPUT,
			    "requested 0x%x and got 0x%x",
			    (guint)count,
			    (guint)rc);
		return FALSE;
	}
	return TRUE;
}

/**
 * fu_input_stream_read_u8:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @value: (out) (nullable): the parsed value
 * @error: (nullable): optional return location for an error
 *
 * Read a value from a stream using a specified endian in a safe way.
 *
 * Returns: %TRUE if @value was set, %FALSE otherwise
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_u8(GInputStream *stream, gsize offset, guint8 *value, GError **error)
{
	guint8 buf = 0;
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!fu_input_stream_read_safe(stream, &buf, sizeof(buf), 0x0, offset, sizeof(buf), error))
		return FALSE;
	if (value != NULL)
		*value = buf;
	return TRUE;
}

/**
 * fu_input_stream_read_u16:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @value: (out) (nullable): the parsed value
 * @endian: an endian type, e.g. %G_LITTLE_ENDIAN
 * @error: (nullable): optional return location for an error
 *
 * Read a value from a stream using a specified endian in a safe way.
 *
 * Returns: %TRUE if @value was set, %FALSE otherwise
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_u16(GInputStream *stream,
			 gsize offset,
			 guint16 *value,
			 FuEndianType endian,
			 GError **error)
{
	guint8 buf[2] = {0};
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!fu_input_stream_read_safe(stream, buf, sizeof(buf), 0x0, offset, sizeof(buf), error))
		return FALSE;
	if (value != NULL)
		*value = fu_memread_uint16(buf, endian);
	return TRUE;
}

/**
 * fu_input_stream_read_u24:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @value: (out) (nullable): the parsed value
 * @endian: an endian type, e.g. %G_LITTLE_ENDIAN
 * @error: (nullable): optional return location for an error
 *
 * Read a value from a stream using a specified endian in a safe way.
 *
 * Returns: %TRUE if @value was set, %FALSE otherwise
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_u24(GInputStream *stream,
			 gsize offset,
			 guint32 *value,
			 FuEndianType endian,
			 GError **error)
{
	guint8 buf[3] = {0};
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!fu_input_stream_read_safe(stream, buf, sizeof(buf), 0x0, offset, sizeof(buf), error))
		return FALSE;
	if (value != NULL)
		*value = fu_memread_uint24(buf, endian);
	return TRUE;
}

/**
 * fu_input_stream_read_u32:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @value: (out) (nullable): the parsed value
 * @endian: an endian type, e.g. %G_LITTLE_ENDIAN
 * @error: (nullable): optional return location for an error
 *
 * Read a value from a stream using a specified endian in a safe way.
 *
 * Returns: %TRUE if @value was set, %FALSE otherwise
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_u32(GInputStream *stream,
			 gsize offset,
			 guint32 *value,
			 FuEndianType endian,
			 GError **error)
{
	guint8 buf[4] = {0};
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!fu_input_stream_read_safe(stream, buf, sizeof(buf), 0x0, offset, sizeof(buf), error))
		return FALSE;
	if (value != NULL)
		*value = fu_memread_uint32(buf, endian);
	return TRUE;
}

/**
 * fu_input_stream_read_u64:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @value: (out) (nullable): the parsed value
 * @endian: an endian type, e.g. %G_LITTLE_ENDIAN
 * @error: (nullable): optional return location for an error
 *
 * Read a value from a stream using a specified endian in a safe way.
 *
 * Returns: %TRUE if @value was set, %FALSE otherwise
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_read_u64(GInputStream *stream,
			 gsize offset,
			 guint64 *value,
			 FuEndianType endian,
			 GError **error)
{
	guint8 buf[8] = {0};
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);
	if (!fu_input_stream_read_safe(stream, buf, sizeof(buf), 0x0, offset, sizeof(buf), error))
		return FALSE;
	if (value != NULL)
		*value = fu_memread_uint64(buf, endian);
	return TRUE;
}

/**
 * fu_input_stream_read_buf:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @length: number of bytes to read
 * @error: (nullable): optional return location for an error
 *
 * Read a byte array from a stream in a safe way.
 *
 * Returns: (transfer full): buffer
 *
 * Since: 2.0.0
 **/
GByteArray *
fu_input_stream_read_buf(GInputStream *stream, gsize offset, gsize length, GError **error)
{
	g_autoptr(GByteArray) buf = g_byte_array_sized_new(length);
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);
	g_byte_array_set_size(buf, length);
	if (!fu_input_stream_read_safe(stream, buf->data, buf->len, 0x0, offset, buf->len, error))
		return NULL;
	return g_steal_pointer(&buf);
}

/**
 * fu_input_stream_read_bytes:
 * @stream: a #GInputStream
 * @offset: offset in bytes into @buf to copy from
 * @length: number of bytes to read
 * @error: (nullable): optional return location for an error
 *
 * Read a byte array from a stream in a safe way.
 *
 * Returns: (transfer full): buffer
 *
 * Since: 2.0.0
 **/
GBytes *
fu_input_stream_read_bytes(GInputStream *stream, gsize offset, gsize length, GError **error)
{
	g_autoptr(GByteArray) buf = NULL;
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);
	buf = fu_input_stream_read_buf(stream, offset, length, error);
	if (buf == NULL)
		return NULL;
	return g_byte_array_free_to_bytes(g_steal_pointer(&buf)); /* nocheck */
}

/**
 * fu_input_stream_size:
 * @stream: a #GInputStream
 * @val: (out): size in bytes
 * @error: (nullable): optional return location for an error
 *
 * Reads the total possible of the stream.
 *
 * Returns: %TRUE for success
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_size(GInputStream *stream, gsize *val, GError **error)
{
	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	if (!g_seekable_seek(G_SEEKABLE(stream), 0, G_SEEK_END, NULL, error)) {
		g_prefix_error(error, "seek to end: ");
		return FALSE;
	}
	if (val != NULL)
		*val = g_seekable_tell(G_SEEKABLE(stream));

	/* success */
	return TRUE;
}

static gboolean
fu_input_stream_compute_checksum_cb(const guint8 *buf,
				    gsize bufsz,
				    gpointer user_data,
				    GError **error)
{
	GChecksum *csum = (GChecksum *)user_data;
	g_checksum_update(csum, buf, bufsz);
	return TRUE;
}

/**
 * fu_input_stream_compute_checksum:
 * @stream: a #GInputStream
 * @checksum_type: a #GChecksumType
 * @error: (nullable): optional return location for an error
 *
 * Generates the checksum of the entire stream.
 *
 * Returns: the hexadecimal representation of the checksum, or %NULL on error
 *
 * Since: 2.0.0
 **/
gchar *
fu_input_stream_compute_checksum(GInputStream *stream, GChecksumType checksum_type, GError **error)
{
	g_autoptr(GChecksum) csum = g_checksum_new(checksum_type);

	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), NULL);
	g_return_val_if_fail(error == NULL || *error == NULL, NULL);

	if (!fu_input_stream_chunkify(stream, fu_input_stream_compute_checksum_cb, csum, error))
		return NULL;
	return g_strdup(g_checksum_get_string(csum));
}

static gboolean
fu_input_stream_compute_sum8_cb(const guint8 *buf, gsize bufsz, gpointer user_data, GError **error)
{
	guint8 *value = (guint8 *)user_data;
	*value += fu_sum8(buf, bufsz);
	return TRUE;
}

/**
 * fu_input_stream_compute_sum8:
 * @stream: a #GInputStream
 * @value: (out): value
 * @error: (nullable): optional return location for an error
 *
 * Returns the arithmetic sum of all bytes in the stream.
 *
 * Returns: %TRUE for success
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_compute_sum8(GInputStream *stream, guint8 *value, GError **error)
{
	return fu_input_stream_chunkify(stream, fu_input_stream_compute_sum8_cb, value, error);
}

/**
 * fu_input_stream_chunkify:
 * @stream: a #GInputStream
 * @func_cb: (scope async): function to call with chunks
 * @user_data: user data to pass to @func_cb
 * @error: (nullable): optional return location for an error
 *
 * Split the stream into blocks and calls a function on each chunk.
 *
 * Returns: %TRUE for success
 *
 * Since: 2.0.0
 **/
gboolean
fu_input_stream_chunkify(GInputStream *stream,
			 FuInputStreamChunkifyFunc func_cb,
			 gpointer user_data,
			 GError **error)
{
	g_autoptr(FuChunkArray) chunks = NULL;

	g_return_val_if_fail(G_IS_INPUT_STREAM(stream), FALSE);
	g_return_val_if_fail(func_cb != NULL, FALSE);
	g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

	chunks = fu_chunk_array_new_from_stream(stream, 0x0, 0x8000, error);
	if (chunks == NULL)
		return FALSE;
	for (gsize i = 0; i < fu_chunk_array_length(chunks); i++) {
		g_autoptr(FuChunk) chk = NULL;
		chk = fu_chunk_array_index(chunks, i, error);
		if (chk == NULL)
			return FALSE;
		if (!func_cb(fu_chunk_get_data(chk), fu_chunk_get_data_sz(chk), user_data, error))
			return FALSE;
	}
	return TRUE;
}
