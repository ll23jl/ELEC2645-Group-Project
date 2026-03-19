#ifndef GAME_1_H
#define GAME_1_H

#include "Menu.h"
#include <stdint.h>
#include "Joystick.h"
#include "LCD.h"

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


// =========================================================== Character definitions ===========================================================

// Character states
typedef enum {
    CHAR_IDLE = 0,      // Not moving
    CHAR_WALKING,       // Moving in a direction
    CHAR_DASHING,        // Fast movement
    CHAR_JUMPING        // Jumping
} CharacterState_1;

// Character structure
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    CharacterState_1 state;         // Current state
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Counter for animation timing
    uint8_t dash_counter;           // Frames remaining in dash
    uint8_t jump_counter;           // Frames remaining in jump
} Character_1;

// Character constants
#define CHAR_SPEED 5                // Pixels per frame (normal)
#define CHAR_DASH_SPEED 10          // Pixels per frame (dashing)
#define CHAR_JUMP_SPEED 8           // Pixels per frame (jumping)
#define CHAR_DASH_DURATION 30       // Frames (dash lasts this long)
#define CHAR_JUMP_DURATION 5       // Frames (jump lasts this long)
#define GRAVITY -10

// Character function prototypes
void Character_Init(Character_1* character);

void Character_Update(Character_1* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed);

void Character_Draw(Character_1* character);

void update_character(Joystick_t* joy);

void render_game(void);

MenuState Game1_Run(void);

#endif // GAME_1_H
