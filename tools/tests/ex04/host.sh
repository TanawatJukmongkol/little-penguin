# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
checkpatch_clean $R/ex04/src/*.c $R/ex04/include/*.h
check "module builds" "[ -f $R/ex04/keyboard-driver.ko ]"
