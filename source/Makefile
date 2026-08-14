CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
WINDOWS_CC ?= x86_64-w64-mingw32-gcc
WINDOWS_EXE ?= PimpWyld30-Windows-x64.exe

all: pwyld20

pwyld20: pwyld20.c ansi_scenes.inc coat_only.inc gun_dealer.inc lay_photo.inc boyfriend_alley.inc
	$(CC) $(CFLAGS) -o $@ $< -lm

test: pwyld20
	./pwyld20 --self-test

windows: pwyld20.c ansi_scenes.inc coat_only.inc gun_dealer.inc lay_photo.inc boyfriend_alley.inc
	$(WINDOWS_CC) $(CFLAGS) -static -Wl,--subsystem,console -o $(WINDOWS_EXE) $< -lm

web: pwyld20.c ansi_scenes.inc coat_only.inc gun_dealer.inc lay_photo.inc boyfriend_alley.inc
	emcc $(CFLAGS) -sASYNCIFY -sALLOW_MEMORY_GROWTH -sENVIRONMENT=web \
		-sEXIT_RUNTIME=1 --pre-js ../web/mirror-pre.js -o ../web/pimpwyld.js $< -lm

clean:
	rm -f pwyld20 $(WINDOWS_EXE)

.PHONY: all test windows web clean
