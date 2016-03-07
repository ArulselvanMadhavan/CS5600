High Level Instructions to run:
1. untar the hw2.tar.gz
2. make check

Sample commands to run the master:
./master --workerpath ./worker --wait_mechanism select -x 10 -n 100
./master --workerpath ./worker --wait_mechanism poll -x 10 -n 100
./master --workerpath ./worker --wait_mechanism epoll -x 10 -n 100
./master --workerpath ./worker --wait_mechanism sequential -x 10 -n 100

Sample commands to run the worker
./worker -x 10 -n 10

You can also use the 'make check' to test the program
make file targets -> all,clean,dist,build,check

