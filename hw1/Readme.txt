High Level Instructions to run:
1. untar the hw1.tar.gz
2. ./test
3. kill -12 PID
4. ./restart myckpt

You will see the numbers getting printed from where they left off before getting killed.

You can also use the 'make check'


Detailed Instructions:
How to compile?
1. How to create libckpt.a -> gcc -c -o ckpt.o ckpt.c && ar rcs libckpt.a ckpt.o
2. How to create an executable with libckpt protection ->
    gcc -g -static -L[path to the root directory of libckpt] -lckpt -Wl,-u,init_signal -o test test.c
3.  gcc -g -static -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 -o restart restart.c


How to run?
1. Run the executable -> ./test
2. It will print numbers in steps of 1
3. Kill the process
4. A Checkpoint with the name of 'myckpt' will be created.

How to restore?
1. ./restart myckpt
2. It will print the numbers from the place we stop.

MakeFile Name: makefile

LogFile for creating Checkpoint: writeLog.txt
LogFile for restoring Checkpoint: readLog.txt
