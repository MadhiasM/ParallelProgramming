CC := mpicc
CFLAGS = -O3 -lm  -std=c99 
LIBS = -lm

ALL: EA4_1 EA4_3 

EA4_1: EA4_1.cpp
	$(CC) $(CFLAGS) $< $(LIBS) -o $@

EA4_3: EA4_3.cpp
	$(CC) $(CFLAGS) $< $(LIBS) -o $@
