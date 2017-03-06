bin_PROGRAMS = rec

rec_SOURCES = main.c rec-alsa-devices.c rec-alsa-mixer.c
rec_CFLAGS = $(REC_CFLAGS)

rec_LDADD = $(REC_LIBS) -lm

-include $(top_srcdir)/git.mk
