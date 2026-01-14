FROM debian:bookworm-20251229-slim as dev-core

# QEMU dependencies
RUN apt-get update -y
RUN apt-get install -y --no-install-recommends \
    qemu-system-x86 qemu-utils qemu-kvm qemu-system-gui ovmf

# Essential dependencies
RUN apt-get install -y \
    openssl libssl-dev \
    perl \
    elfutils libdw-dev \
    cpio xz-utils \
    zlib1g zlib1g-dev \
    rsync \
    ncurses-bin libncurses-dev \
    libelf1 libelf-dev \
    python3 \
    ccache

# kernel builds
RUN apt-get install -y \
    build-essential \
    git make binutils bc bison flex \
    util-linux

# Install clang / tools
RUN apt-get install -y \
    llvm-16 clang-16 lld-16 \
    clangd \
    bear

# Update cc to clang-16
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-16 100 \
 && update-alternatives --set cc /usr/bin/clang-16

# Fix linux's hardcoded paths
RUN ln -f /usr/bin/clang-16        /usr/bin/clang \
 && ln -f /usr/bin/clang++-16      /usr/bin/clang++ \
 && ln -f /usr/bin/ld.lld-16       /usr/bin/ld.lld \
 && ln -f /usr/bin/llvm-ar-16      /usr/bin/llvm-ar \
 && ln -f /usr/bin/llvm-nm-16      /usr/bin/llvm-nm \
 && ln -f /usr/bin/llvm-objcopy-16 /usr/bin/llvm-objcopy \
 && ln -f /usr/bin/llvm-strip-16   /usr/bin/llvm-strip \
 && ln -f /usr/bin/llvm-readelf-16 /usr/bin/llvm-readelf
