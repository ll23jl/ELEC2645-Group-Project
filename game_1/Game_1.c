#include "Game_1.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Menu.h"
#include "LCD.h"
#include "sprites.h"
#include "rooms.h"
#include "Character.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32_hal_legacy.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_intsup.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control
extern Joystick_cfg_t joystick_cfg; // Joystick configuration
extern Joystick_t joystick_data; // Joystick data structure



// ===== CHARACTER FSM VARIABLES =====

// Global character object
Character game_character;

// Global NPC character
Character npc_character;


// Other variables
uint8_t day_counter;

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

    instruction(); // Show instructions at the start of the game

    // Initialize Character
    Character_Init(&game_character);
    
    // make screen black
    LCD_Fill_Buffer(0);
    LCD_Refresh(&cfg0);

    // Set initial room
    current_room_index[0] = 1;
    current_room_index[1] = 1;      // Start in centre room
    change_room(); // Load initial room data and NPC state

    // Initial game states
    day_counter = 1; // Start at day 1
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_tone(&buzzer_cfg, 2000, 30);  // 1kHz at 30% volume
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

            buzzer_tone(&buzzer_cfg, 1200, 30);  // 1.2kHz at 30% volume
            HAL_Delay(50);  // Brief beep duration
            buzzer_off(&buzzer_cfg);  // Stop the buzzer  

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

        // lose condition: health reaches 0
        if(game_character.health == 0) {
            game_over();
            // go back to menu after game over screen
            exit_state = MENU_STATE_HOME;

            break;  // Exit game loop
        }

        // win condition: survive 100 days
        if(day_counter > 99) {
            // player wins the game
            game_win();
            // go back to menu after game over screen
            exit_state = MENU_STATE_HOME;

            break;  // Exit game loop
        }
    }
    
    return exit_state;  // Tell main where to go next
}



// ===== GAME FUNCTIONS =====

// Render the game and character state to the LCD
void render_game(void) {
    // Clear screen buffer
    LCD_Fill_Buffer(0);

    // Draw environment
    render_blocks(&sleep_point);
    
    // Draw character at current position with animation
    Character_Draw(&game_character);

    // Draw NPC if present in the room
    if(is_npc) {
        Character_Draw(&npc_character);
    }

    // Display character stats info
    LCD_printString("Health:", 10, 5, 8, 1);
    char health_str[6];
    // display health as percentage of max health (1000) to keep it within 3 digits for display
    sprintf(health_str, "%d", (game_character.health * 100) / 1000);
    LCD_printString(health_str, 55, 5, 8, 2);

    LCD_printString("Day:", 105, 5, 8, 1);
    // display day counter
    char day_str[6];
    sprintf(day_str, "%d", (day_counter));
    LCD_printString(day_str, 130, 5, 8, 2);
    
    LCD_printString("Food:", 175, 5, 8, 1);
    char food_str[6];
    // display food as percentage of max food (1000) to keep it within 3 digits for display
    sprintf(food_str, "%d", (game_character.food * 100) / 1000);
    LCD_printString(food_str, 210, 5, 8, 2);
    
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
}

// Display new day screen and pause briefly to show day progression
void new_day(void){
    // Clear screen buffer
    LCD_Fill_Buffer(0);
    LCD_printString("Day", 65, 110, 8, 4);
    char day_str[6];
    sprintf(day_str, "%d", (day_counter));
    LCD_printString(day_str, 145, 110, 8, 4);
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
    HAL_Delay(1500);
}

// Display instructions screen at the start of the game and wait for player to press button to start the game
void instruction(void) {
    // Clear screen buffer
    LCD_Fill_Buffer(0);
    LCD_printString("Instructions:", 45, 20, 8, 2);
    LCD_printString("Use joystick to move", 60, 50, 8, 1);
    LCD_printString("Press joystick to Dash", 56, 70, 8, 1);
    LCD_printString("Press left button to Jump", 49, 90, 8, 1);
    LCD_printString("Eat mice to increase food", 49, 110, 8, 1);
    LCD_printString("Sleep on pillow to progress day", 33, 130, 8, 1);
    LCD_printString("Press joystick to start the game", 28, 150, 8, 1);
    LCD_printString("Press right button to return to menu", 13, 170, 8, 1);
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
    while(1){ 
        Input_Read();
        if (current_input.btn3_pressed) {
            break;  // Exit instruction screen on button press
        }
    }
}

// Display game over screen when player loses and pause briefly before returning to menu
void game_over(void){
    // Clear screen buffer
    LCD_Fill_Buffer(0);
    LCD_printString("Game Over", 15, 110, 8, 4);
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
    HAL_Delay(2000);
}

// Display win screen when player wins and pause briefly before returning to menu
void game_win(void){
    // Clear screen buffer
    LCD_Fill_Buffer(0);
    LCD_printString("You Win!", 30, 110, 8, 4);
    // Refresh LCD to display this frame
    LCD_Refresh(&cfg0);
    HAL_Delay(2000);
}