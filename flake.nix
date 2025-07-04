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
          stdenv = pkgs.gcc14Stdenv;
        } {
          name = "kernel-dev-shell";

          buildInputs = with pkgs; [
            git
            stdenv
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
            linux_6_14
            bear
          ];
        };
      });
}
