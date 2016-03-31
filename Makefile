
all: lib
	cd TestCot; $(MAKE) all
	cd TryCot; $(MAKE) all

clean:
	cd DzCot; $(MAKE) clean
	cd TestCot; $(MAKE) clean
	cd TryCot; $(MAKE) clean

lib:
	cd DzCot; $(MAKE) all

.PHONY: all clean lib
