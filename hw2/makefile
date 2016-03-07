#declare the variables
ARG1=--workerpath ./worker --wait_mechanism sequential -x 10 -n 100

all:	check

default: check

clean:
	rm -rf master main worker hello

build:
	gcc -o master master.c
	gcc worker.c -lm -o worker

dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar

check:	clean build
	(sleep 2)
	./master --workerpath ./worker --wait_mechanism select -x 10 -n 5
	(sleep 2)
	./master --workerpath ./worker --wait_mechanism poll -x 10 -n 5
	(sleep 2)
	./master --workerpath ./worker --wait_mechanism epoll -x 10 -n 5
	(sleep 2)
	./master --workerpath ./worker --wait_mechanism sequential -x 10 -n 5