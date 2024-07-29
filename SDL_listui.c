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

#include "SDL_listui.h"


typedef struct ListUI_Item
{
    char               *label;
    struct ListUI_Item *next;
    struct ListUI_Item *prev;

    // Event listeners
    struct {
	ListUI_OnSelectCallback *fn;
	void * ctx;
    } on_select;
    struct {
	ListUI_OnActivateCallback *fn;
	void * ctx;
    } on_activate;
} ListUI_Item;


struct SDL_ListUI
{
    char        *title;
    ListUI_Item *first;
    ListUI_Item *last;

    // Rendering settings
    SDL_Color text_color;
    SDL_Color activate_color;
    SDL_Color selected_color;

    // Rendering state
    ListUI_Item *top;
    ListUI_Item *selected;
    ListUI_Item *bottom;

    // Event listeners
    struct {
	ListUI_OnDestroyCallback *fn;
	void * ctx;
    } on_destroy;
};


static ListUI_Item* ListUI_ItemSplit(ListUI_Item* item)
{
    ListUI_Item *fast = item;
    ListUI_Item *slow = item;
    ListUI_Item *ret;

    while(fast->next && fast->next->next) {
	fast = fast->next->next;
	slow = slow->next;
    }

    ret = slow->next;
    slow->next = 0;

    return ret;
}


static ListUI_Item* ListUI_ItemMerge(ListUI_Item* a, ListUI_Item* b,
				     ListUI_CompareCallback* cmp)
{
    if(!a) {
	return b;
    }
    if(!b) {
	return a;
    }

    if(cmp(a->label, b->label) < 0) {
	a->next = ListUI_ItemMerge(a->next, b, cmp);
	a->next->prev = a;
	a->prev = 0;
	return a;
    } else {
	b->next = ListUI_ItemMerge(a, b->next, cmp);
	b->next->prev = b;
	b->prev = 0;
	return b;
    }
}


static ListUI_Item* ListUI_ItemMergeSort(ListUI_Item* item,
					 ListUI_CompareCallback* cmp)
{
    ListUI_Item *other;

    if(!item || !item->next) {
	return item;
    }

    other = ListUI_ItemSplit(item);

    item  = ListUI_ItemMergeSort(item, cmp);
    other = ListUI_ItemMergeSort(other, cmp);

    return ListUI_ItemMerge(item, other, cmp);
}


int ListUI_DefaultCompareCallback(const char* s1, const char* s2) {
    int i = SDL_strcasecmp(s1, s2);
    if(i == 0) {
	i = SDL_strcmp(s1, s2);
    }
    return i;
}


void ListUI_Sort(SDL_ListUI* l, ListUI_CompareCallback* cmp)
{
    if(!l->first) {
	return;
	}

    if(!cmp) {
	cmp = ListUI_DefaultCompareCallback;
    }

    l->top = l->selected;
    l->bottom = 0;
    l->first = l->last = ListUI_ItemMergeSort(l->first, cmp);

    while(l->last->next) {
	l->last = l->last->next;
    }
}


SDL_ListUI* ListUI_Create(const char* title)
{
    SDL_ListUI* l = SDL_calloc(1, sizeof(SDL_ListUI));

    l->text_color.r = 240;
    l->text_color.g = 240;
    l->text_color.b = 240;
    l->text_color.a = 120;

    l->activate_color.r = 240;
    l->activate_color.g = 240;
    l->activate_color.b = 240;
    l->activate_color.a = 255;

    l->selected_color.r = 120;
    l->selected_color.g = 120;
    l->selected_color.b = 0;
    l->selected_color.a = 255;

    ListUI_SetTitle(l, title);

    return l;
}


void ListUI_Clear(SDL_ListUI* l)
{
    ListUI_Item *next;

    while(l->first) {
	next = l->first->next;
	SDL_free(l->first->label);
	SDL_free(l->first);
	l->first = next;
    }
    l->top = l->selected = l->bottom = 0;
}


void ListUI_Destroy(SDL_ListUI* l)
{

    if(l->on_destroy.fn) {
	l->on_destroy.fn(l->on_destroy.ctx, l);
    }

    ListUI_Clear(l);
    SDL_free(l->title);
    SDL_free(l);
}


void ListUI_OnDestroy(SDL_ListUI* l, ListUI_OnDestroyCallback* fn, void* ctx)
{
    l->on_destroy.fn = fn;
    l->on_destroy.ctx = ctx;
}


void ListUI_SetTitle(SDL_ListUI* l, const char* title)
{
    if(!title) {
	title = "";
    }
    if(l->title) {
	SDL_free(l->title);
    }
    l->title = SDL_strdup(title);
}


void ListUI_SetTextColor(SDL_ListUI* l, SDL_Color c)
{
    l->text_color = c;
}


void ListUI_SetSelectedColor(SDL_ListUI* l, SDL_Color c)
{
    l->selected_color = c;
}


void ListUI_SetActivateTextColor(SDL_ListUI* l, SDL_Color c)
{
    l->activate_color = c;
}


Uint64 ListUI_AppendItem(SDL_ListUI* l, const char* label)
{
    ListUI_Item* item = SDL_calloc(1, sizeof(ListUI_Item));

    if(!label) {
	label = "";
    }
    item->label = SDL_strdup(label);

    if(!l->first) {
	l->first = l->last = item;
    } else {
	item->prev = l->last;
	l->last->next = item;
	l->last = item;
    }

    return (Uint64)item;
}


static ListUI_Item* ListUI_GetItem(SDL_ListUI* l, Uint64 id)
{
    for(ListUI_Item* it=l->first; it!=0; it=it->next) {
	if(id == (Uint64)it) {
	    return it;
	}
    }
    return 0;
}


