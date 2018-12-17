
#include <stdarg.h>
#include <stdio.h>
#include <sapi/hal.hpp>
#include <sapi/var.hpp>
#include <sapi/sys.hpp>
#include <sapi/fmt.hpp>

#define PUBLISHER "Stratify Labs, Inc (C) 2018"

static void show_usage(const Cli & cli);

typedef struct {
	i2c_attr_t attr;
	u8 port;
	u8 slave_addr;
	int action;
	int offset;
	int value;
	int nbytes;
	bool pointer_16;
	bool is_map;
} options_t;

static void scan_bus(const options_t & options);
static void scan_bus_message(const options_t & options);
static void read_bus(const options_t & options);
static void write_bus(const options_t & options);

Messenger * m_current_messenger;

static void i2c_open(I2C & i2c, const options_t & options);
static bool parse_options(const Cli & cli, options_t & options);

enum {
	ACTION_SCAN,
	ACTION_READ,
	ACTION_WRITE,
	ACTION_TOTAL
};

int main(int argc, char * argv[]){
	Cli cli(argc, argv);
	cli.set_publisher(PUBLISHER);
	I2CAttr i2c_attr;
	Messenger messenger(1024);
	int ret;

	options_t options;

	if( cli.is_option("-w") == false ){ //-w needs the -v option for the value
		cli.handle_version();
	}

	if( cli.is_option("--help") || cli.is_option("-h") ){
		show_usage(cli);
	}

	if( cli.is_option("-message") ){
		mcu_pin_t channels;
		channels = cli.get_option_pin("-message");
		if( (ret = messenger.start("/dev/fifo", channels.port, channels.pin)) < 0 ){
			printf("Failed to start messenger %d\n", ret);
			exit(1);
		}
		m_current_messenger = &messenger;
		messenger.set_timeout(250);
	} else {
		m_current_messenger = 0;
	}

	memset(&options, 0, sizeof(options));
	if( parse_options(cli, options) ){

		if( m_current_messenger == 0 ){
			printf("I2C Port:%d Bitrate:%ldbps PU:%d",
					options.port,
					options.attr.freq,
					(options.attr.o_flags & I2C::FLAG_IS_PULLUP) != 0);

			if( options.attr.pin_assignment.sda.port != 0xff ){
				printf(" sda:%d.%d scl:%d.%d\n",
						options.attr.pin_assignment.sda.port,
						options.attr.pin_assignment.sda.pin,
						options.attr.pin_assignment.scl.port,
						options.attr.pin_assignment.scl.pin
				);
			} else {
				printf(" default pin assignment\n");
			}

		}

		switch(options.action){
		case ACTION_SCAN:
			if( m_current_messenger ){
				scan_bus_message(options);
			} else {
				printf("Probe:\n");
				scan_bus(options);
			}
			break;
		case ACTION_READ:
			printf("Read: %d bytes from 0x%X at %d\n", options.nbytes, options.slave_addr, options.offset);
			read_bus(options);
			break;
		case ACTION_WRITE:
			printf("Write: %d to %d on 0x%X\n", options.value, options.offset, options.slave_addr);
			write_bus(options);
			break;
		default:
			show_usage(cli);
			break;
		}

	} else {
		show_usage(cli);
	}

	return 0;
}

void i2c_open(I2C & i2c, const options_t & options){
	if( i2c.open(I2C::RDWR) < 0 ){
		perror("Failed to open I2C port");
		exit(1);
	}

	if( i2c.set_attributes(options.attr) < 0 ){
		i2c.close();
		perror("Failed to set I2C attributes");
		exit(1);
	}
}


