/*
 * librd - Rapid Development C library
 *
 * Copyright (c) 2012, Magnus Edenhill
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Various encoding/decoding schemes.
 */
#include "rdencoding.h"


uint64_t rd_varint_decode_u64 (const void *buf,
			       size_t size, int *vlenp) {
	const unsigned char *s = buf;
	const unsigned char *end = s + size;
	uint64_t uval = 0;
	int bits = 0;

	*vlenp = 0;
	
	while (&s[*vlenp] < end) {
		if (s[*vlenp] < 0x80) {
			if (*vlenp > 9) {
				/* Overflow */
				*vlenp = -*vlenp;
				return 0;
			}
			return uval | (((uint64_t)s[(*vlenp)++]) << bits);
		}

		uval |= ((uint64_t)(s[(*vlenp)++] & 0x7f)) << bits;
		bits += 7;
	}

	if (*vlenp == 0 || s[(*vlenp)-1] >= 0x80) {
	  /* Buffer underflow, indicate that we need more data. */
	  *vlenp = -((*vlenp)+1);
	}

	return 0;
}


int64_t rd_varint_decode_s64 (const void *buf, size_t size, int *vlenp) {
	uint64_t uval;
	int64_t val;

	uval = rd_varint_decode_u64(buf, size, vlenp);

	if (*vlenp <= 0)
		return 0;

	val = uval >> 1;
	if (uval & 1)
		val ^= val;

	return val;
}


int rd_varint_encode_u64 (uint64_t uval, void *dest, size_t size) {
	unsigned char *s = dest;
	const unsigned char *end = s + size;

	while (uval >= 0x80) {
		if (s >= end)
			return -1;

		*(s++) = (uval & 0xff) | 0x80;
		uval >>= 7;
	}

	if (s >= end)
		return -1;

	*(s++) = uval & 0xff;

	return (int)(s - (unsigned char *)dest);
}


int rd_varint_encode_s64 (int64_t val, void *dest, size_t size) {
	uint64_t uval = (uint64_t)val << 1;
	if (val < 0)
		uval ^= uval;

	return rd_varint_encode_u64(uval, dest, size);
}
