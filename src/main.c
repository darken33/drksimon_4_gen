#include <genesis.h>

#include "gfx.h"
#include "sprite.h"
#include "sfx.h"
#include "maths.h"


/* ===================================================================== 
 * Drk Simon 
 * =====================================================================
 * The simon says game for genesis
 * November 2017
 * By Philippe Bousquet <darken33@free.fr>
 * This software is under Gnu General Public License v3
 */

/**********************************************************************/
/** DATA TYPES DEFINITION                                            **/ 
/**********************************************************************/

/**********************************************************************/
/** CONSTANTS                                                        **/
/**********************************************************************/
// Game Version
const char*	GAME_VERSION 		= "v1.0 - 2017";

// Game Status
const u16  	STATUS_TITLE 		= 1; // Title 
const u16 	STATUS_START_GAME 	= 2; // Starting the game
const u16 	STATUS_IN_GAME 		= 3; // Game pending
const u16 	STATUS_END_GAME		= 4; // End game
const u16 	STATUS_SCORE 		= 5; // Score screen


/**********************************************************************/
/** DATAS                                                            **/
/**********************************************************************/

// Board
Sprite* coin_spr;
Sprite* cmd_spr[3];
Sprite* title_spr;

/**********************************************************************/
/** SPECIFIC RANDOM FUNCTION                                         **/
/**********************************************************************/
u16 rand(u16 num) {
	return (getSubTick() + random()) % num;
}

/**********************************************************************/
/** GAME FUNCTION                                                    **/
/**********************************************************************/
// Clear the Screen
void clearScreen() {
    SYS_disableInts();
	VDP_fadeOut(0, (4 * 16) - 1, SCREEN_FADE_RATE, FALSE);
	VDP_init();
    VDP_setScreenWidth320();
	VDP_setTextPlan(PLAN_B);	
	VDP_clearTextArea(0, 0, 24, 80);
    SPR_init(80, 16 * (32 + 16 + 8), 16 * (32 + 16 + 8));
    SYS_enableInts();
    VDP_waitVSync();
}

// Init title
void initTitle() {
	game.blink_title = 0;
    VDP_drawImage(PLAN_A, &title_image, 0, 0);
	VDP_setTextPlan(PLAN_B);	
    VDP_fadeIn(0, (4 * 16) - 1, title_image.palette->data, SCREEN_FADE_RATE, FALSE);
    VDP_setPalette(PAL3, palette_grey);
	VDP_setTextPalette(PAL3);	
    VDP_waitVSync();
} 
// Show title
void showTitle() {
	if (game.best_score > 0) {
		sprintf(game.str_bestsc, "%lu", game.best_score);
		VDP_drawText(game.str_bestsc, 0, 0);
	}
	VDP_drawText(GAME_VERSION, 0, 27);
	// Blink the text
	if (game.blink_title > TITLE_BLINK_FREQ/2) {
		VDP_drawText("Press START", 15, 12);
	}
	else {
		VDP_clearTextLine(12);
	}
	game.blink_title++;
	if (game.blink_title > TITLE_BLINK_FREQ) {
		game.blink_title = 0;
	}
}  


/**********************************************************************/
/** MAIN LOOP                                                        **/
/**********************************************************************/
int main() {
	// Initialize VDP & SPR
    SYS_disableInts();
    game.best_score = 0;
    VDP_init();
    VDP_setScreenWidth320();
    SPR_init(80, 16 * (32 + 16 + 8), 16 * (32 + 16 + 8));
    SYS_enableInts();
    
    // Set palette to Black
    VDP_setPaletteColors(0, (u16*) palette_black, 64);

    // JOY initialization
	JOY_init();
	JOY_setEventHandler( &myJoyHandler );

    // Prepare palettes 
	memcpy(&game.palette[0], road_image.palette->data, 16*2);
    memcpy(&game.palette[16], cars_sprite.palette->data, 16*2);
    memcpy(&game.palette[32], title_sprite.palette->data, 16*2);
    memcpy(&game.palette[48], palette_grey, 16*2);
	
	// Introduction 
	VDP_setTextPlan(PLAN_B);
	VDP_drawText("Philippe Bousquet", 12, 10);
	VDP_drawText("<Darken33>", 15, 11);
	VDP_drawText("presents", 16, 15);
    VDP_fadeIn(0, (4 * 16) - 1, (u16*) palette_grey, 20, FALSE);
	VDP_waitVSync();
    waitMs(5000);
	
	// Display title
	clearScreen();
	initTitle();
	game.status = STATUS_TITLE;

	// Control loop
    while (TRUE) {
		if (game.status == STATUS_TITLE) {
			showTitle();
		}
		else if (game.status == STATUS_START_GAME) {
			clearScreen();
			startGame();
		}
		else if (game.status == STATUS_SCORE) {
			showScore();
		}
        VDP_waitVSync();
    }

    return 0;
}