void scan_bus(const options_t & options){
	I2C i2c(options.port);
	int i;
	char c;

	i2c_open(i2c, options);

	for(i=0; i <= 127; i++){
		if( i % 16 == 0 ){
			printf("0x%02X:", i);
		}
		if( i != 0 ){
			i2c.prepare(i, I2C::FLAG_PREPARE_DATA);
			if( i2c.read(&c, 1) == 1 ){
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

void scan_bus_message(const options_t & options){
	I2C i2c(options.port);
	int i;
	char c;
	bool is_active;
	Son message;
	char message_buffer[256];
	Data message_data(message_buffer, 256);
	String address;

	i2c_open(i2c, options);

	for(i=0; i <= 127; i++){

		address.sprintf("0x%X", i);

		if( i != 0 ){
			i2c.prepare(i, I2C::FLAG_PREPARE_DATA);
			if( i2c.read(&c, 1) == 1 ){
				is_active = true;
			} else {
				is_active = false;
			}
		} else {
			is_active = false;
		}

		message.get_error();
		message.create_message(message_data);
		message.open_object("");
		message.write("type", "address");
		message.write("address", address);
		message.write("value", is_active);
		message.close_object();
		message.close();

		message.open_message(message_data);
		errno = 0;
		if( m_current_messenger->send_message(message) < 0 ){
			printf("failed to send message %d %d\n", message.get_error(), errno);
			exit(1);
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
	i2c.prepare(options.slave_addr);

	printf("Read 0x%X %d %d\n", options.slave_addr, options.offset, options.nbytes);
	ret = i2c.read(options.offset, buffer, options.nbytes);
	if( ret > 0 ){
		for(i=0; i < ret; i++){

			if( options.is_map ){
				printf("{ 0x%02X, 0x%02X },\n", i + options.offset, buffer[i]);
			} else {
				printf("Reg[%03d or 0x%02X] = %03d or 0x%02X\n",
						i + options.offset, i + options.offset,
						buffer[i], buffer[i]);
			}
		}
	} else {
        printf("Failed to read 0x%X (%d)\n", options.slave_addr, i2c.get_error());
	}

	i2c.close();
}

void write_bus(const options_t & options){
	I2C i2c(options.port);
	int ret;

	i2c_open(i2c, options);
	i2c.prepare(options.slave_addr);

	ret = i2c.write(options.offset, &options.value, 1);
	if( ret < 0 ){
        printf("Failed to write 0x%X (%d)\n", options.slave_addr, i2c.get_error());
	}

	i2c.close();
}

bool parse_options(const Cli & cli, options_t & options){
	I2CAttr i2c_attr;
	options.action = ACTION_TOTAL;
	options.slave_addr = 0;
	options.offset = 0;
	options.value = 0;
	options.pointer_16 = false;
	options.nbytes = 1;
	options.is_map = false;

	if( cli.handle_i2c(i2c_attr) == true ){

		options.attr = i2c_attr.attributes();
		options.port = i2c_attr.port();
		options.slave_addr = i2c_attr.slave_addr();

		if( cli.is_option("-o") ){
			options.offset = cli.get_option_value("-o");
		}

		if( cli.is_option("-p16") ){
			options.pointer_16 = true;
		}

		if( cli.is_option("-r") ){
			options.action = ACTION_READ;

			if( cli.is_option("-n") ){
				options.nbytes = cli.get_option_value("-n");
			} else {
				show_usage(cli);
			}

			if( options.nbytes > 1024 ){
				options.nbytes = 1024;
			}

		}

		if( cli.is_option("-w") ){
			options.action = ACTION_WRITE;

			if( cli.is_option("-v") ){
				options.value = cli.get_option_value("-v");
			} else {
				show_usage(cli);
			}
		}

		if( cli.is_option("-scan") ){
			options.action = ACTION_SCAN;
		}

		if( cli.is_option("-map") ){
			options.is_map = true;
		}

		return true;
	}

	return false;
}

void show_usage(const Cli & cli){
	printf("usage: %s -i2c <port> [i2c options] [action] [action options]\n", cli.name());
	printf("\nI2C Options:\n");
	printf("\t-i2c <port> -- I2C port number (eg: /dev/i2c1)\n");
	printf("\t-freq <bitrate> -- I2C bitrate (default is 100K)\n");
	printf("\t-sda <X.Y> -- SDA Pin\n");
	printf("\t-scl <X.Y> -- SCL Pin\n");
	printf("\t-pu -- Use Internal Pullup\n");

	printf("\nActions:\n");
	printf("\t-r -- Read from bus\n");
	printf("\t-w -- Write a byte to the bus\n");
	printf("\t-scan -- Scan the bus\n");

	printf("\nAction Options:\n");
	printf("\t-o <value> -- Offset to read/write\n");
	printf("\t-n <value> -- Number of bytes to read (not used for write, required for read)\n");
	printf("\t-v <value>	 -- Value to write (not used for read, required for write)\n");
	printf("\t-map -- Show output as a register map that can be copied to C source code\n");

	exit(0);
}

