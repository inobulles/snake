#include "game.h"

static int draw(uint64_t _, uint64_t _game, uint64_t _dt) {
	game_t* game = (void*) _game;
	double dt = *(double*) &_dt;

	return game_draw(game, dt);
}

int main(void) {
	game_t* game = calloc(1, sizeof *game);

	// setup window

	game->width  = 800;
	game->height = 600;

	game->win = win_create(game->width, game->height);
	win_set_caption(game->win, "Snake");

	game->framebuffer = win_get_fb(game->win, 32);

	game_init(game);

	win_register_cb(game->win, WIN_CB_DRAW, draw, game);
	win_loop(game->win);

	printf("Score: %d\n", game->score);

	win_delete(game->win);
	game_free(game);
	free(game);

	return 0;
}
