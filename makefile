#NOTE : for 64 bit OS : Use -m64 in compile and load flags
CC = g++
CFLAGS = -c -Wall -g -m32
LDFLAGS = -Wl,-Bstatic -lgmp -Wl,-Bdynamic -lcryptopp -lpthread -m32
FIND = find

find : $(FIND)

$(FIND) : ispresent.o fully_homomorphic.o utilities.o circuit.o security_settings.o
  $(CC) -o $@ ispresent.o fully_homomorphic.o utilities.o circuit.o security_settings.o $(LDFLAGS)

ispresent.o : ispresent.cpp
	$(CC) $(CFLAGS) ispresent.cpp

utilities.o : utilities.c
	$(CC) $(CFLAGS) utilities.c

fully_homomorphic.o : fully_homomorphic.cpp fully_homomorphic.h type_defs.h
	$(CC) $(CFLAGS) fully_homomorphic.cpp -lgmp -lcryptopp -lpthreads

circuit.o : circuit.cpp
	$(CC) $(CFLAGS) circuit.cpp

security_settings.o : security_settings.cpp
	$(CC) $(CFLAGS) security_settings.cpp

clean :
	rm -rf *.o find
