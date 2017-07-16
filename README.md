The C4 Operating System
=======================

### What is it?

C4-OS aims to be a relatively complete hobby operating system built around the
C4 kernel, which is an L4-like microkernel. Right now, it's composed of a the
kernel (non-SMP, thread-based IPC), a few low-level utility libraries such as
the c4 runtime, some PC device drivers, some test programs, and a forth
interpreter.

### What can it do?

* Multithreading, userland drivers and pagers
* Syncronous, asyncronous and channel IPC
* Read and write PATA drives in PIO mode
  * DMA soon lol
* Read from ext2 file systems, incomplete
  * Write support coming soon
* Framebuffer display with bitmap fonts
* Load static ELF executables from disk
* Load and interpret forth programs from disk

### What's in the works? (timeline: next few months)

* Capability-oriented kernel API, currently being written
  * This is currently the main bottleneck for expanding the system,
    thread-oriented IPC was much more limiting than I had thought it would be.
	Once this is working and stable, much more sophisticated programs
	can be written.
* SMP support
  * currently being merged into the kernel tree, following capabilities
  * Will involve a scheduler rewrite and lots of new locking code
* Software ports
  * lua, libpng, freetype2, zlib, the usual suspects
  * currently on hold for capabilities, since that will have major impacts on
    memory management
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
* Other
  * Mayyyyybe a posix compatibility layer, still not sure about this one.

### What's in the future? (timeline: 1+ years)

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
