# make CROSS_COMPILE=aarch64-none-linux-gnu-

# if statement checking if cross_compile specified
# Runs either local host compile or aarch
ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
default: crosscompile
else
default: 
	gcc writer.c -o writer
endif

# Removes the writer executable and object files
clean:
	rm -f writer
	rm -f *.o

# Compiles writer for aarch machine. Changed to get executable for assignment 3
crosscompile:
	# $(CROSS_COMPILE)gcc -g -Wall -c -o writer.o writer.c
	$(CROSS_COMPILE)gcc writer.c -o writer
	file writer
	file writer > ../assignments/assignment2/fileresult.txt