SDL_bool ListUI_SetItemLabel(SDL_ListUI* l, Uint64 id, const char* label)
{
    ListUI_Item* it = ListUI_GetItem(l, id);

    if(it) {
	if(it->label) {
	    SDL_free(it->label);
	}
	it->label = SDL_strdup(label);
	return SDL_TRUE;
    }

    return SDL_FALSE;
}


SDL_bool ListUI_OnSelect(SDL_ListUI* l, Uint64 id,
			 ListUI_OnSelectCallback* fn, void* ctx)
{
    ListUI_Item* it = ListUI_GetItem(l, id);

    if(it) {
	it->on_select.fn = fn;
	it->on_select.ctx = ctx;
	return SDL_TRUE;
    }

    return SDL_FALSE;
}


SDL_bool ListUI_OnActivate(SDL_ListUI* l, Uint64 id,
			   ListUI_OnActivateCallback* fn, void* ctx)
{
    ListUI_Item* it = ListUI_GetItem(l, id);

    if(it) {
	it->on_activate.fn = fn;
	it->on_activate.ctx = ctx;
	return SDL_TRUE;
    }

    return SDL_FALSE;
}


void ListUI_NavigateItemUp(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound)
{
    if(!l->selected || !l->selected->prev) {
	if(wraparound) {
	    l->bottom = l->selected = l->last;
	    l->top = 0;
	}
    } else {
	l->selected = l->selected->prev;
	if(l->selected == l->top && l->top->prev) {
	    l->top = l->top->prev;
	    l->bottom = l->bottom->prev;
	}
    }

    if(l->selected && l->selected->on_select.fn && !silent) {
	l->selected->on_select.fn(l->selected->on_select.ctx, l,
				  (Uint64)l->selected);
    }
}


void ListUI_NavigateItemDown(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound)
{
    if(!l->selected || !l->selected->next) {
	if(wraparound) {
	    l->top = l->selected = l->first;
	    l->bottom = 0;
	}
    } else {
	l->selected = l->selected->next;
	if(l->selected == l->bottom && l->bottom->next) {
	    l->bottom = l->bottom->next;
	    l->top = l->top->next;
	}
    }

    if(l->selected && l->selected->on_select.fn && !silent) {
	l->selected->on_select.fn(l->selected->on_select.ctx, l,
				  (Uint64)l->selected);
    }
}


void ListUI_NavigatePageUp(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound)
{
    int n = 0;

    for(ListUI_Item* it=l->top; it && it!=l->bottom; it=it->next) {
	n++;
    }
    for(int i=0; i<n-1; i++) {
	ListUI_NavigateItemUp(l, SDL_TRUE, wraparound);
    }
    if(n) {
	ListUI_NavigateItemUp(l, silent, wraparound);
    }
}


void ListUI_NavigatePageDown(SDL_ListUI* l, SDL_bool silent, SDL_bool wraparound)
{
    int n = 0;

    for(ListUI_Item* it=l->top; it && it!=l->bottom; it=it->next) {
	n++;
    }
    for(int i=0; i<n-1; i++) {
	ListUI_NavigateItemDown(l, SDL_TRUE, wraparound);
    }
    if(n) {
	ListUI_NavigateItemDown(l, silent, wraparound);
    }
}


void ListUI_ActivateSelected(SDL_ListUI* l)
{
    if(!l->selected || !l->selected->on_activate.fn) {
	return;
    }

    l->selected->on_activate.fn(l->selected->on_activate.ctx, l,
				(Uint64)l->selected);
}


static void ListUI_RenderText(SDL_Renderer* renderer, const char* text,
			      TTF_Font* font, int x, int y, SDL_Color color)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


void ListUI_Render(SDL_ListUI* l, SDL_Renderer* renderer, TTF_Font* font)
{
    int item_height = (int)TTF_FontHeight(font);
    int padding = item_height / 4;
    ListUI_Item* it;
    SDL_Color color;
    SDL_Rect rect;
    int x = 0;
    int y = 0;
    int w, h;

    if(SDL_GetRendererOutputSize(renderer, &w, &h)) {
	return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Render title
    y += padding;
    ListUI_RenderText(renderer, l->title, font, x+padding, y, l->activate_color);
    y += item_height;

    // Render horizontal line
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = padding / 4;
    SDL_SetRenderDrawColor(renderer, l->activate_color.r, l->activate_color.g,
			   l->activate_color.b, l->activate_color.a);
    SDL_RenderFillRect(renderer, &rect);
    y += padding;

    if(!l->selected) {
	l->selected = l->top = l->first;
    }

    // Cursor moved from top to bottom, figure out how many items we can fit
    if(!l->top && l->bottom) {
	l->top = l->bottom;
	for(int i=y; i < h-item_height-padding && l->top->prev; i+=item_height) {
	    l->top = l->top->prev;
	}
    }

    // Render list of items
    it = l->top;
    while(y < h-item_height && it) {
	if(it == l->selected) {
	    rect.x = x;
	    rect.y = y;
	    rect.w = w;
	    rect.h = TTF_FontHeight(font);
	    SDL_SetRenderDrawColor(renderer,
				   l->selected_color.r, l->selected_color.g,
				   l->selected_color.b, l->selected_color.a);
	    SDL_RenderFillRect(renderer, &rect);
	}
	if (it->on_activate.fn) {
	    color = l->activate_color;
	} else {
	    color = l->text_color;
	}

	ListUI_RenderText(renderer, it->label, font, x+padding, y, color);

	l->bottom = it;
	it = it->next;
	y += item_height;
    }
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
