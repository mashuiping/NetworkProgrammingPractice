test:commandLineParser.o clientAndServer.o main.o
	g++ commandLineParser.o clientAndServer.o main.o -o test -L../../../build/release-install/lib -lmuduo_net -lmuduo_base -lboost_program_options

commandLineParser.o:commandLineParser.h commandLineParser.cc
	g++ -c commandLineParser.cc -o commandLineParser.o

clientAndServer.o:clientAndServer.cc clientAndServer.h
	g++ -c clientAndServer.cc -o clientAndServer.o -I../../../build/release-install/include
main.o:
	g++ -c main.cc -o main.o -I../../../build/release-install/include

clean:
	rm -rf *.o test
