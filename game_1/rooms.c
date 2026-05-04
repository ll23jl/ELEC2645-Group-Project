// file containing all room data for the game

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "LCD.h"
#include "sprites.h"
#include "game_1.h"
#include "rooms.h"

const room* current_room; // global variable to hold the current room data
Sleep_point sleep_point; // global variable to hold the sleep point data
uint16_t current_room_index[2] = {0, 0}; // index to track which room we are in
int8_t is_npc = 0; // flag to indicate if NPC is present in the current room


// change room
void change_room(void) {
    //assign new room to current room
    current_room = map[current_room_index[0]][current_room_index[1]];

    // Randomly decide if NPC is present in this room 
    // Reduce chance as days progress to increase difficulty
    uint8_t npc_chance = 75 - day_counter; // Start at 75% chance and decrease by 1% each day
    is_npc = (rand() % 100) < npc_chance; // NPC is present if random number is less than npc_chance
    if (is_npc) {
        NPC_init(&npc_character); // initialize NPC if present
    } 
    Room_Init(&sleep_point); // initialize room objects like sleep points

}

// place blocks in the environment based on room data
void render_blocks(Sleep_point* sleep_point) {
    for (uint8_t i = 0; i < 15; i++) {
        for (uint8_t j = 0; j < 15; j++) {
            if (current_room->tiles[i][j] == 1) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_top_left, 0);
            }
            else if (current_room->tiles[i][j] == 2) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_top, 0);
            }
            else if (current_room->tiles[i][j] == 3) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_top_right, 0);
            }
            else if (current_room->tiles[i][j] == 4) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_mid_left, 0);
            }
            else if (current_room->tiles[i][j] == 5) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_mid, 0);
            }
            else if (current_room->tiles[i][j] == 6) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_mid_right, 0);
            }
            else if (current_room->tiles[i][j] == 7) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom_left, 0);
            }
            else if (current_room->tiles[i][j] == 8) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom, 0);
            }
            else if (current_room->tiles[i][j] == 9) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom_right, 0);
            }
            else if (current_room->tiles[i][j] == 10) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_top_end, 0);
            }
            else if (current_room->tiles[i][j] == 11) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_vertical, 0);
            }
            else if (current_room->tiles[i][j] == 12) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_bottom_end, 0);
            }
            else if (current_room->tiles[i][j] == 13) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_left_end, 0);
            }
            else if (current_room->tiles[i][j] == 14) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_horizontal, 0);
            }
            else if (current_room->tiles[i][j] == 15) {
                LCD_Draw_Sprite_directional(j * 16, i * 16, 16, 16, (uint8_t*)wall_right_end, 0);
            }
            else {
                // empty tile, do nothing
            }
        }
    }
    if (current_room_index[0] == sleep_point->room_index[0] && current_room_index[1] == sleep_point->room_index[1]) {
        LCD_Draw_Sprite_directional(sleep_point->x, sleep_point->y, 32, 32, (uint8_t*)pillow, 0);
    }    
    
}

// Other objects initialisation
void Room_Init(Sleep_point* sleep_point) {
    sleep_point->x = 104;
    sleep_point->y = 112;
    sleep_point->width = 10;
    sleep_point->height = 10;
    sleep_point->room_index[0] = 1;
    sleep_point->room_index[1] = 1;
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
    {&room_1, &room_2, &room_3},
    {&room_4, &room_5, &room_6},
    {&room_7, &room_8, &room_9}
};

// room top left
const room room_1 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 14, 14, 15, 0, 0, 0, 0, 1, 14, 14, 14, 14, 14, 14}, 
        {6, 0, 0, 0, 0, 0, 13, 14, 9, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}
    }
};
// room top middle
const room room_2 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 14, 14, 2, 14, 14, 15, 0, 0, 0, 0, 1, 14, 14, 14}, 
        {0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {0, 0, 0, 11, 0, 0, 0, 0, 0, 1, 14, 9, 0, 0, 0}, 
        {14, 14, 14, 9, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 1}, 
        {0, 0, 0, 0, 0, 0, 0, 1, 14, 9, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 4}, 
        {14, 14, 14, 14, 14, 14, 14, 9, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 3, 0, 0, 4, 2, 2, 2}
    }
};

