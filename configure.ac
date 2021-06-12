AC_INIT([budgie-indicator-applet], 0.7.0, [fossfreedom@ubuntu.com], [budgie-indicator-applet], [https://github.com/ubuntubudgie/budgie-indicator-applet])
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

UBUNTU_INDICATOR_API_VERSION=3
UBUNTU_INDICATOR_REQUIRED_VERSION=0.3.90
UBUNTU_INDICATOR_NG_VERSION=0.5
UBUNTU_INDICATOR_PKG=indicator$INDICATOR_API_VERSION-0.4

AYATANA_INDICATOR_API_VERSION=3
AYATANA_INDICATOR_REQUIRED_VERSION=0.6.0
AYATANA_INDICATOR_NG_VERSION=0.6.0
AYATANA_INDICATOR_PKG=ayatana-indicator$INDICATOR_API_VERSION-0.4

AC_ARG_WITH([ayatana-indicators],
             [AS_HELP_STRING([--with-ayatana-indicators],
                             [build against Ayatana Indicators])],
             [with_ayatana_indicators='yes'],
             [with_ayatana_indicators='no']
)

AC_ARG_WITH([ubuntu-indicators],
             [AS_HELP_STRING([--with-ubuntu-indicators],
             [build against Ubuntu Indicators])],
             [with_ubuntu_indicators='yes'],
             [with_ubuntu_indicators='no']
)

###
### Look for Ayatana Indicators
###

PKG_CHECK_EXISTS(ayatana-indicator3-0.4,
                 [have_ayatanaindicator="yes"],
                 [have_ayatanaindicator="no"])

PKG_CHECK_EXISTS(ayatana-indicator3-0.4 >= $AYATANA_INDICATOR_NG_VERSION,
                 [have_ayatanaindicator_ng="yes"],
                 [have_ayatanaindicator_ng="no"])

###
### Look for Ubuntu Indicators
###

PKG_CHECK_EXISTS(indicator3-0.4,
                 [have_ubuntuindicator="yes"],
                 [have_ubuntuindicator="no"])

PKG_CHECK_EXISTS(indicator3-0.4 >= $UBUNTU_INDICATOR_NG_VERSION,
                 [have_ubuntuindicator_ng="yes"],
                 [have_ubuntuindicator_ng="no"])

### decide on what Indicators implementation to use...

if   test "x$have_ayatanaindicator" == "xyes" &&
     test "x$have_ubuntuindicator" != "xyes" &&
     test "x$with_ubuntu_indicators" != "xyes"; then

    # use Ayatana Indicators (because they are present, and noone is enforcing Ubuntu Indicators)
    use_ayatanaindicator="yes";
    indicator_enforced="no";

elif test "x$have_ubuntuindicator" == "xyes" &&
     test "x$have_ayatanaindicator" != "xyes" &&
     test "x$with_ayatana_indicators" != "xyes"; then

    # use Ubuntu Indicators (because they are present, and noone is enforcing Ayatana Indicators)
    use_ubuntuindicator="yes";
    indicator_enforced="no";

elif test "x$have_ubuntuindicator" == "xyes" &&
     test "x$have_ayatanaindicator" == "xyes" &&
     test "x$with_ayatana_indicators" == "xyes"; then

    # both Indicator implementations are present, and we are asked to use Ayatana Indicators
    use_ayatanaindicator=yes;
    indicator_enforced=yes;

elif test "x$have_ubuntuindicator" == "xyes" &&
     test "x$have_ayatanaindicator" == "xyes" &&
     test "x$with_ubuntu_indicators" == "xyes"; then

    # both Indicator implementations are present, and we are asked to use Ubuntu Indicators
    use_ubuntuindicator=yes;
    indicator_enforced=yes;

elif test "x$have_ubuntuindicator" == "xyes" &&
     test "x$have_ayatanaindicator" != "xyes" &&
     test "x$with_ayatana_indicators" == "xyes"; then

    AC_MSG_ERROR([Ubuntu Indicators are present, but you want to build budgie-indicator-applet against Ayatana Indicators. This does not match.])

elif test "x$have_ubuntuindicator" != "xyes" &&
     test "x$have_ayatanaindicator" == "xyes" &&
     test "x$with_ubuntu_indicators" == "xyes"; then

    AC_MSG_ERROR([Ayatana Indicators are present, but you want to build budgie-indicator-applet against Ubuntu Indicators. This does not match.])

else

    AC_MSG_ERROR([Either Ayatana Indicators or Ubuntu Indicators are required to build budgie-indicator-applet.])

fi

### prepare Ayatana or Ubuntu Indicators implementation for the build, regarding to the decision reached above...

if   test "x$use_ayatanaindicator" == "xyes"; then

    AM_CONDITIONAL(WITH_AYATANA_INDICATOR, true)
    AM_CONDITIONAL(WITH_UBUNTU_INDICATOR, false)
    AC_DEFINE(HAVE_AYATANA_INDICATOR, 1, "Ayatana Indicators Support")
    AC_DEFINE(HAVE_UBUNTU_INDICATOR, 0, "DISABLED: Ubuntu Indicators Support")
    AC_DEFINE(HAVE_UBUNTU_INDICATOR_NG, 0, "DISABLED: Ubuntu Indicators NG Support")

    if test "x$indicator_enforced" == "xyes"; then
        AC_MSG_NOTICE([Using Ayatana Indicators for this build (as requested via configure option).])
    else
        AC_MSG_NOTICE([Using Ayatana Indicators for this build.])
    fi

    if test "x$have_ayatanaindicator_ng" = "xyes"; then
        PKG_CHECK_MODULES(AYATANA_INDICATOR_NG, ayatana-indicator3-0.4 >= $AYATANA_INDICATOR_NG_VERSION
                          libayatana-ido3-0.4 >= 0.4.0,
                          [AC_DEFINE(HAVE_AYATANA_INDICATOR_NG, 1, "New style indicators support")])
    elif test "x$have_ayatanaindicator" = "xyes"; then
        PKG_CHECK_MODULES(AYATANA_INDICATOR, ayatana-indicator3-0.4 >= $AYATANA_INDICATOR_REQUIRED_VERSION)
    fi

    AC_SUBST(AYATANA_INDICATOR_CFLAGS)
    AC_SUBST(AYATANA_INDICATOR_LIBS)

elif test "x$use_ubuntuindicator" == "xyes"; then

    # both Indicator implementations are present, and we are asked to use Ubuntu Indicators
    AM_CONDITIONAL(WITH_UBUNTU_INDICATOR, true)
    AM_CONDITIONAL(WITH_AYATANA_INDICATOR, false)
    AC_DEFINE(HAVE_UBUNTU_INDICATOR, 1, "Ubuntu Indicators Support")
    AC_DEFINE(HAVE_AYATANA_INDICATOR, 0, "No Ayatana Indicators Support")
    AC_DEFINE(HAVE_AYATANA_INDICATOR_NG, 0, "No Ayatana Indicators NG Support")

    if test "x$indicator_enforced" == "xyes"; then
        AC_MSG_NOTICE([Using Ubuntu Indicators for this build (as requested via configure option).])
    else
        AC_MSG_NOTICE([Using Ubuntu Indicators for this build.])
    fi

    if test "x$have_ubuntuindicator_ng" = "xyes"; then
        PKG_CHECK_MODULES(UBUNTU_INDICATOR_NG, indicator3-0.4 >= $UBUNTU_INDICATOR_NG_VERSION
                          libido3-0.1 >= 13.10,
                          [AC_DEFINE(HAVE_UBUNTU_INDICATOR_NG, 1, "New style indicators support")])
    elif test "x$have_ubuntuindicator" = "xyes"; then
        PKG_CHECK_MODULES(UBUNTU_INDICATOR, indicator3-0.4 >= $UBUNTU_INDICATOR_REQUIRED_VERSION)
    fi

    AC_SUBST(UBUNTU_INDICATOR_CFLAGS)
    AC_SUBST(UBUNTU_INDICATOR_LIBS)

fi

if test "x$use_ubuntuindicator" = "xyes"; then
       INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
       INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`
fi

if test "x$use_ayatanaindicator" = "xyes"; then
       INDICATORDIR=`$PKG_CONFIG --variable=indicatordir ayatana-indicator3-0.4`
       INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir ayatana-indicator3-0.4`
fi

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
