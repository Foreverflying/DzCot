
all:
	@cd DzCot; make all
	@cd TestCot; make all
	@cd TryCot; make all

clean:
	@cd DzCot; make clean
	@cd TestCot; make clean
	@cd TryCot; make clean

.PHONY: all clean
