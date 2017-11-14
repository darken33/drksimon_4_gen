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
// Data for the game control
typedef struct {
	u16 status;
	u16 blink_title;
	u16 palette[64];
	u32 best_score;
	char str_score[64];
	char str_coins[40];
	char str_fscore[40];
	char str_bestsc[40];
} game_data;

// Data for a standard sprite (coins, cars, ...)
typedef struct {
	u16 active;
	u16 column;
	Vect2D_f16 position;
	Sprite* sprite;
} sprite_data;

// Data for thye player
typedef struct {
	Vect2D_f16 position;
	Sprite* sprite;
	u32 score;
	u16 coins;
	fix16 speed;
	u16 flying;
	u16 landing;
	u16 horn_effect;
	u16 horn_col;
} player_data;

// Data for the road
typedef struct {
	u16 scroll;
	u16 nbcars;
} road_data;

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

// Timers
const u16 	TIMER_SPEED_UP		= 0; // Increase player speed
const u16 	TIMER_ACTIVE_CARS	= 1; // Timer to activate a car
const u16 	TIMER_ADD_CARS		= 2; // Timer to add a new car
const u16 	TIMER_FLYING		= 3; // Timer of flying duration
const u16 	TIMER_HORN			= 4; // Timer of horn effect duration
const u16 	TIMER_MUSIC			= 5; // Timer of horn effect duration

// Duration 
const u32	TIME_SPEED_UP 		= 1 * SUBTICKPERSECOND;        // 1s
const u32	TIME_ACTIVE_CARS	= 1 / 100 * SUBTICKPERSECOND;  // 100ms
const u32	TIME_ADD_CARS 		= 15 * SUBTICKPERSECOND;       // 15s
const u32	TIME_FLYING 		= 10 * SUBTICKPERSECOND;       // 10s
const u32	TIME_LANDING 		= 9 * SUBTICKPERSECOND;       // 9s
const u32	TIME_HORN 			= 5 * SUBTICKPERSECOND;        // 5s
const u32	TIME_MUSIC 			= 5 * SUBTICKPERSECOND;        // 5s

const u16 	TITLE_BLINK_FREQ	= 40;	
const u16   SCREEN_FADE_RATE	= 30;   

// Speed
const fix16	SPEED_INC 			= 1;
const fix16	MIN_SPEED 			= 1;
const fix16	MAX_SPEED 			= 5;
const fix16	CAR_SPEED 			= 3;

// Score
const u16 	COIN_SCORE 			= 50;
const u16 	CAR_SCORE 			= 10;

// Nb coins to activate actions
const u16	COIN_BRAKE			= 1;
const u16	COIN_HORN			= 5;
const u16	COIN_PLANE			= 10;

// Player position
const fix32	PLAYER_Y 			= 176;
const fix32	PLAYER_MIN_X 		= 0;
const fix32	PLAYER_MAX_X 		= 154;

// Sprite positions
const fix32	SPRITE_MIN_Y 		= -50;
const fix32	SPRITE_MAX_Y 		= 240;
const u16	SPR_BORDER_X		= 13;
const u16	SPR_BORDER_Y		= 2;
const u16	CAR_WIDTH			= 22;
const u16	CAR_HEIGHT			= 44;
const u16	COIN_WIDTH			= 12;
const u16	COIN_BORDER			= 6;
const u16 	MAX_CARS			= 6;

// About the Road
const u16 	MAX_SCROLL 			= 239;
const u16 	COLUMN_START		= 10;
const u16 	COLUMN_WIDTH 		= 33;
const u16	COLUMN_NUMBER		= 5;

/**********************************************************************/
/** DATAS                                                            **/
/**********************************************************************/
// Game
game_data game;

// Player
player_data player;

// Road
road_data road;

// Cars 
sprite_data cars[20];
u16 last_col[2];

// Coin 
sprite_data coin;

