
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)cc
CXX     = $(CROSS_COMPILE)c++
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

ARCH = $(shell $(CC) -dumpmachine | tail -n 1 | awk -F '-' '{print $$1}')
OS = $(shell uname -s | tr '[A-Z]' '[a-z]')

ifneq ($(OS), linux)
    $(error this Makefile only support linux)
endif

ifneq ($(filter $(ARCH), i386 i486 i586 i686),)
    ARCH_STR = i386
else
    ifneq ($(filter $(ARCH), x86_64 arm),)
        ARCH_STR = $(ARCH)
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
SRC_DIRS = . ./thirdparty ./$(OS) ./$(OS)/$(ARCH_STR)
TMP_DIR = ../../buildtmp/$(OS)-$(ARCH)/$(PROJECT)
OUT_DIR = ../../output/$(OS)-$(ARCH)
INC_DIR = -I./$(OS) -I./$(OS)/$(ARCH_STR)
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
LIB_SHARED = $(OUT_DIR)/lib$(PROJECT).so
LIB_STATIC = $(OUT_DIR)/lib$(PROJECT).a

vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)

$(shell mkdir -p $(TMP_DIR) $(OUT_DIR))

all: $(OBJS)
	$(MAKE) $(LIB_SHARED) $(LIB_STATIC)

clean:
	rm -f $(LIB_SHARED) $(LIB_STATIC); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

%.d: %.c
	@set -e; \
	$(CC) $(CFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

sinclude $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.s
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

$(LIB_SHARED): $(OBJS)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(LIB_STATIC): $(OBJS)
	$(AR) rcs $@ $^
