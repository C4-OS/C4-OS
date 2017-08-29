Things to do security-wise:

- Implement 'context' objects which determine whether a thread can request
  physical memory, whether it can subscribe to interrupts, how many objects
  can be created in that context and which types, etc
- When sending a new object message, don't leave the kernel object pointer
  in the userland-facing message
- When mapping frames into address spaces, make sure that the page permissions
  match the requested mapping permissions
- Make sure that requested page mappings won't overwrite the kernel's address
  space
