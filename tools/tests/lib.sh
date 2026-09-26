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

# ok NAME / ko NAME: print a green OK or red KO result line.
ok() {
	echo -e "\e[32mOK\e[0m - $1"
}
ko() {
	echo -e "\e[31mKO\e[0m - $1"
}

# check NAME CMD: run CMD (with a timeout, so a hang fails instead of stalling)
# and print an OK/KO line, plus CMD's output on failure.
check() {
	if out=$(timeout 60 sh -c "$2" 2>&1); then
		ok "$1"
	else
		ko "$1"
		echo "$out" | sed 's/^/#   /'
	fi
}

# as_user_check NAME CMD: check, as the unprivileged user.
as_user_check() {
	check "$1" "su $U -s /bin/sh -c '$2'"
}

# checkpatch_clean FILE...: no checkpatch --strict errors or warnings (CHECKs
# are allowed). Needs K, the kernel tree.
checkpatch_clean() {
	check "checkpatch: no errors or warnings" \
		"! $K/scripts/checkpatch.pl --strict --no-tree --terse -f $* | grep -E 'ERROR|WARNING'"
}
