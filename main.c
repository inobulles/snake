#include "game.h"

int main(void) {
	game_t* game = calloc(1, sizeof *game);

	// setup vga

	vga_init();

	int mode_count = vga_get_mode_count();
	vga_mode_t* modes = vga_get_modes();

	vga_mode_t* mode = &modes[0];

	// mode->width = 800;//1024;
	// mode->height = 576;//768;

	for (int i = 0; i < mode_count; i++) {
		mode = &modes[i];

		if (mode->width == 1024 && mode->height == 768 && mode->bpp == 32) {
			break;
		}
	}

	vga_set_mode(mode);
	game->framebuffer = vga_get_framebuffer();

	game->width = mode->width;
	game->height = mode->height;

	game->fps = mode->fps;

	int score = play_game(game);

	if (score >= 0) {
		printf("Score: %d\n", score);
	}

	vga_reset();
	free(game);

	return 0;
}