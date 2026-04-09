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
Character_1 game_character;

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

//Get character state name
const char* get_char_state_name(CharacterState_1 state) {
    switch (state) {
        case CHAR_IDLE:    return "IDLE";
        case CHAR_WALKING_L: return "WALK L";
        case CHAR_WALKING_R: return "WALK R";
        case CHAR_DASHING_L: return "DASH L";
        case CHAR_DASHING_R: return "DASH R";
        case CHAR_JUMPING_L: return "JUMP L";
        case CHAR_JUMPING_R: return "JUMP R";
        default:           return "???";
    }
}



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

// ===== CHARACTER FUNCTIONS =====

// Initialize character at screen center
void Character_Init(Character_1* character) {
    character->x = 120;
    character->y = 120;
    character->prev_x = 120;
    character->prev_y = 120;
    character->state = CHAR_IDLE;
    character->animation_frame = 0;
    character->frame_counter = 0;
    character->dash_counter = 0;
    character->jump_counter = 0;
    character->width = 32;
    character->height = 32;
}

// Update character position and state based on joystick input and button presses
void Character_Update(Character_1* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed) {
    
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
            if (current_room->tiles[i][j] == 1) // if there is a block in the space
            {
                block current_block;
                current_block.x = j * 16;     // Calculate block's x centre position
                current_block.y = i * 16;     // Calculate block's y centre position
                current_block.width = 10;       // Block width
                current_block.height = 10;      // Block height

                // handle x direction collisions
                if (collision(new_x, character->y,
                              character->width, character->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    new_x = character->x;
                    break;
                }
                // handle y direction collisions
                if (collision(new_x, new_y,
                              character->width, character->height, 
                              current_block.x, current_block.y,
                              current_block.width, current_block.height)) {
                    
                    // Collision detected - cancel movement
                    new_y = character->y;
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
        character->state = CHAR_DASHING_R;
    } else if (character->dash_counter > 0 && move_x == -1) {
        character->state = CHAR_DASHING_L;
    } else if (character->jump_counter > 0 && move_x == 1) {
        character->state = CHAR_JUMPING_R;
    } else if (character->jump_counter > 0 && move_x == -1) {
        character->state = CHAR_JUMPING_L;
    } else if (is_moving && move_x == 1) {
        character->state = CHAR_WALKING_R;
    } else if (is_moving && move_x == -1) {
        character->state = CHAR_WALKING_L;
    } else {
        character->state = CHAR_IDLE;
    }
    
}

// Draw character sprite based on current state and animation frame
void Character_Draw(Character_1* character) {
    
    int16_t x_pos = character->x - 16;  // 32x32 sprite 
    int16_t y_pos = character->y - 16;

    character->frame_counter++;
    character->animation_frame = (character->frame_counter / ANIMATION_FRAME_RATE) % 4; // 4 frames per animation cycle
    
    switch (character->state) {
        case CHAR_IDLE:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle1, 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle2, 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle3, 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_idle4, 0);
            }
            break;
        
        case CHAR_WALKING_L:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk1, 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk2, 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk3, 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk4, 0);
            }
            break;

        case CHAR_WALKING_R:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk1, 1);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk2, 1);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk3, 1);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_walk4, 1);
            }
            break;
        
        case CHAR_DASHING_L:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run1, 0);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run2, 0);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run3, 0);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, 0);
            }
            break;

        case CHAR_DASHING_R:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run1, 1);
            } else if (character->animation_frame == 1) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run2, 1);
            } else if (character->animation_frame == 2) {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run3, 1);
            } else {
                LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, 1);
            }
            break;

        case CHAR_JUMPING_R:
            LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, 1);
            break;

        case CHAR_JUMPING_L:
            LCD_Draw_Sprite(x_pos, y_pos, 32, 32, (uint8_t*)cat_run4, 0);
            break;
    }
}

// Render the game and character state to the LCD
void render_game(void) {
    // Clear screen buffer
    LCD_Fill_Buffer(0);

    // Draw environment
    render_blocks();
    
    // Draw character at current position with animation
    Character_Draw(&game_character);
    
    // Draw debug info
    LCD_printString("St:", 10, 5, 1, 2);
    LCD_printString((char*)get_char_state_name(game_character.state), 44, 5, 1, 2);
    
    char pos_str[24];
    sprintf(pos_str, "X:%d Y:%d", game_character.x, game_character.y);
    LCD_printString(pos_str, 120, 5, 1, 2);
    
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
}

// Calls Character_Update with current joystick input and button states
void update_character(Joystick_t* joy) {
    // Check if dash was pressed and clear the flag
    uint8_t dash_pressed = dash_button_pressed;
    dash_button_pressed = 0;

    uint8_t jump_pressed = jump_button_pressed;
    jump_button_pressed = 0;
    
    // Update character FSM with current input
    Character_Update(&game_character, joy, dash_pressed, jump_pressed);
}


