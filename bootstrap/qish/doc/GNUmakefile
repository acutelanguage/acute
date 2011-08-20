# file Qish/doc/Makefile

# we use the Hevea LaTeX to HTML translator
# see http://para.inria.fr/~maranget/hevea/
HEVEA=hevea
LATEX=latex
DVIPS=dvips
# hevea -version output two lines, the second one being something like
# library directory: /usr/lib/hevea
HEVEALIBDIR:=$(shell $(HEVEA) -version | grep library | cut '-d ' -f3)
TEXINPUTS:=$(TEXINPUTS):$(HEVEALIBDIR)
export TEXINPUTS
RM=rm -vf
.PHONY: all clean html ps pdf halfclean

# the basename of our documents
DOCUMENTS=qishintro

HTML_DOCUMENTS=$(patsubst %, %.html, $(DOCUMENTS))
PS_DOCUMENTS=$(patsubst %, %.ps, $(DOCUMENTS))

.SUFFIXES: .html .tex .ps .hva .sty .aux .haux

all: html ps pdf

clean:
	$(RM) *~ *.log *.dvi *.ps *.pdf *.html *.aux *.haux *.toc *.htoc *.out

html: $(HTML_DOCUMENTS)

ps: $(PS_DOCUMENTS)

pdf: $(PS_DOCUMENTS)
	for f in $(PS_DOCUMENTS) ; do ps2pdf $$f $$(basename $$f .ps).pdf ; done
halfclean: all
	$(RM) *~ *.log *.dvi *.aux *.haux *.toc *.htoc
.tex.dvi:
	$(LATEX) $<
	$(LATEX) $<

.dvi.ps:
	$(DVIPS) -o $@ $<

.tex.html:
	$(HEVEA) $<
	$(HEVEA) $<

#eof $Id: GNUmakefile 25 2005-07-08 08:54:46Z basile $