# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
check ".config enables LOCALVERSION_AUTO" "grep -qx CONFIG_LOCALVERSION_AUTO=y $R/ex00/.config"
check "kernel.log is from a clean 6.14.0 build" "grep -q 'Linux version 6.14.0 ' $R/ex00/kernel.log"
check "kernel.log shows uname -a" "grep -qE 'uname\[[0-9]+\]: Linux .* 6\.14\.0 ' $R/ex00/kernel.log"
check "kernel.log is plain text" "! grep -qP '\\x1b|\\r' $R/ex00/kernel.log"
