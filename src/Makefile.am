AM_CPPFLAGS = 				\
	-I$(top_srcdir)			\
	-DG_LOG_DOMAIN=\"zevdocs\"	\
	$(WARN_CFLAGS)

AM_LDFLAGS =		\
	$(WARN_LDFLAGS)

app_headers =			\
	dh-app.h		\
	dh-assistant.h		\
	dh-preferences.h	\
	dh-settings-app.h	\
	dh-tab.h		\
	dh-tab-label.h		\
	dh-util-app.h		\
	dh-web-view.h		\
	dh-window.h		\
	tepl-info-bar.h		\
	$(NULL)

app_c_files =			\
	dh-app.c		\
	dh-assistant.c		\
	dh-main.c		\
	dh-preferences.c	\
	dh-settings-app.c	\
	dh-tab.c		\
	dh-tab-label.c		\
	dh-util-app.c		\
	dh-web-view.c		\
	dh-window.c		\
	tepl-info-bar.c		\
	$(NULL)

app_built_sources =	\
	dh-resources.c	\
	$(NULL)

BUILT_SOURCES = 		\
	$(app_built_sources)	\
	$(NULL)

# The application
bin_PROGRAMS = zevdocs

zevdocs_SOURCES =	\
	$(app_headers)	\
	$(app_c_files)	\
	$(NULL)

# Do not distribute generated files.
nodist_zevdocs_SOURCES =	\
	$(app_built_sources)

zevdocs_CPPFLAGS =	\
	$(AM_CPPFLAGS)

zevdocs_CFLAGS =		\
	$(ZEVDOCS_CFLAGS)

zevdocs_LDADD = 				\
	$(zevdocs_LIBS)				\
	$(LIBM)					\
        $(top_builddir)/devhelp/libdevhelp-3.la

zevdocs_LDFLAGS =	\
	$(AM_LDFLAGS)

resource_files =		\
	dh-assistant.ui		\
	dh-preferences.ui	\
	dh-window.ui		\
	help-overlay.ui		\
	menus.ui		\
	$(NULL)

dh-resources.c: dh.gresource.xml $(resource_files)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source --c-name dh $(srcdir)/dh.gresource.xml

EXTRA_DIST =			\
	dh.gresource.xml	\
	$(resource_files)	\
	$(NULL)

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
