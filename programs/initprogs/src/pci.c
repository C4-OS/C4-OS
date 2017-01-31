#include <c4rt/c4rt.h>
#include <stdint.h>
#include <stdbool.h>

unsigned long display = 0;

// PCI io ports
enum {
	PCI_CONFIG_ADDRESS = 0xcf8,
	PCI_CONFIG_DATA    = 0xcfc,
};

// PCI class values
enum {
	PCI_CLASS_OLD           = 0,
	PCI_CLASS_MASS_STORAGE  = 1,
	PCI_CLASS_NETWORK       = 2,
	PCI_CLASS_DISPLAY       = 3,
	PCI_CLASS_MULTIMEDIA    = 4,
	PCI_CLASS_MEMORY        = 5,
	PCI_CLASS_BRIDGE        = 6,
	PCI_CLASS_SIM_COM       = 7,
	PCI_CLASS_BASE_PERIPH   = 8,
	PCI_CLASS_INPUT_DEV     = 9,
	PCI_CLASS_DOCK_STAT     = 0xa,
	PCI_CLASS_PROCESSOR     = 0xb,
	PCI_CLASS_SERIAL_BUS    = 0xc,
	PCI_CLASS_WIRELESS      = 0xd,
	PCI_CLASS_INTELLI_IO    = 0xe,
	PCI_CLASS_SATALLITE_COM = 0xf,
	PCI_CLASS_CRYPT         = 0x10,
	PCI_CLASS_DATAAC_SP     = 0x11,
	PCI_CLASS_RESERVED      = 0x12,
	/* Everything else between 0x11 and 0xff is reserved, and shouldn't be used. */
	PCI_CLASS_UNDEFINED     = 0xff
};

// PCI subclass values for PCI_CLASS_MASS_STORAGE
enum {
	PCI_SCLASS_MASS_SCSI   = 0,
	PCI_SCLASS_MASS_IDE    = 1,
	PCI_SCLASS_MASS_FLOPPY = 2,
	PCI_SCLASS_MASS_IPI    = 3,
	PCI_SCLASS_MASS_RAID   = 4,
	PCI_SCLASS_MASS_ATA    = 5,
	PCI_SCLASS_MASS_SATA   = 6,
	PCI_SCLASS_MASS_SAS    = 7,
	PCI_SCLASS_MASS_OTHER  = 0x80
};

// Config offsets for header types of 0x0
enum {
	PCI_CONFIG_VENDOR      = 0,
	PCI_CONFIG_DEVICE      = 2,
	PCI_CONFIG_COMMAND     = 4,
	PCI_CONFIG_STATUS      = 6,
	PCI_CONFIG_REV_ID      = 8,
	PCI_CONFIG_PROG_IF     = 9,
	PCI_CONFIG_SUBCLASS    = 0xa,
	PCI_CONFIG_CLASS       = 0xb,
	PCI_CONFIG_CACHE_SIZE  = 0xc,
	PCI_CONFIG_LAT_TIMER   = 0xd,
	PCI_CONFIG_HEAD_TYPE   = 0xe,
	PCI_CONFIG_BIST        = 0xf,
	PCI_CONFIG_BAR0        = 0x10,
	PCI_CONFIG_BAR1        = 0x14,
	PCI_CONFIG_BAR2        = 0x18,
	PCI_CONFIG_BAR3        = 0x1c,
	PCI_CONFIG_BAR4        = 0x20,
	PCI_CONFIG_BAR5        = 0x24,
	PCI_CONFIG_CARDBUS     = 0x28,
	PCI_CONFIG_SUBSYS_VEND = 0x2c,
	PCI_CONFIG_SUBSYS_ID   = 0x2e,
	PCI_CONFIG_EXPAND_ROM  = 0x30,
	PCI_CONFIG_CAPABLE     = 0x34,
	PCI_CONFIG_INT_LINE    = 0x3c,
	PCI_CONFIG_INT_PIN     = 0x3d,
	PCI_CONFIG_MIN_GRANT   = 0x3e,
	PCI_CONFIG_MAX_LAT     = 0x3f
};

typedef struct pci_device {
	uint8_t bus, slot, func;
	bool valid;
} pci_device_t;

static void putchar( char c ){
	message_t msg = {
		.type = 0xbabe,
		.data = { c },
	};

	c4_msg_send( &msg, display );
}

static void puts( const char *s ){
	for ( ; *s; s++ ){
		putchar(*s);
	}
}

static void print_num( unsigned n ){
	char buf[32];
	unsigned i = 0;

	if ( n ){
		for ( ; n; n /= 10, i++ ){
			buf[i] = n % 10 + '0';
		}

		buf[i] = '\0';
		
	} else {
		buf[i++] = '0';
		buf[i]   = '\0';
	}

	while ( i-- ){
		putchar( buf[i] );
	}
}

