# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
check "insmod main.ko" "insmod $S/ex01/main.ko"
check "prints Hello world! on load" 'dmesg | tail -n 1 | grep -q "Hello world!"'
check "rmmod main" "rmmod main"
check "prints Cleaning up module. on unload" 'dmesg | tail -n 1 | grep -q "Cleaning up module."'
