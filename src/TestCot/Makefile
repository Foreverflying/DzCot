
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

SRC_DIRS = .
INC_DIR = -I../googletest/googletest/include -I../../include
LIB_DIR = -L$(OUT_DIR)
LIBS = -lDzCot -lgtest -lpthread
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

vpath %.h $(SRC_DIRS)
vpath %.hpp $(SRC_DIRS)
vpath %.hxx $(SRC_DIRS)
vpath %.hh $(SRC_DIRS)
vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
vpath %.cxx $(SRC_DIRS)
vpath %.cc $(SRC_DIRS)

$(shell mkdir -p $(TMP_DIR) $(OUT_DIR))

all: $(TARGET)

clean:
	rm -f $(TARGET); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

$(TMP_DIR)/%.d: %.c
	@set -e; \
	$(CC) $(CFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#$(TMP_DIR)\/\1.o $@ :#g' > $@; \
	[ -s $@ ] || rm -f $@

$(TMP_DIR)/%.d: %.cpp
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#$(TMP_DIR)\/\1.o $@ :#g' > $@; \
	[ -s $@ ] || rm -f $@

$(TMP_DIR)/%.d: %.cxx
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#$(TMP_DIR)\/\1.o $@ :#g' > $@; \
	[ -s $@ ] || rm -f $@

$(TMP_DIR)/%.d: %.cc
	@set -e; \
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's#\($*\)\.o[ :]*#$(TMP_DIR)\/\1.o $@ :#g' > $@; \
	[ -s $@ ] || rm -f $@

$(TMP_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TMP_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(TMP_DIR)/%.o: %.cxx
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(TMP_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -c -o $@ $<

$(TARGET): $(addprefix $(TMP_DIR)/, $(OBJS))
	$(CXX) -o $@ $^ $(LDFLAGS)

-include $(addprefix $(TMP_DIR)/, $(DEPS))
