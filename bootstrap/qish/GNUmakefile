# file Qish/GNUmakefile
# emacs Time-stamp: <2005 Jul 08 10h14 CEST {GNUmakefile} Basile STARYNKEVITCH>
# Copyright © 2002 Basile STARYNKEVITCH
# prcs $Id: GNUmakefile 25 2005-07-08 08:54:46Z basile $
SUFFIXES += .i
RM=rm -vf
MV=mv
CP=cp -v
INSTALL=install
MKDIR=mkdir -vp
# $Format: "RELEASE=$ReleaseVersion$"$
RELEASE=1.0pre1
# $Format: "PRCSVERSION=$ProjectVersion$"$
PRCSVERSION=1.9

.PHONY: all prog clean lib dist install test
all: lib prog

lib:
	$(MAKE) -C lib lib

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C doc clean
#	$(MAKE) -C ruko clean
	$(RM) core *~ .*.prcs_aux

prog:
#	$(MAKE) -C beth prog

test: all
#	$(MAKE) -C beth test

dist:
	$(MAKE) clean
	$(RM) -r $(HOME)/tmp/Qish_$(RELEASE)/
	$(MKDIR) $(HOME)/tmp/Qish_$(RELEASE)/
	cd  $(HOME)/tmp/Qish_$(RELEASE)/; svn co file:///P/basile/Svn_Qish .; \
   	   cd doc; $(MAKE) halfclean
#	$(CP) doc/*.{html,ps,pdf} $(HOME)/tmp/Qish_$(RELEASE)/doc
#	$(RM) $(HOME)/tmp/Qish_$(RELEASE)/.Qish.prcs_aux
	tar -C $(HOME)/tmp --exclude=.svn -czvf $(HOME)/tmp/qish-$(RELEASE).tar.gz Qish_$(RELEASE)/
	$(RM) -r $(HOME)/tmp/Qish_$(RELEASE)/

install: lib prog
	$(MAKE) -C lib install

#eof $Id: GNUmakefile 25 2005-07-08 08:54:46Z basile $