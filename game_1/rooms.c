// file containing all room data for the game

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "LCD.h"
#include "sprites.h"
#include "game_1.h"
#include "rooms.h"

const room* current_room; // global variable to hold the current room data
uint16_t current_room_index[2] = {0, 0}; // index to track which room we are in
int8_t is_npc = 0; // flag to indicate if NPC is present in the current room

// change room
void change_room(void) {
    //assign new room to current room
    current_room = map[current_room_index[0]][current_room_index[1]];
    is_npc = rand() % 2; // randomly decide if NPC is present in this room (50% chance)
    if (is_npc) {
        NPC_init(&npc_character); // initialize NPC if present
    }

}

// place blocks in the environment based on room data
void render_blocks(void) {
    for (uint8_t i = 0; i < 15; i++) {
        for (uint8_t j = 0; j < 15; j++) {
            if (current_room->tiles[i][j] == 1) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_full, 0);
            }
            else if (current_room->tiles[i][j] == 2) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_top, 0);
            }
            else if (current_room->tiles[i][j] == 3) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_right, 0);
            }
            else if (current_room->tiles[i][j] == 4) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom, 0);
            }
            else if (current_room->tiles[i][j] == 5) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_left, 0);
            }
            else if (current_room->tiles[i][j] == 6) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_top_left, 0);
            }
            else if (current_room->tiles[i][j] == 7) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_top_right, 0);
            }
            else if (current_room->tiles[i][j] == 8) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom_right, 0);
            }
            else if (current_room->tiles[i][j] == 9) {
                LCD_Draw_Sprite(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom_left, 0);
            }
            else {
                // empty tile, do nothing
            }
        }
    }
}

// check collisions between a character and an object in the environment using AABB collision detection
uint8_t collision(uint16_t c_x, uint16_t c_y, uint16_t c_w, uint16_t c_h,
                  uint16_t o_x, uint16_t o_y, uint16_t o_w, uint16_t o_h) {

    int dx1 = (int)c_x - (int)o_x;
    int dx2 = (int)o_x - (int)c_x;

    int dy1 = (int)c_y - (int)o_y;
    int dy2 = (int)o_y - (int)c_y;

    int sum_w = (c_w + o_w) / 2;
    int sum_h = (c_h + o_h) / 2;

    return (dx1 <= sum_w && dx2 <= sum_w) &&
           (dy1 <= sum_h && dy2 <= sum_h);
}


// map
const room* map[3][3] = {
    {NULL,    NULL,    NULL},
    {&room_2, &room_1, &room_3},
    {NULL,    NULL,    NULL}
};

// room 1 centre
const room room_1 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }
};

// room 2 left
const room room_2 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }
};

// room 3 right
const room room_3 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0}
    }
};
