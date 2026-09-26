# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
if [ ! -e /proc/mymounts ]; then
	echo "ok - /proc/mymounts # SKIP ex09 isn't built into this kernel"
	return 0
fi
check "first line is root /" "head -n 1 /proc/mymounts | grep -qE '^root +/\$'"
check "lists /proc as proc" "grep -qE '^proc +/proc\$' /proc/mymounts"
check "one line per mount" "[ \$(wc -l < /proc/mymounts) -eq \$(wc -l < /proc/mounts) ]"
check "no kernel BUG or Oops" "! dmesg | grep -qE 'BUG:|Oops|Call Trace'"
