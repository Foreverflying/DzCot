
ARCH = $(shell uname -m)
OS = $(shell uname)

ifeq ($(OS), Linux)
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
TMP_DIR = ../buildtmp/$(ARCH)/$(PROJECT)
OUT_DIR = ../output/$(ARCH)
INC_DIR =
LIB_DIR = -L$(OUT_DIR)
LIBS = -lpthread
CFLAGS += -fPIC

SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
ASMS = $(notdir $(wildcard $(addsuffix /*.s,$(SRC_DIRS))))
CFLAGS += $(INC_DIR)
LDFLAGS += $(LIB_DIR) $(LIBS)
DEPS = $(SRCS:.c=.d)
OBJS = $(SRCS:.c=.o) $(ASMS:.s=.o)
SHARE_LIB = $(OUT_DIR)/lib$(PROJECT).so
STATIC_LIB = $(OUT_DIR)/lib$(PROJECT).a

vpath %.c $(SRC_DIRS)
vpath %.s $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)

$(shell mkdir -p $(TMP_DIR) $(OUR_DIR))

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
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $^
