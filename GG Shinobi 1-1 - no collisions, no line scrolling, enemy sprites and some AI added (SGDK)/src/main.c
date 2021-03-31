/*By Pigsy 31 March 2021
Remake of Level 1-1 of GG Shinobi

Background artwork my own, but based on GG Shinobi

Sprite artwork:
Modified shinobi and soldier sprite taken from Revenge of Shinobi
Other sprite work mostly my own, with some inspiration and/or small sections taken from Splatterhouse and Spiderman games

Just a first draft, many bugs (details provided in comments throughout), plus the artwork is still unfinished and will be reworked and improved on in the future
*/

#include <genesis.h>
#include <resources.h>


//ALL SPRITE ANIMATION

#define ANIM_STAND  0
#define ANIM_WALK   1
#define ANIM_SWORD  2
#define ANIM_CROUCH 3
#define ANIM_CROUCH_ATTACK 4
#define ANIM_JUMP 5
#define ANIM_JUMP_ATTACK 6

#define SOLDIER_ANIM_SIT 0
#define SOLDIER_ANIM_RELOAD 1
#define SOLDIER_ANIM_SHOOT 2

#define BOMBER_ANIM_WAIT 0
#define BOMBER_ANIM_CLIMB 1



//MAP SET UP AND RELATED VARIABLES

#define MAP_WIDTH 2536
#define MAP_HEIGHT 472

Map* bgb;
Map* bga;

s16 camPosX;
s16 camPosY;

u16 bgBaseTileIndex[2];



//PLAYER SPRITE AND RELATED VARIABLES

#define MIN_PLAYER_POSX 10
#define MAX_PLAYER_POSX 2450

Sprite* player;
int playerX = 100;
fix16 playerY = FIX16(360);
int player_vel_x = 1;
bool faceLeft = FALSE;
fix16 player_vel_y = FIX16(0);
int player_height = 80;
const int floor_height = 440;
fix16 gravity = FIX16(0.2);
bool jumping = FALSE;
bool swapButton = FALSE; //Has X button been pushed? (to change shinobi colour)
int shinobiC = 0; //Will eventually be able to cycle between all 5 shinobi/ninja colours, but just green and red included for now



//ENEMY SOLDIER SPRITE AND RELATED VARIABLES

#define MAX_BULLETS 2
#define BULLET_COUNTER 10
#define BULLET_VELOCITY 2
#define RELOAD_COUNTER 120

Sprite* bullet;
int bulletX;
int bulletY;

int shooting_tempo = 0;

struct bullets{
    int a, x, y;
    int type;
    Sprite* spr;
};

struct bullets bullet_list[MAX_BULLETS];

Sprite* soldier1;
int soldier1X = 873;
int soldier1Y = 249;



//BOMBER ENEMY SPRITE AND RELATED VARIABLES

#define BOMB_EXPLODE_TIME 100
#define ANIM_EXPLOSION 0
#define EXPLOSION_LIFESPAN 30

Sprite* bomber1;
int bomber1X = 256;
int bomber1Y = 365;

Sprite* bomber2;
int bomber2X = 875;
int bomber2Y = 341;

Sprite* explosion;
int explosionX;
int explosionY;

int bomb_timer = 0;
int currentExplosionDuration = 0;
bool explode = FALSE;



//MASKED ENEMY SPRITE AND RELATED VARIABLES

Sprite* jason1;
int jason1X = 416;
int jason1Y = 307;

Sprite* jason2;
int jason2X = 775;
int jason2Y = 283;

Sprite* jason3;
int jason3X = 945;
int jason3Y = 234;



// FUNCTION PROTOTYPES
//Note: the soldierAI and related bullet functions work fine when the bomberAI and manage explosion functions are deleted/commented out 
//(firing 2 bullets, then pausing, then firing another two bullets, and so on...)
//However, once bomberAI and manageExplosions are enabled (as it is here), the soldier fires the bullets as normal, but they don't travel across the screen
//I have as yet been unable to ascertain the cause of this bug. Very strange!

static void handleInput();
static void setSpritePosition(Sprite* sprite, s16 x, s16 y);
static void setCameraPosition(s16 x, s16 y);
static void updateCameraPosition();
static void soldierAI();
static void createBulletList();
static void createBullet();
static void manageBullet();
static void bomberAI();
static void manageExplosion();
static void changeShinobi();
//No AI behaviour included for the masked/jason sprite at present


