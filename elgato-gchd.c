#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>

#include <libusb-1.0/libusb.h>

/* constants */
#define ELGATO_VENDOR			0x0fd9
#define GAME_CAPTURE_HD_PID_0	0x0044
#define GAME_CAPTURE_HD_PID_1	0x004e
#define GAME_CAPTURE_HD_PID_2	0x0051

#define EP_OUT			0x02
#define INTERFACE_NUM	0x00
#define CONFIGURATION	0x01

#define DATA_BUF		0x4000

/* global structs */
static struct libusb_device_handle *devh = NULL;

int init_dev_handler() {
	if (libusb_init(NULL)) {
		fprintf(stderr, "Error initializing libusb.");
		return 1;
	}

	libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_DEBUG);

	devh = libusb_open_device_with_vid_pid(NULL, ELGATO_VENDOR, GAME_CAPTURE_HD_PID_0);
	if (devh) {
		return 0;
	}

	devh = libusb_open_device_with_vid_pid(NULL, ELGATO_VENDOR, GAME_CAPTURE_HD_PID_1);
	if (devh) {
		return 0;
	}

	devh = libusb_open_device_with_vid_pid(NULL, ELGATO_VENDOR, GAME_CAPTURE_HD_PID_2);
	if (devh) {
		return 0;
	}

	fprintf(stderr, "Unable to find device.");
	return 1;
}

int get_interface() {
	if (libusb_kernel_driver_active(devh, INTERFACE_NUM)) {
		libusb_detach_kernel_driver(devh, INTERFACE_NUM);
	}

	if (libusb_set_configuration(devh, CONFIGURATION)) {
		fprintf(stderr, "Could not activate configuration.");
		return 1;
	}

	if (libusb_claim_interface(devh, INTERFACE_NUM)) {
		fprintf(stderr, "Failed to claim interface.");
		return 1;
	}

	return 0;
}

void clean_up() {
	if (devh) {
		libusb_release_interface(devh, INTERFACE_NUM);
		libusb_close(devh);
	}

	libusb_exit(NULL);
}

void read_config(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength) {
	unsigned char *recv;

	recv = calloc(wLength, sizeof(unsigned char));
	libusb_control_transfer(devh, 0xc0, bRequest, wValue, wIndex, recv, wLength, 0);
	free(recv);
}

void write_config2(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1) {
	unsigned char send[2] = {data0, data1};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 2, 0);
}

void write_config3(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1, unsigned char data2) {
	unsigned char send[3] = {data0, data1, data2};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 3, 0);
}

void write_config4(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3) {
	unsigned char send[4] = {data0, data1, data2, data3};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 4, 0);
}

void write_config5(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4) {
	unsigned char send[5] = {data0, data1, data2, data3, data4};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 5, 0);
}

void write_config6(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4, unsigned char data5) {
	unsigned char send[6] = {data0, data1, data2, data3, data4, data5};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 6, 0);
}

void write_config8(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4, unsigned char data5, unsigned char data6, unsigned char data7) {
	unsigned char send[8] = {data0, data1, data2, data3, data4, data5, data6, data7};
	libusb_control_transfer(devh, 0x40, bRequest, wValue, wIndex, send, 8, 0);
}

void load_firmware(const char *file) {
	int transfer;

	FILE *bin;
	bin = fopen(file, "rb");

	/* get filesize */
	fseek(bin, 0L, SEEK_END);
	long filesize = ftell(bin);
	rewind(bin);

	/* read firmware from file to buffer and bulk transfer to device */
	for (int i = 0; i <= filesize; i += DATA_BUF) {
		unsigned char data[DATA_BUF] = {0};
		int bytes_remain = filesize - i;

		if ((bytes_remain) > DATA_BUF) {
			bytes_remain = DATA_BUF;
		}

		fread(data, bytes_remain, 1, bin);

		libusb_bulk_transfer(devh, 0x02, data, bytes_remain, &transfer, 0);
	}

	fclose(bin);
}

/* unidentified repetitive sequence 1 */
void sequence1() {
	write_config5(0xbd, 0x0000, 0x3300, 0xab, 0xa9, 0x0f, 0xa4, 0x55);
	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);

	read_config(0xbc, 0x0900, 0x001c, 2);
	read_config(0xbd, 0x0000, 0x3300, 3);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x02);

	read_config(0xbc, 0x0900, 0x001c, 2);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);
	write_config2(0xbc, 0x0900, 0x0018, 0x00, 0x02);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);
	write_config2(0xbc, 0x0900, 0x0018, 0x00, 0x02);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);
}

