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

int score = 0;
char score_string[3];

FILE *best_score_file = NULL;
int best_score = 0;
char best_score_string[5];
int best_score_was_updated = 0;

int isOnMenu = 1;
int isOnHowToPlay = 0;

void must_init(bool test, const char *description)
{
    if (test)
        return;

    printf("couldn't initialize %s\n", description);
    exit(1);
}

void get_best_score()
{
    best_score_file = fopen("bestScore.txt", "r");
    if (best_score_file != NULL)
    {
        fgets(best_score_string, 4, best_score_file);
        best_score = atoi(best_score_string);
    }
    else
    {
        best_score_file = fopen("bestScore.txt", "w");
        fputc('0', best_score_file);
    }
    fclose(best_score_file);
}

int is_best_score()
{
    if (score > best_score)
    {
        return 1;
    }
    return 0;
}

void update_best_score()
{
    best_score_file = fopen("bestScore.txt", "w");
    fputs(score_string, best_score_file);
    fclose(best_score_file);
    best_score_was_updated = 1;
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
    ALLEGRO_BITMAP *floor;

    ALLEGRO_BITMAP *pipe_sheet;
    ALLEGRO_BITMAP *pipe[2];

    ALLEGRO_BITMAP *game_over;
    ALLEGRO_BITMAP *scoreboard;
    ALLEGRO_BITMAP *medal_sheet;
    ALLEGRO_BITMAP *medal[4];
    ALLEGRO_BITMAP *new_best;
    ALLEGRO_BITMAP *buttons;
    ALLEGRO_BITMAP *bt_ok;
    ALLEGRO_BITMAP *bt_menu;

    ALLEGRO_BITMAP *get_ready;
    ALLEGRO_BITMAP *how_to_play;

    ALLEGRO_BITMAP *game_title;
    ALLEGRO_BITMAP *play_button;

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
    // Inicializa sprite do fundo
    sprites.background = al_load_bitmap("./assets/background.png");
    must_init(sprites.background, "background");

    sprites.floor = al_load_bitmap("./assets/floor.png");
    must_init(sprites.floor, "floor");

    // Inicializa sprites dos canos
    sprites.pipe_sheet = al_load_bitmap("./assets/pipe.png");
    must_init(sprites.pipe_sheet, "pipe");

    sprites.pipe[0] = sprite_grab(sprites.pipe_sheet, 0, 0, PIPE_W, PIPE_H);
    sprites.pipe[1] = sprite_grab(sprites.pipe_sheet, PIPE_W + 2, 0, PIPE_W, PIPE_H);

    // Inicializa sprite do scoreboard
    sprites.scoreboard = al_load_bitmap("./assets/scoreboard.png");
    must_init(sprites.scoreboard, "scoreboard");

    sprites.new_best = al_load_bitmap("./assets/new.png");
    must_init(sprites.new_best, "new best score");

    sprites.buttons = al_load_bitmap("./assets/buttons.png");
    must_init(sprites.buttons, "buttons");

    sprites.bt_menu = sprite_grab(sprites.buttons, 0, 0, 40, 14);
    sprites.bt_ok = sprite_grab(sprites.buttons, 0, 16, 40, 14);

    // Inicializa sprite das medalhas
    sprites.medal_sheet = al_load_bitmap("./assets/medal.png");
    must_init(sprites.medal_sheet, "medal");

    sprites.medal[0] = sprite_grab(sprites.medal_sheet, 0, 72, 22, 22);
    sprites.medal[1] = sprite_grab(sprites.medal_sheet, 0, 48, 22, 22);
    sprites.medal[2] = sprite_grab(sprites.medal_sheet, 0, 24, 22, 22);
    sprites.medal[3] = sprite_grab(sprites.medal_sheet, 0, 0, 22, 22);

    // Inicializa os sprites do menu
    sprites.game_title = al_load_bitmap("./assets/flappybird.png");
    must_init(sprites.game_title, "title");

    sprites.play_button = al_load_bitmap("./assets/playbutton.png");
    must_init(sprites.play_button, "play");

    // Inicializa os sprites do tutorial
    sprites.how_to_play = al_load_bitmap("./assets/howtoplay.png");
    must_init(sprites.how_to_play, "tutorial");

    sprites.get_ready = al_load_bitmap("./assets/getready.png");
    must_init(sprites.get_ready, "ready");

    // Inicializa sprite do gameover
    sprites.game_over = al_load_bitmap("./assets/gameover.png");
    must_init(sprites.game_over, "gameover");

    // Inicializa sprites do jogador
    sprites.player_sheet = al_load_bitmap("./assets/player.png");
    must_init(sprites.player_sheet, "playersheet");

    sprites.player = sprite_grab(sprites.player_sheet, 0, 0, PLAYER_W, PLAYER_H);
}

