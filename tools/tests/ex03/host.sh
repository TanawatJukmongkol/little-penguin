# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
# Not loaded in the guest: the subject's loop compares against a pointer.
checkpatch_clean $R/ex03/src/*.c $R/ex03/include/*.h
check "module builds" "[ -f $R/ex03/main.ko ]"
