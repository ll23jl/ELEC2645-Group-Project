// file containing all room data for the game

#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "sprites.h"
#include "game_1.h"
#include "rooms.h"



// place blocks in the environment based on room data
void render_blocks(void) {
    for (uint8_t i = 0; i < 15; i++) {
        for (uint8_t j = 0; j < 15; j++) {
            if (room_1[i][j] == 1) {
                LCD_Draw_Sprite(j * 16 - 8, i * 16 - 8, 16, 16, (uint8_t*)grass_block, 0);
            }
        }
    }
}

// check collisions with a block in the environment
uint8_t collision(uint16_t c_x, uint16_t c_y, uint16_t c_w, uint16_t c_h, uint16_t o_x, uint16_t o_y, uint16_t o_w, uint16_t o_h) {

    uint16_t dx = abs(c_x - o_x);
    uint16_t dy = abs(c_y - o_y);

    return (dx <= (c_w/2 + o_w/2)) &&
           (dy <= (c_h/2 + o_h/2));
}

// room 1
const uint8_t room_1[15][15] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, 
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