void sprites_denit()
{
    al_destroy_bitmap(sprites.player);

    al_destroy_bitmap(sprites.background);
    al_destroy_bitmap(sprites.floor);

    al_destroy_bitmap(sprites.pipe_sheet);
    al_destroy_bitmap(sprites.pipe[0]);
    al_destroy_bitmap(sprites.pipe[1]);

    al_destroy_bitmap(sprites.player_sheet);
    al_destroy_bitmap(sprites.scoreboard);
    al_destroy_bitmap(sprites.new_best);

    al_destroy_bitmap(sprites.medal_sheet);
    al_destroy_bitmap(sprites.medal[0]);
    al_destroy_bitmap(sprites.medal[1]);
    al_destroy_bitmap(sprites.medal[2]);
    al_destroy_bitmap(sprites.medal[3]);

    al_destroy_bitmap(sprites.game_over);
    al_destroy_bitmap(sprites.buttons);
    al_destroy_bitmap(sprites.bt_menu);
    al_destroy_bitmap(sprites.bt_ok);

    al_destroy_bitmap(sprites.game_title);
    al_destroy_bitmap(sprites.play_button);

    al_destroy_bitmap(sprites.how_to_play);
    al_destroy_bitmap(sprites.get_ready);
}

// --- audio ---

// --- hud ---

void score_draw(ALLEGRO_FONT *font, int isAlive)
{
    sprintf(score_string, "%i", score);
    if (isAlive)
    {
        al_draw_text(font, al_map_rgb(255, 255, 255), (BUFFER_W / 2) - 2, 10, 0, score_string);
    }
    else
    {
        al_draw_text(font, al_map_rgb(251, 120, 88), BUFFER_W - 40, BUFFER_H - 153, 0, score_string);
    }
}

void scoreboard_draw(ALLEGRO_FONT *font)
{
    al_draw_bitmap(sprites.game_over, BUFFER_W / 9 + 8, BUFFER_W / 3 - 5, 0);
    al_draw_bitmap(sprites.scoreboard, BUFFER_W / 9, BUFFER_H / 3, 0);
    sprintf(best_score_string, "%i", best_score);
    al_draw_text(font, al_map_rgb(251, 120, 88), BUFFER_W - 40, BUFFER_H - 133, 0, best_score_string);
    al_draw_bitmap(sprites.bt_ok, 27, 150, 0);
    al_draw_bitmap(sprites.bt_menu, 77, 150, 0);
    if (best_score_was_updated)
    {
        al_draw_bitmap(sprites.new_best, 82, BUFFER_H - 142, 0);
    }
    if (score < 10)
    {
        al_draw_bitmap(sprites.medal[0], (BUFFER_W / 9) + 13, (BUFFER_H / 3) + 21, 0);
    }
    if (score >= 10 && score < 20)
    {
        al_draw_bitmap(sprites.medal[1], (BUFFER_W / 9) + 13, (BUFFER_H / 3) + 21, 0);
    }
    if (score >= 20 && score < 30)
    {
        al_draw_bitmap(sprites.medal[2], (BUFFER_W / 9) + 13, (BUFFER_H / 3) + 21, 0);
    }
    if (score >= 30)
    {
        al_draw_bitmap(sprites.medal[3], (BUFFER_W / 9) + 13, (BUFFER_H / 3) + 21, 0);
    }
}

