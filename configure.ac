AC_INIT([budgie-indicator-applet], 0.0.4, [fossfreedom@ubuntu.com], [budgie-indicator-applet], [https://github.com/budgie-remix/budgie-indicator-applet])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz foreign subdir-objects])
AC_PROG_CC
AC_PROG_CC_STDC
LT_PREREQ(2.2)
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr/local)
AM_SILENT_RULES([yes])
LT_INIT([disable-static])
AC_CONFIG_MACRO_DIR([m4])

m4_define([budgie_required_version], [1])

# Note budgie-1.0 depends on peas, gtk, etc. Sorta your package managers
# business to make sure those deps are satisfied.
# Also note this is just for the C and Vala subprojects!
PKG_CHECK_MODULES(BUDGIE_PLUGIN,
        budgie-1.0 >= budgie_required_version
)

INDICATOR_API_VERSION=3
INDICATOR_REQUIRED_VERSION=0.3.90
INDICATOR_NG_VERSION=12.10.2
INDICATOR_PKG=indicator$INDICATOR_API_VERSION-0.4

PKG_CHECK_MODULES(INDICATOR, $INDICATOR_PKG >= $INDICATOR_NG_VERSION
                      libido3-0.1 >= 13.10,
                      [AC_DEFINE(HAVE_INDICATOR_NG, 1, "New style indicators support")])
                      
AC_SUBST(INDICATOR_CFLAGS)
AC_SUBST(INDICATOR_LIBS)

INDICATORDIR=`$PKG_CONFIG --variable=indicatordir $INDICATOR_PKG`
INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir $INDICATOR_PKG`

AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)


AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT

AC_MSG_RESULT([
        budgie-indicator-applet $VERSION
        ========

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        sysconfdir:             ${sysconfdir}
        exec_prefix:            ${exec_prefix}
        bindir:                 ${bindir}
        datarootdir:            ${datarootdir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
