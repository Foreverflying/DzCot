
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CXX     = $(CROSS_COMPILE)g++
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

ARCH = $(shell $(CC) -dumpmachine | tail -n 1 | awk -F '-' '{print $$1}')
OS = $(shell $(CC) -dumpmachine | tail -n 1 | awk -F '-' '{print $$3}')

ifeq ($(OS), linux)
    OS_STR = linux
else
    $(error this Makefile only support linux)
endif

ifeq ($(ARCH), x86_64)
    ARCH_STR = x64
else
    ifeq ($(ARCH), i686)
        ARCH_STR = x86
    else
        $(error this Makefile does not support arch $(ARCH))
    endif
endif

ifdef DEBUG
    CFLAGS += -g3 -D_DEBUG
else
    CFLAGS += -O2
endif

PROJECT = DzCot
SRC_DIRS = . ./thirdparty ./$(OS_STR) ./$(OS_STR)/$(ARCH_STR)
TMP_DIR = ../../buildtmp/$(OS)-$(ARCH)/$(PROJECT)
OUT_DIR = ../../output/$(OS)-$(ARCH)
INC_DIR =
LIB_DIR = -L$(OUT_DIR)
LIBS = -lpthread
STATIC_LIBS =
SHARED_LIBS =
CFLAGS += -fPIC

SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
ASMS = $(notdir $(wildcard $(addsuffix /*.s,$(SRC_DIRS))))
CFLAGS += $(INC_DIR)
LDFLAGS += $(LIB_DIR) $(LIBS) -Wl,-Bstatic $(STATIC_LIBS) -Wl,-Bdynamic $(SHARED_LIBS)
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o) $(ASMS:.s=.o)
SHARE_LIB = $(OUT_DIR)/lib$(PROJECT).so
STATIC_LIB = $(OUT_DIR)/lib$(PROJECT).a

vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)

$(shell mkdir -p $(TMP_DIR) $(OUT_DIR))

all: $(OBJS)
	$(MAKE) $(SHARE_LIB) $(STATIC_LIB)

clean:
	rm -f $(SHARE_LIB) $(STATIC_LIB); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

%.d: %.c
	@set -e; \
	$(CC) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

sinclude $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.s
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

$(SHARE_LIB): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $^
