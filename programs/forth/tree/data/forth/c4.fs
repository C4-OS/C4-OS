: print ( addr -- )
  while dup c@ 0 != begin
      dup c@ emit
      1 +
  repeat drop
;

: puts ( addr -- )
  print cr
;

: hex ( n -- )
  "0x" print .x cr drop
;

: print-memfield ( n name -- )
  print ": " print . " cells (" print
  cells .  " bytes)" print
  cr drop
;

: meminfo
  push-meminfo
  "available memory:" print cr
  "     calls" print-memfield
  "parameters" print-memfield
  "      data" print-memfield
;

: make-msgbuf create 8 cells allot ;
: get-type    @ ;
: set-type    ! ;
: is-keycode? get-type 0xbabe = ;
: get-keycode 2 cells + @ ;

1  value debug-msg
2  value map-msg
3  value mapto-msg
4  value unmap-msg
5  value grant-msg
6  value grantto-msg
7  value requestphys-msg
8  value pagefault-msg
9  value dumpmaps-msg
10 value stop-msg
11 value continue-msg
12 value end-msg
13 value kill-msg

make-msgbuf buffer

: msgtest
  buffer recvmsg
  if buffer is-keycode? then
      "got a keypress: " print
      buffer get-keycode . cr drop
  else
      "got message with type " print
      buffer get-type . cr drop
  end
;

: do-send  buffer set-type buffer swap sendmsg ;
: stop     stop-msg     do-send ;
: continue continue-msg do-send ;
: debug    debug-msg    do-send ;
: kill     kill-msg     do-send ;
: dumpmaps dumpmaps-msg do-send ;
: ping     1234         do-send ;

0xffffbeef value list-start
list-start value [[
: ]] ;

: help
  [[
    "  [filename] exec : load the elf from [filename] in the initfs and run it"
    "  [addr] [len] /x : interactive hex dump of [len] words at [addr]"
    "  [id] continue   : continue thread [id]"
    "  [id] dumpmaps   : dump memory maps for the given thread [id] over debug"
    "  meminfo         : display the amount of forth memory available"
    "  memmaps         : display memory maps for the current address space"
    "  print-archives  : list available base words"
    "  ls              : list files contained in the initfs"
    "  help            : print this help"
  ]] puts-list
;

: initfs-print ( name -- )
  tarfind dup tarsize
  "size: " print .   cr drop
  "addr: " print hex cr
;

: puts-list
  while dup list-start != begin
    puts
  repeat
;

: dumpmaps-list
  while dup list-start != begin
    dumpmaps
  repeat
;

( Hex dumper definitions )
: hex-len ( n -- len )
  if dup 0 = then drop 1 end

  0 swap while dup 0 > begin
    0x10 /
    swap 1 + swap
  repeat drop
;

: hex-pad ( n -- n )
  dup hex-len 8 swap - while dup 0 > begin
    0x30 emit
    1 -
  repeat drop
;

: /x-hex-label swap hex-pad .x swap 0x20 0x3a emit emit ;
: /x-key       key drop ;

: /x-emitchars ( addr -- )
  0 while dup 16 < begin
    swap
      if dup c@ 0x19 > then
        dup c@ emit
      else
        0x2e emit
      end
      1 +
    swap
    1 +
  repeat drop drop
;

: /x-chars
  0x7c emit over 4 cells - /x-emitchars 0x7c emit
;

: /x ( addr n -- )
  /x-hex-label

  while dup 0 > begin
    swap
      dup @ hex-pad .x drop
      1 cells +
    swap

    0x20 emit
    1 -

    if dup 96 mod 0 = then
      /x-chars
      cr 0x3a emit
      /x-key

      if dup 0 != then
        /x-hex-label
      end

    else if dup 4 mod 0 = then
      /x-chars
      cr
      /x-hex-label
    end
    end

  repeat
  drop drop
;

: space 0x20 emit ;
: mem-p dup @ hex-pad .x drop 1 cells + ;

: memmaps ( -- )
  0xfcffe010 while dup @ 0 != begin
    mem-p " -> " print mem-p ", " print
    dup @ . drop space 1 cells +
    "pages" print
    cr
    1 cells +
  repeat drop
;

"All systems are go, good luck" print cr

