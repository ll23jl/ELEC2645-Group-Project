#ifndef ROOMS_H
#define ROOMS_H

#include <stdint.h>
#include <stdio.h>



// Block structure
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    uint8_t width;                 // Collision width
    uint8_t height;                // Collision height
}block;


typedef struct{
    uint8_t block_count;
    block* blocks;
}current_room;


// room functions
void render_blocks(void);
uint8_t collision(uint16_t c_x, uint16_t c_y, uint16_t c_w, uint16_t c_h, uint16_t o_x, uint16_t o_y, uint16_t o_w, uint16_t o_h);

// starting room
extern const uint8_t room_1[15][15];


#endif // ROOMS_H