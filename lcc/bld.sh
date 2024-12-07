#!/bin/bash
# bld.sh

ASSRT() {
  eval "test $1"

  if [ $? -ne 0 ]; then
    echo "ASSRT ERROR, `date`: `basename ${BASH_SOURCE[1]}`:${BASH_LINENO[0]}, not true: '$1'" >&2
    exit 1
  fi
}  # ASSRT

echo ruff format [a-zA-Z]*.py
ruff format [a-zA-Z]*.py
ASSRT "$? -eq 0"

echo ruff check -q [a-zA-Z]*.py
ruff check -q [a-zA-Z]*.py
ASSRT "$? -eq 0"

echo flake8 [a-zA-Z]*.py
flake8 [a-zA-Z]*.py
ASSRT "$? -eq 0"

echo pylint -sn -r n [a-zA-Z]*.py
pylint -sn -r n [a-zA-Z]*.py
ASSRT "$? -eq 0"

echo ruff format plugins/[a-zA-Z]*.py
ruff format plugins/[a-zA-Z]*.py
ASSRT "$? -eq 0"

echo ruff check -q plugins/[a-zA-Z]*.py
ruff check -q plugins/[a-zA-Z]*.py
ASSRT "$? -eq 0"

echo flake8 plugins/[a-zA-Z]*.py
flake8 plugins/[a-zA-Z]*.py
ASSRT "$? -eq 0"

echo pylint -sn -r n plugins/[a-zA-Z]*.py
pylint -sn -r n plugins/[a-zA-Z]*.py
ASSRT "$? -eq 0"
