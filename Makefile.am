#EXTRA_DIST = autoconfigure.sh
#SUBDIRS = src data packaging doc
#EXTRA_DIST = xkbd.desktop xkbd.xpm

desktopdir = $(datadir)/applications/
desktop_DATA = mbinputmgr.desktop

pixmapsdir = $(datadir)/pixmaps/
pixmaps_DATA = mbinputmgr.png

EXTRA_DIST = .autoipkg mbinputmgr.desktop mbinputmgr.png 

LDADD = @X_LIBS@ -lX11 -lXext $(MB_LIBS) 
CFLAGS = @CFLAGS@ @X_CFLAGS@ -DDATADIR=\"$(datadir)\"


if ENABLE_GNOME

INCLUDES = -I$(top_srcdir) -I$(includedir) $(GRANDR_APPLET_CFLAGS) \
       -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
       -DGNOME_DISABLE_DEPRECATED \
       -DGTK_DISABLE_DEPRECATED \
       -DGDK_DISABLE_DEPRECATED \
       -DG_DISABLE_DEPRECATED \
       -DPREFIX=\""$(prefix)/"\" \
       -DLIBDIR=\""$(libdir)/"\" \
       -DSYSCONFDIR=\""$(sysconfdir)/"\"

libexec_PROGRAMS = ginputmgr

ginputmgr_SOURCES =         \
        mbinputmgr.c mbinputmgr.h mbinputmgr-applet.c

ginputmgr_LDADD = $(GRANDR_APPLET_LIBS) $(LDADD)

else

bin_PROGRAMS = mbinputmgr 
mbinputmgr_SOURCES = mbinputmgr.c mbinputmgr.h mbinputmgr-tray.c
mbinputmgr_LDADD   = $(LDADD)

endif

ipkg:
	autoipkg