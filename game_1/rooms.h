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

// Room structure
typedef struct{
    uint8_t tiles[15][15];
}room;


// room functions
void render_blocks(void);
uint8_t collision(uint16_t c_x, uint16_t c_y, uint16_t c_w, uint16_t c_h, uint16_t o_x, uint16_t o_y, uint16_t o_w, uint16_t o_h);
void change_room(room new_room);

// rooms
extern room current_room; 
extern const room room_1;
extern const room room_2;

#endif // ROOMS_H