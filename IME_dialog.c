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

#include <stdint.h>
#include <stdlib.h>

#include "IME_dialog.h"


typedef enum SceImeDialogStatus
{
    SCE_IME_DIALOG_STATUS_NONE,
    SCE_IME_DIALOG_STATUS_RUNNING,
    SCE_IME_DIALOG_STATUS_FINISHED
} SceImeDialogStatus;


typedef int (*SceImeTextFilter)(wchar_t*, uint32_t*, const wchar_t*, uint32_t);


typedef struct SceImeDialogParam
{
    int userId;
    enum {
	SCE_IME_TYPE_DEFAULT,
	SCE_IME_TYPE_BASIC_LATIN,
	SCE_IME_TYPE_URL,
	SCE_IME_TYPE_MAIL,
	SCE_IME_TYPE_NUMBER
    } type;
    uint64_t supportedLanguages;
    enum {
	SCE_IME_ENTER_LABEL_DEFAULT,
	SCE_IME_ENTER_LABEL_SEND,
	SCE_IME_ENTER_LABEL_SEARCH,
	SCE_IME_ENTER_LABEL_GO,
    } enterLabel;
    enum {
	SCE_IME_INPUT_METHOD_DEFAULT
    } inputMethod;
    SceImeTextFilter filter;
    uint32_t option;
    uint32_t maxTextLength;
    wchar_t *inputTextBuffer;
    float posx;
    float posy;
    enum {
	SCE_IME_HALIGN_LEFT,
	SCE_IME_HALIGN_CENTER,
	SCE_IME_HALIGN_RIGHT
    } halign;
    enum {
	SCE_IME_VALIGN_TOP,
	SCE_IME_VALIGN_CENTER,
	SCE_IME_VALIGN_BOTTOM
    } valign;
    const wchar_t *placeholder;
    const wchar_t *title;
    int8_t reserved[16];
} SceImeDialogParam;


typedef struct SceImeDialogResult
{
    enum {
	SCE_IME_DIALOG_END_STATUS_OK,
	SCE_IME_DIALOG_END_STATUS_USER_CANCELED,
	SCE_IME_DIALOG_END_STATUS_ABORTED,
    } outcome;
    int8_t reserved[12];
} SceImeDialogResult;


int sceUserServiceGetForegroundUser(int*);

int sceImeDialogInit(const SceImeDialogParam*, void*);
int sceImeDialogGetResult(SceImeDialogResult*);
int sceImeDialogTerm(void);

SceImeDialogStatus sceImeDialogGetStatus(void);


static IME_Dialog_OnOutcomeCallback* g_outcome_fn;
static SceImeDialogStatus g_status;
static void* g_outcome_ctx = 0;
static wchar_t g_title[0x80];
static wchar_t g_text[0x800];

static SceImeDialogParam g_param =
{
    .title = g_title,
    .inputTextBuffer = g_text,
    .maxTextLength = sizeof(g_text) / sizeof(g_text[0])
};


void IME_Dialog_OnOutcome(IME_Dialog_OnOutcomeCallback *fn, void* ctx)
{
    g_outcome_fn = fn;
    g_outcome_ctx = ctx;
}


int IME_Dialog_SetTitle(const char* title)
{
    if(mbstowcs(g_title, title, sizeof(g_title)) < 0) {
	return -1;
    }
    return 0;
}


int IME_Dialog_SetText(const char* text)
{
    if(mbstowcs(g_text, text, sizeof(g_text)) < 0) {
	return -1;
    }
    return 0;
}


int IME_Dialog_GetText(char* text, size_t size)
{
    if(wcstombs(text, g_text, size) < 0) {
	return -1;
    }
    return 0;
}


int IME_Dialog_Display(void)
{
    int err;
    if((err=sceUserServiceGetForegroundUser(&g_param.userId))) {
	return err;
    }
    if((err=sceImeDialogInit(&g_param, NULL))) {
	return err;
    }
    return 0;
}


int IME_Dialog_PullStatus(void)
{
    SceImeDialogStatus status = sceImeDialogGetStatus();
    SceImeDialogResult result = {0};
    int err;

    if(g_status == status) {
	return 0;
    } else {
	g_status = status;
    }

    switch(status) {
    case SCE_IME_DIALOG_STATUS_NONE:
	return 0;
    case SCE_IME_DIALOG_STATUS_RUNNING:
	return 1;
    case SCE_IME_DIALOG_STATUS_FINISHED:
	break;
    default:
	return -1;
    }

    if((err=sceImeDialogGetResult(&result))) {
	return -1;
    }
    if((err=sceImeDialogTerm())) {
	return err;
    }

    switch(result.outcome) {
    case SCE_IME_DIALOG_END_STATUS_OK:
	if(g_outcome_fn) {
	    g_outcome_fn(g_outcome_ctx, IME_DIALOG_COMPLETED);
	}
	return 0;
    case SCE_IME_DIALOG_END_STATUS_USER_CANCELED:
	if(g_outcome_fn) {
	    g_outcome_fn(g_outcome_ctx, IME_DIALOG_CANCELED);
	}
	return 0;
    case SCE_IME_DIALOG_END_STATUS_ABORTED:
	if(g_outcome_fn) {
	    g_outcome_fn(g_outcome_ctx, IME_DIALOG_ABORTED);
	}
	return 0;
    default:
	return -1;
    }
    return -1;
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