void configure_dev() {
	read_config(0xbc, 0x0800, 0x0094, 4);
	read_config(0xbc, 0x0800, 0x0098, 4);
	read_config(0xbc, 0x0800, 0x0010, 4);
	read_config(0xbc, 0x0800, 0x0014, 4);
	read_config(0xbc, 0x0800, 0x0018, 4);

	write_config2(0xbc, 0x0900, 0x0000, 0x00, 0x00);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);
	read_config(0xbc, 0x0900, 0x0074, 2);
	read_config(0xbc, 0x0900, 0x01b0, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);

	/* this is an important step for sending the firmware */
	write_config2(0xbc, 0x0900, 0x0074, 0x00, 0x04);
	write_config2(0xbc, 0x0900, 0x01b0, 0x00, 0x00);

	/* load "idle" firmware */
	load_firmware("firmware/mb86h57_h58_idle.bin");

	/* begin of highly (!) experimental part - thank you jedahan! */
	write_config2(0xbc, 0x0900, 0x0070, 0x00, 0x04);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);
	read_config(0xbc, 0x0000, 0x0010, 2);
	read_config(0xbc, 0x0000, 0x0012, 2);
	read_config(0xbc, 0x0000, 0x0014, 2);
	read_config(0xbc, 0x0000, 0x0016, 2);
	read_config(0xbc, 0x0000, 0x0018, 2);
	read_config(0xbc, 0x0000, 0x001a, 2);
	read_config(0xbc, 0x0000, 0x001c, 2);
	read_config(0xbc, 0x0000, 0x001e, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);

	write_config6(0xb8, 0x0000, 0x0000, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);

	read_config(0xbc, 0x0800, 0x2008, 2);
	read_config(0xbc, 0x0900, 0x0074, 2);
	read_config(0xbc, 0x0900, 0x01b0, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);
	read_config(0xbc, 0x0800, 0x2008, 2);

	write_config2(0xbc, 0x0900, 0x0074, 0x00, 0x04);
	write_config2(0xbc, 0x0900, 0x01b0, 0x00, 0x00);
	write_config5(0xbd, 0x0000, 0x3300, 0xab, 0xa9, 0x0f, 0xa4, 0x55);
	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x00);

	read_config(0xbc, 0x0900, 0x001c, 2);
	read_config(0xbd, 0x0000, 0x3300, 3);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x00);

	read_config(0xbc, 0x0900, 0x001c, 2);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x04);
	write_config2(0xbc, 0x0900, 0x0018, 0x00, 0x00);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);
	write_config2(0xbc, 0x0900, 0x0018, 0x00, 0x02);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);

	/* TODO: check for specific condition in a loop */
	sequence1();
	sequence1();
	sequence1();
	sequence1();
	sequence1();
	sequence1();

	write_config5(0xbd, 0x0000, 0x3300, 0xab, 0xa9, 0x0f, 0xa4, 0x55);
	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);

	read_config(0xbc, 0x0900, 0x001c, 2);
	read_config(0xbd, 0x0000, 0x3300, 3);

	write_config2(0xbc, 0x0900, 0x0014, 0x00, 0x06);
	write_config2(0xbc, 0x0900, 0x0018, 0x00, 0x02);

	read_config(0xbc, 0x0900, 0x0014, 2);
	read_config(0xbc, 0x0900, 0x0018, 2);

	write_config2(0xbc, 0x0900, 0x0000, 0x00, 0x00);

	write_config8(0xbc, 0x0001, 0x1000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02);
	write_config8(0xbc, 0x0001, 0x1000, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x04, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00);
	write_config8(0xbc, 0x0001, 0x1004, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80);
	write_config8(0xbc, 0x0001, 0x1100, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04);
	write_config8(0xbc, 0x0001, 0x1240, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04);
	write_config8(0xbc, 0x0001, 0x1100, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03);
	write_config8(0xbc, 0x0001, 0x1240, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03);
	write_config8(0xbc, 0x0001, 0x1104, 0x61, 0xa8, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1244, 0x1f, 0x40, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1104, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1244, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1108, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1108, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1248, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1248, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x110c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x110c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1110, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1110, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x124c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x124c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1250, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1250, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1124, 0x00, 0x00, 0x10, 0x11, 0x00, 0x00, 0x1f, 0xff);
	write_config8(0xbc, 0x0001, 0x1264, 0x00, 0x00, 0x10, 0x11, 0x00, 0x00, 0x1f, 0xff);
	write_config8(0xbc, 0x0001, 0x1128, 0x01, 0x0f, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1268, 0x01, 0x0f, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1134, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1274, 0x00, 0x1f, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x112c, 0x01, 0x10, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x126c, 0x01, 0x10, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x112c, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1f, 0xff);
	write_config8(0xbc, 0x0001, 0x126c, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1f, 0xff);
	write_config8(0xbc, 0x0001, 0x1268, 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x1f, 0xff);
	write_config8(0xbc, 0x0001, 0x1130, 0x00, 0xe0, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1270, 0x00, 0xe0, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x113c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x127c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x113c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x127c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1140, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1280, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1140, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff);
	write_config8(0xbc, 0x0001, 0x1280, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff);
	write_config8(0xbc, 0x0001, 0x1138, 0x10, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00);

	write_config2(0xbc, 0x0000, 0x1144, 0x00, 0xb0);
	write_config2(0xbc, 0x0000, 0x1146, 0x11, 0x00);
	write_config2(0xbc, 0x0000, 0x1148, 0x00, 0xc1);
	write_config2(0xbc, 0x0000, 0x114a, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x114c, 0x00, 0x01);
	write_config2(0xbc, 0x0000, 0x114e, 0xe1, 0x10);
	write_config2(0xbc, 0x0000, 0x1150, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x1152, 0xe0, 0x1f);

	write_config8(0xbc, 0x0001, 0x1278, 0x10, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00);

	write_config2(0xbc, 0x0000, 0x1284, 0x00, 0xb0);
	write_config2(0xbc, 0x0000, 0x1286, 0x11, 0xb0);
	write_config2(0xbc, 0x0000, 0x1288, 0x00, 0xc1);
	write_config2(0xbc, 0x0000, 0x128a, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x128c, 0x00, 0x01);
	write_config2(0xbc, 0x0000, 0x128e, 0xe1, 0x10);
	write_config2(0xbc, 0x0000, 0x1290, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x1292, 0xe0, 0x1f);

	write_config8(0xbc, 0x0001, 0x113c, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xff, 0x00);

	write_config2(0xbc, 0x0000, 0x1010, 0x7f, 0xf0);
	write_config2(0xbc, 0x0000, 0x1012, 0x0b, 0xff);
	write_config2(0xbc, 0x0000, 0x1014, 0xff, 0xc1);
	write_config2(0xbc, 0x0000, 0x1016, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x1018, 0xf0, 0x00);

	write_config8(0xbc, 0x0001, 0x127c, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xff, 0x00);

	write_config2(0xbc, 0x0000, 0x1370, 0x7f, 0xf0);
	write_config2(0xbc, 0x0000, 0x1372, 0x0b, 0xff);
	write_config2(0xbc, 0x0000, 0x1374, 0xff, 0xc1);
	write_config2(0xbc, 0x0000, 0x1376, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x1378, 0xf0, 0x00);

	write_config8(0xbc, 0x0001, 0x100c, 0x00, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x100c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x100c, 0x30, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1130, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1138, 0x00, 0x36, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);

	write_config2(0xbc, 0x0000, 0x1176, 0x37, 0x00);
	write_config2(0xbc, 0x0000, 0x11a4, 0xff, 0x03);
	write_config2(0xbc, 0x0000, 0x11a6, 0xe1, 0x0f);
	write_config2(0xbc, 0x0000, 0x11a8, 0xf0, 0x00);
	write_config2(0xbc, 0x0000, 0x1174, 0x02, 0xb0);
	write_config2(0xbc, 0x0000, 0x1178, 0x01, 0xc1);
	write_config2(0xbc, 0x0000, 0x117a, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x117c, 0xe1, 0x00);
	write_config2(0xbc, 0x0000, 0x117e, 0xf0, 0x0c);
	write_config2(0xbc, 0x0000, 0x1180, 0x05, 0x04);
	write_config2(0xbc, 0x0000, 0x1182, 0x48, 0x44);
	write_config2(0xbc, 0x0000, 0x1184, 0x4d, 0x56);
	write_config2(0xbc, 0x0000, 0x1186, 0x88, 0x04);
	write_config2(0xbc, 0x0000, 0x1188, 0x0f, 0xff);
	write_config2(0xbc, 0x0000, 0x118a, 0xfc, 0xfc);
	write_config2(0xbc, 0x0000, 0x118c, 0x1b, 0xf0);
	write_config2(0xbc, 0x0000, 0x118e, 0x11, 0xf0);
	write_config2(0xbc, 0x0000, 0x1190, 0x14, 0x05);
	write_config2(0xbc, 0x0000, 0x1192, 0x08, 0x48);
	write_config2(0xbc, 0x0000, 0x1194, 0x44, 0x4d);
	write_config2(0xbc, 0x0000, 0x1196, 0x56, 0xff);
	write_config2(0xbc, 0x0000, 0x1198, 0x1b, 0x44);
	write_config2(0xbc, 0x0000, 0x119a, 0x3f, 0x28);
	write_config2(0xbc, 0x0000, 0x119c, 0x04, 0x64);
	write_config2(0xbc, 0x0000, 0x119e, 0x00, 0x28);
	write_config2(0xbc, 0x0000, 0x11a0, 0x3f, 0x2a);
	write_config2(0xbc, 0x0000, 0x11a2, 0x02, 0x7e);

	write_config8(0xbc, 0x0001, 0x100c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x100c, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1270, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1278, 0x00, 0x36, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);

	write_config2(0xbc, 0x0000, 0x12b6, 0x37, 0x00);
	write_config2(0xbc, 0x0000, 0x12e4, 0xff, 0x03);
	write_config2(0xbc, 0x0000, 0x12e6, 0xe1, 0x0f);
	write_config2(0xbc, 0x0000, 0x12e8, 0xf0, 0x00);
	write_config2(0xbc, 0x0000, 0x12b4, 0x02, 0xb0);
	write_config2(0xbc, 0x0000, 0x12b8, 0x01, 0xc1);
	write_config2(0xbc, 0x0000, 0x12ba, 0x00, 0x00);
	write_config2(0xbc, 0x0000, 0x12bc, 0xe1, 0x00);
	write_config2(0xbc, 0x0000, 0x12be, 0xf0, 0x0c);
	write_config2(0xbc, 0x0000, 0x12c0, 0x05, 0x04);
	write_config2(0xbc, 0x0000, 0x12c2, 0x48, 0x44);
	write_config2(0xbc, 0x0000, 0x12c4, 0x4d, 0x56);
	write_config2(0xbc, 0x0000, 0x12c6, 0x88, 0x04);
	write_config2(0xbc, 0x0000, 0x12c8, 0x0f, 0xff);
	write_config2(0xbc, 0x0000, 0x12ca, 0xfc, 0xfc);
	write_config2(0xbc, 0x0000, 0x12cc, 0x1b, 0xf0);
	write_config2(0xbc, 0x0000, 0x12ce, 0x11, 0xf0);
	write_config2(0xbc, 0x0000, 0x12d0, 0x14, 0x05);
	write_config2(0xbc, 0x0000, 0x12d2, 0x08, 0x48);
	write_config2(0xbc, 0x0000, 0x12d4, 0x44, 0x4d);
	write_config2(0xbc, 0x0000, 0x12d6, 0x56, 0xff);
	write_config2(0xbc, 0x0000, 0x12d8, 0x1b, 0x44);
	write_config2(0xbc, 0x0000, 0x12da, 0x3f, 0x28);
	write_config2(0xbc, 0x0000, 0x12dc, 0x04, 0x64);
	write_config2(0xbc, 0x0000, 0x12de, 0x00, 0x28);
	write_config2(0xbc, 0x0000, 0x12e0, 0x3f, 0x2a);
	write_config2(0xbc, 0x0000, 0x12e2, 0x02, 0x7e);

	write_config8(0xbc, 0x0001, 0x1710, 0x10, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00);

	write_config2(0xbc, 0x0000, 0x1710, 0x00, 0xfc);
	write_config2(0xbc, 0x0000, 0x1712, 0x00, 0x24);
	write_config2(0xbc, 0x0000, 0x1714, 0x00, 0xf8);
	write_config2(0xbc, 0x0000, 0x1716, 0x00, 0xe8);
	write_config2(0xbc, 0x0000, 0x1718, 0x00, 0xda);
	write_config2(0xbc, 0x0000, 0x171a, 0x00, 0xc6);
	write_config2(0xbc, 0x0000, 0x171c, 0x00, 0x76);
	write_config2(0xbc, 0x0000, 0x171e, 0x00, 0xea);

	write_config8(0xbc, 0x0001, 0x1500, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00);
	write_config8(0xbc, 0x0001, 0x1500, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00);
	write_config8(0xbc, 0x0001, 0x1504, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1610, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1610, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1610, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1610, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1510, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1610, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1518, 0x0f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1618, 0x0f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1518, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1618, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1518, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x151c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1618, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x161c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1524, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0x00);
	write_config8(0xbc, 0x0001, 0x1524, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x152c, 0x02, 0xd0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x152c, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1624, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xff, 0x00);
	write_config8(0xbc, 0x0001, 0x1624, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x162c, 0x01, 0x40, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x162c, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1530, 0x00, 0x00, 0x3e, 0x80, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1534, 0x36, 0xb0, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1534, 0x00, 0x00, 0x1f, 0x40, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1538, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1630, 0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1634, 0x17, 0x70, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1634, 0x00, 0x00, 0x07, 0x08, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1638, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1550, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00);
	write_config8(0xbc, 0x0001, 0x1550, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1554, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1554, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1554, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1558, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1570, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1670, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1570, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1670, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1570, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1670, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x157c, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x167c, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x157c, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x167c, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x157c, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x167c, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x157c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00);
	write_config8(0xbc, 0x0001, 0x167c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00);
	write_config8(0xbc, 0x0001, 0x157c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70);
	write_config8(0xbc, 0x0001, 0x167c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70);
	write_config8(0xbc, 0x0001, 0x157c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
	write_config8(0xbc, 0x0001, 0x167c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
	write_config8(0xbc, 0x0001, 0x1580, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1680, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1580, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1680, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1580, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00);
	write_config8(0xbc, 0x0001, 0x1680, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00);
	write_config8(0xbc, 0x0001, 0x1580, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1680, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff);
	write_config8(0xbc, 0x0001, 0x1684, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1684, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1688, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x20, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a08, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a0c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a64, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a64, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1a68, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a68, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1a6c, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a6c, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff);
	write_config8(0xbc, 0x0001, 0x1a04, 0x01, 0x80, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00);
	write_config8(0xbc, 0x0001, 0x1a14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60);
	write_config8(0xbc, 0x0001, 0x1a14, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10);
	write_config8(0xbc, 0x0001, 0x1a14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08);
	write_config8(0xbc, 0x0001, 0x1a14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06);
	write_config8(0xbc, 0x0001, 0x1a14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);
	write_config8(0xbc, 0x0001, 0x1a04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01);

	write_config6(0xb8, 0x0000, 0x0000, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00);
	/* end of highly experimental part */

	/* load "enc" firmware */
	load_firmware("firmware/mb86h57_h58_enc_h.bin");

	read_config(0xbc, 0x0000, 0x0010, 2);
	read_config(0xbc, 0x0000, 0x0012, 2);
	read_config(0xbc, 0x0000, 0x0014, 2);
	read_config(0xbc, 0x0000, 0x0016, 2);
}

void receive_data() {
	int transfer;
	unsigned char data[DATA_BUF] = {0};

	libusb_bulk_transfer(devh, 0x84, data, DATA_BUF, &transfer, 0);
	printf("%d bytes received\n", transfer);

	for (int i = 0; i < DATA_BUF; i++) {
		printf("%x ", data[i]);
	}

	printf("\n");
}

int main() {
	/* initialize device handler */
	if (init_dev_handler()) {
		goto end;
	}

	/* detach kernel driver and claim interface */
	if (get_interface()) {
		goto end;
	}

	/* configure device */
	configure_dev();

/*
	while(1) {
		receive_data();
	}
*/

end:
	/* clean up */
	clean_up();

	return 0;
}
