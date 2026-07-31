# Lightweight Nix-based dev container: the actual kernel-build toolchain
# (clang/lld, qemu, OVMF, etc.) is defined once in ../../../flake.nix.
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
RUN nix profile install nixpkgs#direnv \
 && ln -sf "$(command -v bash)" /bin/bash

RUN printf 'eval "$(direnv hook bash)"\n' >> /root/.bashrc

# The repo is bind-mounted from the host, so it's owned by the host user's
# uid, not container root. Both git and the flake's libgit2-based fetcher
# refuse to touch a repo they don't own unless it's allow-listed.
RUN git config --global --add safe.directory /ws

# Bake the devShell's package closure into the image. flake.lock pins exact
# versions, so as long as it doesn't change, the store paths built here are
# byte-for-byte what direnv resolves at runtime from the bind-mounted /ws.
COPY flake.nix flake.lock /opt/flake/
RUN nix develop /opt/flake#default --command true

# direnv's allow-list is keyed on the .envrc's path plus a hash of its
# content, not on the container filesystem, so approving it here still
# holds once the bind mount swaps this placeholder for the real /ws (same
# content, same hash). This means direnv works from a plain `docker compose
# up` too, without depending on the devcontainer.json postCreateCommand.
COPY .envrc /ws/.envrc
RUN direnv allow /ws/.envrc
