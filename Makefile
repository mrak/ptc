CC=clang
CFLAGS=-c -Wall -Ideps/chan
LDFLAGS=
LDLIBS=-lpthread
EXE=ptc
SRCS := $(wildcard *.c)
DEPS := $(wildcard deps/chan/*.c)
OBJS =  $(SRCS:.c=.o) $(DEPS:.c=.o)

all: $(SRCS) ptc

$(EXE): $(OBJS)

clean:
	$(RM) $(OBJS) $(EXE)
