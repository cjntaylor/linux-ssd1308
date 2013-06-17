#include "ssd1308.h"

ssd1308_bitmap block = {
    4,
    4,
    {{1, 1, 1, 1},
     {1, 1, 1, 1},
     {1, 1, 1, 1},
     {1, 1, 1, 1}}
};

int main(int argc, char** argv) {

    SSD1308->init();
    SSD1308->draw(1, 1, &block);

    return 0;
}
