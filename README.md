# Flappy Bird Allegro 5

A Flappy Bird clone made using Allegro 5. It was a project made in a C Language course..

## How to play

1. First of all, you need to install Allegro 5

    Read the <a href="https://github.com/liballeg/allegro_wiki/wiki/Quickstart#installation">installation guide</a> for your operational system.

2. Clone the repository

    ```bash
     $ git clone https://github.com/Campz/FlappyBirdAllegro.git
    ```

    or download as zip and extract to a folder.

3. Enter in the directory

    Linux:

    ```bash
    $ gcc game.c -o game $(pkg-config allegro-5 allegro_font-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 --libs --cflags)
    $ ./game
    ```
    Windows:

    ```bash
    $ gcc game.c -o game -lallegro -lallegro_font -lallegro_primitives -lallegro_audio -lallegro_acodec -lallegro_image
    $ ./game
    ```
## Gameplay

Use the spacebar to control the bird's fly and avoid obstacles

## Preview