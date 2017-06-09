
#include <stdio.h>
#include <stfy/hal.hpp>
#include <stfy/var.hpp>
#include <stfy/sys.hpp>


#define VERSION "0.2"
#define PUBLISHER "Stratify Labs, Inc (C) 2016"

static void show_usage(const char * name);

typedef struct {
	int port;
	int bitrate;
	int pinassign;
	bool pu;
	int action;
	int slave_addr;
	int offset;
	int value;
	int nbytes;
	bool pointer_16;
} options_t;

static void probe_bus(const options_t & options);
static void read_bus(const options_t & options);
static void write_bus(const options_t & options);

static void i2c_open(I2C & i2c, const options_t & options);

static void parse_options(const Cli & cli, options_t & options);

enum {
	ACTION_PROBE,
	ACTION_READ,
	ACTION_WRITE,
	ACTION_TOTAL
};

int main(int argc, char * argv[]){

	Cli cli(argc, argv);

	cli.set_version(VERSION);
	cli.set_publisher(PUBLISHER);
	options_t options;

	if( cli.is_option("--help") ){
		show_usage(argv[0]);
		exit(0);
	}

	parse_options(cli, options);


	printf("I2C Port:%d Bitrate:%dbps PU:%d Pinassign:%d\n",
			options.port,
			options.bitrate,
			options.pu,
			options.pinassign);


	switch(options.action){
	case ACTION_PROBE:
		printf("Probe:\n");
		probe_bus(options);
		break;
	case ACTION_READ:
		printf("Read: %d bytes from 0x%X at %d\n", options.nbytes, options.slave_addr, options.offset);
		read_bus(options);
		break;
	case ACTION_WRITE:
		printf("Write: %d to %d on 0x%X\n", options.value, options.offset, options.slave_addr);
		write_bus(options);
		break;
	}

	return 0;
}

void i2c_open(I2C & i2c, const options_t & options){
	int flags = I2C::MASTER;

	if( i2c.open(I2C::RDWR) < 0 ){
		perror("Failed to open I2C port");
		exit(1);
	}

	if( options.pu ){
		flags |= I2C::PULLUP;
	}

	if( i2c.set_attr(options.bitrate, options.pinassign,flags) < 0 ){
		i2c.close();
		perror("Failed to set I2C attributes");
		exit(1);
	}
}


void probe_bus(const options_t & options){
	I2C i2c(options.port);
	int i;
	char c;

	i2c_open(i2c, options);

	for(i=0; i <= 127; i++){
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

void read_bus(const options_t & options){
	I2C i2c(options.port);
	int ret;
	int i;
	char buffer[options.nbytes];
	memset(buffer, 0, options.nbytes);

	i2c_open(i2c, options);
	i2c.setup(options.slave_addr);

	ret = i2c.read(options.offset, buffer, options.nbytes);
	if( ret > 0 ){
		for(i=0; i < ret; i++){
			printf("Reg[%03d or 0x%02X] = %03d or 0x%02X\n",
					i + options.offset, i + options.offset,
					buffer[i], buffer[i]);
		}
	} else {
		printf("Failed to read 0x%X (%d)\n", options.slave_addr, i2c.err());
	}

	i2c.close();
}

void write_bus(const options_t & options){
	I2C i2c(options.port);
	int ret;

	i2c_open(i2c, options);
	i2c.setup(options.slave_addr);

	ret = i2c.write(options.offset, &options.value, 1);
	if( ret < 0 ){
		printf("Failed to write 0x%X (%d)\n", options.slave_addr, i2c.err());
	}

	i2c.close();
}

void parse_options(const Cli & cli, options_t & options){
	if( cli.is_option("--version") ){
		cli.print_version();
		exit(0);
	}

	options.port = 0;
	options.bitrate = 100000;
	options.pinassign = 0;
	options.pu = false;
	options.action = ACTION_PROBE;
	options.slave_addr = 0;
	options.offset = 0;
	options.value = 0;
	options.pointer_16 = false;

	if( cli.is_option("-p") ){
		options.port = cli.get_option_value("-p");
	} else if( cli.is_option("--port") ){
		options.port = cli.get_option_value("--port");
	}

	if( cli.is_option("-b") ){
		options.bitrate = cli.get_option_value("-b");
	} else if( cli.is_option("--bitrate") ){
		options.bitrate = cli.get_option_value("--bitrate");
	}

	if( cli.is_option("-pa") ){
		options.pinassign = cli.get_option_value("-pa");
	} else if( cli.is_option("--pinassign") ){
		options.pinassign = cli.get_option_value("--pinassign");
	}

	if( cli.is_option("-o") ){
		options.offset = cli.get_option_value("-o");
	} else if( cli.is_option("--offset") ){
		options.offset = cli.get_option_value("--offset");
	}

	if( cli.is_option("-a") ){
		options.slave_addr = cli.get_option_value("-a");
	} else if( cli.is_option("--addr") ){
		options.slave_addr = cli.get_option_value("--addr");
	}

	if( cli.is_option("-n") ){
		options.nbytes = cli.get_option_value("-n");
	} else if( cli.is_option("--nbytes") ){
		options.nbytes = cli.get_option_value("--nbytes");
	}

	if( cli.is_option("-v") ){
		options.value = cli.get_option_value("-v");
	} else if( cli.is_option("--value") ){
		options.value = cli.get_option_value("--value");
	}

	if( options.nbytes > 1024 ){
		options.nbytes = 1024;
	}

	if( cli.is_option("-pu") || cli.is_option("--pullup") ){
		options.pu = true;
	}

	if( cli.is_option("-p16") || cli.is_option("--pointer16") ){
		options.pointer_16 = true;
	}

	if( cli.is_option("-r") || cli.is_option("--read") ){
		options.action = ACTION_READ;
	}

	if( cli.is_option("-w") || cli.is_option("--write") ){
		options.action = ACTION_WRITE;
	}
}

void show_usage(const char * name){
	printf("%s version: %s by %s\n", name, VERSION, PUBLISHER);
	printf("usage: %s [-p port] [-b bitrate] [-pa pinassign]\n", name);
	printf("\t-p port: I2C port number (default is 0, ie: /dev/i2c0)\n");
	printf("\t-b bitrate: I2C bitrate (default is 100K)\n");
	printf("\t-pa pinassign: I2C pin assignment (default is zero)\n");
}
