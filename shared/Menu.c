#include "Menu.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include "sprites.h"
#include "Game_3.h"
#include "Buzzer.h"

extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

// Menu options
static const char* menu_options[] = {
    "Mouse Chase",
    "Game 2", 
    "Beat Escape"
};
#define NUM_MENU_OPTIONS 3

// Frame rate for menu (in milliseconds)
#define MENU_FRAME_TIME_MS 30  // ~33 FPS

// Sprite animations
uint8_t m_frame_counter = 0;
uint8_t m_animation_frame = 0;

/**
 * @brief Render the home menu screen
 */
static void render_home_menu(MenuSystem* menu) {
    LCD_Fill_Buffer(0);
    
    // Title
    LCD_printString("MAIN MENU", 50, 10, 1, 3);
    
    // Menu options with selection highlight
    for (int i = 0; i < NUM_MENU_OPTIONS; i++) {
        uint16_t y_pos = 70 + (i * 40);
        uint8_t text_size = 2;
        
        if (i == menu->selected_option) {
            // Highlight selected option with inverted colors
            // Draw a rectangle around selected option
            // We'll use simple marker instead
            LCD_printString(">", 50, y_pos, 1, text_size);  // Arrow pointing to selection
        }
        
        LCD_printString((char*)menu_options[i], 70, y_pos, 1, text_size);

    }

    if (menu->selected_option == 0){
            LCD_Set_Palette(PALETTE_CUSTOM);
            if (m_animation_frame == 0) {
                LCD_Draw_Sprite_directional(10, 54, 32, 32, (uint8_t*)cat_run1, 0);
            } else if (m_animation_frame == 1) {
                LCD_Draw_Sprite_directional(10, 54, 32, 32, (uint8_t*)cat_run2, 0);
            } else if (m_animation_frame == 2) {
                LCD_Draw_Sprite_directional(10, 54, 32, 32, (uint8_t*)cat_run3, 0);
            } else {
                LCD_Draw_Sprite_directional(10, 53, 32, 32, (uint8_t*)cat_run4, 0);
            }
    }
    else if (menu->selected_option == 1){
            LCD_Set_Palette(PALETTE_GREYSCALE);
            if (m_animation_frame == 0) {
                LCD_Draw_Sprite_directional(10, 94, 32, 32, (uint8_t*)cat_run1, 0);
            } else if (m_animation_frame == 1) {
                LCD_Draw_Sprite_directional(10, 94, 32, 32, (uint8_t*)cat_run2, 0);
            } else if (m_animation_frame == 2) {
                LCD_Draw_Sprite_directional(10, 94, 32, 32, (uint8_t*)cat_run3, 0);
            } else {
                LCD_Draw_Sprite_directional(10, 94, 32, 32, (uint8_t*)cat_run4, 0);
            }
    }
    else if (menu->selected_option == 2){
            LCD_Set_Palette(PALETTE_DEFAULT);
            if (m_animation_frame == 0) {
                LCD_Draw_Sprite_directional(8, 144, 24, 24, (uint8_t*)player_sprite, 0);
            } else if (m_animation_frame == 1) {
                LCD_Draw_Sprite_directional(11, 144, 24, 24, (uint8_t*)player_sprite, 0);
            } else if (m_animation_frame == 2) {
                LCD_Draw_Sprite_directional(14, 144, 24, 24, (uint8_t*)player_sprite, 1);
            } else {
                LCD_Draw_Sprite_directional(11, 144, 24, 24, (uint8_t*)player_sprite, 1);
            }
    }
    else {
            
        }
    
    // Instructions
    LCD_printString("Press BT3", 50, 240, 1, 1);
    
    LCD_Refresh(&cfg0);
}

// ==============================================
// PUBLIC API IMPLEMENTATION
// ==============================================

void Menu_Init(MenuSystem* menu) {
    menu->selected_option = 0;
}

MenuState Menu_Run(MenuSystem* menu) {
    static Direction last_direction = CENTRE;  // Track last direction for debouncing
    MenuState selected_game = MENU_STATE_HOME;  // Which game was selected

    LCD_Set_Palette(PALETTE_DEFAULT); 
    
    buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_tone(&buzzer_cfg, 2000, 30);  // 1kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer

    // Menu's own loop - runs until game is selected
    while (1) {
        uint32_t frame_start = HAL_GetTick();
        
        // Read input
        Input_Read();
        
        // Read current joystick position
        Joystick_Read(&joystick_cfg, &joystick_data);
        
        // Handle joystick navigation (up/down to select option)
        Direction current_direction = joystick_data.direction;
        
        if (current_direction == S && last_direction != S) {  // Joystick pushed DOWN
            // Move selection down
            buzzer_tone(&buzzer_cfg, 1200, 30);  // 1.2kHz at 30% volume
            HAL_Delay(50);  // Brief beep duration
            buzzer_off(&buzzer_cfg);  // Stop the buzzer
            menu->selected_option++;
            if (menu->selected_option >= NUM_MENU_OPTIONS) {
                menu->selected_option = 0;  // Wrap around
            }
        } 
        else if (current_direction == N && last_direction != N) {  // Joystick pushed UP
            // Move selection up
            buzzer_tone(&buzzer_cfg, 1200, 30);  // 1.2kHz at 30% volume
            HAL_Delay(50);  // Brief beep duration
            buzzer_off(&buzzer_cfg);  // Stop the buzzer
            if (menu->selected_option == 0) {
                menu->selected_option = NUM_MENU_OPTIONS - 1;  // Wrap around
            } else {
                menu->selected_option--;
            }
        }
        
        last_direction = current_direction;
        
        // Handle button press to select current option
        if (current_input.btn3_pressed) {
            // User pressed button - select the highlighted option
            buzzer_tone(&buzzer_cfg, 1000, 30);  // 1kHz at 30% volume
            HAL_Delay(50);  // Brief beep duration
            buzzer_tone(&buzzer_cfg, 2000, 30);  // 1kHz at 30% volume
            HAL_Delay(50);  // Brief beep duration
            buzzer_off(&buzzer_cfg);  // Stop the buzzer
            if (menu->selected_option == 0) {
                selected_game = MENU_STATE_GAME_1;
            } else if (menu->selected_option == 1) {
                selected_game = MENU_STATE_GAME_2;
            } else if (menu->selected_option == 2) {
                selected_game = MENU_STATE_GAME_3;
            }
            break;  // Exit menu loop - game selected!
        }
        
    

        // Render menu
        render_home_menu(menu);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < MENU_FRAME_TIME_MS) {
            HAL_Delay(MENU_FRAME_TIME_MS - frame_time);
        }

        m_frame_counter ++;
        m_animation_frame = (m_frame_counter/4) % 4;
    }
    
    return selected_game;  // Return which game was selected
}
