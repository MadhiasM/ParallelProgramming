
CPP = g++ -std=c++11 -fopenmp
LIBS= -lm 

default: EA3_1.bin EA3_2.bin  EA3_3.bin EA3_4.bin

%.bin: %.o $(OBJ) $(HEADER)
	$(CPP)   $< -o $@ $(OBJ) $(LIBS) 

%.o: %.c
	$(CPP) -c  $< -o $@

clean:
	rm -rf *.bin
