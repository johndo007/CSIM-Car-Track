C	   = g++
LIB	 =  -I/usr/csshare/pkgs/csim_cpp-19.0/lib
CSIM	= /usr/csshare/pkgs/csim_cpp-19.0/lib/csim.cpp.a
RM	  = /bin/rm -f
CFLAG   = -DCPP -DGPP
 
INPUT = part1.cpp
TARGET = csim
  
$(TARGET): $(INPUT)
	$(C) $(CFLAG) $(LIB) $(INPUT) $(CSIM) -lm -o $(TARGET)

output:
	python test.py

check:
	cat output.txt | grep Total
	cat output.txt | grep Average
	cat output2.txt | grep Total
	cat output2.txt | grep Average
 
clean:
	$(RM) *.o *.output core *~ *# $(TARGET) output.txt output2.txt
