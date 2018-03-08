
CPP_OPTIONS   := -Wall -O3 -std=c99 -fPIC

all:: 
	gcc  blake2b.c -c $(CPP_OPTIONS) -o blake2b.o
	gcc  blake2bp.c -c $(CPP_OPTIONS) -o blake2bp.o
	ar rcs libblake2.a blake2b.o blake2bp.o
	ranlib libblake2.a

install::
	install -d $(libdir)
	install -d $(prefix)/include/BLAKE2/sse
	install -m 0644 libblake2.a $(libdir)
	install -m 0644 $(BUILDDIR)/BLAKE2/*.h $(prefix)/include/BLAKE2/sse


clean::
	rm -f libblake2.a
	rm -f ./*.o
