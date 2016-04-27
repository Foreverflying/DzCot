
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
OS = $(shell uname -s | tr '[A-Z]' '[a-z]')

ifdef DEBUG
    CFLAGS += -g3 -D_DEBUG
else
    CFLAGS += -O2
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

PROJECT = TryCot
SRC_DIRS = .
TMP_DIR = ../../buildtmp/$(OS)-$(ARCH)/$(PROJECT)
OUT_DIR = ../../output/$(OS)-$(ARCH)
INC_DIR = -I../../include -I../DzCot/$(OS) -I../DzCot/$(OS)/$(ARCH_STR)
LIB_DIR = -L$(OUT_DIR)
LIBS = -lDzCot
STATIC_LIBS =
SHARED_LIBS =
CPPFLAGS += -std=c++0x
LDFLAGS += -Wl,-rpath=\$$ORIGIN

C_SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
CPP_SRCS = $(notdir $(wildcard $(addsuffix /*.cpp,$(SRC_DIRS))))
CXX_SRCS = $(notdir $(wildcard $(addsuffix /*.cxx,$(SRC_DIRS))))
CC_SRCS = $(notdir $(wildcard $(addsuffix /*.cc,$(SRC_DIRS))))
CFLAGS += $(INC_DIR)
CPPFLAGS += $(CFLAGS)
LDFLAGS += $(LIB_DIR) $(LIBS) -Wl,-Bstatic $(STATIC_LIBS) -Wl,-Bdynamic $(SHARED_LIBS)
DEPS = $(CPP_SRCS:.cpp=.d) $(CXX_SRCS:.cxx=.d) $(CC_SRCS:.cc=.d) $(C_SRCS:.c=.d)
OBJS = $(DEPS:.d=.o)
TARGET = $(OUT_DIR)/$(PROJECT)

vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
vpath %.cxx $(SRC_DIRS)
vpath %.cc $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)

$(shell mkdir -p $(TMP_DIR) $(OUT_DIR))

all: $(OBJS)
	$(MAKE) $(TARGET)

clean:
	rm -f $(TARGET); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

%.d: %.c
	@set -e; \
	$(CC) $(CFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

%.d: %.cpp
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

%.d: %.cxx
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

%.d: %.cc
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

sinclude $(DEPS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.cxx
	$(CXX) $(CPPFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.cc
	$(CXX) $(CPPFLAGS) -c -o $(TMP_DIR)/$@ $^

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
