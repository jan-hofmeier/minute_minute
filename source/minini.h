/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _MININI_H
#define _MININI_H

#include "types.h"

typedef int (*minini_handler)(const char* key, const char* value);

bool minini_get_bool(const char* value, bool default_value);
long long minini_get_int(const char* value, long long default_value);
unsigned long long minini_get_uint(const char* value, unsigned long long default_value);
double minini_get_real(const char* value, double default_value);
size_t minini_get_bytes(const char* value, void* out, size_t max);

FILE* minini_open_config(void);
int minini_init(void);

int mcp_ini();
int boot_ini();

#endif
