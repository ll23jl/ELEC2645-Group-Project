#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <string.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;

#define SCREEN_W 240U
#define SCREEN_H 240U
#define GAME_FRAME_TIME_MS 30U

#define PLAY_X 12U
#define PLAY_Y 36U
#define PLAY_W 216U
#define PLAY_H 188U

#define LANE_COUNT 3U
#define HIT_ZONE_Y 172U
#define PLAYER_Y 172U
#define ENEMY_FAR_Y 204U
#define ENEMY_NEAR_Y 188U

#define NOTE_TRAVEL_MS 2100U
#define HIT_WINDOW_MS 310U
#define PERFECT_WINDOW_MS 80U
#define GREAT_WINDOW_MS 155U
#define AUTO_COLLECT_WINDOW_MS 145U
#define LIFE_LOST_PAUSE_MS 900U
#define SCREEN_FLASH_MS 135U
#define FEEDBACK_MS 420U

#define SCORE_OK 10U
#define SCORE_GREAT 15U
#define SCORE_PERFECT 25U
#define SCORE_STREAK_BONUS 5U
#define SCORE_MISS_PENALTY 5U

#define STREAK_PUSH_EVERY 5U
#define CATCH_MAX 100U
#define MISS_DANGER 20U
#define STREAK_PUSHBACK 16U
#define CATCH_AFTER_LIFE_LOSS 24U
#define PLAYER_MAX_LIVES 3U
#define NOTE_VOLUME 30U
#define NOTE_RADIUS 9U
#define MAX_SONG_NOTES 64U

#define PLAYER_SPRITE_H 24U
#define PLAYER_SPRITE_W 24U
#define ENEMY_SPRITE_H 24U
#define ENEMY_SPRITE_W 24U

#define CAVE_PATH_TOP_Y 92U
#define CAVE_PATH_BOTTOM_Y 220U
#define CAVE_PATH_TOP_LEFT 100U
#define CAVE_PATH_TOP_RIGHT 140U
#define CAVE_PATH_BOTTOM_LEFT 20U
#define CAVE_PATH_BOTTOM_RIGHT 220U

#define MAIN_MENU_COUNT 5U
#define PAUSE_MENU_COUNT 3U
#define RESULT_MENU_COUNT 2U
#define SONG_COUNT 3U

#define FILL_THE_OUTLINE 0U
#define FILL_SOLID 1U

//Colour Palette
#define COL_BG 0U
#define COL_TEXT 1U
#define COL_RED 2U
#define COL_GREEN 3U
#define COL_BLUE 4U
#define COL_ORANGE 5U
#define COL_YELLOW 6U
#define COL_PINK 7U
#define COL_PURPLE 8U
#define COL_NAVY 9U
#define COL_GOLD 10U
#define COL_VIOLET 11U
#define COL_BROWN 12U
#define COL_GREY 13U
#define COL_CYAN 14U
#define COL_MAGENTA 15U

// defining frequencies for songs
#define BE_FREQ_D5 587
#define BE_FREQ_C5 523
#define BE_FREQ_E5 659
#define BE_FREQ_G5 784
#define BE_FREQ_A5 880
#define BE_FREQ_F5 698
#define BE_FREQ_B4 494
#define BE_FREQ_G4 392
#define BE_FREQ_A4 440
#define BE_FREQ_E4 330
#define BE_FREQ_LOW 165

#define BE_ARRAY_LEN(a)((uint32_t)(sizeof(a) / sizeof((a)[0])))

typedef enum {
    BE_STATE_SPLASH = 0,
    BE_STATE_MAIN_MENU,
    BE_STATE_INSTRUCTIONS,
    BE_STATE_SONG_SELECT,
    BE_STATE_HIGH_SCORES,
    BE_STATE_PLAYING,
    BE_STATE_PAUSED,
    BE_STATE_LIFE_LOST,
    BE_STATE_GAME_OVER,
    BE_STATE_RESULTS
} BeatEscapeState;

typedef enum {
    BE_FEEDBACK_NONE = 0,
    BE_FEEDBACK_OK,
    BE_FEEDBACK_GREAT,
    BE_FEEDBACK_PERFECT,
    BE_FEEDBACK_MISS,
    BE_FEEDBACK_LIFE
} BeatFeedback;

typedef struct {
    uint8_t lane;
    uint8_t beats_to_next;
    uint8_t sustain_beats;
} SongStep;

typedef struct {
    const char* title;
    const char* subtitle;
    const SongStep* steps;
    const uint16_t* melody_hz;
    uint32_t step_count;
    uint16_t beat_ms;
    uint16_t lead_in_ms;
} SongDefinition;

typedef struct {
    uint8_t lane;
    uint16_t frequency_hz;
    uint32_t hit_time_ms;
    uint16_t duration_ms;
    uint8_t resolved;
    uint8_t hit;
} SongNote;

static const uint16_t lane_freqs_hz[LANE_COUNT] = {BE_FREQ_C5, BE_FREQ_E5, BE_FREQ_G5};

