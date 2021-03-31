/*
GG Shinobi Megadrive Remake by Pigsy
Just a single, unmoving screen to test out line scrolling effects
I need to find a way to get the mountains/buildings section to scroll at less than 1 speed
Otherwise they looks fine. I just need to work out how to replicate the effect with large background MAPs

I also attempted to get the Game Gear music for the level playing, but it's not working (perhaps unsurprisingly)

*/

#include <genesis.h>
#include <resources.h>

#define ANIM_STAND  0
#define ANIM_WALK   1
#define ANIM_SWORD  2
#define ANIM_CROUCH 3
#define ANIM_CROUCH_ATTACK 4
#define ANIM_JUMP 5
#define ANIM_JUMP_ATTACK 6

Sprite* player;
int playerX = 20;
int playerY = 100;
int playerV = 1;
bool swapButton = FALSE;
int shinobiC = 0;
bool faceLeft = FALSE;

fix16 buildings_scroll_speed = 1;
fix16 road_and_fence_scroll_speed = 20;

s16 buildings[64];
s16 largeFence[80];
s16 road[56];
s16 smallFence[24];

static void handleInput();
static void changeShinobi();



int main()
{
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    XGM_setLoopNumber(-1);
    XGM_startPlay(rush);

    u16 ind;

    

    for (int i = 0; i < 64; i++) buildings[i] = 0;
    for (int i = 0; i < 80; i++) largeFence[i] = 0;
    for (int i = 0; i < 56; i++) road[i] = 0;
    for (int i = 0; i < 24; i++) smallFence[i] = 0;

    VDP_setPalette(PAL0, highway.palette->data);
    VDP_setPalette(PAL2, cars.palette->data);
    ind = TILE_USERINDEX;
    VDP_drawImageEx(BG_B, &highway, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += highway.tileset->numTile;
    VDP_drawImageEx(BG_A, &cars, TILE_ATTR_FULL(PAL2, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
    ind += cars.tileset->numTile;

    

    VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);

    

    VDP_setPalette(PAL1, shinobi_sprite_green.palette->data);

    SPR_init(0, 0, 0);
    player = SPR_addSprite(&shinobi_sprite_green, playerX, playerY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    SPR_setAnim(player, ANIM_STAND);
    


    while(1)
    {
        
        VDP_setHorizontalScrollLine(BG_B, 0, buildings, 64, CPU);
        for(int i = 0; i < 64; i++) buildings[i] += buildings_scroll_speed;

        VDP_setHorizontalScrollLine(BG_B, 64, largeFence, 80, CPU);
        for(int i = 0; i < 80; i++) largeFence[i] += road_and_fence_scroll_speed;

        VDP_setHorizontalScrollLine(BG_B, 144, road, 56, CPU);
        for(int i = 0; i < 56; i++) road[i] -= road_and_fence_scroll_speed;

        VDP_setHorizontalScrollLine(BG_B, 200, smallFence, 24, CPU);
        for(int i = 0; i < 24; i++) smallFence[i] += road_and_fence_scroll_speed;
        
        

        handleInput();
        
        if (swapButton == TRUE) changeShinobi();
        

        SPR_update();

        SYS_doVBlankProcess();
    }
    return (0);
}

static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);
    if (value & BUTTON_LEFT) {
        faceLeft = TRUE;
         
        SPR_setHFlip(player, TRUE); 
        SPR_setPosition(player, playerX -= playerV, playerY);
        SPR_setAnim(player, ANIM_WALK);
        }
    if (value & BUTTON_RIGHT) {
        faceLeft = FALSE;
        SPR_setHFlip(player, FALSE);
        SPR_setPosition(player, playerX += playerV, playerY);
        SPR_setAnim(player, ANIM_WALK);
    }
    if (value & BUTTON_B) {
        SPR_setAnim(player, ANIM_SWORD);
    }
    if (value & BUTTON_X) {
        swapButton = TRUE;
    }
    if ((!(value & BUTTON_LEFT)) && (!(value & BUTTON_RIGHT)) && (!(value & BUTTON_B))){
        SPR_setAnim(player, ANIM_STAND);
    }
}

static void changeShinobi()
{
    
        shinobiC +=1;
        if (shinobiC > 1) shinobiC = 0;
        if (shinobiC == 0) {
            SPR_releaseSprite(player);
            VDP_setPalette(PAL1, shinobi_sprite_green.palette->data);
            player = SPR_addSprite(&shinobi_sprite_green, playerX, playerY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
            if (faceLeft == TRUE) SPR_setHFlip(player, TRUE);
        }
        else if (shinobiC == 1) {
            SPR_releaseSprite(player);
            VDP_setPalette(PAL1, shinobi_sprite_red.palette->data);
            player = SPR_addSprite(&shinobi_sprite_red, playerX, playerY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
            if (faceLeft == TRUE) SPR_setHFlip(player, TRUE);
        }
        swapButton = FALSE;
        
}