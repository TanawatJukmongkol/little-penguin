# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
checkpatch_clean $R/ex05/src/*.c $R/ex05/include/*.h
check "module builds" "[ -f $R/ex05/fortytwo.ko ]"
