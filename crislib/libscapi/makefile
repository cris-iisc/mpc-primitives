export builddir=$(abspath ./build)
export prefix=$(abspath ./install)
CXX=g++
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ARCH := $(shell getconf LONG_BIT)
SHARED_LIB_EXT:=.so
INCLUDE_ARCHIVES_START = -Wl,-whole-archive # linking options, we prefer our generated shared object will be self-contained.
INCLUDE_ARCHIVES_END = -Wl,-no-whole-archive 
SHARED_LIB_OPT:=-shared

export uname_S
export ARCH
export SHARED_LIB_EXT
export INCLUDE_ARCHIVES_START
export INCLUDE_ARCHIVES_END
export SHARED_LIB_OPT
export exec_prefix=$(prefix)
export includedir=$(prefix)/include
export bindir=$(exec_prefix)/bin
export libdir=$(prefix)/lib

SLib           = libscapi.a
CPP_FILES     := $(wildcard src/*/*.cpp)
C_FILES     := $(wildcard src/*/*.c)
OBJ_FILES     := $(patsubst src/%.cpp,obj/%.o,$(CPP_FILES))
OBJ_FILES     += $(patsubst src/%.c,obj/%.o,$(C_FILES))
OUT_DIR        = obj obj/mid_layer obj/circuits obj/comm obj/infra obj/interactive_mid_protocols obj/primitives obj/circuits_c obj/cryptoInfra
INC            = -Iinstall/include -Iinstall/include/OTExtensionBristol -Iinstall/include/libOTe -Iinstall/include/libOTe/cryptoTools
GCC_STANDARD = c++14
CPP_OPTIONS   := -std=$(GCC_STANDARD) $(INC)  -maes -mpclmul -mbmi2 -Wall -Wno-uninitialized -Wno-format -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-sign-compare -Wno-parentheses -O3
$(COMPILE.cpp) = g++ -c $(CPP_OPTIONS) -o $@ $<
LIBRARIES_DIR  = -Linstall/lib
LD_FLAGS = 
SUMO = no


all: libs libscapi tests
	echo $(WITH_EMP)
ifeq ($(GCC_STANDARD), c++11)
    libs: compile-openssl compile-boost compile-json compile-ntl compile-blake compile-emp-tool compile-emp-ot compile-emp-m2pc compile-otextension-bristol
