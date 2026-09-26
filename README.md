# little-penguin

## 1. Introduction

**little-penguin** is the 42 school introduction to Linux kernel development.
Its ten assignments (`ex00`–`ex09`) cover the basic kernel workflow:

| Exercise | Topic |
| --- | --- |
| `ex00` | Build and boot a v6.14 kernel with `CONFIG_LOCALVERSION_AUTO` |
| `ex01` | A "Hello world" loadable module |
| `ex02` | A patch to the kernel `Makefile` (`EXTRAVERSION = -thor_kernel`) |
| `ex03` | Fix a module so it follows the kernel coding style |
| `ex04` | A module that loads itself when a USB keyboard is plugged in (udev) |
| `ex05` | A misc character device, `/dev/fortytwo` |
| `ex06` | Build `linux-next` and patch it (a real fix, [sent to LKML](https://lore.kernel.org/lkml/20260915202534.468958-1-tanawat.jukmon@gmail.com/)) |
| `ex07` | A debugfs interface (`id`, `jiffies`, `foo`) |
| `ex08` | Fix and clean up a broken misc device (`/dev/reverse`) |
| `ex09` | A simple clone of sysfs `/proc/mounts` as `/proc/mymounts` |

This repository also contains the tooling around the assignments:

- a reproducible toolchain (Nix flake or dev container),
- a libvirt/QEMU VM that boots the freshly built kernel on a Linux From Scratch disk,
- an automated test suite that checks each exercise on the host and inside the VM.

## 2. Installation

### Requirements

- An x86_64 Linux host with KVM (`/dev/kvm` must be accessible to your user).
- `libvirtd` running, reachable as `qemu:///session` (rootless).
- About 30 GB of free disk space: two kernel trees plus the VM image.
- The ex04 tests only: a USB keyboard dongle and `spicy` (spice-gtk) on the host.

Clone the repository, and the rest is fetched by `make`:

```sh
git clone <this repo> little-penguin
cd little-penguin
```

### 2.1 Development environment

The whole toolchain (clang/LLVM, bfd `ld`, make, bear, qemu, OVMF, libvirt,
gdb, …) is declared once in [tools/flake.nix](tools/flake.nix). There are two
ways to use it.

#### Option 1: direnv + Nix flake (recommended)

You need [Nix](https://nixos.org/download/) with flakes enabled, and
[direnv](https://direnv.net/) hooked into your shell.

```sh
# ~/.config/nix/nix.conf
experimental-features = nix-command flakes
```

```sh
direnv allow        # once; the root .envrc runs `use flake ./tools`
```

After that, the dev shell loads whenever you `cd` into the repo. It also
exports `OVMF_PATH` and `QEMU_BIN` pointing into the Nix store, and the VM
targets use those values. In VS Code, the `mkhl.direnv` extension gives
clangd the same environment.

#### Option 2: Dev container (For non-nix systems, and 42 campuses)

[.devcontainer/devcontainer.json](.devcontainer/devcontainer.json) builds
[tools/Dockerfile](tools/Dockerfile) through
[tools/docker-compose.yaml](tools/docker-compose.yaml). The image is
`nixos/nix` with the flake's packages already built in, so the container
starts with the full toolchain and `direnv` pre-approved.

- **VS Code:** *Dev Containers: Reopen in Container*.
- **CLI:** `docker compose -f tools/docker-compose.yaml up -d`, then open a
  shell in `little-penguin-dev-container-1`.

The repo is bind-mounted at `/ws`. The container gets `/dev/kvm`, the host's
X11 socket (for `make vm-gui`) and full capabilities, so the VM can run
inside it.

> Nix builds `tools/flake.nix` as pinned by `tools/flake.lock`. Update the lock
> file on purpose only (`nix flake update --flake ./tools`): a change rebuilds
> the dev container image.

### 2.2 Virtual disk image

The VM boots the kernel you built (`-kernel bzImage`, via OVMF/UEFI) on a
Linux From Scratch root filesystem from the earlier **ft_linux** project. The
image is not tracked in git. Put it here:

```
little-penguin/
└── ft_linux/
    └── lfs.qcow2        # qcow2, root filesystem on /dev/sda4
```

The defaults are set at the top of the root [Makefile](Makefile) and can be
overridden:

| Variable | Default | Meaning |
| --- | --- | --- |
| `VM_DISK` | `ft_linux/lfs.qcow2` | Disk image path, relative to the repo root |
| `ROOT_PART` | `/dev/sda4` | Root partition passed on the kernel command line |
| `VM_NAME` | `little-penguin` | libvirt domain name |

The guest needs `systemd`, `udev`, and an unprivileged user named `lfs`. The
tests use that user to check permissions.

### 2.3 Shared volume (QEMU 9p)

The VM definition in [tools/vm/little-penguin.xml](tools/vm/little-penguin.xml)
exports **the whole repository root** to the guest over virtio-9p, under the
mount tag `qemu_share`. The guest kernel config (`ex00/.config`) enables
`CONFIG_NET_9P`, `CONFIG_NET_9P_VIRTIO` and `CONFIG_9P_FS` for this. Mount it
inside the guest with:

```sh
mkdir -p /mnt/qemu_share
mount -t 9p -o trans=virtio,version=9p2000.L qemu_share /mnt/qemu_share
```

or permanently in the guest's `/etc/fstab`:

```
qemu_share  /mnt/qemu_share  9p  trans=virtio,version=9p2000.L,nofail  0 0
```

A module built on the host can then be loaded straight from the guest, for
example `insmod /mnt/qemu_share/ex05/fortytwo.ko`. The test runner mounts the
share by itself if it isn't mounted yet.

The share uses `accessmode="squash"`, so files the guest creates belong to
your host user.

Other VM endpoints:

- **gdb stub:** host port `1122` (`make debug`).
- **SSH:** host port `2222` forwards to guest port `22`.

## 3. Project structure

```
little-penguin/
├── Makefile              # kernel build, VM, tests; drives every exercise
├── ft_linux/lfs.qcow2    # VM disk (not tracked)
├── linux/                # v6.14 tree (cloned by `make linux`, not tracked)
├── linux-next/           # linux-next tree (cloned by ex06, not tracked)
├── ex00/ ex02/ ex06/     # "custom" exercises: kernel .config, patches, kernel.log
├── ex01/ ex03/ … ex08/   # out-of-tree module exercises
├── ex09/                 # in-tree driver, symlinked into linux/drivers/misc/
└── tools/
    ├── flake.nix, flake.lock
    ├── Dockerfile, docker-compose.yaml
    ├── vm/little-penguin.xml   # libvirt domain template (envsubst'd)
    └── tests/                  # test suite (see `make test`)
```

### Conventions

- **Allow-list `.gitignore`s.** Every directory ignores `*` and re-includes
  only its real sources. Build output, `compile_commands.json` and kernel
  trees can never be committed by accident. When you add a new kind of file,
  add it to the allow-list next to it.
- **Exercise types.** `SRCS_INTREE`, `SRCS_PROJECT` and `SRCS_CUSTOM` in the
  root [Makefile](Makefile) sort the exercises into three kinds:
  - **Module exercises** (`ex01`, `ex03`–`ex05`, `ex07`, `ex08`): code in
    `src/*.c`, headers in `include/*.h`, and a `Makefile` that builds a `.ko`
    out of tree (`M=`) against `$(KERN_BUILD)`. The default is
    `/lib/modules/$(uname -r)/build`; the root Makefile passes the repo's tree.
    Udev rules and helper scripts sit next to the sources, for example
    `ex05/99-fortytwo.rules`.
  - **In-tree driver** (`ex09`): `src/` is symlinked into
    `linux/drivers/misc/mymounts` and hooked into the Kconfig and Makefile
    there. It is built into the kernel image with `CONFIG_MYMOUNTS=y`.
    `make clean` in `ex09` removes the hooks again.
  - **Custom exercises** (`ex00`, `ex02`, `ex06`): wrappers around the root
    Makefile for a given tree (`KERN_BUILD=linux` or `linux-next`). Each keeps
    its own `.config` and `*.patch` files and the resulting `kernel.log`.
    Patches are applied with `git am`, so the version string doesn't get a
    `-dirty` suffix.
- **Coding style.** Code follows the kernel coding style and must pass
  `checkpatch.pl --strict` with no errors or warnings. `make format` runs it
  with `--fix-inplace`. `.clang-format` is a symlink to the kernel's own.
- **Toolchain.** clang/LLVM (`LLVM=1`) does the compiling and bfd `ld` does
  the linking (`LD=ld`), because `ld.lld` breaks the x86 real-mode
  trampoline. `CC` is fixed to `cc` so kbuild doesn't rebuild everything when
  the shell's `CC` changes.
- **Tests.** `tools/tests/exNN/` holds up to three scripts per exercise:
  - `host.sh`: logs, config, patch, checkpatch and build checks, run on the host.
  - `guest.sh`: loads and exercises the module inside the VM, as root.
  - `proof.sh`: writes the `exNN/proof.log` transcript that is handed in.

  They print `OK - …` and `KO - …` lines through the helpers in
  [tools/tests/lib.sh](tools/tests/lib.sh).

## 4. Makefile targets

Run these from the repo root. `KERN_BUILD` selects the kernel tree and
defaults to `linux`; for example, `make build KERN_BUILD=linux-next`.

### Kernel

| Target | Description |
| --- | --- |
| `make` / `make all` | Clone the tree if needed, then build it |
| `make linux` | Shallow-clone Linux v6.14 into `$(KERN_BUILD)` and seed `.config` from `ex00/.config` |
| `make config` | `menuconfig`, then save the result back to `ex00/.config` |
| `make savecfg` | Copy `$(KERN_BUILD)/.config` to `ex00/.config` |
| `make mrproper` | `make mrproper` in the tree, then restore the saved `.config` |
| `make build` | Build the kernel (`-j` = 1.5 × cores; override with `BUILD_JOBS=`) |

### Exercises

| Target | Description |
| --- | --- |
| `make driver` | Build every exercise's module or driver against `$(KERN_BUILD)` |
| `make format` | Run `checkpatch.pl --strict --fix-inplace` on every exercise (**edits sources**) |
| `make clean` | Clean every exercise and the test logs |
| `make fclean` | `clean`, plus remove `compile_commands.json` and run `make clean` in the kernel tree. **This deletes `bzImage`**; the next `make build` or `make test` rebuilds it. |
| `make re` | `fclean` + `all` |

Each exercise directory can also be built on its own:

- **Module exercises:** `make`, `make clean`, `make fclean`, `make format`,
  `make re`. Pass `KERN_BUILD=../linux` to build against the repo's tree.
- **Custom exercises** (`ex00`, `ex02`, `ex06`): `make` runs
  clone → patch → build → boot and writes `kernel.log`. `make patch` and
  `make patch-r` apply and revert the exercise's patch. `make vm`,
  `make vm-gui` and `make debug` work on that exercise's tree.

### Virtual machine

| Target | Description |
| --- | --- |
| `make vm` | Define and start the VM with `$(KERN_BUILD)`'s `bzImage`, attached to the serial console (leave with `Ctrl+]`) |
| `make vm-gui` | Same, with a virtio GPU and no serial console; connect with virt-manager or `spicy` |
| `make vm-clean` | Destroy and undefine the VM |
| `make debug` | Attach `gdb` (TUI) to the VM's gdb stub on `:1122`, with `vmlinux` symbols |
| `make log` | Boot headless, run `LOG_RUN` (default `uname -a`), power off, and save the serial console to `LOG` (default `kernel.log`), with CRs and ANSI escape codes removed |

Useful variables: `CMDLINE`, `KERNEL_IMG`, `BOOT_TIMEOUT` (default 180 s),
`LOG_RUN`, `LOG`.

### Tests

| Target | Description |
| --- | --- |
| `make test` | Build the kernel image (if missing) and the modules, run host checks, boot the VM for guest checks, print every result and a summary. Exits non-zero on any `KO` or missing log. |
| `make test EX="ex05 ex07"` | Run only some exercises |
| `make proof` | Boot the VM and regenerate `exNN/proof.log` for exercises that have a `proof.sh` |
| `make kasan` | Enable KASAN (generic, inline, vmalloc) in `$(KERN_BUILD)/.config` and rebuild |
| `make test-kasan` | `kasan` + `test`. Every guest test also fails if it caused a new KASAN report. |
| `make test EX=kasan` | Sanity check: a module that triggers a known out-of-bounds bug, to confirm KASAN reports it |

Logs go to `tools/tests/exNN/{host,guest}.log`. The guest's serial console goes
to `tools/tests/serial.log`, the first place to look when guest logs are
missing. The ex04 guest test and proof need the USB keyboard dongle
(`0x0c45:0xfefe`) plugged into the host; it is passed through to the VM
automatically.

> `make mrproper` in `ex00` or `ex06` restores the saved `.config`, which also
> switches KASAN off again.

## 5. Use of LLMs disclaimer

### This project made use of LLM, primarily for:
1. Miscellaneous and grunt work
2. Write automation and toolings (tests, and vm automation)
3. Rapid prototyping, and ideas (after solid project base)
4. Research aid, and formatting standards (with `make format`)
5. Partial driver development aid

### This project does not use LLM for:
1. Initial setup prototyping phase (no LLM involved)
2. Decision making
3. Project design and orcastration
4. Workflow, and tooling design (very human)
5. Vibe coding (manually write code, when LLM hallucinates)

## 6. Resources

The kernel development community. (n.d.). *Linux kernel coding style*. The Linux Kernel documentation. Retrieved September 26, 2026, from https://docs.kernel.org/process/coding-style.html

The kernel development community. (n.d.). *Building external modules*. The Linux Kernel documentation. Retrieved September 26, 2026, from https://docs.kernel.org/kbuild/modules.html

The kernel development community. (n.d.). *Submitting patches: The essential guide to getting your code into the kernel*. The Linux Kernel documentation. Retrieved September 26, 2026, from https://docs.kernel.org/process/submitting-patches.html

The kernel development community. (n.d.). *Kernel Address Sanitizer (KASAN)*. The Linux Kernel documentation. Retrieved September 26, 2026, from https://docs.kernel.org/dev-tools/kasan.html

Corbet, J., Rubini, A., & Kroah-Hartman, G. (2005). *Linux device drivers* (3rd ed.). O'Reilly Media. https://lwn.net/Kernel/LDD3/

libvirt Project. (n.d.). *Domain XML format*. libvirt. Retrieved September 26, 2026, from https://libvirt.org/formatdomain.html

QEMU Project. (n.d.). *9p-virtio: Documentation/9psetup*. QEMU Wiki. Retrieved September 26, 2026, from https://wiki.qemu.org/Documentation/9psetup

Nix Project. (n.d.). *Flakes*. Nix reference manual. Retrieved September 26, 2026, from https://nix.dev/manual/nix/stable/command-ref/new-cli/nix3-flake

Jukmongkol, T. (2026, September 15). *[PATCH] fs/buffer: fix NULL deref on folio-less bh in __bh_submit* [Mailing list post]. Linux Kernel Mailing List. https://lore.kernel.org/lkml/20260915202534.468958-1-tanawat.jukmon@gmail.com/
