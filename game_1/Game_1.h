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

// Character constants
#define CHAR_SPEED 5                // Pixels per frame (normal)
#define CHAR_DASH_SPEED 10          // Pixels per frame (dashing)
#define CHAR_JUMP_SPEED 5           // Pixels per frame (jumping)
#define CHAR_DASH_DURATION 30       // Frames (dash lasts this long)
#define CHAR_JUMP_DURATION 5        // Frames (jump lasts this long)
#define GRAVITY -10
#define jump_height -4

// NPC constants
#define NPC_SPEED 3                 // Pixels per frame

// Character states
typedef enum {

    // Player states
    CHAR_IDLE,              // Not moving
    CHAR_WALKING,           // Moving
    CHAR_DASHING,           // Fast movement
    CHAR_JUMPING,           // Jumping

    // NPC states
    NPC_IDLE,               // Not moving
    NPC_WALKING,            // Moving
} CharacterState;

// Character structure
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    int16_t prev_x;                 // Previous X position
    int16_t prev_y;                 // Previous Y position
    int8_t direction;               // -1 for left, 1 for right
    CharacterState state;           // Current state
    uint8_t animation_frame;        // frame index for animation
    uint8_t frame_counter;          // Counter for animation timing
    uint8_t dash_counter;           // Frames remaining in dash
    uint8_t jump_counter;           // Frames remaining in jump
    uint8_t width;                  // Collision width
    uint8_t height;                 // Collision height
    uint16_t health;                // Health points 
    uint16_t food;                  // Hunger level 
} Character;

// Global character instances
extern Character game_character;  // Player character
extern Character npc_character;   // NPC character


// Character function prototypes

void Character_Init(Character* character);

void Character_Update(Character* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed);

void Character_Draw(Character* character);

void update_character(Joystick_t* joy);

// NPC function prototypes

void NPC_init(Character* npc);
void NPC_Update(Character* npc, uint8_t x);
void update_npc(void);

// Game function prototypes

void render_game(void);

MenuState Game1_Run(void);


#endif // GAME_1_H
