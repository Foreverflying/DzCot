
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
	CPPFLAGS += -g3 -D_DEBUG
else
	CPPFLAGS += -O2
endif

PROJECT = TryCot
LIBS = -L$(OUT_DIR) -lDzCot
TMP_DIR = ../buildtmp/$(ARCH)/$(PROJECT)
OUT_DIR = ../output/$(ARCH)
SRC_DIRS = .

CPP_SRCS = $(wildcard *.cpp)
C_SRCS = $(wildcard *.c)
CFLAGS = $(CPPFLAGS)
DEPS = $(CPP_SRCS:.cpp=.d) $(C_SRCS:.c=.d)
OBJS = $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
TARGET = $(OUT_DIR)/$(PROJECT)

vpath %.c $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)
vpath %.d $(TMP_DIR)
vpath %.o $(TMP_DIR)


all: $(OUT_DIR) $(OBJS)
	$(MAKE) $(TARGET)

clean:
	rm -f $(TARGET); cd $(TMP_DIR); rm -f $(DEPS) $(OBJS)

.PHONY: all clean

%.d: %.cpp $(TMP_DIR)
	@set -e; \
	$(CXX) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@

%.d: %.c $(TMP_DIR)
	@set -e; \
	$(CC) -MM $< | sed -e 's#\($*\)\.o[ :]*#\1.o $@ :#g' > $(TMP_DIR)/$@; \
	[ -s $(TMP_DIR)/$@ ] || rm -f $(TMP_DIR)/$@


sinclude $(DEPS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -o $(TMP_DIR)/$@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(TMP_DIR)/$@ $^

$(TARGET): $(OBJS)
	$(CXX) $(LIBS) -o $@ $^

$(TMP_DIR):
	@mkdir -p $(TMP_DIR)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)
