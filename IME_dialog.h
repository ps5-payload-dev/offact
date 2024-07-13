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


/**
 * Potential outcomes of the dialog.
 **/
typedef enum IME_Dialog_Outcome
{
    IME_DIALOG_COMPLETED, // by user
    IME_DIALOG_CANCELED,  // by user
    IME_DIALOG_ABORTED,   // by system
} IME_Dialog_Outcome;


/**
 * Prototype for the callback function that catches an outcome.
 **/
typedef void (IME_Dialog_OnOutcomeCallback)(void*, IME_Dialog_Outcome);


/**
 * Set the callback function that is signaled when a dialog status changes.
 **/
void IME_Dialog_OnOutcome(IME_Dialog_OnOutcomeCallback *fn, void* ctx);


/**
 * Change the title of the dialog.
 **/
int IME_Dialog_SetTitle(const char* text);


/**
 * Change the text of the dialog.
 **/
int IME_Dialog_SetText(const char* text);


/**
 * Get the text of the dialog.
 **/
int IME_Dialog_GetText(char* text, size_t size);


/**
 * Display the dialog.
 **/
int IME_Dialog_Display(void);


/**
 * Get the state of the dialog, and signal changes to the calbback function.
 **/
int IME_Dialog_PullStatus(void);


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