// Explosion
sprite_data explosion;

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
/** EXPLOSION FUNCTION                                               **/
/**********************************************************************/
 void activeExplosion(fix32 x, fix32 y) {
	explosion.position.x=x;
	explosion.position.y=y;	
	explosion.active=TRUE;
	SPR_setVisibility(explosion.sprite, VISIBLE);
 }

/**********************************************************************/
/** COIN FUNCTION                                                    **/
/**********************************************************************/
// test Collision Beetween Player and Coin
u16 testCoinCollision() {
	u16 ret = FALSE;
	if ((coin.position.x+COIN_BORDER >= player.position.x+SPR_BORDER_X  && coin.position.x+COIN_BORDER <= player.position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (coin.position.y+COIN_BORDER >= player.position.y+SPR_BORDER_Y && coin.position.y+COIN_BORDER <= player.position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		ret = TRUE;	
	}  
	else if ((coin.position.x+COIN_BORDER+COIN_WIDTH >= player.position.x+SPR_BORDER_X  && coin.position.x+COIN_BORDER+COIN_WIDTH <= player.position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (coin.position.y+COIN_BORDER >= player.position.y+SPR_BORDER_Y && coin.position.y+COIN_BORDER <= player.position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		ret = TRUE;	
	}  
	else if ((coin.position.x+COIN_BORDER >= player.position.x+SPR_BORDER_X  && coin.position.x+COIN_BORDER <= player.position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (coin.position.y+COIN_BORDER+COIN_WIDTH >= player.position.y+SPR_BORDER_Y && coin.position.y+COIN_BORDER+COIN_WIDTH <= player.position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		ret = TRUE;	
	}  
	else if ((coin.position.x+COIN_BORDER+COIN_WIDTH >= player.position.x+SPR_BORDER_X  && coin.position.x+COIN_BORDER+COIN_WIDTH <= player.position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (coin.position.y+COIN_BORDER+COIN_WIDTH >= player.position.y+SPR_BORDER_Y && coin.position.y+COIN_BORDER+COIN_WIDTH <= player.position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		ret = TRUE;	
	}  
	return ret;
}

/**********************************************************************/
/** CARS FUNCTION                                                    **/
/**********************************************************************/
// Get the Min Y postion of the active cars
fix32 getMinY() {
	int j=0;
	fix32 minY = 0;
	while (j < MAX_CARS) {
		if (cars[j].active == TRUE && cars[j].position.y < minY) {
			minY = cars[j].position.y;
		}		
		j++;
	}	
	return minY;
}

// active a car
void activeCar(int car_idx) {
	int col = rand(COLUMN_NUMBER);
	while (col == last_col[0] || col == last_col[1] || (player.horn_effect == TRUE && player.horn_col == col)) {
		col = rand(COLUMN_NUMBER);
	}
	last_col[1] = last_col[0];
	last_col[0] = col;
	fix32 y = getMinY(); 
	cars[car_idx].column = col; 
	if (y < SPRITE_MIN_Y + CAR_HEIGHT) {
		cars[car_idx].position.y = y - CAR_HEIGHT;
	}
	else {
		cars[car_idx].position.y = SPRITE_MIN_Y;
	}
	cars[car_idx].position.x= last_col[0] * COLUMN_WIDTH + COLUMN_START;
	cars[car_idx].active=TRUE;
	SPR_setAnim(cars[car_idx].sprite, rand(3));
	SPR_setVisibility(cars[car_idx].sprite, VISIBLE);
}

// unactive a car
void unactiveCar(int car_idx) {
 	cars[car_idx].active=FALSE;
	SPR_setVisibility(cars[car_idx].sprite, HIDDEN);
	SPR_setPosition(cars[car_idx].sprite, 0, -50);
}
  
// test Collision Beetween Player and Cars
u16 testCollision(i) {
	u16 ret = FALSE;
	if ((player.position.x+SPR_BORDER_X >= cars[i].position.x+SPR_BORDER_X && player.position.x+SPR_BORDER_X <= cars[i].position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (player.position.y+SPR_BORDER_Y >= cars[i].position.y+SPR_BORDER_Y && player.position.y+SPR_BORDER_Y <= cars[i].position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		activeExplosion(player.position.x+SPR_BORDER_X-16, player.position.y+SPR_BORDER_Y-16);	
		ret = TRUE;	
	}  
	else if ((player.position.x+SPR_BORDER_X+CAR_WIDTH >= cars[i].position.x+SPR_BORDER_X && player.position.x+SPR_BORDER_X+CAR_WIDTH <= cars[i].position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (player.position.y+SPR_BORDER_Y >= cars[i].position.y+SPR_BORDER_Y && player.position.y+SPR_BORDER_Y <= cars[i].position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		activeExplosion(player.position.x+SPR_BORDER_X+CAR_WIDTH-16, player.position.y+SPR_BORDER_Y-16);	
		ret = TRUE;	
	}  
	else if ((player.position.x+SPR_BORDER_X >= cars[i].position.x+SPR_BORDER_X && player.position.x+SPR_BORDER_X <= cars[i].position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (player.position.y+CAR_HEIGHT-SPR_BORDER_Y >= cars[i].position.y+SPR_BORDER_Y && player.position.y+CAR_HEIGHT-SPR_BORDER_Y <= cars[i].position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		activeExplosion(player.position.x+SPR_BORDER_X-16, player.position.y-SPR_BORDER_Y+CAR_HEIGHT-16);	
		ret = TRUE;	
	}  
	else if ((player.position.x+SPR_BORDER_X+CAR_WIDTH >= cars[i].position.x+SPR_BORDER_X && player.position.x+SPR_BORDER_X+CAR_WIDTH <= cars[i].position.x+SPR_BORDER_X+CAR_WIDTH) 
		&& (player.position.y+CAR_HEIGHT-SPR_BORDER_Y >= cars[i].position.y+SPR_BORDER_Y && player.position.y+CAR_HEIGHT-SPR_BORDER_Y <= cars[i].position.y+SPR_BORDER_Y+CAR_HEIGHT)) {
		activeExplosion(player.position.x+SPR_BORDER_X+CAR_WIDTH-16, player.position.y-SPR_BORDER_Y+CAR_HEIGHT-16);	
		ret = TRUE;	
	}  
	return ret;
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

// Show score
void showScore() {
	// Blink the text
	VDP_drawText(game.str_score, 10, 10);
	VDP_drawText(game.str_coins, 10, 11);
	VDP_drawText(game.str_fscore, 10, 12);
	if (game.blink_title > TITLE_BLINK_FREQ/2) {
		VDP_drawText(game.str_bestsc, 10, 14);
	}
	else {
		VDP_clearTextLine(14);
	}
	game.blink_title++;
	if (game.blink_title > TITLE_BLINK_FREQ) {
		game.blink_title = 0;
	}
}  

// PlayGame
void playGame() {
	// Update Scrolling
	VDP_setVerticalScroll (PLAN_A, -1*(road.scroll+270));
	road.scroll+=player.speed;
	if (road.scroll > MAX_SCROLL) {
		road.scroll -= MAX_SCROLL + 1;
	}

	// Increase Player Speed 
	if (getTimer(TIMER_SPEED_UP, FALSE) >= TIME_SPEED_UP) {
		startTimer(TIMER_SPEED_UP);
		if (player.speed < MAX_SPEED) {
			player.speed+=SPEED_INC;
		}
	}
	
	// Joystick Events
	u16 joy_btn = JOY_readJoypad(JOY_1);
	if (joy_btn & BUTTON_LEFT) {
		if (player.position.x > PLAYER_MIN_X) {
			player.position.x -= 2;
		}
	}
	if (joy_btn & BUTTON_RIGHT) {
		if (player.position.x < PLAYER_MAX_X) {
			player.position.x += 2;
		}
	}

	// The player still flying 
	if (player.flying == TRUE && getTimer(TIMER_FLYING, FALSE) >= TIME_LANDING && player.landing == FALSE) {
		player.landing = TRUE;
		SPR_setAnim(player.sprite, 2);
	}
		
	// The player still flying 
	if (player.flying == TRUE && getTimer(TIMER_FLYING, FALSE) >= TIME_FLYING) {
		player.landing = FALSE;
		player.flying = FALSE;
		SPR_setAnim(player.sprite, 0);
	}

	// The player horn effect
	if (player.horn_effect == TRUE && getTimer(TIMER_HORN, FALSE) >= TIME_HORN) {
		player.horn_effect = FALSE;
	}
	
	// Test Collision and show Explosion
	u16 i = 0;
	while (i < MAX_CARS) {
		if (cars[i].active == TRUE) {
			if (player.flying == FALSE && testCollision(i) == TRUE) {
				SND_startPlay_4PCM(crash_sfx, sizeof(crash_sfx), SOUND_PCM_CH1, FALSE); 	
				SPR_setPosition(explosion.sprite, explosion.position.x, explosion.position.y);
				game.status = STATUS_END_GAME;
				break;
			}
		}		
		i++;
	}	
	
	// Display the Player 
	SPR_setPosition(player.sprite, player.position.x, player.position.y);
	
	// Update and display cars 
	i = 0;
	while (i < MAX_CARS) {
		// Play motor
		//XGM_startPlayPCM(SFX_MTR, 1, SOUND_PCM_CH1);
		
		if (cars[i].active == TRUE) {
			SPR_setPosition(cars[i].sprite, cars[i].position.x, cars[i].position.y);
			cars[i].position.y += (player.speed - CAR_SPEED);
			// Unactive car
			if (cars[i].position.y > 240) {
				SND_startPlay_4PCM(horn_sfx, sizeof(horn_sfx), SOUND_PCM_CH2, FALSE); 	
				unactiveCar(i);
				player.score+=CAR_SCORE;
				// Active Coin bonus
				if (coin.active == FALSE) {
					coin.active=TRUE;
					coin.position.y = -50;
					coin.position.x= rand(COLUMN_NUMBER) * COLUMN_WIDTH + 20;
					SPR_setPosition(coin.sprite, coin.position.x, coin.position.y);
					SPR_setVisibility(coin.sprite, VISIBLE);
				}
			}
		}
		i++;
	}
	
	// Try to active a car unactived
	if (getTimer(TIMER_ACTIVE_CARS, FALSE) >= TIME_ACTIVE_CARS) {
		startTimer(TIMER_ACTIVE_CARS);
		i=0;
		while (i < (road.nbcars + 1)) {
			if (cars[i].active == FALSE && rand(100) < 5) {
				activeCar(i);
				break;
			}
			i++;
		}
	}
	
	// Update and display coin bonus
	if (coin.active == TRUE) {
		SPR_setPosition(coin.sprite, coin.position.x, coin.position.y);
		coin.position.y += player.speed;
		// Unactive coin
		if (testCoinCollision() == TRUE) {
			SND_startPlay_4PCM(cash_sfx, sizeof(cash_sfx), SOUND_PCM_CH3, FALSE); 	
			player.coins++;
			coin.active=FALSE;
			coin.position.x = 0;
			coin.position.y = -50;
			SPR_setVisibility(coin.sprite, HIDDEN);
			SPR_setPosition(coin.sprite, coin.position.x, coin.position.y);
		}
		else if (coin.position.y > 240) {
			coin.active=FALSE;
			coin.position.x = 0;
			coin.position.y = -50;
			SPR_setVisibility(coin.sprite, HIDDEN);
			SPR_setPosition(coin.sprite, coin.position.x, coin.position.y);
		}
	}
	
	// Add a new Car
	if (getTimer(TIMER_ADD_CARS, FALSE) >= TIME_ADD_CARS) {
		startTimer(TIMER_ADD_CARS);
		if (road.nbcars < MAX_CARS) {
			road.nbcars++;
		}
	}
	
	// Update Board
	if (player.coins >= COIN_BRAKE) SPR_setAnim(cmd_spr[0], 1);
	else SPR_setAnim(cmd_spr[0], 0);
	if (player.coins >= COIN_HORN) SPR_setAnim(cmd_spr[1], 3);
	else SPR_setAnim(cmd_spr[1], 2);
	if (player.coins >= COIN_PLANE) SPR_setAnim(cmd_spr[2], 5);
	else SPR_setAnim(cmd_spr[2], 4);
	
	// Shows sprites
	SPR_update();
	sprintf(game.str_score, "%lu", player.score);
	sprintf(game.str_coins, "%d", player.coins);
	VDP_drawText(game.str_score, 0, 0);
	VDP_clearText(29, 1, 20);
	VDP_drawText(game.str_coins, 29, 1);
	VDP_waitVSync();

	// Increment score
	player.score++;
}
 
// Start game
void startGame() {
    SYS_disableInts();
	game.status = STATUS_IN_GAME;

    // Road initialisation
    road.scroll = 0;
    road.nbcars = 0;
    VDP_drawImage(PLAN_A, &road_image, 0, 0);

	// Explosion intialisation
	explosion.sprite = SPR_addSprite(&explode_sprite, 0, 0, TILE_ATTR(PAL1, TRUE, FALSE, FALSE)); 
	explosion.active = FALSE;
	SPR_setVisibility(explosion.sprite, HIDDEN);
	SPR_setAnim(explosion.sprite, 0);
    
    // Player initialisation
    player.score = 0;
    player.coins = 0;
	player.speed = MIN_SPEED;
	player.position.x = (PLAYER_MIN_X + PLAYER_MAX_X) / 2;
	player.position.y = PLAYER_Y;
	player.sprite = SPR_addSprite(&player_sprite, 0, 0, TILE_ATTR(PAL1, TRUE, FALSE, FALSE)); 
	player.flying = FALSE;
	player.landing = FALSE;
	player.horn_effect = FALSE;
	SPR_setAnim(player.sprite, 0);
	
	// Cars initiailisation
	last_col[0] = -1;
	last_col[1] = -1;
	int i = 0;
	while (i < MAX_CARS) {
		cars[i].sprite=SPR_addSprite(&cars_sprite, 0, -50, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
		unactiveCar(i);
		i++;
	}

	// Coin intialisation
    coin.active = FALSE;
	coin.sprite = SPR_addSprite(&coin_sprite, 0, SPRITE_MIN_Y, TILE_ATTR(PAL2, TRUE, FALSE, FALSE)); 
	SPR_setVisibility(coin.sprite, HIDDEN);
	SPR_setAnim(coin.sprite, 0);
	
	// Board initiailisation	
	coin_spr = SPR_addSprite(&coin_sprite, 210, 0, TILE_ATTR(PAL2, TRUE, FALSE, FALSE)); 
	SPR_setAnim(coin_spr, 0);
	cmd_spr[0] = SPR_addSprite(&cmd_sprite, 215, 32, TILE_ATTR(PAL1, TRUE, FALSE, FALSE)); 
	SPR_setAnim(cmd_spr[0], 0);
	cmd_spr[1] = SPR_addSprite(&cmd_sprite, 215, 80, TILE_ATTR(PAL1, TRUE, FALSE, FALSE)); 
	SPR_setAnim(cmd_spr[1], 2);
	cmd_spr[2] = SPR_addSprite(&cmd_sprite, 215, 128, TILE_ATTR(PAL1, TRUE, FALSE, FALSE)); 
	SPR_setAnim(cmd_spr[2], 4);
	title_spr = SPR_addSprite(&title_sprite, 210, 190, TILE_ATTR(PAL2, TRUE, FALSE, FALSE)); 
	SPR_setAnim(title_spr, 0);
	VDP_setTextPlan(PLAN_B);	
	VDP_setTextPalette(PAL3);	

    SYS_enableInts();
	
    VDP_fadeIn(0, (4 * 16) - 1, game.palette, SCREEN_FADE_RATE, FALSE);
	VDP_waitVSync();
	startTimer(TIMER_SPEED_UP);
	startTimer(TIMER_ACTIVE_CARS);
	startTimer(TIMER_ADD_CARS);
	SND_startPlay_4PCM(motor_sfx, sizeof(motor_sfx), SOUND_PCM_CH1, TRUE); 	
	
	// Game Loop
    while (game.status == STATUS_IN_GAME) {
		playGame();
	}
	SND_stopPlay_4PCM(SOUND_PCM_CH1); 	

	sprintf(game.str_score, "YOUR SCORE : %lu", player.score);
	sprintf(game.str_coins, "           + %d coins", player.coins);
	player.score+=player.coins * COIN_SCORE;
	sprintf(game.str_fscore, "           = %lu", player.score);
	if (player.score > game.best_score) {
		sprintf(game.str_bestsc, "!!! NEW BEST SCORE !!!");
		game.best_score = player.score;
	}
	else {
		sprintf(game.str_bestsc, "BEST SCORE : %lu", game.best_score);
	}
    waitMs(2500);
	// Show the score
	clearScreen();
	initTitle();
	game.status = STATUS_SCORE;
} 

/**********************************************************************/
/** JOYSTICK HANDLER FUNCTION                                        **/
/**********************************************************************/
void myJoyHandler( u16 joy, u16 changed, u16 state) {
	// JOYSTICK 1
	if (joy == JOY_1) {
		if (game.status == STATUS_TITLE) {
			// BUTTON START Pressed
			if (state & BUTTON_START) {
				SND_startPlay_4PCM(horn2_sfx, sizeof(horn2_sfx), SOUND_PCM_CH1, FALSE); 	
				game.status = STATUS_START_GAME;
			}
		}
		else if (game.status == STATUS_IN_GAME) {
			// BUTTON A Pressed : BRAKE
			if (state & BUTTON_A && player.coins >= COIN_BRAKE) {
				SND_startPlay_4PCM(brake_sfx, sizeof(brake_sfx), SOUND_PCM_CH4, FALSE); 	
				player.coins -= COIN_BRAKE;
				player.speed = MIN_SPEED;
			}
			// BUTTON B Pressed : HORN
			if (state & BUTTON_B  && player.coins >= COIN_HORN) {
				SND_startPlay_4PCM(horn2_sfx, sizeof(horn2_sfx), SOUND_PCM_CH4, FALSE); 	
				player.coins -= COIN_HORN;
				player.horn_effect=TRUE;
				player.horn_col = (player.position.x - PLAYER_MIN_X) / ((PLAYER_MAX_X+16 - PLAYER_MIN_X) / COLUMN_NUMBER);
				u16 i = 0;
				while (i < MAX_CARS) {
					if (cars[i].active == TRUE && cars[i].column == player.horn_col) {
						unactiveCar(i);
					}
					i++;
				}
				startTimer(TIMER_HORN);
			}
			// BUTTON C Pressed : PLANE
			if (state & BUTTON_C  && player.coins >= COIN_PLANE) {
				SND_startPlay_4PCM(plane_sfx, sizeof(plane_sfx), SOUND_PCM_CH4, FALSE); 	
				SPR_setAnim(player.sprite, 1);
				player.coins -= COIN_PLANE;
				player.flying = TRUE;
				startTimer(TIMER_FLYING);
			}
		}
		else if (game.status == STATUS_SCORE) {
			// BUTTON START Pressed
			if ((state & BUTTON_START)
				|| (state & BUTTON_A)
				|| (state & BUTTON_B)
				|| (state & BUTTON_C)) {
				SND_startPlay_4PCM(cash_sfx, sizeof(cash_sfx), SOUND_PCM_CH1, FALSE); 	
				clearScreen();
				VDP_clearTextLine(10);
				VDP_clearTextLine(11);
				VDP_clearTextLine(12);
				VDP_clearTextLine(14);
				initTitle();
				game.status = STATUS_TITLE;
			}
		}
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
