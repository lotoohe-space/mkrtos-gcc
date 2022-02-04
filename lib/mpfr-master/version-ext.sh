#!/bin/sh

# This script outputs additional MPFR version information for a
# Git working tree (Git branch, total commit count, commit id,
# and whether the current HEAD is modified). It is called in
# tests/Makefile.am for "make check", but may be used by other
# tools that need such information.
# Note that this does not replace version information found in
# the VERSION file, which may still need to be output in addition
# to the output of this script.

set -e

if [ "x`git rev-parse --is-inside-work-tree 2> /dev/null`" != xtrue ]; then
  echo "$0: This script should be executed from a Git working tree." >&2
  exit 1
fi

# Normally passed by Makefile.am with "GREP=$(GREP)".
GREP=${GREP:-grep}

git tag --contains | sed -n 's/-root$//p' > excluded-branches
gitb=`git branch --format='%(refname:short)' --contains | \
        $GREP -v '^(' | $GREP -v -F -f excluded-branches -x`
rm excluded-branches
gitc=`git rev-list --count HEAD`
gith=`git rev-parse --short HEAD`
gitm=`git diff-index --name-only HEAD`
echo "$gitb-$gitc-$gith${gitm:+ (modified)}"
