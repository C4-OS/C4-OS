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

create file-buffer 1024 allot
0 value cat-file

: print-file ( fname -- )
  "r" open-file
  if 0 = then
    to cat-file
    while file-buffer 1024 cat-file read-line 0 = begin
        file-buffer print drop
    repeat

    cat-file close-file drop
  end
;

: cat ( [[ files ]] -- )
  while dup list-start != begin
    print-file
  repeat
  drop
;

0 value ls-dir

: dirent-name ( dirent* -- char* )
  11 +
;

: ls ( path -- )
  open-dir
  if 0 = then
    to ls-dir
    while ls-dir read-dir 0 = begin
        dirent-name puts
    repeat

    ls-dir close-dir drop
  end
;

: make-msgbuf create 8 cells allot ;
: get-type    @ ;
: set-type    ! ;
: is-keycode? get-type 0xbabe = ;
: get-keycode 2 cells + @ ;

0 value c4_cur_cspace
1 value c4_serv_port
2 value c4_cur_aspace
3 value c4_boot_info
4 value c4_pager
5 value c4_nameserver
6 value c4_def_obj_end

( TODO: add words for new library functions, these don't work anymore )

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
    ( NOTE: this list is in reversed from printed order! )
    "  [filename] exec   : load the elf from [filename] and run it"
    "  memmaps           : display memory maps for the current address space"
    "  meminfo           : display the amount of forth memory available"
    "  [addr] [len] /x   : interactive hex dump of [len] words at [addr]"
    "  [[ files ]] cat   : print all files given in [[ files ]] list"
    "  [path] list-files : generate a list of files at [path]"
    "  [path] print-file : print the file at [path]"
    "  [path] ls         : list files at [path]"
    "  print-archives    : list available base words"
    "  help              : print this help"
  ]] puts-list
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

