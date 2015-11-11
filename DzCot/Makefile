
OS = $(shell uname)
ifeq ($(OS), Linux)
    OS_STR = linux
else
    error( 'this Makefile only support linux' )
endif

ifndef ARCH
	ARCH = $(shell uname -m)
endif

ifeq ($(ARCH), x86_64)
	ARCH_STR = x64
else
	ifeq ($(ARCH), i686)
		ARCH_STR = x32
	else
		$error( 'this makefile only support x86_64 or i686' )
	endif
endif

ifdef DEBUG
	CFLAGS += -g3 -D_DEBUG
else
	CFLAGS += -O2
endif

PROJECT = libDzCot
LIBS = -lpthread
CFLAGS += -fPIC
TMP_DIR = ../buildtmp/$(ARCH)/$(PROJECT)
OUT_DIR = ../output/$(ARCH)
SRC_DIRS = . ./thirdparty ./$(OS_STR) ./$(OS_STR)/$(ARCH_STR)

SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
ASMS = $(notdir $(wildcard $(addsuffix /*.s,$(SRC_DIRS))))
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o) $(ASMS:.s=.o)
SHARE_LIB = $(OUT_DIR)/$(PROJECT).so
STATIC_LIB = $(OUT_DIR)/$(PROJECT).a

vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)


all: $(OUT_DIR) $(OBJS)
	$(MAKE) $(SHARE_LIB) $(STATIC_LIB)

clean:
	rm -f $(SHARE_LIB) $(STATIC_LIB); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

%.d: %.c $(TMP_DIR)
	@set -e; \
	$(CC) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

sinclude $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.s
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

$(SHARE_LIB): $(OBJS)
	$(CC) $(LIBS) -shared -o $@ $^

$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $^

$(TMP_DIR):
	@mkdir -p $(TMP_DIR)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)
