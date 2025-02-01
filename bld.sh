#!/bin/sh
# bld.sh

for F in *.md; do :
  if egrep "<!-- mdtoc-start -->" $F >/dev/null; then :
    # Update doc table of contents (see https://github.com/fordsfords/mdtoc).
    if which mdtoc.pl >/dev/null 2>&1; then LANG=C mdtoc.pl -b "" $F;
    elif [ -x ../mdtoc/mdtoc.pl ]; then LANG=C ../mdtoc/mdtoc.pl -b "" $F;
    else echo "FYI: mdtoc.pl not found; Skipping doc build"; echo ""; fi
  fi
done

echo "Building code"

rm -f lsim_test lsim_main

DEVS="lsim_devs_addword.c lsim_devs_addbit.c lsim_devs_clk.c lsim_devs_dflipflop.c lsim_devs_gnd.c lsim_devs_led.c lsim_devs_mem.c lsim_devs_nand.c lsim_devs_panel.c lsim_devs_probe.c lsim_devs_reg.c lsim_devs_srlatch.c lsim_devs_swtch.c lsim_devs_vcc.c"

gcc -std=c11 -Wall -Wextra -pedantic -Werror -g -o lsim_test lsim_test.c lsim.c lsim_cmd.c lsim_dev.c $DEVS err.c hmap.c cfg.c; if [ $? -ne 0 ]; then exit 1; fi

gcc -std=c11 -Wall -Wextra -pedantic -Werror -g -o lsim_main lsim_main.c lsim.c lsim_cmd.c lsim_dev.c $DEVS err.c hmap.c cfg.c; if [ $? -ne 0 ]; then exit 1; fi

echo "Build successful"
