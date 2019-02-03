The C4 Operating System
=======================

### What is it?

C4-OS aims to be a relatively complete hobby operating system built around the
C4 kernel, which is an L4-like microkernel. Right now, it's composed of a the
kernel (non-SMP, thread-based IPC), a few low-level utility libraries such as
the c4 runtime, some PC device drivers, some test programs, and a forth
interpreter.

### What can it do?

* Multithreading, SMP-capable
* Userland drivers and pagers
* Capability-oriented syscall API
* Syncronous, asyncronous and channel IPC
* Compositing display manager
* Load static ELF executables from disk
* Load and interpret forth programs from disk
* Read and write PATA drives in PIO mode
  * DMA soon lol
* Read from ext2 file systems, incomplete
  * Partial write support, very broken

### What's in the works?

* Software ports
  * lua, libpng, freetype2, zlib, the usual suspects
  * POSIX compatibility layer
* Networking
  * e1000, r8139, virtio drivers planned
  * port of lwip or uip for the TCP stack
* Graphics
  * Framebuffer multiplexer
  * Fancy window manager and programs for it (console, file manager, etc)
* File systems
  * VFS layer, for providing virtual mount points and caching on top of
    filesystems
  * FAT driver
* Platforms
  * x86-64 relatively soon
  * support for some ARM-based SBCs (beaglebone, raspberry pi)

### What's in the future?

* Virtualization
  * Paravirtualization
    * "OS rehosting"
	* based on vCPUs in L4 Fiasco
  * Exposing intel and amd virtualization instruction sets to userland
* support for NetBSD rump kernels, based on work the sel4 folks have done
  * [See here](https://research.csiro.au/tsblog/using-rump-kernels-to-run-unmodified-netbsd-drivers-on-sel4/)

### Wait, but why?

* I like operating systems, became bored of working on my unix clone, and wanted
  to learn more about microkernels. If you happen to be looking for a
  production-ready microkernel, take a look at fiasco or sel4.
