#include "Game_1.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Menu.h"
#include "LCD.h"
#include "sprites.h"
#include "rooms.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32_hal_legacy.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; // Joystick configuration
extern Joystick_t joystick_data; // Joystick data structure


// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

// frame rate for character animation (in frames)
#define ANIMATION_FRAME_RATE 3  // Change sprite every 3 frames

// ===== UTILITY FUNCTIONS =====

// ===== CHARACTER FSM VARIABLES =====

// Global character object
Character game_character;

// Global NPC character
Character npc_character;

// NPC movement counter
int8_t npc_move_counter = 0;
int8_t npc_direction = 0; // -1 for left, 0 for idle, 1 for right

// Dash button state
volatile uint8_t dash_button_pressed = 0;

// Jump button state
volatile uint8_t jump_button_pressed = 0;

// Last debounce timestamp for dash button
volatile uint32_t dash_button_last_interrupt_time = 0;

// Last debounce timestamp for jump button
volatile uint32_t jump_button_last_interrupt_time = 0;

// Debounce delays in milliseconds - prevents multiple triggers from single button press
#define DEBOUNCE_DELAY_DASH 200
#define DEBOUNCE_DELAY_JUMP 500





// ===== MAIN GAME LOOP =====
MenuState Game1_Run(void) {
    // Initialize game state
    LCD_Set_Palette(PALETTE_CUSTOM); 

    // Initialize Character
    Character_Init(&game_character);
    
    // make screen black
    LCD_Fill_Buffer(0);
    LCD_Refresh(&cfg0);

    // Set initial room
    current_room = &room_1;
    current_room_index[0] = 1;
    current_room_index[1] = 1; // Start in centre room
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // Game's own loop - runs until exit condition
    while (1) {
        uint32_t frame_start = HAL_GetTick();
        
        // Read input
        Input_Read();
        
        // Check if button was pressed to return to menu
        if (current_input.btn2_pressed) {
            PWM_SetDuty(&pwm_cfg, 50);  // Reset LED to 50% when returning
            exit_state = MENU_STATE_HOME;
            break;  // Exit game loop
        }
        
        // UPDATE: Game logic                
                
        // Read joystick input
        Joystick_Read(&joystick_cfg, &joystick_data);

        // Check if button was pressed to dash
        if (current_input.btn3_pressed) {
            uint32_t current_time = HAL_GetTick();
            if ((current_time - dash_button_last_interrupt_time) > DEBOUNCE_DELAY_DASH)
            {
                dash_button_last_interrupt_time = current_time;
      
                // Set flag to trigger dash in the character FSM
                dash_button_pressed = 1;
            }
        }

        // Check if button was pressed to jump
        if (current_input.btn4_pressed) {
            uint32_t current_time = HAL_GetTick();
            if ((current_time - jump_button_last_interrupt_time) > DEBOUNCE_DELAY_JUMP)
            {
                jump_button_last_interrupt_time = current_time;
      
                // Set flag to trigger jump in the character FSM
                jump_button_pressed = 1;
            }
        }
        
        // Update character FSM (logic only)
        update_character(&joystick_data);

        // Update NPC logic if npc is present in the room
        if(is_npc) {
            update_npc();
        }
        
        // Render everything to screen
        render_game();
          
        
        // Refresh LCD with new frame
        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }
    
    return exit_state;  // Tell main where to go next
}

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
    character->width = 20;              // - adjusted for better collision feel (smaller than actual 32x32 sprite)
    character->height = 20;             // - adjusted for better collision feel (smaller than actual 32x32 sprite)
    character->health = 500;            // Start with half health
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
    if (jump_pressed && character->jump_counter == 0) {
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
    if (new_y < 0) {  
        current_room_index[0]--; 
        change_room();
        new_y = 240; 
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

    
    // ===== Update state (IDLE, WALKING, DASHING) =====
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    
    if (character->dash_counter > 0 && move_x == 1) {
        character->state = CHAR_DASHING;
        character->direction = 1;
    } else if (character->dash_counter > 0 && move_x == -1) {
        character->state = CHAR_DASHING;
        character->direction = -1;
    } else if (character->jump_counter > 0 && move_x == 1) {
        character->state = CHAR_JUMPING;
        character->direction = 1;
    } else if (character->jump_counter > 0 && move_x == -1) {
        character->state = CHAR_JUMPING;
        character->direction = -1;
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
        x_pos = character->x - 16;
        y_pos = character->y - 16;
        character->frame_counter++;
        character->animation_frame = (character->frame_counter / ANIMATION_FRAME_RATE) % 4; // 4 frames per animation cycle
    }
    
    
    switch (character->state) {
        case CHAR_IDLE:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle4, character->direction == 1 ? 1 : 0);
            }
            break;
        
        case CHAR_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk4, character->direction == 1 ? 1 : 0);
            }
            break;
        
        case CHAR_DASHING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run1, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run2, character->direction == 1 ? 1 : 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run3, character->direction == 1 ? 1 : 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, character->direction == 1 ? 1 : 0);
            }
            break;

        case CHAR_JUMPING:
            LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, character->direction == 1 ? 1 : 0);
            break;


        // NPC states    
        case NPC_IDLE:
            LCD_Draw_Sprite(x_pos, y_pos, 16, 16, (uint8_t*)mouse_sat, npc_direction == 1 ? 1 : 0);
            break;
        case NPC_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 16, 16, (uint8_t*)mouse_walk1, npc_direction == 1 ? 1 : 0);
            } 
            else {
                LCD_Draw_Sprite(x_pos, y_pos, 16, 16, (uint8_t*)mouse_walk2, npc_direction == 1 ? 1 : 0);
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

    // increase health if food is above 50%
    if (game_character.food > 500 && game_character.health < 1000) {
        game_character.health++; // Regenerate health if food is sufficient
    }

    // Update character FSM with current input
    Character_Update(&game_character, joy, dash_pressed, jump_pressed);
}



