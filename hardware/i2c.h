/*
  All the basic i2c logic for truman
 */
#ifndef __HARDWARE_I2C_H__
#define __HARDWARE_I2C_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <linux/types.h>

// Opens a fh for I2C
// Checks reference count
// returns file handler if opened correctly
// returns -1 if error
int I2cOpen(int bus);

// Closes fh for I2C properly
// returns 0 on success
int I2cClose(int bus);

// makes ioctrl call to set slave address
// return 0 on success
int I2cSetSlave(int bus, uint8_t address);

// Reads in data of length to *data on bus
// returns 0 if success
// returns -1 if fail
int I2cRead(int bus, uint8_t reg, uint8_t* data, int length);

// Reads byte to bus
// returns 0 if success
// returns -1 if fail
int I2cReadByte(int bus, uint8_t reg, uint8_t* data);

// Writes data to bus of length
// returns 0 if success
// returns -1 if fail
int I2cWrite(int bus, uint8_t reg, uint8_t* data, int length);

// Writes byte to bus
// returns 0 if success
// returns -1 if fail
int I2cWriteByte(int bus, uint8_t reg, uint8_t data);

#endif
