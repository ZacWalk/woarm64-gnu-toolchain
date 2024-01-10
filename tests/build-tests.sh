rm *.s *.a *.gkd *.exe *.dll

CC=../bin-x86_64-w64-mingw32/bin/x86_64-w64-mingw32-gcc
CXX=../bin-x86_64-w64-mingw32/bin/x86_64-w64-mingw32-g++
DT=../bin-x86_64-w64-mingw32/bin/x86_64-w64-mingw32-dlltool

$CC -g -o testdll.dll -s -shared dll.c -Wl,--subsystem,windows
$DT -k --output-lib libtestdef.a --def dll.def

$CXX -S test-throw.cpp -o test-throw.cpp.s -fdump-final-insns
$CXX -S hello.cpp -o hello.cpp.s -fdump-final-insns
$CC -S hello.c -o hello.c.s -fdump-final-insns
$CC -S test.c -o test.s -fdump-final-insns

$CXX -g test-throw.cpp -o test-throw.exe
$CXX -g hello.cpp -o hello.cpp.exe
$CC -g hello.c -o hello.c.exe
$CC -g test.c -L. -ltestdef -ltestdll -o test.exe