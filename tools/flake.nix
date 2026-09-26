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
      in {
        devShells.default = pkgs.mkShell.override {
          stdenv = pkgs.llvmPackages.stdenv;
        } {
          name = "kernel-dev-shell";

          # The cc-wrapper's userspace hardening flags leak into every kernel
          # compile: strictoverflow's -fwrapv overrides the kernel's own
          # -fno-strict-overflow, which kbuild then rejects under
          # -Werror=unused-command-line-argument.
          hardeningDisable = [ "all" ];

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
            libvirt
            gettext
          ];

          shellHook = ''
            export OVMF_PATH="${pkgs.OVMF.fd}/FV/OVMF.fd"
            export QEMU_BIN="${pkgs.qemu}/bin/qemu-system-x86_64"
          '';
        };
      });
}
