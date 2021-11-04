#include <stdio.h>

#include <aquabsd/alps/vga.h>
#include <aquabsd/alps/kbd.h>

// images

#include "img/bg.h"
#include "img/apple.h"
#include "img/head.h"
#include "img/body.h"
#include "img/curve.h"

// macros and typedefs

#define TILE_SIZE 32

typedef enum {
	TILE_EMPTY,
	TILE_APPLE,
} tile_type_t;

typedef struct {
	tile_type_t type;
} tile_t;

typedef enum {
	UP,
	RIGHT,
	DOWN,
	LEFT,
} direction_t;

typedef struct snake_bit_t snake_bit_t; // forward declaration

struct snake_bit_t {
	snake_bit_t* next; // if tail, this will be 'NULL'
	unsigned fat; // basically, was there an apple on this tile?

	unsigned x, y;
	direction_t direction;
};

typedef struct {
	unsigned running;

	unsigned tiles_x, tiles_y;
	tile_t* map;

	direction_t direction;
	snake_bit_t* snake;

	unsigned width, height;
	uint8_t* framebuffer;
} game_t;

static int curr_rand = 1234567890;

static int randint(int a, int b) {
	curr_rand = (420 * curr_rand + 69) % 0x10001;

	if (curr_rand < 0) {
		curr_rand = -curr_rand;
	}

	return curr_rand % (b - a + 1) + a;
}

static inline tile_t* ref_tile(game_t* game, unsigned x, unsigned y) {
	return &game->map[y * game->tiles_x + x];
}

#define PLOT(x, y, r, g, b) \
	game->framebuffer[4 * ((y) * game->width + (x)) + 0] = (r); \
	game->framebuffer[4 * ((y) * game->width + (x)) + 1] = (g); \
	game->framebuffer[4 * ((y) * game->width + (x)) + 2] = (b);

static void render_image(game_t* game, unsigned img_width, unsigned img_height, const uint8_t* img_data, unsigned x, unsigned y, unsigned width, unsigned height, direction_t direction) {
	const unsigned px_x = width  / img_width;
	const unsigned px_y = height / img_height;

	for (unsigned column = 0; column < width; column++) {
		const uint8_t* img_column = img_data + column / px_x * img_height * 4;

		for (unsigned line = 0; line < height; line++) {
			uint8_t b = img_column[line / px_y * 4 + 0];
			uint8_t g = img_column[line / px_y * 4 + 1];
			uint8_t r = img_column[line / px_y * 4 + 2];
			
			if (img_column[line / px_y * 4 + 3]) { // is opaque?
				switch (direction) {
					case UP:    PLOT(line + x, column + y, r, g, b) break;
					case DOWN:  PLOT(width - line + x - 1, height - column + y - 1, r, g, b) break;
					case LEFT:  PLOT(column + x, height - line + y - 1, r, g, b) break;
					case RIGHT: PLOT(width - column + x - 1, line + y, r, g, b) break;
				}
			}
		}
	}
}

#define RENDER_IMAGE(img, x, y, direction) render_image(game, (img).width, (img).height, (img).pixel_data, (x) * TILE_SIZE, (y) * TILE_SIZE, TILE_SIZE, TILE_SIZE, direction);

static void render_tile(game_t* game, tile_t* tile, unsigned x, unsigned y) {
	RENDER_IMAGE(img_bg, x, y, UP)

	if (tile->type == TILE_APPLE) {
		RENDER_IMAGE(img_apple, x, y, UP)
	}
}

#define RENDER_BIT(img, direction) RENDER_IMAGE((img), bit->x, bit->y, (direction))

static void render_snake(game_t* game) {
	snake_bit_t* bit = game->snake;

	// render head

	RENDER_BIT(img_head, bit->direction)

	// render rest of body & tail

	snake_bit_t* prev = bit;

	while ((bit = bit->next)) {
		if (!bit->next) {
			RENDER_BIT(img_body, prev->direction) // render tail
		}

		else if (prev->direction != bit->direction) {
			int delta = prev->direction - bit->direction;
			RENDER_BIT(img_curve, (bit->direction - (delta == 1 || delta == -3)) % 4)
		}

		else {
			RENDER_BIT(img_body, bit->direction)
		}
		
		// TODO tail too (by checking 'bit->next == NULL')
		// TODO also 'bit->fat'

		prev = bit;
	}
}

