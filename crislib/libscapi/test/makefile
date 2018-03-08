PROGRAM= tests.exe

all:: $(PROGRAM)

$(PROGRAM): 
	g++ -std=c++11 -maes -mavx -I/usr/include/openssl -I../install/include -o tests.exe tests.cpp interactiveMidProtocolsTests.cpp ../libscapi.a -ldl -lpthread -L../install/lib -lssl -lboost_system -lntl -lgmp -lcrypto

tests:: all
	$(Program)

clean::
	rm tests.exe



