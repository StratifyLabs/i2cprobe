
#include <stdio.h>
#include <stfy/hal.hpp>
#include <stfy/var.hpp>
#include <stfy/sys.hpp>


#define VERSION "0.1"
#define PUBLISHER "Stratify Labs, Inc (C) 2016"

static void probe_bus(int port, int bitrate, int pinassign, bool pu);
static void read_bus(int port, int addr, int start, int nbyte);

static void show_buffer(const char * buffer, int nbyte);
static void show_usage(const char * name);
static void show_version(const char * name);

int main(int argc, char * argv[]){
	//parse arguments
	String option;
	String arg;
	int i;

	int port = 0;
	int bitrate = 100000;
	int pinassign = 0;
	bool pu = false;

	for(i=1; i < argc; i++){
		option = argv[i];
		if( (option == "-p") || (option == "-port") ){
			if( argc > i+1 ){
				arg = argv[i+1];
				port = arg.atoi();
			} else {
				show_usage(argv[0]);
				exit(0);
			}
		} else if( (option == "-b") || (option == "-bitrate") ){
			if( argc > i+1 ){
				arg = argv[i+1];
				bitrate = arg.atoi();
			} else {
				show_usage(argv[0]);
				exit(0);
			}
		} else if( (option == "-pa") || (option =="-pinassign") ){
			if( argc > i+1 ){
				arg = argv[i+1];
				pinassign = arg.atoi();
			} else {
				show_usage(argv[0]);
				exit(0);
			}

		} else if( (option == "-pu")  ){
			pu = true;
		} else if( option == "--help" ){
			show_usage(argv[0]);
			exit(0);
		} else if( option == "--version" ){
			show_version(argv[0]);
			exit(0);
		}
	}

	printf("Probe I2C Port:%d At:%dbps PU:%d\n", port, bitrate, pu);

	probe_bus(port, bitrate, pinassign, pu);

	return 0;
}


void probe_bus(int port, int bitrate, int pinassign, bool pu){
	I2C i2c(port);
	int i;
	char c;
	int flags = I2C::MASTER;

	if( i2c.open(I2C::RDWR) < 0 ){
		perror("Failed to open I2C port");
		exit(1);
	}

	if( pu ){
		flags |= I2C::PULLUP;
	}

	if( i2c.set_attr(bitrate, pinassign,flags) < 0 ){
		perror("Failed to set I2C attributes");
		exit(1);
	}

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

	printf("\n");

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

void show_version(const char * name){
	printf("%s version: %s by %s\n", name, VERSION, PUBLISHER);
}

void show_usage(const char * name){
	printf("%s version: %s by %s\n", name, VERSION, PUBLISHER);
	printf("usage: %s [-p port] [-b bitrate] [-pa pinassign]\n", name);
	printf("\t-p port: I2C port number (default is 0, ie: /dev/i2c0)\n");
	printf("\t-b bitrate: I2C bitrate (default is 100K)\n");
	printf("\t-pa pinassign: I2C pin assignment (default is zero)\n");
}
