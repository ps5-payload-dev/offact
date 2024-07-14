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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

/**
 * ListUI renders a simple user interface for a list of labeled items.
 * The UI keeps track of the currently selected item with a cursor than can
 * be moved up or down, as illustrated below. Selected items can be 'activated',
 * to trigger a user-defined action.
 *
 *  List Title
 *  ==========
 *  Item 1
 *  Item 2
 *  -> Selected Item
 *  Item N
 *
 * There are three types of events that can be signaled via callback functions
 * in the API:
 *  1) OnSelect   - events that occur when the cursor points at a new item,
 *  2) OnActivate - events that occur when a selected item is activated,
 *  3) onDestroy  - events that occur when the ListUI instance is about to free
 *                  up all of its memory.
 **/
struct SDL_ListUI;
typedef struct SDL_ListUI SDL_ListUI;


/**
 * Prototype for OnSelect event callbacks.
 **/
typedef void (ListUI_OnSelectCallback)(void *ctx, SDL_ListUI* l, Uint64 id);


/**
 * Prototype for OnActivate event callbacks.
 **/
typedef void (ListUI_OnActivateCallback)(void *ctx, SDL_ListUI* l, Uint64 id);


/**
 * Prototype for OnDestroy event callbacks.
 **/
typedef void (ListUI_OnDestroyCallback)(void *ctx, SDL_ListUI* l);


/**
 * Prototype for sort comparison callbacks.
 **/
typedef int (ListUI_CompareCallback)(const char* s1, const char* s2);


/**
 * Create a new ListUI instance.
 **/
SDL_ListUI* ListUI_Create(const char* title);


/**
 * Free all memory associated with a ListUI instance.
 **/
void ListUI_Destroy(SDL_ListUI* l);


/**
 * Change the callback function that is invoked when the ListUI instance
 * is about to free up all of its memory.
 **/
void ListUI_OnDestroy(SDL_ListUI* l, ListUI_OnDestroyCallback* fn, void* ctx);


/**
 * Change the title of a ListUI instance.
 **/
void ListUI_SetTitle(SDL_ListUI* l, const char* title);


/**
 * Change the text color.
 **/
void ListUI_SetTextColor(SDL_ListUI* l, SDL_Color c);


/**
 * Change the text color of selected items.
 **/
void ListUI_SetSelectedColor(SDL_ListUI* l, SDL_Color c);


/**
 * Change the text color of items that can be activated.
 **/
void ListUI_SetActivateTextColor(SDL_ListUI* l, SDL_Color c);


/**
 * Append a new item at the bottom of a ListUI instance, and
 * return a identifier that is unique to the new item.
 **/
Uint64 ListUI_AppendItem(SDL_ListUI* l, const char* label);


/**
 * Remove all items from the list.
 **/
void ListUI_Clear(SDL_ListUI* l);


/**
 * Change the label of an item with the given identifier.
 **/
SDL_bool ListUI_SetItemLabel(SDL_ListUI* l, Uint64 id, const char* label);


/**
 * Move the selection cursor of the given ListUI instance one step towards the
 * top. Optionally, event generation may be silenced, and wrap-around
 * navigation may be enabled.
 **/
void ListUI_NavigateItemUp(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound);


/**
 * Move the selection cursor of the given ListUI instance one step towards the
 * bottom. Optionally, event generation may be silenced, and wrap-around
 * navigation may be enabled.
 **/
void ListUI_NavigateItemDown(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound);


/**
 * Move the selection cursor of the given ListUI instance one visable page up
 * towards the top. Optionally, event generation may be silenced, and
 * wrap-around navigation may be enabled.
 **/
void ListUI_NavigatePageUp(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound);


/**
 * Move the selection cursor of given ListUI instance one visable page down
 * towards the bottom. Optionally, event generation may be silenced, and
 * wrap-around navigation may be enabled.
 **/
void ListUI_NavigatePageDown(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound);


/**
 * Change the callback function that is invoked when an item with the
 * given identifier is selected.
 **/
SDL_bool ListUI_OnSelect(SDL_ListUI* l, Uint64 id, ListUI_OnSelectCallback* fn,
			 void* ctx);


/**
 * Activate the selected item in a given ListUI instance.
 **/
void ListUI_ActivateSelected(SDL_ListUI* l);


/**
 * Change the callback function that is invoked when an item with the
 * given identifier is activated.
 **/
SDL_bool ListUI_OnActivate(SDL_ListUI* l, Uint64 id, ListUI_OnActivateCallback* fn,
			   void* ctx);


/**
 * Sort the items in the list according the the labels of items.
 **/
void ListUI_Sort(SDL_ListUI* l, ListUI_CompareCallback* cmp);


/**
 * Default soty compare callback function that yields lexical accending order.
 **/
int ListUI_DefaultCompareCallback(const char* s1, const char* s2);


/**
 * Render a ListUI instance with the given font.
 **/
void ListUI_Render(SDL_ListUI* l, SDL_Renderer* renderer, TTF_Font* font);


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
