CC = gcc
CXX = g++
CFLAGS  = -Wall -pipe -DDEBUG -D_NEW_LIC -g -D_GNU_SOURCE -shared -D_REENTRANT
LIB     = -lconfig -ldl -lrt -L../../lib -lttc -g
INCLUDE = -I../spp_inc
OO    = service.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o uin_conf.o stat.o
TARGETS = ../../lib/libRanch.so
all: $(TARGETS)
    stat:tool_stat.cpp
    $(CXX) $(INCLUDE) tool_stat.cpp -o tool_stat stat.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o -g
    cp tool_stat ../../bin
    $(TARGETS): $(OO)
    $(CXX) $(CFLAGS) $(INCLUDE) $(OO) -o $@ $(LIBDIR) $(LIB)
.c.o:
    $(CC)  $(CFLAGS) -c $(INCLUDE) $<
    echo $@
.cpp.o:
    $(CXX) $(CFLAGS) -c $(INCLUDE) $<
    echo $@
    %:%.c
    $(CC) $(CFLAGS) -o $@ $< $(OO) $(LDFLAGS)
    echo $@
clean:
    rm -f *.o
    rm -f $(TARGETS)
    rm -f tool_stat