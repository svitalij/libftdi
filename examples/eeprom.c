/* LIBFTDI EEPROM access example

   This program is distributed under the GPL, version 2
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <ftdi.h>

int main(int argc, char **argv)
{
    struct ftdi_context ftdic;
    struct ftdi_eeprom eeprom;
    unsigned char buf[2048];
    int size;
    int f, i, j;
    int vid = 0x0403;
    int pid = 0x6010;
    char const *desc    = 0;
    char const *serial  = 0;
    int erase = 0;
    int use_defaults = 0;
    int large_chip = 0;

    while ((i = getopt(argc, argv, "d::ev:p:P:S:")) != -1)
    {
        switch (i)
        {
        case 'd':
            use_defaults = 1;
            if(optarg)
                large_chip = 0x66; 
            break;
        case 'e':
            erase = 1;
            break;
        case 'v':
		vid = strtoul(optarg, NULL, 0);
		break;
	case 'p':
		pid = strtoul(optarg, NULL, 0);
		break;
	case 'P':
                desc = optarg;
		break;
	case 'S':
		serial = optarg;
		break;
	default:
		fprintf(stderr, "usage: %s [options]\n", *argv);
		fprintf(stderr, "\t-d[num] Work with default valuesfor 128 Byte "
                        "EEPROM or for 256 Byte EEPROm if some [num] is given\n");
		fprintf(stderr, "\t-e erase\n");
		fprintf(stderr, "\t-v verbose decoding\n");
		fprintf(stderr, "\t-p <number> Search for device with PID == number\n");
		fprintf(stderr, "\t-v <number> Search for device with VID == number\n");
		fprintf(stderr, "\t-P <string? Search for device with given "
                        "product description\n");
		fprintf(stderr, "\t-S <string? Search for device with given "
                        "serial number\n");
		exit(-1);
        }
    }

    // Init
    if (ftdi_init(&ftdic) < 0)
    {
        fprintf(stderr, "ftdi_init failed\n");
        return EXIT_FAILURE;
    }

    // Select first interface
    ftdi_set_interface(&ftdic, INTERFACE_ANY);

    // Open device
    f = ftdi_usb_open_desc(&ftdic, vid, pid, desc, serial);
    if (f < 0)
    {
        fprintf(stderr, "Device VID 0x%04x PID 0x%04x", vid, pid);
        if(desc)
            fprintf(stderr, " Desc %s", desc);
        if(serial)
            fprintf(stderr, " Serial %s", serial);
        fprintf(stderr, "\n");
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", 
		f, ftdi_get_error_string(&ftdic));

        exit(-1);
    }

    if (erase)
    {
        ftdi_eeprom_setsize(&ftdic, &eeprom, 2048);
        f = ftdi_erase_eeprom(&ftdic);
        if (f < 0)
        {
            fprintf(stderr, "Erase failed: %s", 
                    ftdi_get_error_string(&ftdic));
            return -2;
        }
        if (ftdic.eeprom->chip == -1)
            fprintf(stderr, "No EEPROM\n");
        else if (ftdic.eeprom->chip == 0)
            fprintf(stderr, "Internal EEPROM\n");
        else
            fprintf(stderr, "Found 93x%02x\n",ftdic.eeprom->chip);
        return 0;
    }        

    size = 2048;
    memset(buf,0, size);
    ftdic.eeprom = &eeprom;
    if(use_defaults)
    {
        ftdi_eeprom_initdefaults(&ftdic);
        ftdic.eeprom->manufacturer="IKDA";
        ftdic.eeprom->product="CPS-CONN";
        ftdic.eeprom->serial="0001";
        ftdic.eeprom->chip= large_chip;
        ftdic.eeprom->cbus_function[0]= CBUS_BB_RD;
        ftdic.eeprom->cbus_function[1]= CBUS_CLK48;
        ftdic.eeprom->cbus_function[2]= CBUS_IOMODE;
        ftdic.eeprom->cbus_function[3]= CBUS_BB;
        ftdic.eeprom->cbus_function[4]= CBUS_CLK6;
        f=(ftdi_eeprom_build(&ftdic, buf));
        if (f < 0)
        {
            fprintf(stderr, "ftdi_eeprom_build: %d (%s)\n", 
                    f, ftdi_get_error_string(&ftdic));
            exit(-1);
        }
    }
    else
    {
        f = ftdi_read_eeprom(&ftdic, buf);
        if (f < 0)
        {
            fprintf(stderr, "ftdi_read_eeprom: %d (%s)\n", 
                    f, ftdi_get_error_string(&ftdic));
            exit(-1);
        }
    }
    fprintf(stderr, "Chip type %d ftdi_eeprom_size: %d\n", ftdic.type, ftdic.eeprom->size);
    for(i=0; i < ftdic.eeprom->size; i += 16)
    {
	fprintf(stdout,"0x%03x:", i);
	
	for (j = 0; j< 8; j++)
	    fprintf(stdout," %02x", buf[i+j]);
	fprintf(stdout," ");
	for (; j< 16; j++)
	    fprintf(stdout," %02x", buf[i+j]);
	fprintf(stdout," ");
	for (j = 0; j< 8; j++)
	    fprintf(stdout,"%c", isprint(buf[i+j])?buf[i+j]:'.');
	fprintf(stdout," ");
	for (; j< 16; j++)
	    fprintf(stdout,"%c", isprint(buf[i+j])?buf[i+j]:'.');
	fprintf(stdout,"\n");
    }

    f = ftdi_eeprom_decode(&ftdic,buf, size, 1);
    {
        fprintf(stderr, "ftdi_eeprom_decode: %d (%s)\n", 
		f, ftdi_get_error_string(&ftdic));
        exit(-1);
    }
	

    ftdi_usb_close(&ftdic);
    ftdi_deinit(&ftdic);
}
