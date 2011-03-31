/*
 *   Copyright (C) 2007-2010 Tristan Heaven <tristanheaven@gmail.com>
 *
 *   This file is part of GtkHash.
 *
 *   GtkHash is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GtkHash is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GtkHash. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTKHASH_HASH_FUNC_H
#define GTKHASH_HASH_FUNC_H

#include <stdbool.h>

#define HASH_FUNCS_N 27
#define HASH_FUNC_IS_DEFAULT(X) \
	((X) == HASH_FUNC_MD5 || (X) == HASH_FUNC_SHA1 || (X) == HASH_FUNC_SHA256)

// All supported hash functions
enum hash_func_e {
	HASH_FUNC_INVALID     = -1,
	HASH_FUNC_MD2         =  0,
	HASH_FUNC_MD4         =  1,
	HASH_FUNC_MD5         =  2,
	HASH_FUNC_SHA1        =  3,
	HASH_FUNC_SHA224      =  4,
	HASH_FUNC_SHA256      =  5,
	HASH_FUNC_SHA384      =  6,
	HASH_FUNC_SHA512      =  7,
	HASH_FUNC_RIPEMD128   =  8,
	HASH_FUNC_RIPEMD160   =  9,
	HASH_FUNC_RIPEMD256   = 10,
	HASH_FUNC_RIPEMD320   = 11,
	HASH_FUNC_HAVAL128    = 12,
	HASH_FUNC_HAVAL160    = 13,
	HASH_FUNC_HAVAL192    = 14,
	HASH_FUNC_HAVAL224    = 15,
	HASH_FUNC_HAVAL256    = 16,
	HASH_FUNC_TIGER128    = 17,
	HASH_FUNC_TIGER160    = 18,
	HASH_FUNC_TIGER192    = 19,
	HASH_FUNC_GOST        = 20,
	HASH_FUNC_WHIRLPOOL   = 21,
	HASH_FUNC_SNEFRU128   = 22,
	HASH_FUNC_SNEFRU256   = 23,
	HASH_FUNC_CRC32       = 24,
	HASH_FUNC_CRC32B      = 25,
	HASH_FUNC_ADLER32     = 26
};

struct hash_func_s {
	enum hash_func_e id;
	bool enabled;
	const char *name;
	struct {
		char *digest;
		void *lib_data;
	} priv;
};

enum hash_func_e gtkhash_hash_func_get_id_from_name(const char *name);
void gtkhash_hash_func_set_digest(struct hash_func_s *func, char *digest);
const char *gtkhash_hash_func_get_digest(struct hash_func_s *func);
void gtkhash_hash_func_init_all(struct hash_func_s *funcs);
void gtkhash_hash_func_deinit_all(struct hash_func_s *funcs);

#endif