//Song Data
//Mary had a Little Lamb
static const SongStep song_mary_lamb[] = {
    {1,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {1,1,1}, {1,1,1}, {1,2,2}, {0,1,1}, {0,1,1}, {0,2,2}, {1,1,1}, {2,1,1}, {2,2,2}, {1,1,1}, {0,1,1}, {0,1,1}, {0,1,1}, {1,1,1}, {1,1,1}, {1,2,2}, {1,1,1}, {0,1,1}, {0,1,1}, {1,1,1}, {0,1,1}, {0,2,2}
};

static const uint16_t melody_mary_lamb[] = {
    BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_C5, BE_FREQ_D5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_D5, BE_FREQ_D5, BE_FREQ_E5, BE_FREQ_G5, BE_FREQ_G5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_C5, BE_FREQ_D5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_D5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_C5
};

//Twinkle Twinkle little star
static const SongStep song_twinkle_star[] = {
    {0,1,1}, {0,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,2,2}, {1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {0,1,1}, {0,1,1}, {0,2,2}, {2,1,1}, {2,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {0,2,2}, {2,1,1}, {2,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {0,2,2}, {0,1,1}, {0,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,2,2}, {1,1,1}, {1,1,1}, {1,1,1}, {1,1,1}, {0,1,1}, {0,1,1}, {0,2,2}
};

static const uint16_t melody_twinkle_star[] = {
    BE_FREQ_C5, BE_FREQ_C5, BE_FREQ_G5, BE_FREQ_G5, BE_FREQ_A5, BE_FREQ_A5, BE_FREQ_G5, BE_FREQ_F5, BE_FREQ_F5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_D5, BE_FREQ_C5, BE_FREQ_G5, BE_FREQ_G5, BE_FREQ_F5, BE_FREQ_F5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_G5, BE_FREQ_G5, BE_FREQ_F5, BE_FREQ_F5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_C5, BE_FREQ_C5, BE_FREQ_G5, BE_FREQ_G5, BE_FREQ_A5, BE_FREQ_A5, BE_FREQ_G5, BE_FREQ_F5, BE_FREQ_F5, BE_FREQ_E5, BE_FREQ_E5, BE_FREQ_D5, BE_FREQ_D5, BE_FREQ_C5
};

// All of Me (simplified because the original had too many notes)
static const SongStep song_all_of_me_simple[] = {
    {1,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {0,1,1}, {0,2,2}, {1,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {1,1,1}, {2,1,1}, {2,1,1}, {1,1,1}, {0,1,1}, {0,2,2}, {0,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {2,1,1}, {0,2,2}, {2,1,1}, {2,1,1}, {0,1,1}, {0,1,1}, {1,1,1}, {2,1,1}, {1,1,1}, {0,2,2}
};

static const uint16_t melody_all_of_me_simple[] = {
    BE_FREQ_A4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_G4, BE_FREQ_G4, BE_FREQ_A4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_A4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_A4, BE_FREQ_G4, BE_FREQ_G4, BE_FREQ_E4, BE_FREQ_B4, BE_FREQ_B4, BE_FREQ_C5, BE_FREQ_B4, BE_FREQ_G4, BE_FREQ_C5, BE_FREQ_B4, BE_FREQ_G4, BE_FREQ_G4, BE_FREQ_A4, BE_FREQ_B4, BE_FREQ_A4, BE_FREQ_E4
};

//Song list
static const SongDefinition songs[SONG_COUNT] = {
    {
        "Easy", "Mary Had a Little Lamb", song_mary_lamb, melody_mary_lamb, BE_ARRAY_LEN(song_mary_lamb), 520U, 1100U
    },
    {
        "Medium", "Twinkle Twinkle Little Star", song_twinkle_star, melody_twinkle_star, BE_ARRAY_LEN(song_twinkle_star), 520U, 1100U
    },
    {
        "Hard", "All of Me", song_all_of_me_simple, melody_all_of_me_simple, BE_ARRAY_LEN(song_all_of_me_simple), 560U, 1200U
    }
};

//Pixel Art for player
const uint8_t player_sprite[] = {
    255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_ORANGE,COL_ORANGE,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_ORANGE,COL_ORANGE,COL_RED,COL_RED,COL_RED,COL_RED,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,COL_NAVY,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,255,255,255,255,255,255,255,255,
    255,255,255,255,255,COL_NAVY,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,255,255,255,255,255,255,255,255,
    255,255,255,255,255,COL_NAVY,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,255,255,255,255,255,255,255,255,
    255,255,255,255,255,COL_NAVY,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,255,255,255,255,255,255,255,255,
    255,255,255,255,COL_NAVY,COL_NAVY,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,COL_NAVY,255,255,255,255,255,255,
    255,255,255,255,COL_NAVY,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,255,255,255,255,255,255,
    255,255,255,255,COL_NAVY,COL_ORANGE,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,COL_GREY,COL_GREY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,COL_NAVY,COL_ORANGE,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,COL_GREY,COL_GREY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,COL_NAVY,COL_ORANGE,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,COL_GREY,COL_GREY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,COL_NAVY,COL_NAVY,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_ORANGE,COL_GREEN,COL_GREEN,COL_GREEN,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_GREY,COL_GREY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,COL_NAVY,COL_ORANGE,COL_ORANGE,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_GREY,COL_GREY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,COL_NAVY,COL_ORANGE,COL_ORANGE,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_GREY,COL_NAVY,255,255,255,255,255,255,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,COL_ORANGE,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_NAVY,255,255,255,255,255,255,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,COL_ORANGE,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_NAVY,255,255,255,255,255,255,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,COL_ORANGE,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_GREY,COL_NAVY,255,255,255,255,255,255,
    COL_NAVY,COL_NAVY,COL_NAVY,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,
    255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255,
    255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,COL_NAVY,COL_NAVY,255,255,255,255,255,255
};
//Pixel art for enemy
static const uint8_t enemy_sprite[] = {
    255,255,255,255,255,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,255,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,255,COL_NAVY,255,255,255,255,255,255,255,255,255,255,255,255,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,COL_NAVY,COL_RED,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,255,255,COL_NAVY,COL_RED,COL_RED,COL_RED,COL_NAVY,255,255,
    255,255,255,COL_NAVY,COL_RED,COL_RED,COL_NAVY,COL_NAVY,COL_NAVY,255,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,255,COL_NAVY,COL_RED,COL_RED,COL_RED,COL_RED,COL_NAVY,255,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_RED,COL_NAVY,COL_NAVY,255,255,255,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_NAVY,COL_RED,COL_NAVY,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_BROWN,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,255,255,COL_BROWN,COL_NAVY,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_BROWN,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,255,255,COL_BROWN,COL_NAVY,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_BROWN,255,255,COL_NAVY,COL_NAVY,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,255,255,COL_BROWN,COL_NAVY,
    255,255,255,COL_BROWN,COL_RED,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,COL_NAVY,COL_NAVY,COL_RED,COL_BROWN,255,
    255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,
    255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,
    255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,
    255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,
    255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_NAVY,255,255,
    255,255,255,255,255,COL_NAVY,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_PURPLE,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_RED,COL_PURPLE,COL_PURPLE,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,255,COL_NAVY,255,255,255,COL_PURPLE,COL_BLUE,COL_BLUE,COL_BLUE,COL_BLUE,COL_BLUE,COL_BLUE,COL_PURPLE,255,COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,
    255,255,255,255,COL_NAVY,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_NAVY,255,255,
    255,255,255,COL_NAVY,COL_RED,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_RED,COL_NAVY,255,
    255,255,255,COL_NAVY,COL_RED,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_RED,COL_NAVY,255,
    255,255,255,COL_NAVY,COL_RED,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_BLUE,COL_BLUE,255,255,255,255,COL_RED,COL_NAVY,255,
    COL_NAVY,COL_NAVY,COL_NAVY,COL_RED,255,255,255,255,255,COL_BLUE,255,255,255,255,255,255,COL_BLUE,255,255,255,255,255,COL_RED,COL_NAVY,
    COL_NAVY,COL_NAVY,COL_NAVY,255,255,255,255,255,255,COL_NAVY,255,255,255,255,255,255,COL_NAVY,255,255,255,255,255,255,COL_NAVY,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};
//Pixel art for heart
static const uint8_t heart_sprite[] = {
    255,COL_PINK,COL_PINK ,255,255,255,COL_PINK,COL_PINK,255,
    COL_PINK,COL_PINK,COL_PINK,COL_PINK,255,COL_PINK,COL_PINK,COL_PINK,COL_PINK,
    COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,
    COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,
    255,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,255,
    255,255,COL_PINK,COL_PINK,COL_PINK,COL_PINK,COL_PINK,255,255,
    255,255,255,COL_PINK,COL_PINK,COL_PINK,255,255,255,
    255,255,255,255,COL_PINK,255,255,255,255
};

static SongNote song_notes[MAX_SONG_NOTES];
static uint32_t song_note_count = 0;
static uint32_t song_total_length_ms = 0;

static BeatEscapeState game_state;
static uint32_t state_enter_tick = 0;
static Direction last_direction = CENTRE;

static uint8_t main_menu_selection = 0;
static uint8_t song_select_selection = 0;
static uint8_t pause_selection = 0;
static uint8_t result_selection = 0;
static uint8_t selected_song = 0;

static uint8_t selected_lane = 1;
static uint8_t lives = PLAYER_MAX_LIVES;
static uint8_t catch_meter = 0;
static uint16_t score = 0;
static uint16_t streak = 0;
static uint16_t max_streak = 0;
static uint16_t note_hits = 0;
static uint16_t note_misses = 0;
static uint16_t perfect_hits = 0;

static uint16_t high_scores[SONG_COUNT] = {0U, 0U, 0U};
static uint16_t high_streaks[SONG_COUNT] = {0U, 0U, 0U};

static uint32_t song_start_tick = 0;
static uint32_t song_pause_accum_ms = 0;
static uint32_t timer_freeze_tick = 0;
static uint8_t timer_frozen = 0;
static uint32_t flash_until_tick = 0;
static uint32_t feedback_until_tick = 0;
static BeatFeedback current_feedback = BE_FEEDBACK_NONE;
static uint8_t exit_requested = 0;
static uint8_t score_saved_this_run = 0;

static void draw_menu_header(const char* title);
static uint16_t cave_path_left_at_y(uint16_t y);
static uint16_t cave_path_right_at_y(uint16_t y);
static uint16_t lane_centre_x_at_y(uint8_t lane, uint16_t y);

static uint8_t is_up(Direction d) { return (d == N || d == NE || d == NW); }
static uint8_t is_down(Direction d) { return (d == S || d == SE || d == SW); }
static uint8_t is_left(Direction d) { return (d == W || d == NW || d == SW); }
static uint8_t is_right(Direction d) { return (d == E || d == NE || d == SE); }

static uint8_t joy_up_edge(void) { return is_up(joystick_data.direction) && !is_up(last_direction); }
static uint8_t joy_down_edge(void) { return is_down(joystick_data.direction) && !is_down(last_direction); }
static uint8_t joy_left_edge(void) { return is_left(joystick_data.direction) && !is_left(last_direction); }
static uint8_t joy_right_edge(void) { return is_right(joystick_data.direction) && !is_right(last_direction); }

static uint8_t select_pressed(void)
{
    return joy_right_edge() || current_input.btn2_pressed || current_input.btn3_pressed;
}

static uint8_t back_pressed(void)
{
    return joy_left_edge();
}

static void set_state(BeatEscapeState new_state)
{
    game_state = new_state;
    state_enter_tick = HAL_GetTick();
}

static uint16_t clamp_u16(uint16_t value, uint16_t min_v, uint16_t max_v)
{
    if (value < min_v) return min_v;
    if (value > max_v) return max_v;
    return value;
}

static void freeze_song_timer(void)
{
    if (!timer_frozen) {timer_freeze_tick = HAL_GetTick(); timer_frozen = 1;}
}

static void resume_song_timer(void)
{
    if (timer_frozen) {song_pause_accum_ms += (HAL_GetTick() - timer_freeze_tick); timer_frozen = 0;}
}

static uint32_t get_song_elapsed_ms(void)
{
    uint32_t now = HAL_GetTick();
    if (timer_frozen) {now = timer_freeze_tick;}
    return now - song_start_tick - song_pause_accum_ms;
}

static void show_feedback(BeatFeedback feedback)
{
    current_feedback = feedback;
    feedback_until_tick = HAL_GetTick() + FEEDBACK_MS;
}

static const char* feedback_text(BeatFeedback feedback)
{
    switch (feedback) {
        case BE_FEEDBACK_PERFECT: return "PERFECT!";
        case BE_FEEDBACK_GREAT: return "GREAT!";
        case BE_FEEDBACK_OK: return "HIT!";
        case BE_FEEDBACK_MISS: return "MISS!";
        case BE_FEEDBACK_LIFE: return "CAUGHT!";
        default: return "";
    }
}

static uint8_t feedback_colour(BeatFeedback feedback)
{
    switch (feedback) {
        case BE_FEEDBACK_PERFECT: return COL_YELLOW;
        case BE_FEEDBACK_GREAT: return COL_CYAN;
        case BE_FEEDBACK_OK: return COL_GREEN;
        case BE_FEEDBACK_MISS: return COL_ORANGE;
        case BE_FEEDBACK_LIFE: return COL_RED;
        default: return COL_TEXT;
    }
}

//Rendering 
static void draw_centred(const char* text, uint16_t y, uint8_t colour, uint8_t size)
{
    uint16_t len = (uint16_t)strlen(text);
    uint16_t width = (uint16_t)(len * 6U * size);
    uint16_t x = 0U;
    if (width < SCREEN_W) {
        x = (SCREEN_W - width) / 2U;
    }
    LCD_printString(text, x, y, colour, size);
}

static void draw_panel_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    LCD_Draw_Rect(x, y, w, h, COL_BROWN, FILL_THE_OUTLINE);
    LCD_Draw_Rect((uint16_t)(x + 2U), (uint16_t)(y + 2U), (uint16_t)(w - 4U), (uint16_t)(h - 4U), COL_GOLD, FILL_THE_OUTLINE);
}

static void draw_torch(uint16_t x, uint16_t y)
{
    LCD_Draw_Rect(x, (uint16_t)(y + 6U), 4U, 12U, COL_BROWN, FILL_SOLID);
    LCD_Draw_Rect((uint16_t)(x - 2U), y, 8U, 8U, COL_ORANGE, FILL_SOLID);
    LCD_Draw_Rect((uint16_t)(x - 1U), (uint16_t)(y + 1U), 6U, 5U, COL_YELLOW, FILL_SOLID);
}

static void draw_menu_shell(const char* title)
{
    LCD_Fill_Buffer(COL_BG);
    draw_panel_frame(8U, 8U, 224U, 224U);
    //Menu cave background
    for (uint16_t y = 28U; y <= 108U; y++) {
        uint16_t depth = (uint16_t)(y - 28U);
        uint16_t left = (uint16_t)(82U - (depth / 3U));
        uint16_t right = (uint16_t)(158U + (depth / 3U));

        LCD_Draw_Line(16U, y, left, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
        LCD_Draw_Line(right, y, 224U, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
    }
    //Dark floor
    for (uint16_t y = 188U; y <= 220U; y++) {
        LCD_Draw_Line(16U, y, 224U, y, ((y / 6U) % 2U) ? COL_ORANGE : COL_BROWN);
    }
    //Cave roof aesthetics
    for (uint16_t x = 22U; x <= 218U; x = (uint16_t)(x + 24U)) {uint16_t h = (uint16_t)(10U + ((x / 8U) % 14U));
        for (uint16_t yy = 0U; yy < h; yy++) {uint16_t spread = (uint16_t)((h - yy) / 3U);
            LCD_Draw_Line((uint16_t)(x - spread),(uint16_t)(18U + yy), (uint16_t)(x + spread), (uint16_t)(18U + yy), COL_BROWN);
        }
    }
    draw_torch(28U, 62U);
    draw_torch(208U, 62U);

    //Menu Box
    LCD_Draw_Rect(20U, 54U, 200U, 136U, COL_BG, FILL_SOLID);
    LCD_Draw_Rect(20U, 54U, 200U, 136U, COL_BROWN, FILL_THE_OUTLINE);
    LCD_Draw_Rect(24U, 58U, 192U, 128U, COL_GOLD, FILL_THE_OUTLINE);
    draw_menu_header(title);
}

static void draw_menu_header(const char* title)
{
    draw_centred(title, 18U, COL_YELLOW, 3U);
    LCD_Draw_Line(20U, 47U, 220U, 47U, COL_GOLD);
    LCD_Draw_Line(36U, 51U, 204U, 51U, COL_BROWN);
}

static void draw_option_row(uint16_t x, uint16_t y, uint16_t w, const char* label, uint8_t selected, uint8_t muted)
{
    uint8_t text_col = muted ? COL_GREY : COL_TEXT;
    if (selected) {
        LCD_Draw_Rect(x, (uint16_t)(y - 5U), w, 24U, COL_MAGENTA, FILL_THE_OUTLINE);
        LCD_Draw_Rect((uint16_t)(x + 4U), y, 8U, 10U, COL_GOLD, FILL_SOLID);
        LCD_printString(">", (uint16_t)(x + 17U), y, COL_CYAN, 2U);
    } else {
        LCD_Draw_Rect(x, (uint16_t)(y - 5U), w, 24U, COL_PURPLE, FILL_THE_OUTLINE);
    }
    LCD_printString(label, (uint16_t)(x + 36U), y, text_col, 2U);
}

static void render_vertical_menu(const char* title, const char* const* options, uint8_t option_count, uint8_t selected, const char* footer)
{
    draw_menu_shell(title);

    for (uint8_t i = 0U; i < option_count; i++) {
        uint16_t y = (uint16_t)(68U + ((uint16_t)i * 29U));
        draw_option_row(24U, y, 192U, options[i], (uint8_t)(i == selected), 0U);
    }

    if (footer != NULL) {
        LCD_Draw_Rect(24U, 210U, 192U, 16U, COL_BROWN, FILL_THE_OUTLINE);
        draw_centred(footer, 214U, COL_GREY, 1U);
    }

    LCD_Refresh(&cfg0);
}

static void add_catch(uint8_t amount)
{
    uint16_t new_value = (uint16_t)catch_meter + amount;
    catch_meter = (new_value > CATCH_MAX) ? CATCH_MAX : (uint8_t)new_value;
}

static void reduce_catch(uint8_t amount)
{
    if (catch_meter > amount) {
        catch_meter = (uint8_t)(catch_meter - amount);
    } else {
        catch_meter = 0U;
    }
}

static void save_high_score_if_needed(void)
{
    if (score_saved_this_run) {
        return;
    }

    if (selected_song < SONG_COUNT) {
        if (score > high_scores[selected_song]) {
            high_scores[selected_song] = score;
        }
        if (max_streak > high_streaks[selected_song]) {
            high_streaks[selected_song] = max_streak;
        }
    }

    score_saved_this_run = 1U;
}

static void load_song(uint8_t song_index)
{
    const SongDefinition* song;
    uint32_t next_hit;

    if (song_index >= SONG_COUNT) {
        song_index = 0U;
    }

    song = &songs[song_index];
    song_note_count = 0U;
    next_hit = song->lead_in_ms;

    for (uint32_t i = 0U; i < song->step_count && i < MAX_SONG_NOTES; i++) {
        const SongStep* step = &song->steps[i];
        uint8_t lane = (step->lane < LANE_COUNT) ? step->lane : 1U;

        song_notes[song_note_count].lane = lane;
        song_notes[song_note_count].frequency_hz = (song->melody_hz != NULL) ? song->melody_hz[i] : lane_freqs_hz[lane];
        song_notes[song_note_count].hit_time_ms = next_hit;
        song_notes[song_note_count].duration_ms = (uint16_t)((uint32_t)step->sustain_beats * ((uint32_t)song->beat_ms * 3U / 4U));
        song_notes[song_note_count].resolved = 0U;
        song_notes[song_note_count].hit = 0U;
        song_note_count++;
        next_hit += ((uint32_t)step->beats_to_next * (uint32_t)song->beat_ms);
    }
    song_total_length_ms = next_hit + 1500U;
}

static void reset_gameplay_runtime(void)
{
    for (uint32_t i = 0U; i < song_note_count; i++) {
        song_notes[i].resolved = 0U;
        song_notes[i].hit = 0U;
    }
    score = 0U;
    streak = 0U;
    max_streak = 0U;
    note_hits = 0U;
    note_misses = 0U;
    perfect_hits = 0U;
    lives = PLAYER_MAX_LIVES;
    catch_meter = 0U;
    selected_lane = 1U;
    flash_until_tick = 0U;
    feedback_until_tick = 0U;
    current_feedback = BE_FEEDBACK_NONE;
    score_saved_this_run = 0U;

    song_start_tick = HAL_GetTick();
    song_pause_accum_ms = 0U;
    timer_freeze_tick = 0U;
    timer_frozen = 0U;
}

static void start_song_run(void)
{
    load_song(selected_song);
    reset_gameplay_runtime();
    buzzer_off(&buzzer_cfg);
    PWM_SetDuty(&pwm_cfg, 12U);
    set_state(BE_STATE_PLAYING);
}

static int32_t abs_i32(int32_t v)
{
    return (v < 0) ? -v : v;
}

static int find_hittable_note_in_lane(uint32_t elapsed_ms, uint8_t lane)
{
    int best_index = -1;
    uint32_t best_delta = 0xFFFFFFFFU;

    for (uint32_t i = 0U; i < song_note_count; i++) {
        int32_t delta;
        uint32_t abs_delta;

        if (song_notes[i].resolved || song_notes[i].lane != lane) {
            continue;
        }

        delta = (int32_t)song_notes[i].hit_time_ms - (int32_t)elapsed_ms;
        abs_delta = (uint32_t)abs_i32(delta);

        if (abs_delta <= HIT_WINDOW_MS && abs_delta < best_delta) {
            best_delta = abs_delta;
            best_index = (int)i;
        }
    }

    return best_index;
}

static void register_hit(uint32_t note_index, uint32_t abs_delta)
{
    uint16_t earned;
    BeatFeedback fb;

    if (note_index >= song_note_count || song_notes[note_index].resolved) {
        return;
    }

    song_notes[note_index].resolved = 1U;
    song_notes[note_index].hit = 1U;
    note_hits++;
    streak++;

    if (streak > max_streak) {
        max_streak = streak;
    }

    if (abs_delta <= PERFECT_WINDOW_MS) {
        earned = SCORE_PERFECT;
        fb = BE_FEEDBACK_PERFECT;
        perfect_hits++;
    } else if (abs_delta <= GREAT_WINDOW_MS) {
        earned = SCORE_GREAT;
        fb = BE_FEEDBACK_GREAT;
    } else {
        earned = SCORE_OK;
        fb = BE_FEEDBACK_OK;
    }

    if ((streak % STREAK_PUSH_EVERY) == 0U) {
        earned = (uint16_t)(earned + SCORE_STREAK_BONUS);
        reduce_catch(STREAK_PUSHBACK);
    }

    score = (uint16_t)clamp_u16((uint16_t)(score + earned), 0U, 9999U);
    flash_until_tick = HAL_GetTick() + 60U;
    show_feedback(fb);
}

static void trigger_life_lost(void)
{
    if (lives > 0U) {
        lives--;
    }

    flash_until_tick = HAL_GetTick() + SCREEN_FLASH_MS;
    show_feedback(BE_FEEDBACK_LIFE);
    freeze_song_timer();
    buzzer_tone(&buzzer_cfg, BE_FREQ_LOW, 35U);

    if (lives == 0U) {
        save_high_score_if_needed();
        PWM_SetDuty(&pwm_cfg, 90U);
        set_state(BE_STATE_GAME_OVER);
    } else {
        PWM_SetDuty(&pwm_cfg, 82U);
        set_state(BE_STATE_LIFE_LOST);
    }
}

static void register_miss(uint32_t note_index)
{
    if (note_index >= song_note_count || song_notes[note_index].resolved) {
        return;
    }

    song_notes[note_index].resolved = 1U;
    song_notes[note_index].hit = 0U;
    note_misses++;
    streak = 0U;

    if (score > SCORE_MISS_PENALTY) {
        score = (uint16_t)(score - SCORE_MISS_PENALTY);
    } else {
        score = 0U;
    }

    add_catch(MISS_DANGER);
    flash_until_tick = HAL_GetTick() + SCREEN_FLASH_MS;
    show_feedback(BE_FEEDBACK_MISS);

    if (catch_meter >= CATCH_MAX) {
        trigger_life_lost();
    }
}

static void auto_collect_current_lane(void)
{
    uint32_t elapsed_ms = get_song_elapsed_ms();
    int note_index = find_hittable_note_in_lane(elapsed_ms, selected_lane);

    if (note_index >= 0) {
        int32_t delta = (int32_t)song_notes[note_index].hit_time_ms - (int32_t)elapsed_ms;
        uint32_t abs_delta = (uint32_t)abs_i32(delta);

        if (abs_delta <= AUTO_COLLECT_WINDOW_MS) {
            register_hit((uint32_t)note_index, abs_delta);
        }
    }
}

static void update_buzzer_and_pwm(uint32_t elapsed_ms)
{
    uint8_t duty = (uint8_t)(8U + (((uint16_t)catch_meter * 70U) / CATCH_MAX));

    PWM_SetDuty(&pwm_cfg, duty);

    for (uint32_t i = 0U; i < song_note_count; i++) {
        if (elapsed_ms >= song_notes[i].hit_time_ms &&
            elapsed_ms < (song_notes[i].hit_time_ms + song_notes[i].duration_ms)) {
            buzzer_tone(&buzzer_cfg, song_notes[i].frequency_hz, NOTE_VOLUME);
            return;
        }
    }

    buzzer_off(&buzzer_cfg);
}

//Gameplay update logic
static void update_playing_logic(void)
{
    uint32_t elapsed_ms = get_song_elapsed_ms();

    if (joy_left_edge()) {
        if (selected_lane > 0U) {
            selected_lane--;
        }
    } else if (joy_right_edge()) {
        if (selected_lane < (LANE_COUNT - 1U)) {
            selected_lane++;
        }
    }
    if (joy_down_edge() || current_input.btn3_pressed) {
        freeze_song_timer();
        pause_selection = 0U;
        buzzer_off(&buzzer_cfg);
        PWM_SetDuty(&pwm_cfg, 18U);
        set_state(BE_STATE_PAUSED);
        return;
    }
    auto_collect_current_lane();
    for (uint32_t i = 0U; i < song_note_count; i++) {
        if (!song_notes[i].resolved && elapsed_ms > (song_notes[i].hit_time_ms + HIT_WINDOW_MS)) {
            register_miss(i);
            if (game_state != BE_STATE_PLAYING) {
                return;
            }
        }
    }
    if (elapsed_ms >= song_total_length_ms) {
        freeze_song_timer();
        buzzer_off(&buzzer_cfg);
        PWM_SetDuty(&pwm_cfg, 12U);
        result_selection = 0U;
        save_high_score_if_needed();
        set_state(BE_STATE_RESULTS);
        return;
    }
    update_buzzer_and_pwm(elapsed_ms);
}

static void draw_heart(uint16_t x, uint16_t y, uint8_t active)
{
    if (active) {
        LCD_Draw_Sprite(x, y, 8U, 9U, heart_sprite);
    } else {
        LCD_Draw_Rect((uint16_t)(x + 1U), (uint16_t)(y + 1U), 7U, 6U, COL_GREY, FILL_THE_OUTLINE);
    }
}

static void draw_coin_note(uint16_t x, uint16_t y, uint8_t highlight)
{
    LCD_Draw_Circle(x, y, NOTE_RADIUS, COL_GOLD, FILL_SOLID);
    LCD_Draw_Circle(x, y, (uint16_t)(NOTE_RADIUS - 2U), COL_YELLOW, FILL_SOLID);
    LCD_Draw_Circle(x, y, (uint16_t)(NOTE_RADIUS - 4U), COL_GOLD, FILL_THE_OUTLINE);
    LCD_Draw_Line((uint16_t)(x - 3U), y, (uint16_t)(x + 3U), y, highlight ? COL_ORANGE : COL_BROWN);
    LCD_Draw_Line(x, (uint16_t)(y - 3U), x, (uint16_t)(y + 3U), highlight ? COL_ORANGE : COL_BROWN);
}

static void draw_temple_arch(void)
{
    uint16_t y;
    uint16_t x;
    uint16_t depth;
    uint16_t open_left;
    uint16_t open_right;
    uint16_t half_w;
    uint16_t h;
    draw_panel_frame(PLAY_X, PLAY_Y, PLAY_W, PLAY_H);
    //Clear play area
    LCD_Draw_Rect((uint16_t)(PLAY_X + 1U), (uint16_t)(PLAY_Y + 1U), (uint16_t)(PLAY_W - 2U), (uint16_t)(PLAY_H - 2U), COL_BG, FILL_SOLID);
    //Cave walls
    for (y = 38U; y <= 130U; y++) {
        depth = (uint16_t)(y - 38U);
        if (y < 78U) {
            open_left = (uint16_t)(100U - depth);
            open_right = (uint16_t)(140U + depth);
        } else {
            open_left = (uint16_t)(60U - ((y - 78U) / 3U));
            open_right = (uint16_t)(180U + ((y - 78U) / 3U));
        }
        LCD_Draw_Line(14U, y, open_left, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
        LCD_Draw_Line(open_right, y, 226U, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
    }
    //Back of cave
    for (y = 48U; y <= 112U; y++) {
        if (y < 80U) {
            half_w = (uint16_t)(16U + (y - 48U));
        } else {
            half_w = (uint16_t)(16U + (112U - y));
        }

        LCD_Draw_Line((uint16_t)(120U - half_w), y, (uint16_t)(120U + half_w), y, COL_BG);
    }
    //Cave roof aesthetics XD
    for (x = 76U; x <= 160U; x = (uint16_t)(x + 14U)) {h = (uint16_t)(12U + ((x / 7U) % 16U));
        LCD_Draw_Rect(x, 38U, 5U, h, COL_BROWN, FILL_SOLID);
    }
    //Torches
    draw_torch(30U, 110U);
    draw_torch(206U, 110U);
}

static void draw_stone_track(void)
{
    uint16_t y;
    uint16_t left;
    uint16_t right;
    uint16_t width;
    uint16_t lane1_top;
    uint16_t lane2_top;
    uint16_t lane1_bottom;
    uint16_t lane2_bottom;
    uint16_t hit_left;
    uint16_t hit_right;
    uint16_t hit_width;
    uint16_t div1;
    uint16_t div2;
    uint8_t row_col;
    draw_temple_arch();
    //Path
    for (y = CAVE_PATH_TOP_Y; y <= CAVE_PATH_BOTTOM_Y; y++) {
        left = cave_path_left_at_y(y);
        right = cave_path_right_at_y(y);
        row_col = (((y / 14U) % 2U) == 0U) ? COL_ORANGE : COL_BROWN;
        LCD_Draw_Line(left, y, right, y, row_col);
    }
    //more aesthetics
    for (y = CAVE_PATH_TOP_Y; y <= CAVE_PATH_BOTTOM_Y; y = (uint16_t)(y + 18U)) {
        left = cave_path_left_at_y(y);
        right = cave_path_right_at_y(y);
        LCD_Draw_Line((uint16_t)(left + 4U), y, (uint16_t)(right - 4U), y, COL_BROWN);
    }
    LCD_Draw_Line(CAVE_PATH_TOP_LEFT, CAVE_PATH_TOP_Y, CAVE_PATH_BOTTOM_LEFT, CAVE_PATH_BOTTOM_Y, COL_GOLD);
    LCD_Draw_Line((uint16_t)(CAVE_PATH_TOP_LEFT + 3U), CAVE_PATH_TOP_Y, (uint16_t)(CAVE_PATH_BOTTOM_LEFT + 6U), CAVE_PATH_BOTTOM_Y, COL_ORANGE);
    LCD_Draw_Line(CAVE_PATH_TOP_RIGHT, CAVE_PATH_TOP_Y, CAVE_PATH_BOTTOM_RIGHT, CAVE_PATH_BOTTOM_Y, COL_GOLD);
    LCD_Draw_Line((uint16_t)(CAVE_PATH_TOP_RIGHT - 3U), CAVE_PATH_TOP_Y, (uint16_t)(CAVE_PATH_BOTTOM_RIGHT - 6U), CAVE_PATH_BOTTOM_Y, COL_ORANGE);
    //Lane aesthetics
    width = (uint16_t)(CAVE_PATH_TOP_RIGHT - CAVE_PATH_TOP_LEFT);
    lane1_top = (uint16_t)(CAVE_PATH_TOP_LEFT + (width / 3U));
    lane2_top = (uint16_t)(CAVE_PATH_TOP_LEFT + ((width * 2U) / 3U));
    width = (uint16_t)(CAVE_PATH_BOTTOM_RIGHT - CAVE_PATH_BOTTOM_LEFT);
    lane1_bottom = (uint16_t)(CAVE_PATH_BOTTOM_LEFT + (width / 3U));
    lane2_bottom = (uint16_t)(CAVE_PATH_BOTTOM_LEFT + ((width * 2U) / 3U));
    LCD_Draw_Line(lane1_top, CAVE_PATH_TOP_Y, lane1_bottom, CAVE_PATH_BOTTOM_Y, COL_GOLD);
    LCD_Draw_Line((uint16_t)(lane1_top + 1U), CAVE_PATH_TOP_Y, (uint16_t)(lane1_bottom + 2U), CAVE_PATH_BOTTOM_Y, COL_ORANGE);
    LCD_Draw_Line(lane2_top, CAVE_PATH_TOP_Y, lane2_bottom, CAVE_PATH_BOTTOM_Y, COL_GOLD);
    LCD_Draw_Line((uint16_t)(lane2_top - 1U), CAVE_PATH_TOP_Y, (uint16_t)(lane2_bottom - 2U), CAVE_PATH_BOTTOM_Y, COL_ORANGE);
    //Hit zone
    hit_left = (uint16_t)(cave_path_left_at_y(HIT_ZONE_Y) + 4U);
    hit_right = (uint16_t)(cave_path_right_at_y(HIT_ZONE_Y) - 4U);
    hit_width = (uint16_t)(hit_right - hit_left);
    LCD_Draw_Rect(hit_left, HIT_ZONE_Y, hit_width, 20U, COL_ORANGE, FILL_THE_OUTLINE);
    LCD_Draw_Rect((uint16_t)(hit_left + 2U), (uint16_t)(HIT_ZONE_Y + 2U), (uint16_t)(hit_width - 4U), 16U, COL_GOLD, FILL_THE_OUTLINE);
    div1 = (uint16_t)(hit_left + (hit_width / 3U));
    div2 = (uint16_t)(hit_left + ((hit_width * 2U) / 3U));
    LCD_Draw_Line(div1, HIT_ZONE_Y, div1, (uint16_t)(HIT_ZONE_Y + 20U), COL_GOLD);
    LCD_Draw_Line(div2, HIT_ZONE_Y, div2, (uint16_t)(HIT_ZONE_Y + 20U), COL_GOLD);
}

static uint16_t cave_path_left_at_y(uint16_t y)
{
    uint16_t clamped_y = y;
    if (clamped_y < CAVE_PATH_TOP_Y) {
        clamped_y = CAVE_PATH_TOP_Y;
    }
    if (clamped_y > CAVE_PATH_BOTTOM_Y) {
        clamped_y = CAVE_PATH_BOTTOM_Y;
    }

    return (uint16_t)(CAVE_PATH_TOP_LEFT - (((uint32_t)(clamped_y - CAVE_PATH_TOP_Y) * (CAVE_PATH_TOP_LEFT - CAVE_PATH_BOTTOM_LEFT)) / (CAVE_PATH_BOTTOM_Y - CAVE_PATH_TOP_Y)));
}

static uint16_t cave_path_right_at_y(uint16_t y)
{
    uint16_t clamped_y = y;
    if (clamped_y < CAVE_PATH_TOP_Y) {
        clamped_y = CAVE_PATH_TOP_Y;
    }
    if (clamped_y > CAVE_PATH_BOTTOM_Y) {
        clamped_y = CAVE_PATH_BOTTOM_Y;
    }
    return (uint16_t)(CAVE_PATH_TOP_RIGHT + (((uint32_t)(clamped_y - CAVE_PATH_TOP_Y) * (CAVE_PATH_BOTTOM_RIGHT - CAVE_PATH_TOP_RIGHT)) / (CAVE_PATH_BOTTOM_Y - CAVE_PATH_TOP_Y)));
}

static uint16_t lane_centre_x_at_y(uint8_t lane, uint16_t y)
{
    uint16_t left = cave_path_left_at_y(y);
    uint16_t right = cave_path_right_at_y(y);
    uint16_t width = (uint16_t)(right - left);
    if (lane >= LANE_COUNT) {
        lane = 1U;
    }
    return (uint16_t)(left + (((uint32_t)width * ((uint32_t)lane * 2U + 1U)) / (LANE_COUNT * 2U)));
}

static void render_active_notes(uint32_t elapsed_ms)
{
    for (uint32_t i = 0U; i < song_note_count; i++) {
        int32_t age_ms;
        uint16_t lane;
        uint16_t x;
        uint16_t y;
        if (song_notes[i].resolved && song_notes[i].hit) {
            continue;
        }
        age_ms = (int32_t)elapsed_ms - (int32_t)(song_notes[i].hit_time_ms - NOTE_TRAVEL_MS);
        if (age_ms < 0 || age_ms > (int32_t)(NOTE_TRAVEL_MS + HIT_WINDOW_MS)) {
            continue;
        }
        lane = song_notes[i].lane;
        y = (uint16_t)(CAVE_PATH_TOP_Y + (((uint32_t)age_ms * (HIT_ZONE_Y - CAVE_PATH_TOP_Y)) / NOTE_TRAVEL_MS));
        x = lane_centre_x_at_y((uint8_t)lane, y);
        if (song_notes[i].resolved && !song_notes[i].hit) {
            LCD_Draw_Circle(x, y, NOTE_RADIUS, COL_RED, FILL_SOLID);
            LCD_Draw_Line((uint16_t)(x - 5U), (uint16_t)(y - 5U), (uint16_t)(x + 5U), (uint16_t)(y + 5U), COL_TEXT);
            LCD_Draw_Line((uint16_t)(x + 5U), (uint16_t)(y - 5U), (uint16_t)(x - 5U), (uint16_t)(y + 5U), COL_TEXT);
        } else {
            draw_coin_note(x, y, (uint8_t)(lane == selected_lane));
        }
    }
}

static void draw_player_and_enemy(void)
{
    uint16_t player_mid_x = lane_centre_x_at_y(selected_lane, PLAYER_Y);
    uint16_t player_x = (uint16_t)(player_mid_x - (PLAYER_SPRITE_W / 2U));
    uint16_t player_y = (uint16_t)(PLAYER_Y - ((HAL_GetTick() / 140U) % 2U));

    uint16_t enemy_y = (uint16_t)(ENEMY_FAR_Y -
        (((uint16_t)catch_meter * (ENEMY_FAR_Y - ENEMY_NEAR_Y)) / CATCH_MAX));
    uint16_t enemy_mid_x = lane_centre_x_at_y(selected_lane, enemy_y);
    uint16_t enemy_x = (uint16_t)(enemy_mid_x - (ENEMY_SPRITE_W / 2U));

    LCD_Draw_Rect((uint16_t)(player_x + 3U), (uint16_t)(player_y + PLAYER_SPRITE_H), 18U, 2U, COL_GREY, FILL_SOLID);
    LCD_Draw_Rect((uint16_t)(enemy_x + 3U), (uint16_t)(enemy_y + ENEMY_SPRITE_H), 18U, 2U, COL_GREY, FILL_SOLID);

    LCD_Draw_Sprite(enemy_x, enemy_y, ENEMY_SPRITE_H, ENEMY_SPRITE_W, enemy_sprite);
    LCD_Draw_Sprite(player_x, player_y, PLAYER_SPRITE_H, PLAYER_SPRITE_W, player_sprite);
}

static void render_hud(uint32_t elapsed_ms)
{
    char line[32];
    const SongDefinition* song = &songs[selected_song];

    LCD_Draw_Rect(2U, 2U, 236U, 30U, COL_BG, FILL_SOLID);
    LCD_Draw_Rect(2U, 2U, 236U, 30U, COL_BROWN, FILL_THE_OUTLINE);
    LCD_Draw_Line(4U, 31U, 236U, 31U, COL_GOLD);

    sprintf(line, "Score:%u", score);
    LCD_printString(line, 7U, 7U, COL_TEXT, 1U);

    sprintf(line, "x%u", streak);
    LCD_printString(line, 82U, 7U, COL_YELLOW, 1U);
    LCD_printString(song->title, 124U, 7U, COL_CYAN, 1U);

    for (uint8_t i = 0U; i < PLAYER_MAX_LIVES; i++) {
        draw_heart((uint16_t)(176U + ((uint16_t)i * 18U)), 18U, (uint8_t)(i < lives));
    }
    LCD_printString("Danger", 7U, 20U, COL_ORANGE, 1U);
    LCD_Draw_Rect(50U, 20U, 76U, 8U, COL_GREY, FILL_THE_OUTLINE);
    LCD_Draw_Rect(51U, 21U, (uint16_t)((catch_meter * 74U) / CATCH_MAX), 6U, (catch_meter > 66U) ? COL_RED : ((catch_meter > 33U) ? COL_ORANGE : COL_GREEN), FILL_SOLID);

    if (elapsed_ms < song->lead_in_ms) {
        draw_centred("Get ready...", 228U, COL_YELLOW, 1U);
    } else {
        draw_centred("Stay on the beat   DOWN pause", 228U, COL_GREY, 1U);
    }
}

static void render_feedback_overlay(void)
{
    if (feedback_until_tick > HAL_GetTick() && current_feedback != BE_FEEDBACK_NONE) {
        const char* txt = feedback_text(current_feedback);
        uint8_t col = feedback_colour(current_feedback);
        LCD_Draw_Rect(70U, 86U, 100U, 24U, COL_NAVY, FILL_SOLID);
        LCD_Draw_Rect(70U, 86U, 100U, 24U, col, FILL_THE_OUTLINE);
        draw_centred(txt, 92U, col, 2U);
    }
}

static void render_playing_screen(const char* overlay_title, const char* overlay_subtitle)
{
    uint32_t elapsed_ms = get_song_elapsed_ms();
    LCD_Fill_Buffer(COL_BG);

    if (flash_until_tick > HAL_GetTick()) {
        LCD_Draw_Rect(0U, 0U, SCREEN_W, SCREEN_H, COL_RED, FILL_THE_OUTLINE);
        LCD_Draw_Rect(2U, 2U, (uint16_t)(SCREEN_W - 4U), (uint16_t)(SCREEN_H - 4U), COL_RED, FILL_THE_OUTLINE);
    }

    render_hud(elapsed_ms);
    draw_stone_track();
    render_active_notes(elapsed_ms);
    draw_player_and_enemy();
    render_feedback_overlay();

    if (overlay_title != NULL) {
        LCD_Draw_Rect(28U, 92U, 184U, 56U, COL_NAVY, FILL_SOLID);
        LCD_Draw_Rect(28U, 92U, 184U, 56U, COL_TEXT, FILL_THE_OUTLINE);
        draw_centred(overlay_title, 104U, COL_TEXT, 2U);
        if (overlay_subtitle != NULL) {
            draw_centred(overlay_subtitle, 126U, COL_YELLOW, 1U);
        }
    }
    LCD_Refresh(&cfg0);
}

static void update_menu_selection(uint8_t* selection, uint8_t count)
{
    if (joy_down_edge()) {
        *selection = (uint8_t)((*selection + 1U) % count);
    } else if (joy_up_edge()) {
        if (*selection == 0U) {
            *selection = (uint8_t)(count - 1U);
        } else {
            (*selection)--;
        }
    }
}

//Screen renderers
static void render_splash(void)
{
    uint16_t pulse = (uint16_t)((HAL_GetTick() / 220U) % 2U);
    LCD_Fill_Buffer(COL_BG);
    draw_panel_frame(8U, 8U, 224U, 224U);

    //Cave walls and tunnel
    for (uint16_t y = 34U; y <= 150U; y++) {
        uint16_t depth = (uint16_t)(y - 34U);
        uint16_t left = (uint16_t)(78U - (depth / 4U));
        uint16_t right = (uint16_t)(162U + (depth / 4U));

        LCD_Draw_Line(14U, y, left, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
        LCD_Draw_Line(right, y, 226U, y, ((y / 5U) % 2U) ? COL_BROWN : COL_GREY);
    }
    //Floor
    for (uint16_t y = 170U; y <= 216U; y++) {
        LCD_Draw_Line(16U, y, 224U, y, ((y / 6U) % 2U) ? COL_ORANGE : COL_BROWN);
    }
    //Roof aesthetics
    for (uint16_t x = 24U; x <= 216U; x = (uint16_t)(x + 22U)) {
        uint16_t h = (uint16_t)(12U + ((x / 7U) % 18U));

        for (uint16_t yy = 0U; yy < h; yy++) {
            uint16_t spread = (uint16_t)((h - yy) / 3U);
            LCD_Draw_Line((uint16_t)(x - spread), (uint16_t)(18U + yy), (uint16_t)(x + spread), (uint16_t)(18U + yy), COL_BROWN);
        }
    }
    //Torches
    draw_torch(28U, 120U);
    draw_torch(208U, 120U);
    //Title
    draw_centred("BEAT", 66U, COL_BROWN, 4U);
    draw_centred("ESCAPE", 112U, COL_BROWN, 4U);
    draw_centred("BEAT", 62U, COL_YELLOW, 4U);
    draw_centred("ESCAPE", 108U, pulse ? COL_CYAN : COL_GOLD, 4U);
    //Info
    LCD_Draw_Rect(44U, 156U, 152U, 18U, COL_BROWN, FILL_THE_OUTLINE);
    draw_centred("Press RIGHT or BUTTON to start", 202U, pulse ? COL_TEXT : COL_GREY, 1U);
    LCD_Refresh(&cfg0);
}

static void update_splash(void)
{
    if (joy_right_edge() || joy_down_edge() || current_input.btn2_pressed || current_input.btn3_pressed) {
        main_menu_selection = 0U; set_state(BE_STATE_MAIN_MENU);
    }
}

static void render_main_menu(void)
{
    static const char* const options[MAIN_MENU_COUNT] = {
        "Start Game", "Select Song", "Instructions", "High Scores", "Exit"
    };

    render_vertical_menu("Beat Escape", options, MAIN_MENU_COUNT, main_menu_selection, "UP/DOWN to move, RIGHT to select");
}

static void update_main_menu(void)
{
    update_menu_selection(&main_menu_selection, MAIN_MENU_COUNT);

    if (select_pressed()) {
        switch (main_menu_selection) {
            case 0U:
                start_song_run();
                break;
            case 1U:
                song_select_selection = selected_song;
                set_state(BE_STATE_SONG_SELECT);
                break;
            case 2U:
                set_state(BE_STATE_INSTRUCTIONS);
                break;
            case 3U:
                set_state(BE_STATE_HIGH_SCORES);
                break;
            default:
                exit_requested = 1U;
                break;
        }
    }
}

static void render_instructions(void)
{
    draw_menu_shell("Instructions");
    LCD_printString("- Move LEFT/RIGHT to choose a lane ", 18U, 62U, COL_TEXT, 1U);
    LCD_printString("- Collect the coins", 18U, 98U, COL_TEXT, 1U);
    LCD_printString("- Perfect timing gets more points", 18U, 134U, COL_TEXT, 1U);
    LCD_printString("- Misses bring the monster closer", 18U, 170U, COL_TEXT, 1U);
    LCD_Draw_Rect(28U, 210U, 184U, 18U, COL_BROWN, FILL_THE_OUTLINE);
    draw_centred("LEFT or BUTTON to return", 214U, COL_TEXT, 1U);
    LCD_Refresh(&cfg0);
}

static void update_instructions(void)
{
    if (back_pressed() || select_pressed()) {
        set_state(BE_STATE_MAIN_MENU);
    }
}

static void render_high_scores(void)
{
    char line[40];

    draw_menu_shell("High Scores");

    for (uint8_t i = 0U; i < SONG_COUNT; i++) {
        uint16_t y = (uint16_t)(66U + ((uint16_t)i * 44U));
        LCD_Draw_Rect(22U, (uint16_t)(y - 4U), 196U, 34U, (i == selected_song) ? COL_GOLD : COL_BROWN, FILL_THE_OUTLINE);
        LCD_Draw_Rect(24U, (uint16_t)(y - 2U), 192U, 30U, COL_BG, FILL_SOLID);
        LCD_printString(songs[i].title, 32U, y, COL_TEXT, 1U);
        sprintf(line, "Best:%u", high_scores[i]);
        LCD_printString(line, 32U, (uint16_t)(y + 14U), COL_GREY, 1U);
        sprintf(line, "Streak:%u", high_streaks[i]);
        LCD_printString(line, 130U, (uint16_t)(y + 14U), COL_GREY, 1U);
    }
    LCD_Draw_Rect(26U, 210U, 188U, 18U, COL_BROWN, FILL_THE_OUTLINE);
    draw_centred("LEFT or BUTTON to return", 214U, COL_TEXT, 1U);
    LCD_Refresh(&cfg0);
}

static void update_high_scores(void)
{
    if (back_pressed() || select_pressed()) {
        set_state(BE_STATE_MAIN_MENU);
    }
}

static void render_song_select(void)
{
    draw_menu_shell("Select Song");
    for (uint8_t i = 0U; i < SONG_COUNT; i++) {
        uint16_t y = (uint16_t)(66U + ((uint16_t)i * 40U));
        uint8_t selected = (uint8_t)(i == song_select_selection);
        uint8_t active = (uint8_t)(i == selected_song);

        LCD_Draw_Rect(22U, (uint16_t)(y - 4U), 196U, 30U, selected ? COL_GOLD : COL_BROWN, FILL_THE_OUTLINE);
        LCD_Draw_Rect(24U, (uint16_t)(y - 2U), 192U, 26U, COL_BG, FILL_SOLID);
        LCD_Draw_Rect(30U, y, 10U, 10U, selected ? COL_YELLOW : COL_GREY, FILL_SOLID);
        LCD_printString(songs[i].title, 48U, y, selected ? COL_YELLOW : COL_TEXT, 2U);
        LCD_printString(songs[i].subtitle, 48U, (uint16_t)(y + 14U), active ? COL_GOLD : COL_GREY, 1U);
        if (active) {
            LCD_printString("SELECTED", 164U, (uint16_t)(y + 14U), COL_CYAN, 1U);
        }
    }
    LCD_Draw_Rect(24U, 210U, 192U, 18U, COL_BROWN, FILL_THE_OUTLINE);
    draw_centred("RIGHT to play   LEFT to go back", 214U, COL_GREY, 1U);
    LCD_Refresh(&cfg0);
}

static void update_song_select(void)
{
    update_menu_selection(&song_select_selection, SONG_COUNT);

    if (back_pressed()) {
        set_state(BE_STATE_MAIN_MENU);
        return;
    }

    if (select_pressed()) {
        selected_song = song_select_selection;
        start_song_run();
    }
}

static void render_pause_menu(void)
{
    static const char* const options[PAUSE_MENU_COUNT] = {
        "Resume", "Restart Song", "Main Menu"
    };

    render_vertical_menu("Paused", options, PAUSE_MENU_COUNT, pause_selection, "UP/DOWN to move, RIGHT to select");
}

static void update_pause_menu(void)
{
    update_menu_selection(&pause_selection, PAUSE_MENU_COUNT);

    if (back_pressed()) {
        resume_song_timer();
        set_state(BE_STATE_PLAYING);
        return;
    }
    if (select_pressed()) {
        if (pause_selection == 0U) {
            resume_song_timer();
            set_state(BE_STATE_PLAYING);
        } else if (pause_selection == 1U) {
            start_song_run();
        } else {
            buzzer_off(&buzzer_cfg);
            PWM_SetDuty(&pwm_cfg, 16U);
            main_menu_selection = 0U;
            set_state(BE_STATE_MAIN_MENU);
        }
    }
}

static void update_result_menu(void)
{
    update_menu_selection(&result_selection, RESULT_MENU_COUNT);
    if (select_pressed()) {
        if (result_selection == 0U) {
            start_song_run();
        } else {
            buzzer_off(&buzzer_cfg);
            PWM_SetDuty(&pwm_cfg, 16U);
            main_menu_selection = 0U;
            set_state(BE_STATE_MAIN_MENU);
        }
    }
}

static char results_rank(uint16_t accuracy)
{
    if (accuracy >= 95U && note_misses == 0U) return 'S';
    if (accuracy >= 85U) return 'A';
    if (accuracy >= 70U) return 'B';
    if (accuracy >= 55U) return 'C';
    return 'D';
}

static void render_stats_card(const char* title, uint8_t is_game_over)
{
    char line[40];
    uint16_t accuracy = 0U;

    if (song_note_count > 0U) {
        accuracy = (uint16_t)(((uint32_t)note_hits * 100U) / song_note_count);
    }
    draw_menu_shell(title);
    LCD_Draw_Rect(24U, 62U, 192U, 102U, COL_TEXT, FILL_THE_OUTLINE);
    LCD_Draw_Rect(26U, 64U, 188U, 98U, COL_BG, FILL_SOLID);
    LCD_Draw_Rect(30U, 68U, 180U, 14U, is_game_over ? COL_RED : COL_GREEN, FILL_THE_OUTLINE);
    LCD_printString(songs[selected_song].title, 76U, 71U, is_game_over ? COL_RED : COL_GREEN, 1U);

    sprintf(line, "Score: %u", score);
    LCD_printString(line, 40U, 90U, COL_TEXT, 2U);
    sprintf(line, "Hits: %u", note_hits);
    LCD_printString(line, 40U, 116U, COL_TEXT, 1U);
    sprintf(line, "Perfect: %u", perfect_hits);
    LCD_printString(line, 124U, 116U, COL_TEXT, 1U);
    sprintf(line, "Misses: %u", note_misses);
    LCD_printString(line, 40U, 130U, COL_TEXT, 1U);
    sprintf(line, "Max streak: %u", max_streak);
    LCD_printString(line, 124U, 130U, COL_TEXT, 1U);
    sprintf(line, "Accuracy: %u%%", accuracy);
    LCD_printString(line, 40U, 144U, COL_TEXT, 1U);
    sprintf(line, "Rank: %c", results_rank(accuracy));
    LCD_printString(line, 136U, 144U, COL_YELLOW, 1U);

    draw_option_row(36U, 180U, 168U, "Restart", (uint8_t)(result_selection == 0U), 0U);
    draw_option_row(36U, 208U, 168U, "Main Menu", (uint8_t)(result_selection == 1U), 0U);
    LCD_Refresh(&cfg0);
}

MenuState Game3_Run(void)
{
    MenuState exit_state = MENU_STATE_HOME;
    main_menu_selection = 0U;
    song_select_selection = 0U;
    pause_selection = 0U;
    result_selection = 0U;
    selected_song = 0U;
    last_direction = CENTRE;
    exit_requested = 0U;

    buzzer_off(&buzzer_cfg);
    PWM_SetDuty(&pwm_cfg, 16U);
    set_state(BE_STATE_SPLASH);

    while (1) {
        uint32_t frame_start = HAL_GetTick();
        Input_Read();
        Joystick_Read(&joystick_cfg, &joystick_data);
        switch (game_state) {
            case BE_STATE_SPLASH:
                update_splash();
                render_splash();
                break;
            case BE_STATE_MAIN_MENU:
                update_main_menu();
                render_main_menu();
                break;
            case BE_STATE_INSTRUCTIONS:
                update_instructions();
                render_instructions();
                break;
            case BE_STATE_SONG_SELECT:
                update_song_select();
                render_song_select();
                break;
            case BE_STATE_HIGH_SCORES:
                update_high_scores();
                render_high_scores();
                break;
            case BE_STATE_PLAYING:
                update_playing_logic();
                render_playing_screen(NULL, NULL);
                break;
            case BE_STATE_PAUSED:
                update_pause_menu();
                render_pause_menu();
                break;
            case BE_STATE_LIFE_LOST:
                render_playing_screen("CAUGHT!", "One life lost");
                if ((HAL_GetTick() - state_enter_tick) >= LIFE_LOST_PAUSE_MS) {
                    catch_meter = CATCH_AFTER_LIFE_LOSS;
                    buzzer_off(&buzzer_cfg);
                    resume_song_timer();
                    set_state(BE_STATE_PLAYING);
                }
                break;
            case BE_STATE_GAME_OVER:
                update_result_menu();
                render_stats_card("Game Over", 1U);
                break;
            case BE_STATE_RESULTS:
                update_result_menu();
                render_stats_card("Song Clear", 0U);
                break;
            default:
                buzzer_off(&buzzer_cfg);
                PWM_SetDuty(&pwm_cfg, 16U);
                return exit_state;
        }
        last_direction = joystick_data.direction;
        {
            uint32_t frame_time = HAL_GetTick() - frame_start;
            if (frame_time < GAME_FRAME_TIME_MS) {
                HAL_Delay(GAME_FRAME_TIME_MS - frame_time);
            }
        }
        if (exit_requested) {
            break;
        }
    }
    buzzer_off(&buzzer_cfg);
    PWM_SetDuty(&pwm_cfg, 16U);
    return exit_state;
}