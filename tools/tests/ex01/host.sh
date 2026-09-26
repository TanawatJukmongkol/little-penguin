# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
checkpatch_clean $R/ex01/src/*.c
check "module builds" "[ -f $R/ex01/main.ko ]"
