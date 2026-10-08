# make CROSS_COMPILE=aarch64-none-linux-gnu-

# if statement checking if cross_compile specified
# Runs either local host compile or aarch


# If CC is defined then run cross compile
# Default set as gcc
ifneq ($(CC), gcc)
default: crosscompile
else
default: 
	$(CC) aesdsocket.c -o aesdsocket
endif 

all: default

# Removes the aesdsocket file
clean:
	rm -f aesdsocket

# Compiles writer for aarch machine. Changed to get executable for assignment 3
crosscompile:
	$(CC) aesdsocket.c -o aesdsocket
