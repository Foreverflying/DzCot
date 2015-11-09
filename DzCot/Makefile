
OS = $(shell uname)
ifeq ($(OS), Linux)
    OS_STR = linux
else
    error( 'this Makefile only support linux' )
endif

ARCH = $(shell uname -m)
ifeq ($(ARCH), x86_64)
    ARCH_STR = x64
else
    ifeq ($(ARCH), i686)
        ARCH_STR = x32
    else
        $error( 'this makefile only support x86_64 or i686' )
    endif
endif

CFLAGS = -D__linux__
TEMP_DIR = ../buildtmp/$(OS_STR)/$(ARCH_STR)
SRC_DIRS = . ./$(OS_STR) ./$(OS_STR)/$(ARCH_STR)

SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
#SRCS = $(nodir $(SRCS))

DEPS = $(SRCS:.c=.d)

DzCot_obj = $(SRCS:.c=.o)

vpath %.c $(SRC_DIRS)
vpath %.d $(TEMP_DIR)

test:
	@echo $(DEPS)
	@echo $(SRCS)
	@echo $(DzCot_obj)

%.d: %.c
	@set -ex; \
	$(CC) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TEMP_DIR)/$@; \
	[ -s $(TEMP_DIR)/$@ ] || rm -f $(TEMP_DIR)/$@

sinclude $(DEPS)
	
all: DzCot TestCot TryCot

DzCot:
	@cd DzCot;
	make all;

TestCot:
	@cd TestCot
	make all;

TryCot:
	@cd TryCot
	make all;

clean:
	@rm -f $(SOURCE:.c=.d)
