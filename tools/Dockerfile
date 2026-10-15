# Lightweight Nix-based dev container: the actual kernel-build toolchain
# (clang/lld, qemu, OVMF, etc.) is defined once in tools/flake.nix.
# flake.nix stays the single source of truth, but its closure is built
# into this image below so containers start with the toolchain already
# present instead of fetching/building it from scratch on first
# `direnv allow`. direnv still does the runtime activation (via the
# bind-mounted /ws), which is then instant since the store paths already
# exist in the image.
FROM nixos/nix:2.35.1

RUN mkdir -p /etc/nix \
 && echo "experimental-features = nix-command flakes" >> /etc/nix/nix.conf

# git and bash-interactive already ship in the base image's default profile.
# gnused must be in the default profile (not just the devShell): the VS Code
# server bootstrap and the devcontainer CLI run plain `sed` before direnv.
RUN nix profile install nixpkgs#direnv nixpkgs#gnused nixpkgs#nix-ld \
 && ln -sf "$(command -v bash)" /bin/bash

# VS Code's server ships a prebuilt node binary expecting an FHS loader at
# /lib64/ld-linux-x86-64.so.2. nix-ld provides that shim and forwards to the
# real Nix glibc loader with NIX_LD_LIBRARY_PATH; Nix-built binaries ignore it.
# The /usr/lib links satisfy VS Code's check-requirements.sh. `nix build` (not
# `nix eval`) so the store paths actually exist instead of dangling.
RUN glibc="$(nix build --no-link --print-out-paths nixpkgs#glibc.out)" \
 && gcclib="$(nix build --no-link --print-out-paths nixpkgs#gcc.cc.lib)" \
 && mkdir -p /usr/lib /lib64 \
 && ln -sf "$glibc/lib/libc.so.6" /usr/lib/libc.so.6 \
 && ln -sf "$gcclib/lib/libstdc++.so.6" /usr/lib/libstdc++.so.6 \
 && ln -sf /root/.nix-profile/libexec/nix-ld /lib64/ld-linux-x86-64.so.2 \
 && ln -sfn "$glibc/lib/ld-linux-x86-64.so.2" /usr/lib/nix-ld-target \
 && mkdir -p /usr/lib/nix-ld-libs \
 && ln -sf "$glibc"/lib/* "$gcclib"/lib/* /usr/lib/nix-ld-libs/
ENV NIX_LD=/usr/lib/nix-ld-target \
    NIX_LD_LIBRARY_PATH=/usr/lib/nix-ld-libs

RUN printf 'eval "$(direnv hook bash)"\n' >> /root/.bashrc

# The repo is bind-mounted from the host, so it's owned by the host user's
# uid, not container root. Both git and the flake's libgit2-based fetcher
# refuse to touch a repo they don't own unless it's allow-listed.
RUN git config --global --add safe.directory /ws

# Bake the devShell's package closure into the image. flake.lock pins exact
# versions, so as long as it doesn't change, the store paths built here are
# byte-for-byte what direnv resolves at runtime from the bind-mounted /ws.
COPY tools/flake.nix tools/flake.lock /opt/flake/
RUN nix develop /opt/flake#default --command true

# direnv's allow-list is keyed on the .envrc's path plus a hash of its
# content, not on the container filesystem, so approving it here still
# holds once the bind mount swaps this placeholder for the real /ws (same
# content, same hash). This means direnv works from a plain `docker compose
# up` too, without depending on the devcontainer.json postCreateCommand.
COPY .envrc /ws/.envrc
RUN direnv allow /ws/.envrc
