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

VERSION_TAG := $(shell git describe --abbrev=10 --dirty --always --tags)

CFLAGS := -O1 -g -Wall -Wno-format-truncation -DVERSION_TAG=\"$(VERSION_TAG)\"
LDADD := `$(PS5_PAYLOAD_SDK)/bin/prospero-sdl2-config --cflags --libs`
LDADD += -lSDL2_ttf `$(PS5_PAYLOAD_SDK)/bin/prospero-freetype-config --libs`
LDADD += -lSceRegMgr -lSceImeDialog -lSceUserService -lSDL2main

ELF := OffAct.elf

all: $(ELF)

readme.h: README.md
	xxd -i $< $@

main.c: readme.h

$(ELF): main.c offact.c IME_dialog.c SDL_listui.c
	$(CC) $(CFLAGS) -o $@ $(LDADD) $^

clean:
	rm -f $(ELF) readme.h OffAct.zip

upload: $(ELF)
	curl -T $^ ftp://$(PS5_HOST):2121/data/homebrew/OffAct/$^

install: $(ELF) homebrew.js sce_sys/icon0.png
	install -Dm 644 sce_sys/icon0.png -t "${DESTDIR}/${PREFIX}/OffAct/sce_sys"
	install -Dm 644 homebrew.js -t "${DESTDIR}/${PREFIX}/OffAct"
	install -Dm 755 $(ELF) -t "${DESTDIR}/${PREFIX}/OffAct"

dist: $(ELF) homebrew.js sce_sys/icon0.png
	zip -r OffAct.zip $^
