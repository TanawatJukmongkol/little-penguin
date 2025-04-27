LOCAL_VER="-tjukmong"
KERNEL_VER="6.14.0"
TAR_DIR=/sources
# SRC_DIR=/usr/src/kernel-$KERNEL_VER
SRC_DIR=./linux

BUILD_JOBS=$(( $( nproc ) * 3 / 2 ))
MAKE_FLAGS="-l $( nproc )"

pushd $SRC_DIR

make -j$BUILD_JOBS $MAKE_FLAGS modules_install

cp -v arch/x86/boot/bzImage /boot/vmlinuz-"$KERNEL_VER$LOCAL_VER"
cp -v System.map /boot/System.map-"$KERNEL_VER"
cp -v .config /boot/config-"$KERNEL_VER"

if [ ! -d /usr/share/doc/linux-$KERNEL_VER ]; then
  install -d /usr/share/doc/linux-$KERNEL_VER
  cp -r Documentation/* /usr/share/doc/linux-$KERNEL_VER
fi

popd
