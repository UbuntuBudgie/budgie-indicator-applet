include $(top_srcdir)/common.mk

plugindir = $(libdir)/budgie-desktop/plugins

appindicatorappletdir = $(plugindir)/appindicator-applet

appindicatorapplet_LTLIBRARIES = \
	libappindicatorapplet.la

libappindicatorapplet_la_SOURCES = \
	applet-main.c \
	eggaccelerators.c \
	eggaccelerators.h \
	AppIndicatorApplet.c \
	AppIndicatorApplet.h \
	AppIndicatorPanelApplet.c
	

libappindicatorapplet_la_CFLAGS = \
	$(BUDGIE_PLUGIN_CFLAGS) \
	$(AM_CFLAGS) \
	-DG_LOG_DOMAIN=\""Indicator-Applet"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_APPLET \
	-I$(srcdir)/.. \
	$(APPLET_CFLAGS) \
	$(INDICATOR_CFLAGS)

libappindicatorapplet_la_LIBADD = \
	$(BUDGIE_PLUGIN_LIBS) \
	$(APPLET_LIBS) \
	$(INDICATOR_LIBS) \
	-lX11

libappindicatorapplet_la_LDFLAGS = \
	-module \
	-avoid-version \
	-shared \
	$(AM_LDFLAGS)

appindicatorapplet_DATA = \
	AppIndicatorApplet.plugin

EXTRA_DIST = \
	AppIndicatorApplet.plugin