// room 3 top right
const room room_3 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 0},
        {0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
        {0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
        {15, 0, 0, 11, 0, 0, 1, 14, 14, 14, 14, 15, 0, 0, 4}, 
        {0, 0, 0, 11, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 11, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {2, 14, 14, 9, 0, 0, 11, 0, 0, 13, 14, 14, 14, 14, 0}, 
        {6, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {6, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 2, 2, 3, 0, 0, 4, 14, 15, 0, 0, 0, 0, 0, 4}, 
        {8, 8, 8, 9, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 4}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 0, 0, 4}
    }
};

// room mid left
const room room_4 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {6, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 1, 14, 14, 14, 14, 14, 14, 14, 3, 0, 0, 0}, 
        {6, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 7, 3, 0, 0}, 
        {6, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 7, 14, 14}, 
        {6, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 7, 15, 0, 0, 13, 2, 15, 0, 0, 13, 14, 14}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 1, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2}
    }
};
// room mid middle
const room room_5 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 8, 8, 8, 8, 8, 9, 0, 0, 7, 8, 8, 8},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 14, 14, 14, 2, 2, 14, 14, 14, 2, 2, 14, 14, 14, 2}, 
        {0, 0, 0, 0, 7, 9, 0, 0, 0, 7, 9, 0, 0, 0, 7}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {15, 0, 0, 13, 14, 3, 0, 0, 0, 1, 14, 15, 0, 0, 13}, 
        {0, 0, 0, 0, 0, 4, 14, 14, 14, 9, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {14, 14, 14, 14, 14, 9, 0, 0, 0, 0, 0, 0, 1, 14, 14}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 6, 0, 0, 0, 4, 2, 2}
    }
};

// room 3 mid right
const room room_6 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 0, 0, 4},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 4},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 4},
        {3, 0, 0, 0, 0, 0, 1, 14, 14, 14, 14, 9, 0, 0, 4}, 
        {8, 14, 14, 15, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 11, 0, 0, 10, 0, 0, 0, 0, 4}, 
        {14, 14, 14, 14, 14, 14, 6, 0, 0, 7, 14, 14, 14, 14, 0}, 
        {0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4},  
        {0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {14, 14, 14, 15, 0, 0, 7, 14, 14, 14, 14, 3, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 4}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 6, 0, 0, 4}
    }
};

// room bottom left
const room room_7 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 14, 14, 14, 14, 15, 0, 0, 1, 3, 0, 0, 0, 0, 0},
        {6, 0, 0, 0, 0, 0, 0, 0, 4, 8, 14, 14, 14, 14, 14}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 13, 14, 14, 14, 14, 9, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {0, 14, 14, 14, 14, 14, 14, 14, 15, 0, 0, 0, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 14, 14, 14},
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2}
    }
};
// room bottom middle
const room room_8 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 8, 8, 8, 8, 8, 6, 0, 0, 0, 4, 8, 8},
        {0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
        {14, 14, 14, 14, 14, 14, 14,14, 14, 14, 14, 14, 14, 14, 2}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {14, 14, 14, 14, 14, 15, 0,0, 13, 14, 14, 3, 0, 0, 7}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0}, 
        {14, 14, 15, 0, 0, 13, 14, 14, 3, 0, 0, 7, 14, 14, 14}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2}
    }
};

// room 3 bottom right
const room room_9 = {
    .tiles = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {8, 8, 8, 8, 8, 8, 8, 0, 8, 8, 8, 9, 0, 0, 4},
        {0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 4},
        {0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 4},
        {0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 13, 14, 14, 14, 0}, 
        {3, 0, 0, 13, 14, 14, 14, 6, 0, 0, 0, 0, 0, 0, 4}, 
        {6, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 4}, 
        {6, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 4}, 
        {8, 14, 14, 14, 15, 0, 0, 7, 14, 14, 14, 15, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, 
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0}
    }
};