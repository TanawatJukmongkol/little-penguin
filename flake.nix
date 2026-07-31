{
  description = "Linux kernel development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11"; # You can pin a version if needed
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        # clang (pre-20, as used here via LLVM=1) never consumes
        # -fno-strict-overflow, and kbuild's top Makefile unconditionally
        # adds it, then promotes the resulting "unused argument" warning to
        # a hard error via -Werror. This wraps `cc` itself instead of
        # living as a path-relative script (the old ./cc-wrapper), so every
        # Makefile in the repo (root and each exercise) can just say
        # `CC = cc` and get the suppression, regardless of which directory
        # make runs from.
        ccWrapper = pkgs.writeShellScriptBin "cc" ''
          exec ${pkgs.llvmPackages.clang}/bin/clang "$@" -Wno-unused-command-line-argument
        '';
      in {
        devShells.default = pkgs.mkShell.override {
          # The Makefile always builds with LLVM=1, which makes the kernel's
          # tools/scripts/Makefile.include probe `cc` to decide whether it's
          # clang. A gcc `cc` alongside a clang HOSTCC (forced by LLVM=1)
          # trips that probe and adds a GCC-only warning flag to clang
          # builds, breaking tools/objtool. Use a clang+lld stdenv so `cc`
          # actually is clang, matching what the old devcontainer did with
          # `update-alternatives --set cc clang-16`.
          stdenv = pkgs.llvmPackages.stdenv;
        } {
          name = "kernel-dev-shell";

          buildInputs = with pkgs; [
            git
            gnumake
            binutils
            bc
            bison
            flex
            openssl
            perl
            elfutils
            cpio
            xz
            libz
            rsync
            ncurses
            ncurses.dev
            libelf
            python3
            ccache
            util-linux
            bear
            llvmPackages.clang
            llvmPackages.bintools
            qemu
            OVMF.fd
            gdb
            ccWrapper
          ];

          shellHook = ''
            export OVMF_PATH="${pkgs.OVMF.fd}/FV/OVMF.fd"
            # Must win over the stdenv's own `cc` (plain clang, no
            # suppression) and over llvmPackages.clang's `cc` symlink.
            export PATH="${ccWrapper}/bin:$PATH"
          '';
        };
      });
}
