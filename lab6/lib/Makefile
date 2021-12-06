# makefile_lib
.PHONY: all clean
# all: put.o rand.o

# put.o: put.c
# 	$(GCC) -c -g put.c $(CFLAG)
# rand.o: rand.c
# 	$(GCC) -c -g rand.c $(CFLAG)

# clean:
# 	-rm -f *.o


ASM_SRC		= $(sort $(wildcard *.S))
C_SRC       = $(sort $(wildcard *.c))
OBJ		    = $(patsubst %.S,%.o,$(ASM_SRC)) $(patsubst %.c,%.o,$(C_SRC))

all:$(OBJ)


%.o:%.S
	${GCC}  ${CFLAG} -c $<

%.o:%.c
	${GCC}  ${CFLAG} -c $<

clean:
	$(shell rm *.o >/dev/null)
