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
long score = 0;
char score_string[3];

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

    ALLEGRO_BITMAP *pipe_sheet;

    ALLEGRO_BITMAP *pipe[2];

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

    sprites.pipe_sheet = al_load_bitmap("./assets/pipe.png");
    must_init(sprites.pipe_sheet, "pipe");

    sprites.pipe[0] = sprite_grab(sprites.pipe_sheet, 0, 0, PIPE_W, PIPE_H);
    sprites.pipe[1] = sprite_grab(sprites.pipe_sheet, PIPE_W + 2, 0, PIPE_W, PIPE_H);

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

    al_destroy_bitmap(sprites.pipe_sheet);
    al_destroy_bitmap(sprites.pipe[0]);
    al_destroy_bitmap(sprites.pipe[1]);

    al_destroy_bitmap(sprites.player_sheet);
    al_destroy_bitmap(sprites.background);
}

// --- audio ---

    ALLEGRO_SAMPLE *background_sound;
    ALLEGRO_SAMPLE *jump_sound;
    ALLEGRO_SAMPLE *fp2;
    ALLEGRO_SAMPLE *fp3;
    ALLEGRO_SAMPLE *fp4;
    ALLEGRO_SAMPLE *fp5;

void audio_init()
{
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(128);

    background_sound = al_load_sample("./sounds/fp0.wav");
    must_init(background_sound, "background_song0");
    
    jump_sound = al_load_sample("./sounds/jump_sound.wav");
    must_init(jump_sound, "background_song1");

    fp2 = al_load_sample("./sounds/fp2.wav");
    must_init(fp2, "background_song2");

    fp3 = al_load_sample("./sounds/fp3.wav");
    must_init(fp3, "background_song3");

    fp4 = al_load_sample("./sounds/fp4.wav");
    must_init(fp4, "background_song4");

    fp5 = al_load_sample("./sounds/fp5.wav");
    must_init(fp5, "background_song5");

    al_play_sample(background_sound,0.75,0,1, ALLEGRO_PLAYMODE_LOOP,NULL);
}

void audio_denit(){
    al_destroy_sample(background_sound);
    al_destroy_sample(jump_sound);
    al_destroy_sample(fp2);
    al_destroy_sample(fp3);
    al_destroy_sample(fp4);
    al_destroy_sample(fp5);
}

// --- hud ---

void score_draw(ALLEGRO_FONT *font)
{
    sprintf(score_string, "%i", score);
    al_draw_text(font, al_map_rgb(255, 255, 255), (BUFFER_W / 2) - 2, 10, 0, score_string);
}

// --- player ---

typedef struct PLAYER
{
    float x, y, gravity;
    int isAlive;

} PLAYER;

PLAYER player;

void player_init()
{
    player.x = 10;
    player.y = 100;
    player.gravity = 0;
    player.isAlive = 1;
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

// --- obtacles ---

#define NPIPES_MAX 999
#define PIPE_SPACE_BETWEEN 45

typedef struct PIPE
{
    float x, y;
    int isScored;
} PIPE;

PIPE pipes[NPIPES_MAX];

int isPlayerColliding(float player_x, float player_y, float pipe_x, float pipe_y)
{
    // Se o jogador estiver posicionado na região entre os canos
    if (player_x + PLAYER_W >= pipe_x && player_x <= pipe_x + PIPE_W)
    {
        // E o jogador encontar no cano de cima
        if (player_y >= pipe_y && player_y <= pipe_y + PIPE_H)
        {
            return 1;
        }

        // Ou o jogador encostar no cano de baixo
        if (player_y + PLAYER_H >= pipe_y + PIPE_H + PIPE_SPACE_BETWEEN && player_y + PLAYER_H <= pipe_y + PIPE_H + PIPE_H + PIPE_SPACE_BETWEEN)
        {
            return 1;
        }
    }

    // Se o jogador ultrapassar dos limites da tela
    if (player_y <= 0 || player_y >= BUFFER_H)
    {
        return 1;
    }

    return 0;
}

void pipe_init()
{
    srand(time(NULL));
    // O primeiro cano do jogo é inicializado no final do campo de visão do display
    pipes[0].x = BUFFER_W;
    // Uma coordenada y aleatória é gerado para o cano
    pipes[0].y = (rand() % 100) - 100;

    pipes[0].isScored = 0;
    for (int i = 1; i < NPIPES_MAX; i++)
    {
        // A coordenada x dos demais canos possuem um espaçamento de 80 em relação ao cano anterior a ele
        pipes[i].x = pipes[i - 1].x + 80;
        // Uma coordenada y aleatória é gerado para o cano
        pipes[i].y = (rand() % 100) - 100;

        pipes[0].isScored = 0;
    }
}

void pipe_update()
{
    for (int i = 0; i < NPIPES_MAX; i++)
    {
        // Move todos os canos para a esquerda
        pipes[i].x = pipes[i].x - 0.7;

        // Verfica se o jogador está colidindo com um dos canos e mata ele se positivo
        if (isPlayerColliding(player.x, player.y, pipes[i].x, pipes[i].y))
        {
            player.isAlive = 0;
        }

        // Conta um score para o jogador quando um cano passa por ele
        if (pipes[i].x + PIPE_W < player.x && !pipes[i].isScored && player.isAlive)
        {
            score++;
            pipes[i].isScored = 1;
        }
    }
}

void pipe_draw()
{
    for (int i = 0; i < NPIPES_MAX; i++)
    {
        al_draw_bitmap(sprites.pipe[0], pipes[i].x, pipes[i].y, 0);

        // O cano de baixo é desenhado pegando a coordenada y do de cima somado com a altura do cano + um espaçamento definido
        al_draw_bitmap(sprites.pipe[1], pipes[i].x, pipes[i].y + PIPE_H + PIPE_SPACE_BETWEEN, 0);
    }
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
    pipe_init();
    audio_init();

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
            pipe_update();
            if (player.isAlive)
            {

                player.gravity = player.gravity + 0.3;
                if (player.y < 461 && !key[ALLEGRO_KEY_SPACE])
                {
                    player.y = player.y + player.gravity;
                }
                if (key[ALLEGRO_KEY_SPACE])
                    player.gravity = 0;
                    al_play_sample(jump_sound,0.75,0,1, ALLEGRO_PLAYMODE_ONCE,NULL);
                player.y = player.y - 2.5;
            }

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
            if (player.isAlive)
            {
                player_draw();
            }
            pipe_draw();
            score_draw(font);
            disp_post_draw();
            redraw = false;
        }
    }

    sprites_denit();
    disp_deinit();
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    audio_denit();

    return 0;
}
