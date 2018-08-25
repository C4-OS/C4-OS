- global
  - O2 is broken in userland after new toolchain, maybe breaking syscalls
    in c4rt?
  - kernel-level resource freeing still needs to be implemented
  - sigma0 needs to build a full map of available address space
- stubbywm
  - Performance could be significantly improved by targeting a framerate,
    rather than redrawing on every update event.
    Implementing this requires clock() and usleep() equivalents, needing new
    kernel code.
  - Transparency is slightly broken, due to redrawing areas multiple times
    when updating. Fixing this also improves performance.
    FIX: test for intersections when doing updates, and split intersecting
         updates into multiple, non-intersecting updates