void menu_draw(ALLEGRO_FONT *font)
{
    al_draw_bitmap(sprites.game_title, BUFFER_W / 5, BUFFER_H / 10, 0);
    al_draw_bitmap(sprites.play_button, BUFFER_W / 3.2, BUFFER_H / 3, 0);
    // al_draw_text(font, al_map_rgb(255, 255, 255), BUFFER_W / 12, BUFFER_H - 15, 0, "(c) Campos 2021");
    al_draw_bitmap(sprites.player, BUFFER_W - 30, 50, 0);
}

void tutorial_draw()
{
    al_draw_bitmap(sprites.get_ready, BUFFER_W / 9 + 8, BUFFER_W / 3 - 5, 0);
    al_draw_bitmap(sprites.how_to_play, BUFFER_W / 3.5, BUFFER_H / 2, 0);
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
    player.x = 20;
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

#define NPIPES_MAX 99999
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

        pipes[i].isScored = 0;
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
        // Desenha apenas se o cano estiver no campo de visão da tela
        if (pipes[i].x + PIPE_W > 0 && pipes[i].x < BUFFER_W)
        {
            al_draw_bitmap(sprites.pipe[0], pipes[i].x, pipes[i].y, 0);

            // O cano de baixo é desenhado pegando a coordenada y do de cima somado com a altura do cano + um espaçamento definido
            al_draw_bitmap(sprites.pipe[1], pipes[i].x, pipes[i].y + PIPE_H + PIPE_SPACE_BETWEEN, 0);
        }
    }
}

//--- main ---

int main()
{
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_install_mouse(), "mouse");

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
    al_register_event_source(queue, al_get_mouse_event_source());

    keyboard_init();
    player_init();
    pipe_init();

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
            // Lógica do menu principal
            if (isOnMenu)
            {
                if (key[ALLEGRO_KEY_A])
                {
                    isOnMenu = 0;
                    isOnHowToPlay = 1;
                }
            }
            else
            {
                // Lógica da tela de tutorial
                if (isOnHowToPlay)
                {
                    if (key[ALLEGRO_KEY_SPACE])
                    {
                        isOnHowToPlay = 0;
                    }
                }
                else
                {
                    // Lógica principal do jogo
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
                        player.y = player.y - 2.4;
                    }
                    else
                    {
                        // Lógica da tela de gameover
                        if (key[ALLEGRO_KEY_ESCAPE])
                        {
                            isOnHowToPlay = 1;
                            player_init();
                            pipe_init();
                            score = 0;
                            best_score_was_updated = 0;
                        }
                    }
                }
            }

            for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;

            redraw = true;
            break;

        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (isOnMenu)
            {
                if (event.mouse.x >= DISP_W / 3.2 && event.mouse.x <= (DISP_W / 3.2) + 52 * 3)
                {
                    if (event.mouse.y >= DISP_H / 3 && event.mouse.y <= (DISP_H / 3) + 29 * 3)
                    {
                        isOnMenu = 0;
                        isOnHowToPlay = 1;
                    }
                }
            }
            else
            {
                if (!player.isAlive)
                {
                    if (event.mouse.y >= 150 * DISP_SCALE && event.mouse.y <= 164 * DISP_SCALE)
                    {
                        if (event.mouse.x >= 27 * DISP_SCALE && event.mouse.x <= 67 * DISP_SCALE)
                        {
                            isOnHowToPlay = 1;
                            player_init();
                            pipe_init();
                            score = 0;
                            best_score_was_updated = 0;
                        }
                        if (event.mouse.x >= 77 * DISP_SCALE && event.mouse.x <= 117 * DISP_SCALE)
                        {
                            isOnMenu = 1;
                            player_init();
                            pipe_init();
                            score = 0;
                            best_score_was_updated = 0;
                        }
                    }
                }
            }
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

            if (isOnMenu)
            {
                menu_draw(font);
                al_draw_bitmap(sprites.floor, 0, BUFFER_H - 56, 0);
            }
            else
            {
                if (isOnHowToPlay)
                {
                    tutorial_draw();
                }

                pipe_draw();
                if (player.isAlive)
                {
                    player_draw();
                }
                else
                {
                    get_best_score();
                    if (is_best_score())
                    {
                        update_best_score();
                        get_best_score();
                    }
                    scoreboard_draw(font);
                }
                score_draw(font, player.isAlive);
            }
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
