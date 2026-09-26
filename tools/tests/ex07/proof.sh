# Sourced by run.sh inside the guest, as root; writes ex07/proof.log.
echo "== ex07: debugfs /sys/kernel/debug/fortytwo =="
run uname -r
run "mountpoint -q /sys/kernel/debug || mount -t debugfs none /sys/kernel/debug"
run insmod $S/ex07/debugfs.ko
run ls -ld /sys/kernel/debug
run chown $U /sys/kernel/debug
run ls -ld /sys/kernel/debug /sys/kernel/debug/fortytwo
run ls -l /sys/kernel/debug/fortytwo
echo "-- id: read/write for everyone, same behavior as ex05"
as_user cat /sys/kernel/debug/fortytwo/id
as_user "echo tjukmong > /sys/kernel/debug/fortytwo/id"
as_user "echo wrong_login > /sys/kernel/debug/fortytwo/id"
echo "-- jiffies: read-only for everyone"
as_user cat /sys/kernel/debug/fortytwo/jiffies
run sleep 1
as_user cat /sys/kernel/debug/fortytwo/jiffies
as_user "echo 0 > /sys/kernel/debug/fortytwo/jiffies"
run "echo 0 > /sys/kernel/debug/fortytwo/jiffies"
echo "-- foo: writable by root only, readable by everyone, up to one page"
as_user "echo not_root > /sys/kernel/debug/fortytwo/foo"
run "echo hello from root > /sys/kernel/debug/fortytwo/foo"
as_user cat /sys/kernel/debug/fortytwo/foo
run "head -c 5000 /dev/zero | tr '\\000' a > /sys/kernel/debug/fortytwo/foo"
as_user "wc -c < /sys/kernel/debug/fortytwo/foo"
run "printf 'first line\\n' > /sys/kernel/debug/fortytwo/foo"
as_user cat /sys/kernel/debug/fortytwo/foo
echo "-- concurrent writers/readers on foo"
run "for i in 1 2 3 4; do (for j in \$(seq 200); do echo writer\$i > /sys/kernel/debug/fortytwo/foo; cat /sys/kernel/debug/fortytwo/foo > /dev/null; done) & done; wait; cat /sys/kernel/debug/fortytwo/foo"
echo "-- cleanup on unload"
run rmmod debugfs
run ls /sys/kernel/debug/fortytwo
run "dmesg | grep -cE 'BUG:|Oops|Call Trace'"
