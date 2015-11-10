
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

CFLAGS = -D__linux_
LIBS = -lpthread
TMP_DIR = ../buildtmp/$(OS_STR)/$(ARCH_STR)
OUT_DIR = ../output/$(OS_STR)/$(ARCH_STR)
SRC_DIRS = . ./thirdparty ./$(OS_STR) ./$(OS_STR)/$(ARCH_STR)

SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
ASMS = $(notdir $(wildcard $(addsuffix /*.asm,$(SRC_DIRS))))
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o) $(ASMS:.asm=.o)
LIB_NAME = libDzCot.a
TARGET = $(OUT_DIR)/$(LIB_NAME)

vpath %.c $(SRC_DIRS)
vpath %.asm $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)


all: $(TMP_DIR) $(OUT_DIR) $(TARGET)

test:
	@echo $(DEPS)
	@echo $(SRCS)
	@echo $(DzCot_obj)

clean:
	@rm -f $(TMP_DIR)/*.d $(TMP_DIR)/*.o

.PHONY: all clean

%.d: %.c
	@set -e; \
	$(CC) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

sinclude $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.asm
	$(AS) $(ASFLAGS) -o $(TMP_DIR)/$@ $^

$(TARGET): $(OBJS)
	$(AR) rcs $@ $^
#	$(CC) $(LIBS) -o $@ $^

$(TMP_DIR):
	mkdir -p $(TMP_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
