#include <stdio.h>
#include <aquabsd/alps/vga.h>

// images

#include "img/bg.h"
#include "img/apple.h"

// macros and typedefs

#define TILE_SIZE 32

typedef enum {
	TILE_EMPTY,
	TILE_APPLE,

	// bits of snake

	TILE_HEAD,
	TILE_BODY,
	TILE_FAT,
	TIEL_TAIL,
} tile_type_t;

typedef struct {
	tile_type_t type;
} tile_t;

typedef struct {
	unsigned running;

	unsigned tiles_x, tiles_y;
	tile_t* map;

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

static void render_image(game_t* game, unsigned img_width, unsigned img_height, const uint8_t* img_data, unsigned x, unsigned y, unsigned width, unsigned height) {
	const unsigned px_x = width  / img_width;
	const unsigned px_y = height / img_height;

	for (unsigned column = 0; column < width; column++) {
		const uint8_t* img_column = img_data + column / px_x * img_height * 4;

		for (unsigned line = 0; line < height; line++) {
			uint8_t b = img_column[line / px_y * 4 + 0];
			uint8_t g = img_column[line / px_y * 4 + 1];
			uint8_t r = img_column[line / px_y * 4 + 2];
			
			if (img_column[line / px_y * 4 + 3]) { // is opaque?
				PLOT(line + x, column + y, r, g, b)
			}
		}
	}
}

#define RENDER_IMAGE(img) render_image(game, (img).width, (img).height, (img).pixel_data, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);

static void render_tile(game_t* game, tile_t* tile, unsigned x, unsigned y) {
	if (tile->type == TILE_EMPTY) {
		RENDER_IMAGE(img_bg)
	}

	else if (tile->type == TILE_APPLE) {
		RENDER_IMAGE(img_apple)
	}
}

static void render_world(game_t* game) {
	for (int x = 0; x < game->tiles_x; x++) {
		for (int y = 0; y < game->tiles_y; y++) {
			tile_t* tile = ref_tile(game, x, y);
			render_tile(game, tile, x, y);
		}
	}
}

static void place_apple(game_t* game) {
	int x = randint(0, game->tiles_x - 1);
	int y = randint(0, game->tiles_y - 1);

	tile_t* tile = ref_tile(game, x, y);
	tile->type = TILE_APPLE;
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

	game->tiles_x = mode->width  / TILE_SIZE;
	game->tiles_y = mode->height / TILE_SIZE;

	game->map = calloc(game->tiles_x * game->tiles_y, sizeof *game->map);

	place_apple(game);

	// main loop

	game->running = 1;

	while (game->running) {
		int flip_res = vga_flip();

		if (flip_res == 1) { // draw next frame
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
