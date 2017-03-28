### ===========================================================================
### File: "configure.ac"
###                        Created: 2010-06-24 08:46:27
###              Last modification: 2012-01-28 12:49:10
### Authors: Bernard Desgraupes <bernard.desgraupes@u-paris10.fr>
###          Sylvain Loiseau <sylvain.loiseau@univ-paris13.fr>
### This is part of the R package 'rcqp'.
### ===========================================================================
###
### Process this file with autoconf to produce a configure script.
###



AC_PREREQ(2.60)

AC_INIT([rcqp],[1.0])

# Platform-specific configuration (see configRcqp.mk)
test ${RCQP_PLATFORM:=linux}
echo "RCQP_PLATFORM: ${RCQP_PLATFORM}"
AC_SUBST(RCQP_PLATFORM)

# Site-specific configuration (see configRcqp.mk)
test ${RCQP_SITE:=standard}
echo "RCQP_SITE: ${RCQP_SITE}"
AC_SUBST(RCQP_SITE)


: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
	echo "could not determine R_HOME"
	exit 1
fi

CC=`"${R_HOME}/bin/R" CMD config CC`

RCQP_R_CFLAGS=`"${R_HOME}/bin/R" CMD config CFLAGS`
RCQP_R_LIBR=`"${R_HOME}/bin/R" CMD config --ldflags`
RCQP_R_CPPFLAGS=`"${R_HOME}/bin/R" CMD config --cppflags`

AC_SUBST(RCQP_R_CFLAGS)
AC_SUBST(RCQP_R_LIBR)
AC_SUBST(RCQP_R_CPPFLAGS)

# Cross-compilation with mingw
test ${CROSS_COMPILATION_DIR:=}
AC_SUBST(CROSS_COMPILATION_DIR)

if test "${RCQP_PLATFORM}" = "mingw" -o "${RCQP_PLATFORM}" = "mingwRcqp" ; then
	RCQP_PLATFORM="mingwRcqp"
	RCQP_CROSS_COMPILING=1
else
	RCQP_CROSS_COMPILING=0
fi

if test "${RCQP_CROSS_COMPILING}" = 1; then
	echo "rcqp cross compiling"
	echo "checking for config file 'rcqpcrossrc' ..."
	
	olddir=`pwd`
	cd $(dirname $0)
	if [[ -e `pwd`/rcqpcrossrc ]] ; then
		source `pwd`/rcqpcrossrc
	else
		echo "... not found"
	fi
	cd $olddir
	
	test ${RCQP_CROSS_CC:=i386-mingw32-gcc}
	echo "RCQP_CROSS_CC: '${RCQP_CROSS_CC}'"
	AC_SUBST(RCQP_CROSS_CC)

	if [[ -z $RCQP_CROSS_HOST ]] ; then
		RCQP_CROSS_HOST=${RCQP_CROSS_CC%-*}
		echo "RCQP_CROSS_HOST: '$RCQP_CROSS_HOST'."
	fi
	AC_SUBST(RCQP_CROSS_HOST)

	if [[ -z $RCQP_CROSS_ARCH ]] ; then
		RCQP_CROSS_ARCH=${RCQP_CROSS_CC%%-*}
		echo "RCQP_CROSS_ARCH : '$RCQP_CROSS_ARCH'."
	fi
	AC_SUBST(RCQP_CROSS_ARCH)
	
fi


# C support
# ---------
AC_PROG_CC()
AC_PROG_LEX()
AC_PROG_YACC()

# Support the MacPorts location
LDFLAGS="$LDFLAGS -L/opt/local/lib"

# Check for the required libraries
if test "$RCQP_CROSS_COMPILING" = 0; then
	AC_CHECK_LIB(glib-2.0, g_list_append, [], AC_MSG_ERROR(the glib-2.0 library is required. Please install it.))
	AC_CHECK_LIB(pcre, pcre_free, [], AC_MSG_ERROR(the pcre library is required. Please install it.))
fi

# Check for pkg-config
AC_PATH_PROGS(PKGCONFIG, pkg-config) 



# Process the .in files
# ---------------------
AC_CONFIG_FILES(
[ ../../../pkg/rcqp/src/MakefileCrossCompile
../../../pkg/rcqp/src/cwb/configRcqpCrossCompile.mk
../../../pkg/rcqp/src/cwb/config/platform/mingwRcqp
../../../pkg/rcqp/src/cwb/MakefileRcqpCrossCompile
])

AC_OUTPUT

