#ifndef __I2C_CORE_H__
#define __I2C_CORE_H__

#include "akdefine.h"

void i2c_pin_cfg(T_U32 pin_scl, T_U32 pin_sda);

int i2c_cmd(T_U8 *wr_data, int wr_len, T_U8 *rd_data, int rd_len);

#endif