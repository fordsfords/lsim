#!/bin/bash
# tst.sh - generic test script
#   Tip: script -c ./tst.sh tst.log

SINGLE_T="0"  # Do all tests
if [ -n "$1" ]; then SINGLE_T="$1"; fi

F="tst.sh"  # Could also use `basename $0`
B="./lcc.py"  # Executable under test.

TEST() {
  echo "Test $T [$F:${BASH_LINENO[0]}]: $1 `date`" >$B.$T.log
  cat $B.$T.log
}  # TEST

ASSRT() {
  eval "test $1"

  if [ $? -ne 0 ]; then
    echo "ASSRT ERROR, Test $T [$F:${BASH_LINENO[0]}]: not true: '$1' `date`" | tee -a $B.$T.log
    exit 1
  fi
}  # ASSRT

OK() {
  echo "Test $T [$F:${BASH_LINENO[0]}]: OK"
}  # OK


./bld.sh; ASSRT "$? -eq 0"

rm -f *.log core.*

T=1
if [ "$SINGLE_T" -eq 0 -o "$SINGLE_T" -eq "$T" ]; then :
  TEST
  $B x.lcc 2>&1 | tee -a $B.$T.log;  ST=${PIPESTATUS[0]}; ASSRT "$ST -eq 0"
  # egrep "^test[0-9 ]*: success" $B.$T.log >/dev/null; ST=$?; ASSRT "$ST -eq 0"
  OK
fi
