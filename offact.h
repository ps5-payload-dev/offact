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

#pragma once

#include <stdint.h>
#include <unistd.h>


#define ACCOUNT_NUMB_MAX 16
#define ACCOUNT_TYPE_MAX 17
#define ACCOUNT_NAME_MAX 32


int OffAct_GetAccountName(int account_numb, char val[ACCOUNT_NAME_MAX]);

int      OffAct_GetAccountId(int account_numb, uint64_t* val);
int      OffAct_SetAccountId(int account_numb, uint64_t  val);
uint64_t OffAct_GenAccountId(const char *name);

int OffAct_GetAccountType(int account_numb, char val[ACCOUNT_TYPE_MAX]);
int OffAct_SetAccountType(int account_numb, char val[ACCOUNT_TYPE_MAX]);

int OffAct_GetAccountFlags(int account_numb, int *val);
int OffAct_SetAccountFlags(int account_numb, int  val);


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
