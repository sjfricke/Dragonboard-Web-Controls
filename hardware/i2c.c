#include "i2c.h"

// File handlers if all I2C
#define I2C_MAX_BUS_COUNT 4
// initalize every counter to zero
static int I2C_FH[I2C_MAX_BUS_COUNT];
// Keeps a refernce counter of all used FH
static int I2C_FH_RC[I2C_MAX_BUS_COUNT] = { 0 };

int I2cOpen(int bus) {

  char i2c_path[32];

  // check for valid bus
  if (bus < 0 || bus >= I2C_MAX_BUS_COUNT) {
    printf("ERROR: Bus %d was not available\n", bus);
    return -1;
  }
  
  if (I2C_FH_RC[bus] == 0) {

    sprintf(i2c_path, "/dev/i2c-%d", bus);
    
    I2C_FH[bus] = open(i2c_path, O_RDWR);
    if (I2C_FH[bus] < 0) {
      printf("ERROR: Could not open i2c on bus %d\n", bus);
      return -1;
    } else {
      I2C_FH_RC[bus]++;
      return I2C_FH[bus];
    }
  } else {
    printf("LOG: already open I2C-%d\n", bus);
    I2C_FH_RC[bus]++;
    return I2C_FH[bus];
  }
}

int I2cClose(int bus) {
  
  if (bus < 0 || bus >= I2C_MAX_BUS_COUNT) {
    printf("ERROR: Bus %d was not available\n", bus);
    return -1;
  }

  if (I2C_FH_RC[bus] == 1) {
    // last open reference
    close(I2C_FH[bus]);
    I2C_FH[bus] = 0; // null FH incase
    I2C_FH_RC[bus] = 0;
    return 0;

  } else if (I2C_FH_RC[bus] > 1) {
    // if there is some refernce already
    I2C_FH_RC[bus]--;
    return 0;

  } else {
    printf("ERROR: I2C-%d was not open\n", bus);
    return -1;
  }
}

int I2cSetSlave(int bus, uint8_t address) {
  if (ioctl(I2C_FH[bus], I2C_SLAVE, address) < 0) {
    printf("ERROR: Failed to set bus %d slave of address %d", bus, address);
    return -1;
  }
  return 0;
}


int I2cRead(int bus, uint8_t reg, uint8_t* data, int length) {
  int status;

  if ( write(I2C_FH[bus], &reg, 1) != 1 ) {
    printf("ERROR: Failed to write %d from bus %d\n", reg, bus);
    return -1;
  }

  status = read(I2C_FH[bus], data, length);
  if (status < length) {
    printf("ERROR: Failed to read only %d of %d from bus %d\n", status, length, bus);
    return -1;
  }
  return 0;
}

int I2cReadByte(int bus, uint8_t reg, uint8_t* data) {
 
  if ( write(I2C_FH[bus], &reg, 1) != 1 ) {
    printf("ERROR: Failed to write %d from bus %d\n", reg, bus);
    return -1;
  }

  if ( read(I2C_FH[bus], data, 1) != 1 ) {
    printf("ERROR: Failed to read %d from bus %d\n", *data, bus);
    return -1;
  }
  return 0;
}


int I2cWrite(int bus, uint8_t reg, uint8_t* data, int length) {
  int status;
  uint8_t s_data[length+1];
  s_data[0] = reg;
  memcpy(&s_data[1], data, length);

  status = write(I2C_FH[bus], s_data, length+1);
  if (status < length) {
    printf("ERROR: Failed to write only %d of %d from bus %d\n", status, length, bus);
    return -1;
  }
  return 0;
}

int I2cWriteByte(int bus, uint8_t reg, uint8_t data) {

  uint8_t s_data[2];
  s_data[0] = reg;
  s_data[1] = data;

  if ( write(I2C_FH[bus], s_data, 2) != 2 ) {
    printf("ERROR: Failed to write %d from bus %d\n", data, bus);
    return -1;
  }
  return 0;
}