static void print_hex( unsigned n ){
	char buf[32];
	unsigned i = 0;

	if ( n ){
		for ( ; n; n /= 16, i++ ){
			buf[i] = "0123456789abcdef"[n % 16];
		}

		buf[i] = '\0';
		
	} else {
		buf[i++] = '0';
		buf[i]   = '\0';
	}

	while ( i-- ){
		putchar( buf[i] );
	}
}

static inline pci_device_t pci_device( uint8_t bus,
                                       uint8_t slot,
                                       uint8_t func )
{
	return (pci_device_t){
		.bus   = bus,
		.slot  = slot,
		.func  = func,
		.valid = true,
	};
}

static inline pci_device_t pci_invalid_device( void ){
	return (pci_device_t){
		.valid = false,
	};
}

static uint32_t pci_conf_read_dword( pci_device_t dev, uint32_t offset )
{
	uint32_t address = 0x80000000;

	address |= (dev.bus << 16) | (dev.slot << 11) | (dev.func << 8);
	address |= (offset & 0xfc);

	c4_out_dword( PCI_CONFIG_ADDRESS, address );

	return c4_in_dword( PCI_CONFIG_DATA );
}

static inline uint16_t pci_conf_read_word( pci_device_t dev, uint32_t offset )
{
	uint32_t temp = pci_conf_read_dword( dev, offset );

	return temp >> (offset % 4 * 8);
}

static inline uint8_t pci_conf_read_byte( pci_device_t dev, uint32_t offset )
{
	uint32_t temp = pci_conf_read_dword( dev, offset );

	return temp >> (offset % 4 * 8);
}


static char *pci_strings[] = {
	"Old",
	"Mass storage controller",
	"Network controller",
	"Display controller",
	"Multimedia controller",
	"Memory controller",
	"Bridge device",
	"Simple communications controller",
	"Base system peripheral",
	"Input device controller",
	"Dock station controller",
	"Processor",
	"Serial bus controller",
	"Wireless controller",
	"Intelligent IO controller",
	"Satallite communications controller",
	"Crypto controller",
	"Data acquisition/Signal processing controller"
};

static void pci_dump_devices( void ){
	puts( "scanning pci bus...\n" );

	for ( unsigned bus = 0; bus < 256; bus++ ){
		for ( unsigned device = 0; device < 32; device++ ){
			for ( unsigned function = 0; function < 7; function++ ){
				pci_device_t dev = pci_device( bus, device, function );
				uint16_t vendor = pci_conf_read_word( dev, PCI_CONFIG_VENDOR );

				if ( vendor == 0xffff ){
					if ( function == 0 )
						break;
					// no device, continue checking for others
					continue;
				} 

				uint16_t device_id = pci_conf_read_word( dev, PCI_CONFIG_DEVICE );
				uint16_t class_id  = pci_conf_read_word( dev, PCI_CONFIG_CLASS );

				puts( "[pci] " );
				print_hex( bus );
				putchar( '.' );
				print_hex( device );
				putchar( '.' );
				print_hex( function );

				putchar( ' ' );

				print_hex( vendor );
				putchar( ':' );
				print_hex( device_id );

				puts( ", " );
				puts( pci_strings[class_id] );
				putchar( '\n' );
			}
		}
	}
}

static pci_device_t pci_lookup( uint16_t vendor_id, uint16_t device_id ){
	for ( unsigned bus = 0; bus < 256; bus++ ){
		for ( unsigned device = 0; device < 32; device++ ){
			for ( unsigned function = 0; function < 7; function++ ){
				pci_device_t dev = pci_device( bus, device, function );
				uint16_t vendor = pci_conf_read_word( dev, PCI_CONFIG_VENDOR );

				if ( vendor == 0xffff ){
					if ( function == 0 )
						break;
					// no device, continue checking for others
					continue;
				}

				uint16_t dev_id = pci_conf_read_word( dev, PCI_CONFIG_DEVICE );

				if ( vendor_id == vendor && device_id == dev_id ){
					return pci_device( bus, device, function );
				}
			}
		}
	}

	return pci_invalid_device( );
}

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

unsigned hash_string( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
}

static inline unsigned nameserver_lookup( unsigned server, const char *name ){
	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { hash_string(name) },
	};

	c4_msg_send( &msg, server );
	c4_msg_recieve( &msg, server );

	return msg.data[0];
}

void _start( unsigned long ndisplay ){
	display = ndisplay;

	pci_dump_devices( );
	pci_device_t foo = pci_lookup( 0x10ec, 0x8139 );

	if ( foo.valid ){
		puts( "found a realtek 8139 network card" );
	}

	c4_exit();
}
