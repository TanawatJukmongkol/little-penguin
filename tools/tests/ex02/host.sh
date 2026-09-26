# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
P=$R/ex02/0001-Makefile-set-EXTRAVERSION-to-thor_kernel.patch
check "patch passes checkpatch" "cd $R/linux && ./scripts/checkpatch.pl --strict $P"
check "patch is signed off" "grep -qE '^Signed-off-by: .+ <.+>\$' $P"
check "patch applies to the kernel tree" "git -C $R/linux apply --check $P || git -C $R/linux apply -R --check $P"
check "kernel.log has -thor_kernel in its version" "grep -q 'Linux version 6.14.0-thor_kernel' $R/ex02/kernel.log"
check "kernel.log shows uname -a" "grep -qE 'uname\[[0-9]+\]: Linux .* 6\.14\.0-thor_kernel' $R/ex02/kernel.log"
check "kernel.log is plain text" "! grep -qP '\\x1b|\\r' $R/ex02/kernel.log"
