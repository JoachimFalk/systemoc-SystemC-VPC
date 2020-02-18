## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2003/01/15 16:43:20 $
#
# Makefile.am is part of the jflib distribution of Joachim Falk;
# you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 2 of the License, or (at your option) any later version.
#
# The jflib distributio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# $Log$

RESDIR := $(shell cd $(srcdir) && while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

include $(auxdir)/jfrules.mk
include $(RESDIR)/Rules-TeX-PDF.mk

SUBDIRS =

TEXINPUTS    := figs:$(srcdir)/figs:tex:$(srcdir)/tex:$(RESDIR):$(RESDIR)/beamer_fau_logo/4-3:$(TEXINPUTS)
TEXCLEANDIRS := figs .
TEX_SOURCES  := \
	tex/VPC-Tutorial-slides.tex \
	tex/VPC-Tutorial-report.tex

export SCRTREEMODE_PKG_CONFIG_PATH

SCRTREEMODE_PKG_CONFIG_PATH := $(subst MAGICUNLIKELYPREFIX:,,$(subst :$(top_builddir)/,:$(abs_top_builddir)/,MAGICUNLIKELYPREFIX:$(PKG_CONFIG_PATH)))

src/Makefile: src/Makefile.sample
	@$(MKDIR_P) src && $(SED) \
	    -e 's|[@]prefix[@]|$(prefix)|g' \
	    -e 's|[@]exec_prefix[@]|$(exec_prefix)|g' \
	    -e 's|[@]libdir[@]|$(libdir)|g' \
	  < $^ > $@ || { rm -f $@ && false; }

all-local: src/Makefile
	@$(MAKE) -C src VPATH=$(abs_srcdir)/src all
clean-local:
	@if test -f src/Makefile; then \
	  $(MAKE) -C src VPATH=$(abs_srcdir)/src clean; \
	  rm -f src/Makefile; \
	fi
	@rmdir src 2>/dev/null || :

CLEANFILES=src/Makefile

docdir = ${datarootdir}/doc/SystemC-VPC

nobase_doc_DATA = \
	src/CMakeLists.txt \
	src/Makefile \
	src/source-sink-config-api/cpp/vpc-source-sink-config-api.cpp \
	src/source-sink/cpp/vpc-source-sink.cpp \
	src/source-sink/vpc/src-snk.vpc.xml

doc_DATA = \
	tex/VPC-Tutorial-slides.pdf \
	tex/VPC-Tutorial-report.pdf