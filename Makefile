
all: lib
	cd src/TestCot; $(MAKE) all
	cd src/TryCot; $(MAKE) all

clean:
	cd src/DzCot; $(MAKE) clean
	cd src/TestCot; $(MAKE) clean
	cd src/TryCot; $(MAKE) clean

lib:
	cd src/DzCot; $(MAKE) all

.PHONY: all clean lib
