#ifndef _H_SSD1308
#define _H_SSD1308

#include <stdint.h>


typedef struct {
    uint8_t w;
    uint8_t h;
    uint8_t data[][128];
} ssd1308_bitmap;

typedef struct {
    uint8_t state;
    uint8_t screen[1024];

    // Setup/Reset the display
    void (*init)(void);
    void (*reset)(void);

    // Control the display power
    uint8_t (*display_on)(void);
    uint8_t (*display_off)(void);

    // Draw to the display
    uint8_t (*draw)(uint8_t x, uint8_t y, ssd1308_bitmap* img);

    // Additional functions
    void (*invert)(void);
    void (*clear)(void);
} ssd1308;

extern ssd1308* SSD1308;

#endif
