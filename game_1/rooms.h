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
void change_room(void);

// npc
extern int8_t is_npc; // flag to indicate if NPC is present in the current room

// map
extern const room* map[3][3]; // 3x3 grid of rooms

// current room variables
extern const room* current_room;
extern uint16_t current_room_index[2];

// room layouts
extern const room room_1;
extern const room room_2;
extern const room room_3;
extern const room room_4;
extern const room room_5;
extern const room room_6;
extern const room room_7;
extern const room room_8;
extern const room room_9;

#endif // ROOMS_H