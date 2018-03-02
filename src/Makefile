CPPFLAGS=-O3 -mavx2 -std=c++11 -Wall `pkg-config --cflags mysqlclient`
LOADLIBES=`pkg-config --libs mysqlclient`
contest:digamma.o svb_slda.o mysqlwrapper.o mb_string.o
clean:
	rm contest *.o
