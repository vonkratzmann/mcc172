CC=gcc
#  -I. means search for libraries in the dir, in this case '.' 
CFLAGS=  -Wall -I.
LIBS =  libdaqhats.so.1.3.0.5 
DEPS=scantofile.h utils.h daqhats_utils.h mcc172.h 
OBJS= scantofile.o utils.o 
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

scantofile: $(OBJS)
	$(CC) -o scantofile $(OBJS) $(LIBS)

clean:
	\rm *.o

