#ifndef CHARACTER_H
#define CHARACTER_H
#include "Joystick.h"

// =================== Character definitions ===================

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

// Frame rate for this game (in milliseconds)
#define GAME1_FRAME_TIME_MS 30  // ~33 FPS

// frame rate for character animation (in frames)
#define ANIMATION_FRAME_RATE 3  // Change sprite every 3 frames
// Character states
typedef enum {

    // Player states
    CHAR_IDLE,              // Not moving
    CHAR_WALKING,           // Moving
    CHAR_DASHING,           // Fast movement
    CHAR_JUMPING,           // Jumping
    CHAR_FALLING,           // Falling (stops jumping mid air)

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




// Character function prototypes

void Character_Init(Character* character);

void Character_Update(Character* character, Joystick_t* joy, uint8_t dash_pressed, uint8_t jump_pressed);

void Character_Draw(Character* character);

void update_character(Joystick_t* joy);

// NPC function prototypes

void NPC_init(Character* npc);
void NPC_Update(Character* npc, uint8_t x);
void update_npc(void);

#endif // CHARACTER_H