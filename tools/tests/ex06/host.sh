# Sourced on the host by the Makefile; R is the repo, K the kernel tree.
P=$R/ex06/0001-fs-buffer-fix-NULL-deref-on-folio-less-bh-in-__bh_submit.patch
check "patch passes checkpatch" "cd $R/linux-next && ./scripts/checkpatch.pl --strict $P"
check "kernel.log is from a linux-next build" "grep -qE 'Linux version [^ ]+-next-[0-9]{8}' $R/ex06/kernel.log"
check "kernel.log is plain text" "! grep -qP '\\x1b|\\r' $R/ex06/kernel.log"
