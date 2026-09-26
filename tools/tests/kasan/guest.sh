# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
# Loads a module with two deliberate memory bugs and checks that KASAN reports
# both, which shows the "no new KASAN reports" checks would catch real ones.
if [ $KASAN != 1 ]; then
	ok "KASAN self-test # SKIP this kernel isn't built with KASAN"
	return 0
fi
check "insmod kasan_selftest.ko" "insmod $T/kasan/kasan_selftest.ko"
check "KASAN reports the out-of-bounds read" "dmesg | grep -q 'BUG: KASAN: slab-out-of-bounds in .* \\[kasan_selftest\\]' \\
	|| { dmesg | grep -A 3 'BUG: KASAN'; exit 1; }"
check "KASAN reports the use-after-free read" "dmesg | grep -q 'BUG: KASAN: slab-use-after-free in .* \\[kasan_selftest\\]' \\
	|| { dmesg | grep -A 3 'BUG: KASAN'; exit 1; }"
check "rmmod kasan_selftest" "rmmod kasan_selftest"
