# Shared helpers, sourced by run.sh in the guest and by the Makefile's host
# checks. U is the unprivileged guest user.
U=${U:-lfs}

# run CMD...: print a shell-style transcript line, then the output and status.
run() {
	echo "# $*"
	sh -c "$*" 2>&1
	echo "[exit $?]"
}

# as_user CMD...: same, as the unprivileged user.
as_user() {
	echo "$U\$ $*"
	su $U -s /bin/sh -c "$*" 2>&1
	echo "[exit $?]"
}

# ok NAME / ko NAME: print a green OK or red KO result line. printf, not
# echo -e: dash's echo has no -e and prints it.
ok() {
	printf '\033[32mOK\033[0m - %s\n' "$1"
}
ko() {
	printf '\033[31mKO\033[0m - %s\n' "$1"
}

# check NAME CMD: run CMD (with a timeout, so a hang fails instead of stalling)
# and print an OK/KO line, plus CMD's non-blank output lines on failure.
check() {
	if out=$(timeout 60 sh -c "$2" 2>&1); then
		ok "$1"
	else
		ko "$1"
		echo "$out" | sed -e '/^[[:space:]]*$/d' -e 's/^/#   /'
	fi
}

# as_user_check NAME CMD: check, as the unprivileged user.
as_user_check() {
	check "$1" "su $U -s /bin/sh -c '$2'"
}

# checkpatch_clean FILE...: no checkpatch --strict errors or warnings (CHECKs
# are allowed). Needs K, the kernel tree. Every file must get its own clean
# "total:" line, so a checkpatch that didn't run (or skipped a missing file,
# which still exits 0) fails too.
checkpatch_clean() {
	check "checkpatch: no errors or warnings" \
		"out=\$($K/scripts/checkpatch.pl --strict --no-tree -f $* 2>&1); \
		[ \$(echo \"\$out\" | grep -c '^total: 0 errors, 0 warnings,') -eq $# ] \
			|| { echo \"\$out\"; exit 1; }"
}
