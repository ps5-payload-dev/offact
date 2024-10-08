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

#include "IME_dialog.h"
#include "SDL_listui.h"
#include "offact.h"

#include "font.h"
#include "readme.h"
#include "snd_nav.h"


#define WINDOW_TITLE  "OffAct"
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1280


static Mix_Chunk* snd_nav;
static SDL_ListUI *ui;


static void refreshListUI(void);


/**
 * Obtain a textual label for an account with the given number.
 **/
static int GetItemLabel(int account_numb, char* label, size_t size)
{
    char account_name[ACCOUNT_NAME_MAX];
    char account_type[ACCOUNT_TYPE_MAX];
    Uint64 account_id;
    int account_flags;

    if(OffAct_GetAccountName(account_numb, account_name)) {
	return -1;
    }
    if(!*account_name) {
	return -1;
    }
    if(OffAct_GetAccountId(account_numb, &account_id)) {
	return -1;
    }
    if(OffAct_GetAccountType(account_numb, account_type)) {
	return -1;
    }
    if(OffAct_GetAccountFlags(account_numb, &account_flags)) {
	return -1;
    }

    SDL_snprintf(label, size, "type: ""%2s  "		\
		 "flags: 0x%04x  id: 0x%016lx  name: %s",
		 account_type, account_flags, account_id, account_name);

    return 0;
}


static void OnDialogOutcome(void* ctx, IME_Dialog_Outcome outcome) {
    char account_type[ACCOUNT_TYPE_MAX] = "np";
    int account_numb = (int)(Uint64)ctx;
    int account_flags = 4098;
    Uint64 account_id;
    char buf[255];

    if(outcome != IME_DIALOG_COMPLETED) {
	return;
    }
    if(IME_Dialog_GetText(buf, sizeof(buf)) < 0) {
	return;
    }
    if(sscanf(buf, "0x%lx", &account_id) != 1) {
	return;
    }

    OffAct_SetAccountId(account_numb, account_id);
    OffAct_SetAccountType(account_numb, account_type);
    OffAct_SetAccountFlags(account_numb, account_flags);

    refreshListUI();
}


/**
 * Play nav.wav when a new item is selected.
 **/
static void OnSelectItem(void *ctx, SDL_ListUI *listui, Uint64 item_id)
{
    Mix_PlayChannel(-1, snd_nav, 0);
}


/**
 * Bring up the IME dialog for user input.
 **/
static void OnActivateItem(void *ctx, SDL_ListUI *listui, Uint64 item_id)
{
    int account_numb = (int)(Uint64)ctx;
    char account_name[ACCOUNT_NAME_MAX];
    Uint64 account_id;
    char buf[255];

    if(OffAct_GetAccountName(account_numb, account_name)) {
	return;
    }
    if(OffAct_GetAccountId(account_numb, &account_id)) {
	return;
    }
    if(!account_id) {
	account_id = OffAct_GenAccountId(account_name);
    }
    sprintf(buf, "Enter account id for user %s", account_name);
    if(IME_Dialog_SetTitle(buf) < 0) {
	return;
    }
    sprintf(buf, "0x%lx", account_id);
    if(IME_Dialog_SetText(buf) < 0) {
	return;
    }

    IME_Dialog_OnOutcome(OnDialogOutcome, (void*)(Uint64)account_numb);
    if(IME_Dialog_Display()) {
	return;
    }
}


static void refreshListUI(void) {
    Uint64 item_id;
    char buf[255];

    ListUI_Clear(ui);
    for (int n=1; n<=ACCOUNT_NUMB_MAX; n++) {
	*buf = 0;
	if(GetItemLabel(n, buf, sizeof(buf)) < 0) {
	    continue;
	}

	item_id = ListUI_AppendItem(ui, buf);
	ListUI_OnSelect(ui, item_id, OnSelectItem, 0);
	ListUI_OnActivate(ui, item_id, OnActivateItem, (void*)(Uint64)n);
    }
}


int main(int argc, char* args[])
{
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_RWops* rwops;
    SDL_Event event;
    TTF_Font* font;
    int quit = 0;

    printf("%s\n", README_md);
    printf("The payload was compiled at %s %s\n", __DATE__, __TIME__);

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

    ui = ListUI_Create("Offline Account Activation " VERSION_TAG);
    ListUI_SetSelectedColor(ui, (SDL_Color){0x3b, 0x40, 0x47, 0xff});
    ListUI_SetTextColor(ui, (SDL_Color){0xb9, 0xbb, 0xbb, 0xff});
    ListUI_SetActivateTextColor(ui, (SDL_Color){0xff, 0xff, 0xff, 0xff});
    refreshListUI();

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

	SDL_SetRenderDrawColor(renderer, 0x05, 0x0d, 0x1c, 0xff);
	SDL_RenderClear(renderer);

	ListUI_Render(ui, renderer, font);
	SDL_RenderPresent(renderer);

	IME_Dialog_PullStatus();
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
