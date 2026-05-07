
#include <stdint.h>
#include "Character.h"
#include "Joystick.h"
#include "rooms.h"
#include "Game_1.h"
#include "sprites.h"


// NPC movement counter
int8_t npc_move_counter;
int8_t npc_direction; // -1 for left, 0 for idle, 1 for right

extern Joystick_cfg_t joystick_cfg; // Joystick configuration
extern Joystick_t joystick_data; // Joystick data structure



// ===== PLAYER CHARACTER FUNCTIONS =====

// Initialize character at screen center
void Character_Init(Character* character) {
    character->x = 120;
    character->y = 120;
    character->prev_x = 120;
    character->prev_y = 120;
    character->direction = 1; // Start facing right
    character->state = CHAR_IDLE;
    character->animation_frame = 0;
    character->frame_counter = 0;
    character->dash_counter = 0;
    character->jump_counter = 0;
    character->width = 16;              // - adjusted for better collision feel (smaller than actual 32x32 sprite)
    character->height = 16;             // - adjusted for better collision feel (smaller than actual 32x32 sprite)
    character->health = 1000;            // Start with full health
    character->food = 500;              // Start with half food
}

// Update character position and state based on joystick input and button presses
void Character_Update(Character* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed) {
    
    // Calculate movement based on joystick direction
    int16_t move_x = 0;
    int16_t move_y = 0;
    
    switch (joy->direction) {
        case E:  move_x = 1; break;                 // move right
        case W:  move_x = -1; break;                // move left
        default: move_x = 0; move_y = 0; break;     // no movement
    }
    
    // Handle jump button
    if (jump_pressed && character->jump_counter == 0 && character->state != CHAR_JUMPING && character->state != CHAR_FALLING) {
        character->jump_counter = CHAR_JUMP_DURATION;
    }

    // Handle dash button
    if (dash_pressed && character->dash_counter == 0) {
        character->dash_counter = CHAR_DASH_DURATION;
    }
    
    // Apply movement with speed
    uint8_t current_speed = CHAR_SPEED;
    if (character->dash_counter > 0) {
        current_speed = CHAR_DASH_SPEED;
        character->dash_counter--;
    }
    if (character->jump_counter > 0) {
        current_speed = CHAR_JUMP_SPEED;
        move_y = jump_height;  // Force upward movement during jump
        character->jump_counter--;
    }
    
    int16_t new_x = character->x + (move_x * current_speed);
    int16_t new_y = character->y + (move_y * current_speed) - GRAVITY;
    
    // Handle collisions
    //Check if player overlaps with any objects
    for (uint8_t i = 0; i < 15; i++)           // runs for size of the room - [15][15] blocks
    {   for (uint8_t j = 0; j < 15; j++)
        {
            if (current_room->tiles[i][j] > 0) // if the tile is not empty, check for collision with player
            {
                block current_block;
                current_block.x = j * 16 + 8;       // Calculate block's x centre position ( 16 x 16 pixel sprite)
                current_block.y = i * 16 + 8;       // Calculate block's y centre position
                current_block.width = 16;       // Block width
                current_block.height = 16;      // Block height

                // handle x direction collisions
                if (collision(new_x, character->y,
                              character->width, character->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    // interacting from left
                    if (move_x > 0) {
                        new_x = current_block.x - (current_block.width + character->width) / 2 - 1; // Place character just to the right of block
                    }
                    // interacting from right
                    else if (move_x < 0) {
                        new_x = current_block.x + (current_block.width + character->width) / 2 + 1; // Place character just to the left of block
                    }
                    break;
                }
                // handle y direction collisions
                if (collision(new_x, new_y,
                              character->width, character->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    // falling down onto block
                    if (move_y >= 0) {
                        new_y = current_block.y - (current_block.height + character->height) / 2 - 6; // Place character on top of block
                    }
                    // hitting head on block
                    else if (move_y < 0) {
                        new_y = current_block.y + (current_block.height + character->height) / 2 + 6; // Place character just below block
                    }
                    break;
                }
            }
        }
    }


    // detect room transitions (if character goes beyond screen edges)
    if (new_x < 0) {  
        current_room_index[1]--;
        change_room();
        new_x = 240;
        new_y -= 10;    // small adjustment to prevent immediate re-collision with floor
    }
    if (new_x > 240) { 
        current_room_index[1]++; 
        change_room();
        new_x = 0;
    }
    if (new_y < 10) {  
        current_room_index[0]--; 
        change_room();
        new_y = 230;
        character->jump_counter = CHAR_JUMP_DURATION; // Force jump when moving up to next room to prevent immediate fall back down 
    }
    if (new_y > 240) { 
        current_room_index[0]++; 
        change_room();
        new_y = 0; 
    }

    // update old position ( for background drawing)
    character->prev_x = character->x;
    character->prev_y = character->y;
    
    // update position
    character->x = new_x;
    character->y = new_y;

    // update character state
    cat_state(character, move_x, move_y);
    
    
}

// Draw character sprite based on current state and animation frame
void Character_Draw(Character* character) {
    
    int16_t x_pos, y_pos;

    if (character->state == NPC_IDLE || character->state == NPC_WALKING) {
        // NPCs are smaller, so adjust position for 16x16 sprite
        x_pos = character->x - 8;  
        y_pos = character->y - 8;
        character->frame_counter++;
        character->animation_frame = (character->frame_counter / ANIMATION_FRAME_RATE) % 2; // 2 frames per animation cycle
    } else {
        // Player character uses 32x32 sprite
        if (character->direction == 1) {
            // facing right, adjust position to left for better collision feel
            x_pos = character->x - 22;  
        } else {
            // facing left, adjust position to right for better collision feel
            x_pos = character->x - 10;  
        }
        y_pos = character->y - 19;
        character->frame_counter++;
        character->animation_frame = (character->frame_counter / ANIMATION_FRAME_RATE) % 4; // 4 frames per animation cycle
    }
    
    
    switch (character->state) {
        case CHAR_IDLE:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle4, character->direction == 1 ? 1 : 0);
            }
            break;
        
        case CHAR_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk4, character->direction == 1 ? 1 : 0);
            }
            break;
        
        case CHAR_DASHING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, character->direction == 1 ? 1 : 0);
            }
            break;

        case CHAR_JUMPING:
            LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, character->direction == 1 ? 1 : 0);
            break;

        case CHAR_FALLING:
            LCD_Draw_Sprite_directional(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, character->direction == 1 ? 1 : 0);
            break;    


        // NPC states    
        case NPC_IDLE:
            LCD_Draw_Sprite_directional(x_pos, y_pos, 16, 16, (uint8_t*)mouse_sat, npc_direction == 1 ? 1 : 0);
            break;
        case NPC_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 16, 16, (uint8_t*)mouse_walk1, npc_direction == 1 ? 1 : 0);
            } 
            else {
                LCD_Draw_Sprite_directional(x_pos, y_pos, 16, 16, (uint8_t*)mouse_walk2, npc_direction == 1 ? 1 : 0);
            }
            break;
    }
}

