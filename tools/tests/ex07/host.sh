# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
checkpatch_clean $R/ex07/src/*.c $R/ex07/include/*.h
check "module builds" "[ -f $R/ex07/debugfs.ko ]"
