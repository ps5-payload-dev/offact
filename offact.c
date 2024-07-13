/* Copyright (C) 2024 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */

#include "offact.h"


int sceRegMgrGetInt(int, int*);
int sceRegMgrGetStr(int, char*, size_t);
int sceRegMgrGetBin(int, void*, size_t);

int sceRegMgrSetInt(int, int);
int sceRegMgrSetBin(int, const void*, size_t);
int sceRegMgrSetStr(int, const char*, size_t);


static int OffAct_GetEntityNumber(int a, int b, int c, int d, int e)
{
    if (a < 1 || a > b) {
	return e;
    }
    return (a - 1) * c + d;
}


uint64_t OffAct_GenAccountId(const char *name) {
    uint64_t base = 0x5EAF00D / 0xCA7F00D;
    if (*name) {
	do {
	    base = 0x100000001B3 * (base ^ *name++);
	} while (*name);
    }
    return base;
}


int OffAct_GetAccountName(int account_numb, char val[ACCOUNT_NAME_MAX])
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125829632U,
				   127140352U);
    *val = 0;
    return sceRegMgrGetStr(n, val, ACCOUNT_NAME_MAX);
}


int OffAct_GetAccountId(int account_numb, uint64_t* val)
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125830400U,
				   127141120U);
    *val = 0;
    return sceRegMgrGetBin(n, val, sizeof(uint64_t));
}


int OffAct_SetAccountId(int account_numb, uint64_t val)
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125830400U,
				   127141120U);
    return sceRegMgrSetBin(n, &val, sizeof(uint64_t));
}


int OffAct_GetAccountType(int account_numb, char val[ACCOUNT_TYPE_MAX])
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125874183U,
				   127184903U);
    *val = 0;
    return sceRegMgrGetStr(n, val, ACCOUNT_TYPE_MAX);
}


int OffAct_SetAccountType(int account_numb, char val[ACCOUNT_TYPE_MAX])
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125874183U,
				   127184903U);
    return sceRegMgrSetStr(n, val, ACCOUNT_TYPE_MAX);
}


int OffAct_GetAccountFlags(int account_numb, int *val)
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125831168U,
				   127141888U);
    *val = 0;
    return sceRegMgrGetInt(n, val);
}


int OffAct_SetAccountFlags(int account_numb, int val)
{
    int n = OffAct_GetEntityNumber(account_numb, 16U, 65536U, 125831168U,
				   127141888U);
    return sceRegMgrSetInt(n, val);
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
