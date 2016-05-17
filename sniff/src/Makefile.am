AM_CFLAGS = -DCLIENT_APP_VERSION=0x$(shell cd ~/sniff;git log -1 --pretty=format:%h) `pkg-config --cflags --libs libnl-3.0`
AM_LDFLAGS =

AM_CPPFLAGS = -Wall -Werror
ACLOCAL_AMFLAGS = -I m4
hardware_platform = i686-linux-gnu
bin_PROGRAMS = sniff
sniff_CPPFLAGS = $(CPPFLAGS)
sniff_SOURCES = main.c
sniff_LDFLAGS = -lwit -lwitutil -lpcd8544