// Calls Character_Update with current joystick input and button states
void update_character(Joystick_t* joy) {
    // Check if dash or jump was pressed and clear the flag
    uint8_t dash_pressed = dash_button_pressed;
    dash_button_pressed = 0;

    uint8_t jump_pressed = jump_button_pressed;
    jump_button_pressed = 0;
    
    // handle character stats
    if (game_character.food > 0) {
        game_character.food--; // Decrease food over time
    } else {
        if (game_character.health > 0) {
            game_character.health--; // Decrease health if food is depleted
        }
    }


    // Check for day progression
    // check if player collides with sleep point
    if (current_room_index[0] == sleep_point.room_index[0] && current_room_index[1] == sleep_point.room_index[1]) {
        if (collision(game_character.x, game_character.y,
                      game_character.width, game_character.height, 
                      sleep_point.x+16, sleep_point.y+16,
                      sleep_point.width, sleep_point.height)) {
            if (game_character.food > 700) {            
                if (game_character.health < 1000) {
                    // Sleep to restore health
                    game_character.health = 1000; // Fully restore health
                }
                game_character.food = 500; // Consume some food to sleep
                day_counter++;
                if (day_counter < 100) {
                    new_day();
                    change_room(); // reload room to reset NPC and blocks
                }
            }
        }
    }

    // Update character FSM with current input
    Character_Update(&game_character, joy, dash_pressed, jump_pressed);
}

void cat_state(Character* character, int16_t move_x, int16_t move_y) {
    // ===== Update state (IDLE, WALKING, DASHING) =====
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    
    if (character->dash_counter > 0 && move_x == 1) {
        character->state = CHAR_DASHING;
        character->direction = 1;
    } else if (character->dash_counter > 0 && move_x == -1) {
        character->state = CHAR_DASHING;
        character->direction = -1;
    } else if (character->jump_counter > 0) {
        character->state = CHAR_JUMPING;
        if (move_x == 1) {character->direction = 1;}
        else if (move_x == -1) {character->direction = -1;}
        else { /* do nothing - direction stays the same as prev */ }
    } else if (character->y > character->prev_y) {
        character->state = CHAR_FALLING;
        if (move_x == 1) {character->direction = 1;}
        else if (move_x == -1) {character->direction = -1;}
        else { /* do nothing - direction stays the same as prev */ }
    } else if (is_moving && move_x == 1) {
        character->state = CHAR_WALKING;
        character->direction = 1;
    } else if (is_moving && move_x == -1) {
        character->state = CHAR_WALKING;
        character->direction = -1;
    } else {
        character->state = CHAR_IDLE;
    }
}


// ===== NPC CHARACTER FUNCTIONS =====

