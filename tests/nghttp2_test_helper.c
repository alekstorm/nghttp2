/*
 * nghttp2 - HTTP/2.0 C Library
 *
 * Copyright (c) 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "nghttp2_test_helper.h"

#include <assert.h>

#include <CUnit/CUnit.h>

#include "nghttp2_helper.h"

int unpack_framebuf(nghttp2_frame *frame, nghttp2_bufs *bufs)
{
  nghttp2_buf *buf;

  /* Assuming we have required data in first buffer. We don't decode
     header block so, we don't mind its space */
  buf = &bufs->head->buf;
  return unpack_frame(frame, buf->pos, nghttp2_buf_len(buf));
}

int unpack_frame(nghttp2_frame *frame, const uint8_t *in, size_t len)
{
  ssize_t rv = 0;
  const uint8_t *payload = in + NGHTTP2_FRAME_HDLEN;
  size_t payloadlen = len - NGHTTP2_FRAME_HDLEN;
  size_t payloadoff;

  nghttp2_frame_unpack_frame_hd(&frame->hd, in);
  switch(frame->hd.type) {
  case NGHTTP2_HEADERS:
    payloadoff = ((frame->hd.flags & NGHTTP2_FLAG_PAD_HIGH) > 0) +
      ((frame->hd.flags & NGHTTP2_FLAG_PAD_LOW) > 0);
    rv = nghttp2_frame_unpack_headers_payload
      (&frame->headers, payload + payloadoff, payloadlen - payloadoff);
    break;
  case NGHTTP2_PRIORITY:
    nghttp2_frame_unpack_priority_payload
      (&frame->priority, payload, payloadlen);
    break;
  case NGHTTP2_RST_STREAM:
    nghttp2_frame_unpack_rst_stream_payload
      (&frame->rst_stream, payload, payloadlen);
    break;
  case NGHTTP2_SETTINGS:
    rv = nghttp2_frame_unpack_settings_payload2(&frame->settings.iv,
                                                &frame->settings.niv,
                                                payload, payloadlen);
    break;
  case NGHTTP2_PUSH_PROMISE:
    rv = nghttp2_frame_unpack_push_promise_payload
      (&frame->push_promise, payload, payloadlen);
    break;
  case NGHTTP2_PING:
    nghttp2_frame_unpack_ping_payload
      (&frame->ping, payload, payloadlen);
    break;
  case NGHTTP2_GOAWAY:
    nghttp2_frame_unpack_goaway_payload
      (&frame->goaway, payload, payloadlen);
    break;
  case NGHTTP2_WINDOW_UPDATE:
    nghttp2_frame_unpack_window_update_payload
      (&frame->window_update, payload, payloadlen);
    break;
  default:
    /* Must not be reachable */
    assert(0);
  }
  return rv;
}

int strmemeq(const char *a, const uint8_t *b, size_t bn)
{
  const uint8_t *c;
  if(!a || !b) {
    return 0;
  }
  c = b + bn;
  for(; *a && b != c && *a == *b; ++a, ++b);
  return !*a && b == c;
}

int nvnameeq(const char *a, nghttp2_nv *nv)
{
  return strmemeq(a, nv->name, nv->namelen);
}

int nvvalueeq(const char *a, nghttp2_nv *nv)
{
  return strmemeq(a, nv->value, nv->valuelen);
}

void nva_out_init(nva_out *out)
{
  memset(out->nva, 0, sizeof(out->nva));
  out->nvlen = 0;
}

void nva_out_reset(nva_out *out)
{
  size_t i;
  for(i = 0; i < out->nvlen; ++i) {
    free(out->nva[i].name);
    free(out->nva[i].value);
  }
  memset(out->nva, 0, sizeof(out->nva));
  out->nvlen = 0;
}

void add_out(nva_out *out, nghttp2_nv *nv)
{
  nghttp2_nv *onv = &out->nva[out->nvlen];
  if(nv->namelen) {
    onv->name = malloc(nv->namelen);
    memcpy(onv->name, nv->name, nv->namelen);
  } else {
    onv->name = NULL;
  }
  if(nv->valuelen) {
    onv->value = malloc(nv->valuelen);
    memcpy(onv->value, nv->value, nv->valuelen);
  } else {
    onv->value = NULL;
  }
  onv->namelen = nv->namelen;
  onv->valuelen = nv->valuelen;
  ++out->nvlen;
}

ssize_t inflate_hd(nghttp2_hd_inflater *inflater, nva_out *out,
                   nghttp2_bufs *bufs, size_t offset)
{
  ssize_t rv;
  nghttp2_nv nv;
  int inflate_flags;
  nghttp2_buf_chain *ci;
  nghttp2_buf *buf;
  nghttp2_buf bp;
  int final;
  size_t processed;

  processed = 0;

  for(ci = bufs->head; ci; ci = ci->next) {
    buf = &ci->buf;
    final = nghttp2_buf_len(buf) == 0 || ci->next == NULL;
    bp = *buf;

    if(offset) {
      int n;

      n = nghttp2_min((ssize_t)offset, nghttp2_buf_len(&bp));
      bp.pos += n;
      offset -= n;
    }

    for(;;) {
      inflate_flags = 0;
      rv = nghttp2_hd_inflate_hd(inflater, &nv, &inflate_flags,
                                 bp.pos, nghttp2_buf_len(&bp), final);

      if(rv < 0) {
        return rv;
      }

      bp.pos += rv;
      processed += rv;

      if(inflate_flags & NGHTTP2_HD_INFLATE_EMIT) {
        add_out(out, &nv);
      }
      if(inflate_flags & NGHTTP2_HD_INFLATE_FINAL) {
        break;
      }
    }
  }

  nghttp2_hd_inflate_end_headers(inflater);

  return processed;
}

void frame_pack_bufs_init(nghttp2_bufs *bufs)
{
  /* 2 for PAD_HIGH and PAD_LOW */
  nghttp2_bufs_init2(bufs, 4096, 16, NGHTTP2_FRAME_HDLEN + 2);
}

void bufs_large_init(nghttp2_bufs *bufs, size_t chunk_size)
{
  /* 2 for PAD_HIGH and PAD_LOW */
  nghttp2_bufs_init2(bufs, chunk_size, 16, NGHTTP2_FRAME_HDLEN + 2);
}
