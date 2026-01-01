FROM debian:bookworm-20251229-slim as dev-core

RUN apt-get update -y
RUN apt-get install -y --no-install-recommends \
            qemu-system-x86 qemu-utils qemu-kvm qemu-system-gui ovmf

RUN apt-get install -y \
            build-essential \
            git make binutils bc bison flex \
            openssl libssl-dev \
            perl \
            elfutils libdw-dev \
            cpio xz-utils \
            zlib1g zlib1g-dev \
            rsync \
            ncurses-bin libncurses-dev \
            libelf1 libelf-dev \
            python3 \
            ccache \
            util-linux \
            bear
