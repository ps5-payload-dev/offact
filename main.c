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

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "SDL_listui.h"
#include "offact.h"

#include "font.h"
#include "snd_nav.h"


#define WINDOW_TITLE  "OffAct"
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1280


static Uint64 selected_item_id = 0;
static Mix_Chunk* snd_nav;
static SDL_ListUI *ui;


/**
 * Obtain a textual label for an account with the given index.
 **/
static int GetLabel(int account_index, char* label, size_t size)
{
    char account_name[ACCOUNT_NAME_MAX];
    char account_type[ACCOUNT_TYPE_MAX];
    Uint64 account_id;
    int account_flags;

    if(OffAct_GetAccountName(account_index, account_name)) {
	return -1;
    }
    if(!*account_name) {
	return -1;
    }
    if(OffAct_GetAccountId(account_index, &account_id)) {
	return -1;
    }
    if(OffAct_GetAccountType(account_index, account_type)) {
	return -1;
    }
    if(OffAct_GetAccountFlags(account_index, &account_flags)) {
	return -1;
    }

    SDL_snprintf(label, size, "type: ""%2s  "		\
		 "flags: 0x%04x  id: 0x%016lx  name: %s",
		 account_type, account_flags, account_id, account_name);

    return account_id;
}


/**
 * Play nav.wav when a new item is selected
 **/
static void OnSelect(void *ctx)
{
    selected_item_id = (Uint64)ctx;
    Mix_PlayChannel(-1, snd_nav, 0);
}


/**
 * Activate the selected account, play nav.wav, and update the label.
 **/
static void OnActivate(void *ctx)
{
    char account_type[ACCOUNT_TYPE_MAX] = "np";
    char account_name[ACCOUNT_NAME_MAX];
    int account_index = (Uint64)ctx;
    int account_flags = 4098;
    Uint64 account_id;
    char buf[255];

    if(OffAct_GetAccountName(account_index, account_name)) {
	return;
    }

    account_id = OffAct_AccountName2Id(account_name);

    OffAct_SetAccountId(account_index, account_id);
    OffAct_SetAccountType(account_index, account_type);
    OffAct_SetAccountFlags(account_index, account_flags);

    if(GetLabel(account_index, buf, sizeof(buf))) {
	ListUI_SetItemLabel(ui, selected_item_id, buf);
    }

    ListUI_OnActivate(ui, selected_item_id, 0, 0);
    Mix_PlayChannel(-1, snd_nav, 0);
}


int main(int argc, char* args[])
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_RWops* rwops;
    SDL_Event event;
    TTF_Font* font;
    Uint64 item_id;
    char buf[255];
    int quit = 0;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
	return -1;
    }

    if(!(window=SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
				 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
				 SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN))) {
	return -1;
    }

    if(!(renderer=SDL_CreateRenderer(window, -1, (SDL_RENDERER_PRESENTVSYNC |
						  SDL_RENDERER_SOFTWARE)))) {
	return -1;
    }

    SDL_GameControllerOpen(0);

    if(Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
	return -1;
    }

    if(!(rwops=SDL_RWFromMem(assets_nav_wav, assets_nav_wav_len))) {
	return -1;
    }
    if(!(snd_nav=Mix_LoadWAV_RW(rwops, 1))) {
	return -1;
    }

    if(TTF_Init()  < 0) {
	return -1;
    }
    if(!(rwops=SDL_RWFromMem(assets_font_ttf, assets_font_ttf_len))) {
	return -1;
    }
    if(!(font=TTF_OpenFontRW(rwops, 1, 38))) {
	return -1;
    }

    ui = ListUI_Create("Offline Account Activation");

    for (Uint64 i=1; i<=ACCOUNT_MAX; i++) {
	int account_id = GetLabel(i, buf, sizeof(buf));
	if(account_id == -1) {
	    continue;
	}

	item_id = ListUI_AppendItem(ui, buf);
	ListUI_OnSelect(ui, item_id, OnSelect, (void*)item_id);
	if(account_id == 0) {
	    ListUI_OnActivate(ui, item_id, OnActivate, (void*)i);
	}
    }

    while(!quit) {
	while(SDL_PollEvent(&event) != 0) {
	    if(event.type == SDL_CONTROLLERBUTTONDOWN) {
		switch(event.cbutton.button) {
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
		    ListUI_NavigateItemUp(ui, SDL_FALSE, SDL_TRUE);
		    break;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
		    ListUI_NavigateItemDown(ui, SDL_FALSE, SDL_TRUE);
		    break;
		case SDL_CONTROLLER_BUTTON_A:
		    ListUI_ActivateSelected(ui);
		    break;
		}
	    }
	}

	SDL_SetRenderDrawColor(renderer, 5, 5, 5, 255);
	SDL_RenderClear(renderer);

	ListUI_Render(ui, renderer, font);
	SDL_RenderPresent(renderer);
    }

    ListUI_Destroy(ui);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}


/* Local Variables: */
/* tab-width: 8 */
/* c-basic-offset: 4 */
/* End: */
