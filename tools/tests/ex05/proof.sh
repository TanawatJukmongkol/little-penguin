# Sourced by run.sh inside the guest, as root; writes ex05/proof.log.
echo "== ex05: misc char device /dev/fortytwo =="
run uname -r
run insmod $S/ex05/fortytwo.ko
run ls -l /dev/fortytwo
run cat /dev/fortytwo
run "echo tjukmong > /dev/fortytwo"
run "printf tjukmong > /dev/fortytwo"
run "echo wrong_login > /dev/fortytwo"
run "echo tjukmongx > /dev/fortytwo"
run "dmesg | grep fortytwo: | tail -n 6"
run rmmod fortytwo
run ls -l /dev/fortytwo
