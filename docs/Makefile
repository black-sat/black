# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = _build

ASTSOURCE = $(shell find . -name '*.rst.in')
ASTFILES = $(shell find . -name '*.rst.in' | sed 's/\.in//g')

export PYTHONPATH=../build/src/python

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile $(ASTSOURCE) ast.json.in doxygen.conf

doxygen/xml: doxygen.conf
	doxygen doxygen.conf

%.rst: %.rst.in # %.json.in ast.json.in
	@mkdir -p _build
	cc -x c++ -I../src/lib/include -I. -I_build -E \
		"$(shell dirname $@)/$(shell basename $@ .rst).json.in" | \
		sed '/^#/d;/^$$/d' | \
		jq 'del(..|select(. == null))' > _build/$(shell basename $@ .rst).json
	@python3 gen-ast-rst.py _build/$(shell basename $@ .rst).json $< > $@

.SECONDARY: $(ASTFILES)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile $(ASTFILES) doxygen/xml
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)