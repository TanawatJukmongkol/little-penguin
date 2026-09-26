# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
checkpatch_clean $R/ex08/src/*.c
check "module builds" "[ -f $R/ex08/reverse.ko ]"