// ===== NPC CHARACTER FUNCTIONS =====

// Initialize NPC at screen center with default state
void NPC_init(Character* npc) {
    npc->x = rand() % 240; // Random x position within screen bounds
    npc->y = rand() % 240; // Random y position within screen bounds
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
                    new_x = npc->x;
                    new_y = npc->y - 4;     // small jump to get over obstacle
                    break;
                }
                // handle y direction collisions
                if (collision(new_x, new_y,
                              npc->width, npc->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    new_y = npc->y;
                    break;
                }

                // check for collision with player character
                if (collision(new_x, new_y,
                              npc->width, npc->height, 
                              game_character.x, game_character.y,
                              game_character.width, game_character.height)) {
                    
                    // Collision with player = remove npc and restore some player food
                    is_npc = 0; 
                    game_character.food = (game_character.food + 200 > 1000) ? 1000 : game_character.food + 200; // restore some food
                    break;
                }
            }
        }
    }


    // keep within the room
    if (new_x < 5) {new_x = 5;}
    if (new_x > 230) { new_x = 230;} 
    if (new_y < 0) {new_y = 0;}
    if (new_y > 230) {new_y = 230;}

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
// Changes directions when npc_move_counter reaches 0, which creates a delay between direction changes for smoother movement. 
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

// ===== RENDERING FUNCTION =====

// Render the game and character state to the LCD
void render_game(void) {
    // Clear screen buffer
    LCD_Fill_Buffer(0);

    // Draw environment
    render_blocks();
    
    // Draw character at current position with animation
    Character_Draw(&game_character);

    // Draw NPC if present in the room
    if(is_npc) {
        Character_Draw(&npc_character);
    }

    // Display character stats info
    LCD_printString("Health:", 10, 5, 1, 1);
    char health_str[6];
    // display health as percentage of max health (1000) to keep it within 3 digits for display
    sprintf(health_str, "%d", (game_character.health * 100) / 1000);
    LCD_printString(health_str, 60, 5, 1, 2);
    
    LCD_printString("Food:", 170, 5, 1, 1);
    char food_str[6];
    // display food as percentage of max food (1000) to keep it within 3 digits for display
    sprintf(food_str, "%d", (game_character.food * 100) / 1000);
    LCD_printString(food_str, 210, 5, 1, 2);
    
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
}
