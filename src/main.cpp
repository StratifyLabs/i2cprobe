
#include <stdio.h>
#include <stfy/hal.hpp>
#include <stfy/var.hpp>
#include <stfy/sys.hpp>

static void probe_bus(int port, int bitrate);
static void read_bus(int port, int addr, int start, int nbyte);

static void show_buffer(const char * buffer, int nbyte);
static void show_usage(){}

int main(int argc, char * argv[]){
	//parse arguments
	String option;
	String arg;
	int i;

	int port = 0;
	int bitrate = 100000;

	for(i=1; i < argc; i++){
		option = argv[i];
		if( (option == "-p") || (option == "-port") ){
			if( argc > i+1 ){
				arg = argv[i+1];
				port = arg.atoi();
			} else {
				show_usage();
				exit(0);
			}
		} else if( (option == "-b") || (option == "-bitrate") ){
			if( argc > i+1 ){
				arg = argv[i+1];
				bitrate = arg.atoi();
			} else {
				show_usage();
				exit(0);
			}
		}
	}


	probe_bus(port, bitrate);

	return 0;
}


void probe_bus(int port, int bitrate){
	I2C i2c(port);
	int i;
	char c;
	i2c.init(100000);
	for(i=0; i < 127; i++){
		if( i % 16 == 0 ){
			printf("0x%02X:", i);
		}
		if( i != 0 ){
			i2c.setup(i, I2C::SETUP_WRITE_PTR_TRANSFER);
			if( i2c.read(0, &c, 1) == 1 ){
				printf("0x%02X ", i);
			} else {
				printf("____ ");
			}
		} else {
			printf("____ ");
		}
		if( i % 16 == 15 ){
			printf("\n");
		}
	}

	i2c.close();
}

static void show_buffer(const char * buffer, int nbyte){
	int i;
	for(i=0; i < nbyte; i++){
		if( i % 16 == 0 ){
			printf("0x%02X:", i);
		}
		printf("0x%02X ", buffer[i]);
		if( i % 16 == 15 ){
			printf("\n");
		}
	}
	printf("\n");
}

void read_bus(int port, int addr, int start, int nbyte){
	I2C i2c(port);
	int ret;

	if( i2c.init(100000) < 0 ){
		perror("Failed to init I2C bus");
		exit(1);
	}

	i2c.setup(addr);

	char buffer[nbyte];
	memset(buffer, 0, nbyte);

	ret = i2c.read(start, buffer, nbyte);
	if( ret > 0 ){
		show_buffer(buffer, ret);
	} else {
		printf("Failed to read 0x%X (%d)\n", addr, i2c.err());
	}

	i2c.close();
}
