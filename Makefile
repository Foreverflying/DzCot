
all: lib
	cd TestCot; make all
	cd TryCot; make all

clean:
	cd DzCot; make clean
	cd TestCot; make clean
	cd TryCot; make clean

lib:
	cd DzCot; make all

.PHONY: all clean lib
