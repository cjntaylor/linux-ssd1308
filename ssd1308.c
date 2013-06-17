#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#include "ssd1308.h"

#define I2C_BUS "/dev/i2c-%d"
#define RPI_DEV 0

#define TRUE  1
#define FALSE 0

#define ADDRESS 0x3c
#define COMMAND 0x80
#define DATA    0x40

#define SET_ADDR_MODE 0x20
#define HORIZONTAL    0x00

#define SEND(type,len,buff) i2c_send(RPI_DEV, ADDRESS, type, len, buff)

int i2c_send(uint8_t bus, uint8_t addr, uint8_t reg, uint8_t len, uint8_t* data) {
    uint8_t code = 0;
    uint8_t fd;
    
    // Open the i2c bus
    char device[1024];
    sprintf(device, I2C_BUS, bus);
    if((fd = open(device, O_RDWR) < 0)) { code = 1; goto end; }
    
    // Connect to the device at the given address
    if(ioctl(fd, I2C_SLAVE, addr) < 0) { code = 2; goto end; }

    // Send the register byte to the device first
    uint8_t buff[1] = { reg };
    if(!write(fd, buff, 1)) { code = 3; goto end; }

    // Send data bytes to the device
    if(!write(fd, data, len)) { code = 4; goto end; }
end:
    close(fd);
    return code;
}

void ssd1308_init(void) {
    // Reset the screen state
    SSD1308->state = 0;

    // Clear the screen buffer
    uint8_t i; for(i = 0; i < 1024; i++) SSD1308->screen[i] = 0x0;

    // Turn the display on
    SSD1308->display_on();

    // Put the display in horizontal addressing mode
    uint8_t cmds[2] = { SET_ADDR_MODE, HORIZONTAL }; 
    SEND(COMMAND, 2, cmds);
}

void ssd1308_reset(void) {
    // NOOP for now
}

uint8_t ssd1308_display_on(void) {
    // Don't turn on if already on
    if(SSD1308->state) return FALSE;

    uint8_t cmds[1] = { 0xAF };
    if(!SEND(COMMAND, 1, cmds)) return FALSE;
    return SSD1308->state = TRUE;
}

uint8_t ssd1308_display_off(void) {
    // Don't turn off if already off
    if(!SSD1308->state) return FALSE;
    
    uint8_t cmds[1]= { 0xAE };
    if(!SEND(COMMAND, 1, cmds)) return FALSE;
    return SSD1308->state = TRUE;
}

uint8_t ssd1308_draw(uint8_t x, uint8_t y, ssd1308_bitmap* img) {
    // The top left corner can't be outside the bounds of the screen
    if(x < 0 || x >= 128) return 1;
    if(y < 0 || y >=  64) return 2;

    // Calculate bitmap boundaries
    uint8_t ox = x + img->w;
    uint8_t oy = y + img->h;

    // Truncate the image if it exceeds the boundary of the screen
    ox = (ox >= 128) ? 127 : ox;
    oy = (oy >=  64) ?  63 : oy;

    // Convert the rows to page offsets
    uint8_t s = (y  >> 0x3); // Start page
    uint8_t e = (oy >> 0x3); // End page

    // Move page address pointer to span page offset range
    uint8_t page_cmds[3] = { 0x22, s, e };
    if(!SEND(COMMAND, 3, page_cmds)) return 3;

    // Move column address pointer to span columns
    uint8_t cols_cmds[3] = { 0x21, x, ox-1 };
    if(!SEND(COMMAND, 3, cols_cmds)) return 4;
    
    // Update the screen state in place by collapsing the image bytes into bits
    uint8_t i, j, c = 0;
    for(j = y; j < oy; j++) {
        for(i = x; i < ox; i++) {
            SSD1308->screen[(j<<0x4)+i] ^= (-(img->data[j-y][i-x]) ^ SSD1308->screen[(j<<0x4)+i]) & (1<<(j&0x7));
        }
    }

    // Copy the bytes we modified into a new buffer to send to the screen
    uint8_t buff[(e-s+1)*img->w];
    for(j = s; j <= e; j++) {
        for(i = x; i < ox; i++) {
            buff[c++] = SSD1308->screen[(j<<0x7)+i];
        }
    }

    // Send the bitmap to the screen
    if(!SEND(DATA, (e-s+1)*img->w, buff)) return 5;

    return 0;
}

void ssd1308_invert(void) {
}

void ssd1308_clear(void) {
}

ssd1308 SSD1308_impl = {
    0,  // Power state (off)
    {}, // Screen buffer (NULL)

    ssd1308_init,
    ssd1308_reset,
    ssd1308_display_on,
    ssd1308_display_off,
    ssd1308_draw,
    ssd1308_invert,
    ssd1308_clear
};

ssd1308* SSD1308 = &SSD1308_impl;
