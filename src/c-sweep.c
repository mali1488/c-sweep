#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"

#define GAME_TITLE "C-Sweep"
#define FPS 60
#define WIDTH 600
#define HEIGHT 600
#define GRID_SIZE 10
#define COLOR_OPEN GREEN
#define COLOR_MINE RED
#define COLOR_NOT_VISITED RAYWHITE

typedef enum {
  EASY = 0,
  NORMAL = 1,
  HARD = 2,
  SUPER_HARD = 3
} Difficulty;

typedef enum {
  NOT_VISITED = 0,
  OPEN = 1,
  MINE = 2
} MineState;

typedef enum {
  PLAYING = 0,
  WON = 1,
  LOST = 2
} GameState;

typedef struct {
  MineState tiles[GRID_SIZE*GRID_SIZE];
  bool is_first_move;
  GameState game_state;
} Game;

int tile_index(int row, int col) {
  return row*GRID_SIZE + col;
}

MineState tile_state_at(Game *game, int row, int col) {
  return game->tiles[tile_index(row, col)];
}

void tile_state_update(Game *game, int row, int col, MineState state) {
  game->tiles[tile_index(row, col)] = state;
}

Color color_for_state(MineState state) {
  switch (state) {
  case NOT_VISITED: return COLOR_NOT_VISITED;
  case OPEN: return COLOR_OPEN;
  case MINE: return COLOR_MINE;
  }
}

Color color_for_number_of_adjacent(int adjacent) {
  switch (adjacent) {
  case 0: return COLOR_OPEN;
  case 1: return BLUE;
  case 2: return YELLOW;
  case 3: return ORANGE;
  default: return COLOR_MINE;
  }
}

bool is_valid(int row, int col) {
  if (row < 0 || col < 0) {
    return false;
  }
  if (row >= GRID_SIZE || col >= GRID_SIZE) {
    return false;
  }
  return true;
}

int count_adjacent(Game *game, int row, int col) {
  int number_of_mines = 0;
  for(int i = -1; i < 2; i++) {
    for(int j = -1; j < 2; j++) {
      if (i == 0 && j == 0) {
	continue;
      }
      const int dx = row + i;
      const int dy = col + j;
      if (!is_valid(dx, dy)) {
	continue;
      }
      MineState adjacent = tile_state_at(game, dx, dy);
      if (adjacent == MINE) {
	number_of_mines += 1;
      }
    }
  }
  return number_of_mines;
}

void open_cells_with_non_adjacent_mines(Game *game, int row, int col) {
  if (!is_valid(row, col)) {
    return;
  }
  MineState curr = tile_state_at(game, row, col);
  if (curr == MINE || curr == OPEN) {
    return;
  }
  tile_state_update(game, row, col, OPEN);
  const int adjacent_mines = count_adjacent(game, row, col);
  if (adjacent_mines > 0) {
    return;
  }
  for(int i = -1; i < 2; i++) {
    for(int j = -1; j < 2; j++) {
      if (i == 0 && j == 0) {
	continue;
      }
      open_cells_with_non_adjacent_mines(game, row + i, col + j);
    }
  }
}

void move_mine(Game *game, int row, int col) {
  bool moved = false;
  while (!moved) {
    const int r = rand() % GRID_SIZE;
    const int c = rand() % GRID_SIZE;
    if (row != r && c != col) {
      MineState random_state = tile_state_at(game, row, col);
      if (random_state != MINE) {
	tile_state_update(game, r, c, MINE);
	moved = true;
      }
   }
  }
}

void update_if_won(Game *game) {
  for(int row = 0; row < GRID_SIZE; row++) {
    for(int col = 0; col < GRID_SIZE; col++) {
      MineState state = tile_state_at(game, row, col);
      if (state == NOT_VISITED) {
	return;
      }
    }
  }
  game->game_state = WON;
}

void update_game(Game *game) {
  if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    return;
  }
  Vector2 mouse_pos = GetMousePosition();
  float mine_size = WIDTH / GRID_SIZE;
  int row = mouse_pos.x / mine_size;
  int col = mouse_pos.y / mine_size;
  MineState state = tile_state_at(game, row, col);
  switch (state) {
  case NOT_VISITED: {
    open_cells_with_non_adjacent_mines(game, row, col);
    update_if_won(game);
    break;
  }
  case OPEN:
    break;
  case MINE:
    if (game->is_first_move) {
      tile_state_update(game, row, col, OPEN);
      move_mine(game, row, col);
    } else {
      game->game_state = LOST;
    }
    break;
  }
  game->is_first_move = false;
}

float difficulty_multiplier(Difficulty difficulty) {
  switch (difficulty) {
  case EASY: return 0.1;
  case NORMAL: return 0.2;
  case HARD: return 0.4;
  case SUPER_HARD: return 0.6;
  }
}

void generate_mines(Game *game, Difficulty difficulty) {
  int number_of_mines = GRID_SIZE * GRID_SIZE * difficulty_multiplier(difficulty);
  while (number_of_mines > 0) {
    const int row = rand() % GRID_SIZE;
    const int col = rand() % GRID_SIZE;
    MineState state = tile_state_at(game, row, col);
    if (state == NOT_VISITED) {
      tile_state_update(game, row, col, MINE);
      number_of_mines--;
    }
  }
}

Game game_init() {
  Game game = {
    .is_first_move = true,
    .game_state = PLAYING
  };
  generate_mines(&game, NORMAL);
  return game;
}

void int_to_char(int n, char* buff) {
    sprintf(buff, "%d", n);
}

void render_game(Game game) {
  const float mine_size = WIDTH / GRID_SIZE;
  const float padding = 1;
  for(size_t row = 0; row < GRID_SIZE; row++) {
    for(size_t col = 0; col < GRID_SIZE; col++) {
      const MineState state = tile_state_at(&game, row, col);
      const float x = row * mine_size + padding;
      const float y = col * mine_size + padding;
      Rectangle rec = {
	  .x = x,
	  .y = y,
	  .width = mine_size - padding * 2,
	  .height = mine_size - padding * 2,
      };
      DrawRectangleRec(rec, color_for_state(state));
      if (state == OPEN) {
	const int count = count_adjacent(&game, row, col);
	char buff[8];
	int_to_char(count, buff);

	const int font_size = mine_size * 0.9;
	const int size = MeasureText(buff, font_size);
	const float text_x = x + mine_size / 2 - size / 2;
	const float text_y = y + mine_size / 2 - font_size / 2;
	DrawText(
	    buff,
	    text_x,
	    text_y,
	    font_size,
	    color_for_number_of_adjacent(count)
	);
      }
    }
  }
}

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(WIDTH, HEIGHT, GAME_TITLE);
  SetWindowMinSize(200, 400);
  SetTargetFPS(FPS);

  Game game = game_init();
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    update_game(&game);
    if (game.game_state == LOST) {
      // TODO: Make nice lost screen
      puts("lost =(!");
    }
    if (game.game_state == WON) {
      // TODO: Make nice win screen
      puts("won =)!");
    }
    // TODO: Implement ability to flag unknown tiles
    render_game(game);

    EndDrawing();
  }
  CloseWindow();

  return 0;
}
