
ifndef ARCH
	ARCH = $(shell uname -m)
endif

ifdef DEBUG
	CFLAGS += -g3 -D_DEBUG
else
	CFLAGS += -O2
endif

PROJECT = TestCot
SRC_DIRS = .
TMP_DIR = ../buildtmp/$(ARCH)/$(PROJECT)
OUT_DIR = ../output/$(ARCH)
INC_DIR = -I../include
LIB_DIR = -L$(OUT_DIR)
LIBS = -lDzCot -lgtest
CPPFLAGS += -std=c++0x
LDFLAGS += -Wl,-rpath=\$$ORIGIN

C_SRCS = $(notdir $(wildcard $(addsuffix /*.c,$(SRC_DIRS))))
CPP_SRCS = $(notdir $(wildcard $(addsuffix /*.cpp,$(SRC_DIRS))))
CXX_SRCS = $(notdir $(wildcard $(addsuffix /*.cxx,$(SRC_DIRS))))
CC_SRCS = $(notdir $(wildcard $(addsuffix /*.cc,$(SRC_DIRS))))
CFLAGS += $(INC_DIR)
CPPFLAGS += $(CFLAGS)
LDFLAGS += $(LIB_DIR) $(LIBS)
DEPS = $(CPP_SRCS:.cpp=.d) $(CXX_SRCS:.cxx=.d) $(CC_SRCS:.cc=.d) $(C_SRCS:.c=.d)
OBJS = $(DEPS:.d=.o)
TARGET = $(OUT_DIR)/$(PROJECT)

vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
vpath %.cxx $(SRC_DIRS)
vpath %.cc $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)


all: $(TMP_DIR) $(OUT_DIR) $(OBJS)
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
	$(CXX) $(LDFLAGS) -o $@ $^

$(TMP_DIR):
	@mkdir -p $(TMP_DIR)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)
