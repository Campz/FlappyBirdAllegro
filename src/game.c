#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>

// --- general ---

long frames;
long score;

void must_init(bool test, const char *description)
{
    if (test)
        return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

int between(int lo, int hi)
{
    return lo + (rand() % (hi - lo));
}

float between_f(float lo, float hi)
{
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2)
{
    if (ax1 > bx2)
        return false;
    if (ax2 < bx1)
        return false;
    if (ay1 > by2)
        return false;
    if (ay2 < by1)
        return false;

    return true;
}

// --- display ---

#define BUFFER_W 144
#define BUFFER_H 256

#define DISP_SCALE 3
#define DISP_W (BUFFER_W * DISP_SCALE)
#define DISP_H (BUFFER_H * DISP_SCALE)

ALLEGRO_DISPLAY *disp;
ALLEGRO_BITMAP *buffer;

void disp_init()
{
    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

    disp = al_create_display(DISP_W, DISP_H);
    must_init(disp, "display");

    buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    must_init(buffer, "bitmap buffer");
}

void disp_deinit()
{
    al_destroy_bitmap(buffer);
    al_destroy_display(disp);
}

void disp_pre_draw()
{
    al_set_target_bitmap(buffer);
}

void disp_post_draw()
{
    al_set_target_backbuffer(disp);
    al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);

    al_flip_display();
}

// --- keyboard ---

#define KEY_SEEN 1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init()
{
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT *event)
{
    switch (event->type)
    {
    case ALLEGRO_EVENT_TIMER:
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
            key[i] &= KEY_SEEN;
        break;

    case ALLEGRO_EVENT_KEY_DOWN:
        key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
        break;
    case ALLEGRO_EVENT_KEY_UP:
        key[event->keyboard.keycode] &= KEY_RELEASED;
        break;
    }
}

// --- sprites ---

#define PLAYER_W 17
#define PLAYER_H 12

#define PIPE_W 26
#define PIPE_H 160

typedef struct SPRITES
{
    ALLEGRO_BITMAP *player_sheet;

    ALLEGRO_BITMAP *player;

    ALLEGRO_BITMAP *background;

} SPRITES;

SPRITES sprites;

ALLEGRO_BITMAP *sprite_grab(ALLEGRO_BITMAP *sheet, int x, int y, int w, int h)
{
    ALLEGRO_BITMAP *sprite = al_create_sub_bitmap(sheet, x, y, w, h);
    must_init(sprite, "sprite grab");
    return sprite;
}

void sprites_init()
{
    sprites.background = al_load_bitmap("./assets/background.png");
    must_init(sprites.background, "background");

    sprites.player_sheet = al_load_bitmap("./assets/player.png");
    must_init(sprites.player_sheet, "playersheet");

    sprites.player = sprite_grab(sprites.player_sheet, 0, 0, PLAYER_W, PLAYER_H);
    //sprites.player[1] = sprite_grab(sprites.player_sheet, 28, 0, PLAYER_W, PLAYER_H);
    //sprites.player[2] = sprite_grab(sprites.player_sheet, 56, 0, PLAYER_W, PLAYER_H);
}

void sprites_denit()
{
    al_destroy_bitmap(sprites.player);
    //al_destroy_bitmap(sprites.player[1]);
    //al_destroy_bitmap(sprites.player[2]);

    al_destroy_bitmap(sprites.player_sheet);
    al_destroy_bitmap(sprites.background);
}

// --- audio ---

// --- obtacles ---

// --- hud ---

// --- player ---

typedef struct PLAYER
{
    float x, y, gravity;

} PLAYER;

PLAYER player;

void player_init()
{
    player.x = 10;
    player.y = 100;
    player.gravity = 0;
}

void player_update()
{
    player.gravity = player.gravity + 0.4;
    if (player.y < 461 && !key[ALLEGRO_KEY_SPACE])
    {
        player.y = player.y + player.gravity;
    }
    if (key[ALLEGRO_KEY_SPACE])
    {
        player.gravity = 0;
        player.y = player.y - 5;
    }
}

void player_draw()
{
    al_draw_bitmap(sprites.player, player.x, player.y, 0);
}

//--- main ---

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / 60.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    must_init(queue, "queue");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

    disp_init();

    must_init(al_init_image_addon(), "image");
    sprites_init();

    ALLEGRO_FONT *font = al_create_builtin_font();
    must_init(font, "font");

    must_init(al_init_primitives_addon(), "primitives");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    keyboard_init();
    player_init();

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    al_start_timer(timer);
    while (1)
    {
        al_wait_for_event(queue, &event);

        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            player.gravity = player.gravity + 0.3;
            if (player.y < 461 && !key[ALLEGRO_KEY_SPACE])
            {
                player.y = player.y + player.gravity;
            }
            if (key[ALLEGRO_KEY_SPACE])
                player.gravity = 0;
            player.y = player.y - 3;

            if (key[ALLEGRO_KEY_ESCAPE])
                done = true;

            for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;

            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            key[event.keyboard.keycode] &= KEY_RELEASED;
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done)
            break;

        if (redraw && al_is_event_queue_empty(queue))
        {
            disp_pre_draw();
            al_clear_to_color(al_map_rgb(0, 0, 0));

            al_draw_bitmap(sprites.background, 0, 0, 0);
            player_draw();

            disp_post_draw();
            redraw = false;
        }
    }

    sprites_denit();
    disp_deinit();
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
