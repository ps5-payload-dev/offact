#   Copyright (C) 2024 John Törnblom
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not see
# <http://www.gnu.org/licenses/>.

PS5_HOST ?= ps5

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK is undefined)
endif

CFLAGS := -O2 -Wall -Wno-format-truncation
CFLAGS += `$(PS5_SYSROOT)/bin/sdl2-config --cflags --libs`
CFLAGS += -lSDL2_ttf `$(PS5_SYSROOT)/bin/freetype-config --libs`
CFLAGS += -lSDL2_mixer
CFLAGS += -lSceRegMgr

ELF := OffAct.elf

all: $(ELF)

font.h: assets/font.ttf
	xxd -i $< $@

snd_nav.h: assets/nav.wav
	xxd -i $< $@

main.c: font.h snd_nav.h

$(ELF): main.c SDL_listui.c offact.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ELF) font.h snd_nav.h

upload: $(ELF)
	curl -T $^ ftp://$(PS5_HOST):2121/data/$^
