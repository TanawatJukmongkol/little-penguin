# Sourced by run.sh inside the guest, as root; helpers come from lib.sh.
check "insmod fortytwo.ko" "insmod $S/ex05/fortytwo.ko"
check "/dev/fortytwo exists" '[ -c /dev/fortytwo ]'
check "read returns the login" '[ "$(cat /dev/fortytwo)" = tjukmong ]'
check "login with a newline is accepted" 'echo tjukmong > /dev/fortytwo'
check "login without a newline is accepted" 'printf tjukmong > /dev/fortytwo'
check "wrong login is rejected" '! echo wrong_login > /dev/fortytwo'
check "login with extra characters is rejected" '! echo tjukmongx > /dev/fortytwo'
check "rmmod fortytwo" "rmmod fortytwo"
check "/dev/fortytwo is gone after unload" '[ ! -e /dev/fortytwo ]'
