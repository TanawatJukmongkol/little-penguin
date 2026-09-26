# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
check "insmod reverse.ko" "insmod $S/ex08/reverse.ko"
check "/dev/reverse exists" '[ -c /dev/reverse ]'
check "reverses a line" "echo hello world > /dev/reverse; [ \"\$(cat /dev/reverse | tr -d '\\n')\" = 'dlrow olleh' ]"
check "reverses without a newline" 'printf abc > /dev/reverse; [ "$(cat /dev/reverse)" = cba ]'
check "long writes stop at 4095 bytes" "head -c 5000 /dev/zero | tr '\\000' x > /dev/reverse; [ \$(wc -c < /dev/reverse) -eq 4095 ]"
check "survives concurrent writers" "for i in 1 2 3 4; do (for j in \$(seq 200); do echo r\$i > /dev/reverse; cat /dev/reverse > /dev/null; done) & done; wait; cat /dev/reverse | tr -d '\\n' | grep -qx '[1-4]r'"
check "rmmod reverse" "rmmod reverse"
check "no kernel BUG or Oops" "! dmesg | grep -qE 'BUG:|Oops|Call Trace'"
