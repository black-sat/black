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

.PHONY: help Makefile $(ASTSOURCE) ast.json.in 

_build/ast.json: ast.json.in
	@cc -x c++ -I../src/lib/include -E $< | sed '/^#/d;/^$$/d' | \
		jq 'del(..|select(. == null))' > $@

%.rst: %.rst.in _build/ast.json
	echo lol $@

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile $(ASTFILES)
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)