# vim:ft=automake

BUILT_SOURCES=
EXTRA_DIST=
TESTS=
bin_PROGRAMS=
check_PROGRAMS=
lib_LTLIBRARIES=
man_MANS=
noinst_HEADERS=
noinst_LTLIBRARIES=
noinst_PROGRAMS=

ACLOCAL_AMFLAGS= -I m4

EXTRA_DIST+= README
EXTRA_DIST+= TODO
EXTRA_DIST+= ChangeLog
EXTRA_DIST+= autogen.sh 

include src/include.am

# Cleanup individual files in order to preserve uninstall/etc order
maintainer-clean-local:
	@rm -r -f autom4te.cache
	@rm Makefile.in
	@rm aclocal.m4
	@rm config.in
	@rm config/config.guess
	@rm config/config.sub
	@rm config/depcomp
	@rm config/install-sh
	@rm config/ltmain.sh
	@rm config/missing
	@rm configure
	@rmdir config
	@rm m4/libtool.m4
	@rm m4/ltoptions.m4
	@rm m4/ltsugar.m4
	@rm m4/ltversion.m4
	@rm m4/lt~obsolete.m4
	@rm -f shim-*.tar.gz
	@find ./ | $(GREP) \~$$ | xargs rm -f

# We should write some documentation
# man_MANS+= man/shim.1
# EXTRA_DIST+= $(man_MANS)
