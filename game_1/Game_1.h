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

// Constants
#define TARGET_COUNT 5

// Character constants
#define CHAR_SPEED 5                // Pixels per frame (normal)
#define CHAR_DASH_SPEED 10          // Pixels per frame (dashing)
#define CHAR_JUMP_SPEED 8           // Pixels per frame (jumping)
#define CHAR_DASH_DURATION 30       // Frames (dash lasts this long)
#define CHAR_JUMP_DURATION 5       // Frames (jump lasts this long)
#define GRAVITY -10

// Character states
typedef enum {
    CHAR_IDLE = 0,      // Not moving
    CHAR_WALKING_L,       // Moving left
    CHAR_WALKING_R,       // Moving right
    CHAR_DASHING_L,        // Fast movement left
    CHAR_DASHING_R,        // Fast movement right
    CHAR_JUMPING_L,        // Jumping left
    CHAR_JUMPING_R         // Jumping right

} CharacterState_1;

// Character structure
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    int16_t prev_x;                 // Previous X position
    int16_t prev_y;                 // Previous Y position
    CharacterState_1 state;         // Current state
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Counter for animation timing
    uint8_t dash_counter;           // Frames remaining in dash
    uint8_t jump_counter;           // Frames remaining in jump
    uint16_t radius;                // Collision radius
} Character_1;

// Block structure
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    uint16_t radius;                // Collision radius
}Block;

Block Blocks [TARGET_COUNT];



// Character function prototypes
void Character_Init(Character_1* character);

void Character_Update(Character_1* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed);

void Character_Draw(Character_1* character);

void update_character(Joystick_t* joy);

void render_game(void);

uint8_t collision(uint16_t x1, uint16_t y1, uint16_t r1, uint16_t x2, uint16_t y2, uint16_t r2);

MenuState Game1_Run(void);

void render_blocks(void);

#endif // GAME_1_H