else
libs: compile-openssl compile-boost compile-json compile-libote compile-ntl compile-blake compile-emp-tool compile-emp-ot compile-emp-m2pc compile-otextension-bristol
endif
libscapi: directories $(SLib)
directories: $(OUT_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(SLib): $(OBJ_FILES)
	ar ru $@ $^ 
	ranlib $@

obj/circuits/%.o: src/circuits/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $< 	 
obj/circuits_c/%.o: src/circuits_c/%.c
	gcc -fPIC -mavx -maes -mpclmul -DRDTSC -DTEST=AES128  -O3 -c -o $@ $< 
obj/comm/%.o: src/comm/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $< 	 
obj/infra/%.o: src/infra/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $< 	 
obj/interactive_mid_protocols/%.o: src/interactive_mid_protocols/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $< 	 
obj/primitives/%.o: src/primitives/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $< 	 
obj/mid_layer/%.o: src/mid_layer/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $<
obj/cryptoInfra/%.o: src/cryptoInfra/%.cpp
	g++ -c $(CPP_OPTIONS) -o $@ $<

tests: compile-tests
	cd ./test; ./tests.exe
	
.PHONY: compile-tests
compile-tests:
	@cd ./test; \
	g++ -std=c++14 -maes -mavx -I/usr/include/openssl  -I../install/include -o tests.exe tests.cpp interactiveMidProtocolsTests.cpp ../libscapi.a -lpthread -L../install/lib ../install/lib/libboost_system.a ../install/lib/libboost_thread.a -l:libssl.a -lntl -lgmp -l:libcrypto.a -ldl -lz;
	@cd ..
	
prepare-emp:compile-openssl
ifeq ($(SUMO),yes)
	@mkdir -p $(builddir)/EMP
	@cp -r lib/EMP/. $(builddir)/EMP
	@cmake -DALIGN=16 -DARCH=X64 -DARITH=curve2251-sse -DCHECK=off -DFB_POLYN=251 -DFB_METHD="INTEG;INTEG;QUICK;QUICK;QUICK;QUICK;LOWER;SLIDE;QUICK" -DFB_PRECO=on -DFB_SQRTF=off -DEB_METHD="PROJC;LODAH;COMBD;INTER" -DEC_METHD="CHAR2" -DCOMP="-O3 -funroll-loops -fomit-frame-pointer -march=native -msse4.2 -mpclmul -Wno-unused-function -Wno-unused-variable -Wno-return-type -Wno-discarded-qualifiers" -DTIMER=CYCLE -DWITH="MD;DV;BN;FB;EB;EC" -DWORD=64 $(builddir)/EMP/relic/CMakeLists.txt
	@cd $(builddir)/EMP/relic && $(MAKE)
	@cd $(builddir)/EMP/relic && $(MAKE) install
	@touch prepare-emp
endif

compile-emp-tool:prepare-emp
ifeq ($(SUMO),yes)
	@cd $(builddir)/EMP/emp-tool
	@cmake -DOPENSSL_ROOT_DIR=$(includedir) -DOPENSSL_INCLUDE_DIR=$(includedir) \
	-DOPENSSL_LIBRARIES=$(libdir) -DOPENSSL_CRYPTO_LIBRARY=$(libdir)/libcrypto.a \
	-DOPENSSL_SSL_LIBRARY=$(libdir)/libssl.a \
	-D CMAKE_CXX_FLAGS="-Wno-unused-function -Wno-unused-variable -Wno-return-type" \
	$(builddir)/EMP/emp-tool/CMakeLists.txt
	@cd $(builddir)/EMP/emp-tool/ && $(MAKE)
	@cd $(builddir)/EMP/emp-tool/ && $(MAKE) install
	@touch compile-emp-tool
endif

compile-emp-ot:compile-emp-tool
ifeq ($(SUMO),yes)
	@cd $(builddir)/EMP/emp-ot
	@cmake -DOPENSSL_ROOT_DIR=$(includedir) -DOPENSSL_INCLUDE_DIR=$(includedir) \
	-DOPENSSL_LIBRARIES=$(libdir) -DOPENSSL_CRYPTO_LIBRARY=$(libdir)/libcrypto.a \
	-DOPENSSL_SSL_LIBRARY=$(libdir)/libssl.a \
	-D CMAKE_CXX_FLAGS="-Wno-unused-function -Wno-unused-variable -Wno-return-type" \
	$(builddir)/EMP/emp-ot/CMakeLists.txt
	@cd $(builddir)/EMP/emp-ot/ && $(MAKE)
	@cd $(builddir)/EMP/emp-ot/ && $(MAKE) install
	@touch compile-emp-ot
endif
	
compile-emp-m2pc:compile-emp-ot
ifeq ($(SUMO),yes)
	@cd $(builddir)/EMP/emp-m2pc
	@cmake -DOPENSSL_ROOT_DIR=$(includedir) -DOPENSSL_INCLUDE_DIR=$(includedir) \
	-DOPENSSL_LIBRARIES=$(libdir) -DOPENSSL_CRYPTO_LIBRARY=$(libdir)/libcrypto.a \
	-DOPENSSL_SSL_LIBRARY=$(libdir)/libssl.a \
	-D CMAKE_CXX_FLAGS="-Wno-unused-function -Wno-unused-variable -Wno-return-type" \
	$(builddir)/EMP/emp-m2pc/CMakeLists.txt
	@cd $(builddir)/EMP/emp-m2pc/ && $(MAKE)
	@touch compile-emp-m2pc
endif


compile-blake:
	@echo "Compiling the BLAKE2 library"
	@mkdir -p $(builddir)/BLAKE2/
	@cp -r lib/BLAKE2/sse/. $(builddir)/BLAKE2
	@$(MAKE) -C $(builddir)/BLAKE2
	@$(MAKE) -C $(builddir)/BLAKE2 BUILDDIR=$(builddir)  install
	@touch compile-blake

compile-openssl:
	@mkdir -p $(CURDIR)/install/lib
	@mkdir -p $(CURDIR)/install/include
	@mkdir -p $(builddir)/
	echo "Compiling the openssl library"
	@cp -r lib/openssl/ $(builddir)/openssl
	export CFLAGS="-fPIC"	
	cd $(builddir)/openssl/; ./config --prefix=$(builddir)/openssl/tmptrgt -no-shared
	cd $(builddir)/openssl/; make 
	cd $(builddir)/openssl/; make install
	@cp $(builddir)/openssl/tmptrgt/lib/*.a $(CURDIR)/install/lib/
	@cp -r $(builddir)/openssl/tmptrgt/include/openssl/ $(CURDIR)/install/include/
	@touch compile-openssl

compile-boost:
	@mkdir -p $(CURDIR)/install/lib
	@mkdir -p $(CURDIR)/install/include
	@mkdir -p $(builddir)/
	echo "Compiling the boost library"
	@cp -r lib/boost_1_64_0/ $(builddir)/boost_1_64_0
	@cd $(builddir)/boost_1_64_0/; bash -c "BOOST_BUILD_PATH='./' ./bootstrap.sh --with-libraries=thread,system,log,serialization \
	&& ./b2 -j4"; # compile boost faster with threads
	@cp $(builddir)/boost_1_64_0/stage/lib/*.a $(CURDIR)/install/lib/
	@cp -r $(builddir)/boost_1_64_0/boost/ $(CURDIR)/install/include/
	@touch compile-boost

compile-json:
	@echo "Compiling JSON library..."
	@cp -r lib/JsonCpp $(builddir)/JsonCpp
	@cmake $(builddir)/JsonCpp/CMakeLists.txt
	@$(MAKE) -C $(builddir)/JsonCpp/
	@cp $(builddir)/JsonCpp/src/lib_json/libjsoncpp.a $(CURDIR)/install/lib/
	@touch compile-json

# Support only in c++14
compile-libote:compile-boost
	@echo "Compiling libOTe library..."
	@cp -r lib/libOTe $(builddir)/libOTe
	@mkdir -p $(builddir)/libOTe/cryptoTools/thirdparty/linux/miracl/
	@mv $(builddir)/libOTe/cryptoTools/thirdparty/linux/miracl2/* $(builddir)/libOTe/cryptoTools/thirdparty/linux/miracl/
	@cmake $(builddir)/libOTe/CMakeLists.txt -DCMAKE_BUILD_TYPE=Release
	@$(MAKE) -C $(builddir)/libOTe/
	@cp $(builddir)/libOTe/lib/*.a $(CURDIR)/install/lib/
	@cp $(builddir)/libOTe/cryptoTools/thirdparty/linux/miracl/miracl/source/libmiracl.a $(CURDIR)/install/lib/
	@mv $(CURDIR)/install/lib/liblibOTe.a $(CURDIR)/install/lib/libOTe.a
	@mkdir -p $(CURDIR)/install/include/libOTe
	@cd $(builddir)/libOTe/ && find . -name "*.h" -type f |xargs -I {} cp --parents {} $(CURDIR)/install/include/libOTe
	@cp -r $(builddir)/libOTe/cryptoTools/cryptoTools/gsl $(CURDIR)/install/include/libOTe/cryptoTools/cryptoTools
	@touch compile-libote

compile-ntl:
	echo "Compiling the NTL library..."
	mkdir -p $(builddir)/NTL
	cp -r lib/NTL/unix/. $(builddir)/NTL
	chmod 777 $(builddir)/NTL/src/configure
	cd $(builddir)/NTL/src/ && ./configure CXX=$(CXX) WIZARD=off
	$(MAKE) -C $(builddir)/NTL/src/
	$(MAKE) -C $(builddir)/NTL/src/ PREFIX=$(prefix) install
	touch compile-ntl

compile-otextension-bristol: 
	@echo "Compiling the OtExtension malicious Bristol library..."
	@cp -r lib/OTExtensionBristol $(builddir)/OTExtensionBristol
	@$(MAKE) -C $(builddir)/OTExtensionBristol CXX=$(CXX)
	@$(MAKE) -C $(builddir)/OTExtensionBristol CXX=$(CXX) install
	@touch compile-otextension-bristol

clean-otextension-bristol:
	@echo "Cleaning the otextension malicious bristol build dir..."
	@rm -rf $(builddir)/OTExtensionBristol
	@rm -f compile-otextension-bristol

clean-ntl:
	echo "Cleaning the ntl build dir..."
	rm -rf $(builddir)/NTL
	rm -f compile-ntl

clean-blake:
	@echo "Cleaning blake library"
	@rm -rf $(builddir)/BLAKE2
	@rm -f compile-blake

clean-emp:
	@echo "Cleaning EMP library"
	@rm -rf $(builddir)/EMP
	@rm -f prepare-emp compile-emp-tool compile-emp-ot compile-emp-m2pc

clean-cpp:
	@echo "cleaning .obj files"
	@rm -rf $(OUT_DIR)
	@echo "cleaning lib"
	@rm -f $(SLib)

clean-install:
	@rm -rf install/*

clean-tests:
	@rm -f test/tests.exe

clean-boost:
	@echo "Cleaning boost library"
	@rm -rf $(builddir)/boost_1_64_0
	@rm -f compile-boost
	
clean-openssl:
	@echo "Cleaning openssl library"
	@rm -rf $(builddir)/openssl
	@rm -f compile-openssl

clean-json:
	@echo "Cleaning JSON library"
	@rm -rf $(builddir)/JsonCpp/
	@rm -f compile-json

clean-libote:
	@echo "Cleaning libOTe library"
	@rm -rf $(builddir)/libOTe/
	@rm -f compile-libote

clean: clean-json clean-libote clean-openssl clean-boost clean-emp clean-otextension-bristol clean-ntl clean-install clean-tests

