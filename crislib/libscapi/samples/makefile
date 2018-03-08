INC               =  -I../../boost_1_64_0 -I../install/include -I../lib/OTExtensionBristol -I../install/include/libOTe -I../install/include/libOTe/cryptoTools
CXX               = g++ -std=c++14 $(INC) -O3 -Wall -Wno-unused-function -Wno-unused-variable
LIBS              =  -L../install/lib -L../../boost_1_64_0/stage/lib
LINKER_OPTIONS    =  ../libscapi.a ../install/lib/libOTExtensionBristol.a ../install/lib/libOTe.a ../install/lib/libcryptoTools.a ../install/lib/libmiracl.a ../install/lib/libsimpleot.a $(LIBS) ../install/lib/libcrypto.a -ldl -lboost_log -lboost_system -lboost_thread -lboost_serialization -lboost_filesystem -lpthread ../install/lib/libssl.a -lgmp -lrt
CPP_OPTIONS       = -std=c++14 $(INC) -O3 -Wall -Wno-unused-function
OBJS              = simple_dlog.o simple_sha1.o CommitmentExample.o OTExample.o SigmaProtocolExample.o comm_example.o App1.o YaoParties.o OTExtensionBristolExample.o OTExtensionLiboteExample.o

all: libscapi_example

libscapi_example: $(OBJS)
	$(CXX) examples_main.cpp -o $@ $(OBJS) $(LINKER_OPTIONS)

%.o: Simple/%.cpp
	$(CXX) -c $< -o $@

%.o: Commitment/%.cpp
	$(CXX) -c $< -o $@

%.o: OT/%.cpp
	$(CXX) -c -maes -mpclmul $< -o $@

%.o: OTExtensionLibote/%.cpp
	$(CXX) -c -Wextra -Wno-non-template-friend -pthread -maes -mpclmul -msse4.1 -msse2 -I. $< -o $@
	
%.o: OTExtensionBristol/%.cpp
	$(CXX) -c -Wextra -Wno-non-template-friend -pthread -maes -mpclmul -msse4.1 -msse2 -I. $< -o $@

%.o: SigmaProtocols/%.cpp
	$(CXX) -c $< -o $@
	
%.o: Comm/%.cpp
	$(CXX) -c $< -o $@

%.o: Yao/%.cpp
	$(CXX) -c $< -o $@

clean:
	rm -f libscapi_example
	rm -f ./*.o





