/*
 * nghttp2 - HTTP/2.0 C Library
 *
 * Copyright (c) 2013 Tatsuhiro Tsujikawa
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
#ifndef NGHTTP2_HD_TEST_H
#define NGHTTP2_HD_TEST_H

void test_nghttp2_hd_deflate(void);
void test_nghttp2_hd_deflate_same_indexed_repr(void);
void test_nghttp2_hd_deflate_common_header_eviction(void);
void test_nghttp2_hd_deflate_clear_refset(void);
void test_nghttp2_hd_inflate_indname_noinc(void);
void test_nghttp2_hd_inflate_indname_inc(void);
void test_nghttp2_hd_inflate_indname_inc_eviction(void);
void test_nghttp2_hd_inflate_newname_noinc(void);
void test_nghttp2_hd_inflate_newname_inc(void);
void test_nghttp2_hd_inflate_clearall_inc(void);
void test_nghttp2_hd_inflate_zero_length_huffman(void);
void test_nghttp2_hd_change_table_size(void);
void test_nghttp2_hd_deflate_inflate(void);

#endif /* NGHTTP2_HD_TEST_H */
