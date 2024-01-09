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


# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile $(ASTSOURCE) ast.json.in doxygen.conf

_build/ast.json: ast.json.in
	@mkdir -p _build
	@cc -x c++ -I../src/lib/include -E $< | sed '/^#/d;/^$$/d' | \
		jq 'del(..|select(. == null))' > $@

doxygen/xml: doxygen.conf
	doxygen doxygen.conf

%.rst: %.rst.in _build/ast.json
	@python3 gen-ast-rst.py _build/ast.json $< > $@

.SECONDARY: $(ASTFILES)

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile $(ASTFILES) doxygen/xml
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)