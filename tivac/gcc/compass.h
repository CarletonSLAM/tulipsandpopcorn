
#ifndef __TULIPS_AND_POPCORN_COMPASS__
    #define __TULIPS_AND_POPCORN_COMPASS__

    #define MATH_PI 3.1415926535897932384626

    #define UART_BUFFER_LENGTH 1024
    #define COMPASS_ADDR 0x1D

    #define TEMP_MSB = 0x05
    #define TEMP_LSB = 0x06

    #define COMPASS_CMD_WHO_AM_I 0x0F
    #define COMPASS_RESULT_WHO_AM_I 0x49

    #define COMPASS_CMD_MAGN_X_LSB 0x08
    #define COMPASS_CMD_MAGN_X_MSB 0x09
    #define COMPASS_CMD_MAGN_Y_LSB 0x0A
    #define COMPASS_CMD_MAGN_Y_MSB 0x0B
    #define COMPASS_CMD_MAGN_Z_LSB 0x0C
    #define COMPASS_CMD_MAGN_Z_MSB 0x0D

    #define COMPASS_CMD_ACCELL_X_LSB 0x28
    #define COMPASS_CMD_ACCELL_X_MSB 0x29
    #define COMPASS_CMD_ACCELL_Y_LSB 0x2A
    #define COMPASS_CMD_ACCELL_Y_MSB 0x2B
    #define COMPASS_CMD_ACCELL_Z_LSB 0x2C
    #define COMPASS_CMD_ACCELL_Z_MSB 0x2D


    uint8_t COMPASS_init(void);

    uint8_t COMPASS_get_raw_accel_values(void);
    uint8_t COMPASS_get_raw_magn_values(void);
    void COMPASS_update_heading(void);
    void COMPASS_caliberate_Mode(void);
    void COMPASS_wait_Section_Reached(uint8_t section);

#endif
