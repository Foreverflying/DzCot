
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

ifneq ($(firstword $(MAKEFILE_LIST)), Makefile)
    $(error you need to cd to the same folder which contains this Makefile to run make command)
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

SELF_DIR = $(realpath $(dir $(realpath Makefile)))
ROOT_DIR = $(shell echo $(SELF_DIR) | sed -r 's/(.*)\/src\/.*/\1/')
PROJECT = $(notdir $(SELF_DIR))
TMP_DIR = $(ROOT_DIR)/buildtmp/$(OS)-$(ARCH)/$(PROJECT)
OUT_DIR = $(ROOT_DIR)/output/$(OS)-$(ARCH)
INSTALL_DIR = /usr/local/lib

SRC_DIRS = . ./thirdparty ./$(OS) ./$(OS)/$(ARCH_STR)
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

vpath %.h $(SRC_DIRS)
vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)

$(shell mkdir -p $(TMP_DIR) $(OUT_DIR))

all: $(LIB_SHARED) $(LIB_STATIC)

clean:
	rm -f $(LIB_SHARED) $(LIB_STATIC); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

install_shared: $(LIB_SHARED)
	mkdir -p $(INSTALL_DIR)/DzCot/ && cp $(LIB_SHARED) $(INSTALL_DIR)/DzCot/

install_static: $(LIB_STATIC)
	mkdir -p $(INSTALL_DIR)/DzCot/ && cp $(LIB_STATIC) $(INSTALL_DIR)/DzCot/

uninstall:
	rm -rf $(INSTALL_DIR)/DzCot/

.PHONY: all clean install_shared install_static uninstall

$(TMP_DIR)/%.d: %.c
	@set -e; \
	$(CC) $(CFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#$(TMP_DIR)\/\1.o $@ :#g' > $@; \
	[ -s $@ ] || rm -f $@

$(TMP_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TMP_DIR)/%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB_SHARED): $(addprefix $(TMP_DIR)/, $(OBJS))
	$(CC) -shared -o $@ $^ $(LDFLAGS)

$(LIB_STATIC): $(addprefix $(TMP_DIR)/, $(OBJS))
	$(AR) rcs $@ $^

-include $(addprefix $(TMP_DIR)/, $(DEPS))
