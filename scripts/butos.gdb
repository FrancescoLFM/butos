set architecture i386:x86-64
target remote localhost:1234
break _start
continue
next 1