#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"
#include <stdint.h>
#include "Joystick.h"
#include "LCD.h"
#include "Character.h"

/**
 * @brief Game 1 - Student can implement their own game here
 * 
 * Placeholder for Student 1's game implementation.
 * This structure allows multiple students to work on separate games
 * while sharing common utilities from the shared/ folder.
 * 
 * The menu system calls this function when Game 1 is selected.
 * The function runs its own loop and returns when the game exits.
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME for menu)
 */




// Global character instances
extern Character game_character;  // Player character
extern Character npc_character;   // NPC character

// Global variable
extern uint8_t day_counter; // Tracks the current day in the game
// Dash button state
extern volatile uint8_t dash_button_pressed;

// Jump button state
extern volatile uint8_t jump_button_pressed;

// Game function prototypes

void instruction(void);

void new_day(void);

void render_game(void);

void game_over(void);

void game_win(void);

MenuState Game1_Run(void);


#endif // GAME_1_H