static void render_world(game_t* game) {
	for (int x = 0; x < game->tiles_x; x++) {
		for (int y = 0; y < game->tiles_y; y++) {
			tile_t* tile = ref_tile(game, x, y);
			render_tile(game, tile, x, y);
		}
	}

	render_snake(game);
}

static void place_apple(game_t* game) {
	int x = randint(0, game->tiles_x - 1);
	int y = randint(0, game->tiles_y - 1);

	tile_t* tile = ref_tile(game, x, y);
	tile->type = TILE_APPLE;
}

static void update(game_t* game) {
	snake_bit_t* prev_head = game->snake;

	snake_bit_t* head = calloc(1, sizeof *head);
	head->next = prev_head;

	head->x = prev_head->x;
	head->y = prev_head->y;

	switch ((head->direction = game->direction)) {
		case UP:    head->y--; break;
		case DOWN:  head->y++; break;
		case LEFT:  head->x--; break;
		case RIGHT: head->x++; break; 
	}

	game->snake = head;
	tile_t* tile = ref_tile(game, head->x, head->y); // what tile are we on?

	if (tile->type == TILE_APPLE) {
		tile->type = TILE_EMPTY;
		place_apple(game);

		head->fat = 1;
		return;
	}

	// get tail and delete it
	// and while we're at it, check if any body part is intersecting the head

	snake_bit_t* bit = head;
	int intersection = 0;
	
	while (bit->next->next) {
		bit = bit->next;

		if (bit->x == head->x && bit->y == head->y) {
			intersection = 1;
		}
	}

	free(bit->next);
	bit->next = NULL;

	if (intersection) {
		// make the player lose the game
		printf("intersection\n");
	}
}

int main(void) {
	game_t* game = calloc(1, sizeof *game);

	// setup vga

	vga_init();

	int mode_count = vga_get_mode_count();
	vga_mode_t* modes = vga_get_modes();

	vga_mode_t* mode = &modes[0];

	mode->width = 800;//1024;
	mode->height = 576;//768;

	vga_set_mode(mode);
	game->framebuffer = vga_get_framebuffer();

	game->width = mode->width;
	game->height = mode->height;

	// setup

	curr_rand = (uint64_t) game; // set the seed to something approximatively unpredictable

	kbd_t kbd = kbd_get_default();

	game->tiles_x = mode->width  / TILE_SIZE;
	game->tiles_y = mode->height / TILE_SIZE;

	game->map = calloc(game->tiles_x * game->tiles_y, sizeof *game->map);

	game->direction = UP;

	game->snake = calloc(1, sizeof *game->snake);
	game->snake->direction = game->direction;

	game->snake->x = game->tiles_x / 2;
	game->snake->y = game->tiles_y / 2;

	for (int i = 0; i < 30; i++) {
		place_apple(game);
	}

	// main loop

	game->running = 1;

	unsigned pressed = 0;
	float seconds = 0.0;

	while (game->running) {
		int flip_res = vga_flip();

		if (flip_res == 1) { // draw next frame
			float delta = 1.0 / mode->fps;
			seconds += delta;

			kbd_update(kbd);
			unsigned keypress = 1;

			if      (kbd_poll_button(kbd, KBD_BUTTON_UP   )) game->direction = UP;
			else if (kbd_poll_button(kbd, KBD_BUTTON_DOWN )) game->direction = DOWN;
			else if (kbd_poll_button(kbd, KBD_BUTTON_LEFT )) game->direction = LEFT;
			else if (kbd_poll_button(kbd, KBD_BUTTON_RIGHT)) game->direction = RIGHT;

			else {
				keypress = 0;
			}

			if (game->direction % 2 == game->snake->direction % 2) {
				game->direction = game->snake->direction;
			}

			if (keypress) {
				if (!pressed) {
					seconds = 0;
					update(game);
				}

				pressed = 1;

			} else {
				pressed = 0;
			}

			if (seconds > 0.1) {
				seconds = 0;
				update(game);
			}
			
			render_world(game);
		}

		else if (flip_res == -1) { // quit
			game->running = 0;
		}	
	}

	// cleanup
	
	vga_reset();

	free(game->map);
	free(game);

	return 0;
}