// Initialize NPC in free tile with default state
void NPC_init(Character* npc) {

    for (uint8_t attempt = 0; attempt < 300; attempt++) { // try 300 times to find an empty tile to spawn npc
        uint8_t i, j;
        i = rand() % 12 + 2; // random row other than outer wall
        j = rand() % 12 + 2; // random column other than outer wall

        if (current_room->tiles[i][j] == 0 && current_room->tiles[i+1][j] >0 ) // find an empty tile to spawn npc with a solid block underneath
        {
            npc->x = j * 16 + 8;       // Calculate npc's x centre position ( 16 x 16 pixel sprite)
            npc->y = i * 16;       // Calculate npc's y centre position
            break;
        }
    }

    npc->prev_x = npc->x;
    npc->prev_y = npc->y;
    npc->direction = 1; // Start facing right
    npc->state = NPC_IDLE;
    npc->animation_frame = 0;
    npc->frame_counter = 0;
    npc->dash_counter = 0;
    npc->jump_counter = 0;
    npc->width = 16;              
    npc->height = 16;
    
    npc_move_counter = 10; // start in idle state for 10 frames before moving
    npc_direction = 0; // start idle
}

// Update NPC position and state based on random movement logic
void NPC_Update(Character* npc, uint8_t x) {
    
    
    int8_t move_x = x;
    int8_t move_y = 0;

    int16_t new_x = npc->x + (move_x * NPC_SPEED);
    int16_t new_y = npc->y + (move_y * NPC_SPEED) - GRAVITY;
    
    // Handle collisions
    //Check if NPC overlaps with any objects
    for (uint8_t i = 0; i < 15; i++)           // runs for size of the room - [15][15] blocks
    {   for (uint8_t j = 0; j < 15; j++)
        {
            if (current_room->tiles[i][j] > 0) // if the tile is not empty, check for collision with NPC
            {
                block current_block;
                current_block.x = j * 16 + 8;       // Calculate block's x centre position ( 16 x 16 pixel sprite)
                current_block.y = i * 16 + 8;       // Calculate block's y centre position
                current_block.width = 16;       // Block width
                current_block.height = 16;      // Block height

                // handle x direction collisions
                if (collision(new_x, npc->y,
                              npc->width, npc->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    // interacting from left
                    if (move_x > 0) {
                        new_x = current_block.x - (current_block.width + npc->width) / 2 - 1; // Place npc just to the right of block
                        new_y = npc->y - 4;     // small jump to get over obstacle
                    }
                    // interacting from right
                    else if (move_x < 0) {
                        new_x = current_block.x + (current_block.width + npc->width) / 2 + 1; // Place npc just to the left of block
                        new_y = npc->y - 4;     // small jump to get over obstacle
                    }
                    break;
                }
                // handle y direction collisions
                if (collision(new_x, new_y,
                              npc->width, npc->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    // falling down onto block
                    if (move_y >= 0) {
                        new_y = current_block.y - (current_block.height + npc->height) / 2 - 1; // Place npc on top of block
                    }
                    // hitting head on block
                    else if (move_y < 0) {
                        new_y = current_block.y + (current_block.height + npc->height) / 2 + 1; // Place npc just below block
                    }
                    break;
                }

        
                }
            }
        }
        // check for collision with player character
                if (collision(new_x, new_y,
                              npc->width, npc->height, 
                              game_character.x, game_character.y,
                              game_character.width, game_character.height)) {
                    
                    // Collision with player = remove npc and restore some player food
                    is_npc = 0; 
                    game_character.food = (game_character.food + 200 > 1000) ? 1000 : game_character.food + 200; // restore some food
    }




    // despawn if npc goes beyond screen edges (instead of transitioning to next room like player)
    if (new_x < 0){is_npc = 0;}
    if (new_x > 240) { is_npc = 0;} 
    if (new_y < 10) {is_npc = 0;}
    if (new_y > 240) {is_npc = 0;}

    // update old position
    npc->prev_x = npc->x;
    npc->prev_y = npc->y;
    
    // update position
    npc->x = new_x;
    npc->y = new_y;

    // ===== Update state (IDLE, WALKING, DASHING) =====
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    
    if (is_moving && move_x == 1) {
        npc->state = NPC_WALKING;
        npc->direction = 1;
    } else if (is_moving && move_x == -1) {
        npc->state = NPC_WALKING;
        npc->direction = -1;
    } else {
        npc->state = NPC_IDLE;
    }

}

// Generates random movement for NPC. 
// Changes directions when npc_move_counter reaches 0
// The direction is randomly chosen to be left, right, or stationary. 
// The NPC_Update function is then called to apply this movement logic and update the NPC's position and state accordingly.
void update_npc() {

    
    if (npc_move_counter == 0) {
        // detemine direction for NPC. Generate -1, 0, or 1
        npc_direction = rand() % 3 - 1; 
        // determine number of frames to keep moving in this direction (between 1 and 20)
        npc_move_counter = rand() % 20 + 1;
    }
    
    // Decrement movement counter each frame. When it reaches 0, a new direction will be chosen on the next update.
    npc_move_counter--;
    // Update NPC with current direction. The NPC_Update function will handle the actual movement and state changes based on this direction input.
    //int8_t npc_move_x = npc_direction;

    NPC_Update(&npc_character, npc_direction);
}