int main()
{
    //CREATE BACKGROUND MAPS

    u16 ind;

    VDP_setScreenWidth320();

    VDP_setPalette(PAL0, bgb_map.palette->data);
    VDP_setPalette(PAL1, bga_map.palette->data);

    ind = TILE_USERINDEX;
    bgBaseTileIndex[0] = ind;
    VDP_loadTileSet(&bgb_tileset, ind, DMA);
    ind += bgb_tileset.numTile;
    bgBaseTileIndex[1] = ind;
    VDP_loadTileSet(&bga_tileset, ind, DMA);
    ind += bga_tileset.numTile;

    bgb = MAP_create(&bgb_map, BG_B, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bgBaseTileIndex[0]));
    bga = MAP_create(&bga_map, BG_A, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, bgBaseTileIndex[1]));

    updateCameraPosition();
    SYS_doVBlankProcess();

    //LOAD ALL SPRITES

    VDP_setPalette(PAL2, red_shinobi_sprite.palette->data);
    VDP_setPalette(PAL3, bomber_sprite.palette->data);

    SPR_init(0, 0, 0);
    player = SPR_addSprite(&red_shinobi_sprite, playerX, fix16ToInt(playerY), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setAnim(player, ANIM_STAND);
    bomber1 = SPR_addSprite(&bomber_sprite, bomber1X - camPosX, bomber1Y - camPosY, TILE_ATTR(PAL3, 0, 0, FALSE));
    bomber2 = SPR_addSprite(&bomber_sprite, bomber2X - camPosX, bomber2Y - camPosY, TILE_ATTR(PAL3, 0, 0, FALSE));
    jason1 = SPR_addSprite(&jason_sprite, jason1X - camPosX, jason1Y - camPosY, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    jason2 = SPR_addSprite(&jason_sprite, jason2X - camPosX, jason2Y - camPosY, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    jason3 = SPR_addSprite(&jason_sprite, jason3X - camPosX, jason3Y - camPosY, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    soldier1 = SPR_addSprite(&soldier_sprite, soldier1X - camPosX, soldier1Y - camPosY, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    

    //MISC

    createBulletList();



    while(1)
    {

        handleInput();

        if (swapButton == TRUE) changeShinobi();

        bomberAI();
        if (explode) manageExplosion();
        soldierAI();
        manageBullet();
        
        SPR_update();

        SYS_doVBlankProcess();
    }

    MEM_free(bgb);
    MEM_free(bga);

    return (0);
}


static void handleInput()
{
    u16 value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_LEFT)
    {
        faceLeft = TRUE;
        SPR_setHFlip(player, TRUE);
        playerX -= player_vel_x;
        if (playerX <= MIN_PLAYER_POSX) playerX = MIN_PLAYER_POSX;
        if (jumping == FALSE) SPR_setAnim(player, ANIM_WALK);
    }

    if (value & BUTTON_RIGHT)
    {
        faceLeft = FALSE;
        SPR_setHFlip(player, FALSE);
        playerX += player_vel_x;
        if (playerX >= MAX_PLAYER_POSX) playerX = MAX_PLAYER_POSX;
        if (jumping == FALSE) SPR_setAnim(player, ANIM_WALK);
    }

    if (value & BUTTON_B)
    //Currently, the B button needs to be held down in order for the attack animation to complete
    //I need to find a way to have the animation play faster than the othe animations (walking etc), and
    //complete with just a single push of the B button
    {
        SPR_setAnim(player, ANIM_SWORD);
    }

    if (value & BUTTON_C)
    {
        if (jumping == FALSE)
        {
            jumping = TRUE;
            player_vel_y = FIX16(-6);
        }
    }

    if (value & BUTTON_X) 
    //X button currently oversensitive, making switching between red and green ninjas a litle difficult (too fast!)
    {
        swapButton = TRUE;
    }

    if ( !(value & BUTTON_LEFT) && !(value & BUTTON_RIGHT) && !(value & BUTTON_B) && jumping == FALSE)
    {
        SPR_setAnim(player, ANIM_STAND);
    }

    playerY = fix16Add(playerY, player_vel_y);

    if (jumping == TRUE)
    {
        player_vel_y = fix16Add(player_vel_y, gravity);
        SPR_setAnim(player, ANIM_JUMP);
    }

    if (jumping == TRUE && fix16ToInt(playerY) + player_height >= floor_height) 
    {
        jumping = FALSE;
        player_vel_y = FIX16(0);
        playerY = intToFix16(floor_height - player_height);
        SPR_setAnim(player, ANIM_STAND);
    }

    updateCameraPosition();

    //I had an issue creating structs for each enemy type, so I will do it the long way for now
    setSpritePosition(player, playerX - camPosX, fix16ToInt(playerY) - camPosY);
    setSpritePosition(bomber1, bomber1X - camPosX, bomber1Y - camPosY);
    setSpritePosition(bomber2, bomber2X - camPosX, bomber2Y - camPosY);
    setSpritePosition(jason1, jason1X - camPosX, jason1Y - camPosY);
    setSpritePosition(jason2, jason2X - camPosX, jason2Y - camPosY);
    setSpritePosition(jason3, jason3X - camPosX, jason3Y - camPosY);
    setSpritePosition(soldier1, soldier1X - camPosX, soldier1Y - camPosY);
    
}


static void changeShinobi()
//There are 5 different coloured ninjas in the game, and this will allow cycling through them by pushing the X button (only green and red ninjas included for now)
{
    
        shinobiC += 1;
        if (shinobiC > 1) shinobiC = 0;
        if (shinobiC == 0) {
            SPR_releaseSprite(player);
            VDP_setPalette(PAL2, green_shinobi_sprite.palette->data);
            player = SPR_addSprite(&green_shinobi_sprite, playerX, fix16ToInt(playerY), TILE_ATTR(PAL2, 0, FALSE, FALSE));
            if (faceLeft == TRUE) SPR_setHFlip(player, TRUE);
        }
        else if (shinobiC == 1) {
            SPR_releaseSprite(player);
            VDP_setPalette(PAL2, red_shinobi_sprite.palette->data);
            player = SPR_addSprite(&red_shinobi_sprite, playerX, fix16ToInt(playerY), TILE_ATTR(PAL2, 0, FALSE, FALSE));
            if (faceLeft == TRUE) SPR_setHFlip(player, TRUE);
        }
        swapButton = FALSE;
        
}


static void updateCameraPosition()
{
    // Get player position (pixel)
    s16 px = playerX;
    s16 py = playerY;
    // Current sprite position on screen
    s16 px_scr = px - camPosX;
    s16 py_scr = py - camPosY;

    s16 npx_cam, npy_cam;

    // Adjust new camera position
    if (px_scr > 170) npx_cam = px - 170;
    else if (px_scr < 40) npx_cam = px - 40;
    else npx_cam = camPosX;
    if (py_scr > 140) npy_cam = py - 140;
    else if (py_scr < 60) npy_cam = py - 60;
    else npy_cam = camPosY;

    // Clip camera position
    if (npx_cam < 0) npx_cam = 0;
    else if (npx_cam > (MAP_WIDTH - 320)) npx_cam = (MAP_WIDTH - 320);
    if (npy_cam < 0) npy_cam = 0;
    else if (npy_cam > (MAP_HEIGHT - 224)) npy_cam = (MAP_HEIGHT - 224);

    // Set new camera position
    setCameraPosition(npx_cam, npy_cam);
}


static void setCameraPosition(s16 x, s16 y)
{
    if ((x != camPosX) || (y != camPosY))
    {
        camPosX = x;
        camPosY = y;

        //There appears to be something wrong with the way the vertical crolling works - i.e. it doesn't scroll at all when MAP_scroll to is set to y!
        //I have just set y co-ordinates for both backgrounds to a manual figure for now, until I can work out why there is no vertical scrolling
        MAP_scrollTo(bgb, x >> 3, 50);
        MAP_scrollTo(bga, x, 247);
        
        
    }
}


static void setSpritePosition(Sprite* sprite, s16 x, s16 y)
{
    // Clip out of screen sprites
    if ((x < -100) || (x > 320) || (y < -100) || (y > 240)) SPR_setVisibility(sprite, HIDDEN);
    else
    {
        SPR_setVisibility(sprite, VISIBLE);
        SPR_setPosition(sprite, x, y);
    }
}


static void soldierAI()
{
    if (soldier1X - playerX < 100 && shooting_tempo == 0 && (bullet_list[0].a != 1 || bullet_list[1].a != 1)) 
    //If player is within 100 pixels of player and shooting_tempo=0 (creates gap between the two bullets so they aren't both fired at pretty much the same time)
    //and at least one of the 2 bullets in the list is makred as not fired (a=0), then can create and fire new bullet
    {
        SPR_setAnim(soldier1, SOLDIER_ANIM_SHOOT);
        createBullet(0);
        shooting_tempo = 1;
    }

    else 
    {
        SPR_setAnim(soldier1, SOLDIER_ANIM_SIT);
    }
    
    //Manages how quickly the two bullets are fired one after the other (don't want them both fired at the same time - this creates a little gap/interval)
    if (shooting_tempo > 0) shooting_tempo ++;
    if (shooting_tempo > BULLET_COUNTER) shooting_tempo = 0;
}


static void createBulletList() 
// Create array of bullets fired by soldier (max of 2 bullets on screen at any time)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullet_list[i].a = 0;
        bullet_list[i].x = 0;
        bullet_list[i].y = 0;
        bullet_list[i].type = 0;
        bullet_list[i].spr = NULL;
    }
}


static void createBullet(int type) 
// Create bullets fired by soldier
{
    for (int i = 0; i < MAX_BULLETS; i++) 
    {
        if (bullet_list[i].a == 0) //only create if a bullet is free in the list (max of 2 bullets on screen at the same time)
        {
            bullet_list[i].a = 1; // marks bullet in list as "fired"
            bullet_list[i].x = soldier1X;
            bullet_list[i].y = soldier1Y + 12; //position adjusted so that bullet appears to emerge from muzzle of soldier's gun
            bullet_list[i].type = type; //currently only a sinlge type of bullet exists - will add more later
            bullet_list[i].spr = SPR_addSprite(&bullet_sprite, bullet_list[i].x, bullet_list[i].y, TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
            break;
        }
    }
}


static void manageBullet() 
// Manage bullets fired by soldier
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullet_list[i].a != 0) //Only manage bullets that have been fired (i.e. those marked with a=1)
        {
            if (bullet_list[i].type == 0) bullet_list[i].x -= BULLET_VELOCITY; //I have only created one type of bullet so far, but plan to add other types at a later date

            setSpritePosition(bullet_list[i].spr, bullet_list[i].x - camPosX, bullet_list[i].y - camPosY);

            if (bullet_list[i].x - camPosX < 0) //Delete offscreen bullet and free up a place in the bullet list for another to be fired
            {
                bullet_list[i].a = 0;
                SPR_releaseSprite(bullet_list[i].spr);
            }
        }
    }  
}


static void bomberAI() 
//Effective for the first bomber sprite only for now
{
    if (bomber1X - playerX < 100) //Have the bomber enemy begin to climb as soon as the player approaches within 100 pixels
    {
        SPR_setAnim(bomber1, BOMBER_ANIM_CLIMB);
        bomber1Y -= 1;
        setSpritePosition(bomber1, bomber1X - camPosX, bomber1Y - camPosY);

        if (bomber1Y < 333) //Have the bomber sprite disappear and set explosion to TRUE as soon as he reaches top of the lorry/truck (once the timer reaches explosion time)
        {
            bomber1Y = 333;
            SPR_setAnim(bomber1, BOMBER_ANIM_WAIT);
            bomb_timer += 1;
            if (bomb_timer >= BOMB_EXPLODE_TIME)
            {
                explosionX = bomber1X;
                explosionY = bomber1Y;
                SPR_releaseSprite(bomber1);
                explode = TRUE;
            }

        }
    }
}


static void manageExplosion() 
// Create explosion once the bomber sprite is ready to set off bomb
{
    explosion = SPR_addSprite(&explosion_sprite, explosionX - camPosX, explosionY - camPosY, TILE_ATTR(PAL1, 0, FALSE, FALSE));
    setSpritePosition(explosion, explosionX - camPosX, explosionY - camPosY);
    SPR_setAnim(explosion, ANIM_EXPLOSION);
    
    currentExplosionDuration += 1;

    if (currentExplosionDuration > EXPLOSION_LIFESPAN) //Manages how long explosion is present on screen
    {
        SPR_releaseSprite(explosion);
        explode = FALSE;
    }
}