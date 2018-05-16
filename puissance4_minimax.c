#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>
#include <assert.h>
#include <limits.h>

#define HEIGHT     2
#define WIDTH      3
#define NB_LINES   6
#define NB_COLS    7

#define NO_PLAYER  0
#define PLAYER_1   1
#define PLAYER_2   2
#define BOARD      3

#define COLOR_PLAYER_1  COLOR_RED
#define COLOR_PLAYER_2  COLOR_YELLOW
#define COLOR_BOARD     COLOR_BLUE


struct game_state {
	unsigned char *board;
	unsigned char turn;
	unsigned char winner;
	int candidate_pos;
};


static void
draw_disc(int startx, int starty, int posx, int posy, unsigned char player)
{
	int x = posx * WIDTH + 1;
	int y = posy * HEIGHT + 1;

	attron(COLOR_PAIR(player));

	mvaddch(starty + y, startx + x, ACS_BOARD);
	mvaddch(starty + y, startx + x + 1, ACS_BOARD);

	attroff(COLOR_PAIR(player));
}


static void
draw_discs(int startx, int starty, unsigned char *board)
{
	for (int i = 0; i < NB_LINES; i++)
		for (int j = 0; j < NB_COLS; j++) {
			unsigned char player = board[i * NB_COLS + j];
			if (player != NO_PLAYER)
				draw_disc(startx, starty, j, i, player);
		}
}


static void
draw_board(int startx, int starty)
{
	int endy = starty + NB_LINES * HEIGHT;
	int endx = startx + NB_COLS * WIDTH;

	attron(COLOR_PAIR(BOARD));

	// draw grid
	for (int i = starty; i <= endy; i += HEIGHT)
		for (int j = startx; j <= endx; j++)
			mvaddch(i, j, ACS_HLINE);

	for (int i = startx; i <= endx; i += WIDTH)
		for (int j = starty; j <= endy; j++)
			mvaddch(j, i, ACS_VLINE);

	// draw joints
	for (int i = startx + WIDTH; i <= endx - WIDTH; i += WIDTH) {
		mvaddch(starty, i, ACS_TTEE);
		mvaddch(endy, i, ACS_BTEE);
	}

	for (int i = starty + HEIGHT; i <= endy - HEIGHT; i += HEIGHT) {
		mvaddch(i, startx, ACS_LTEE);
		mvaddch(i, endx, ACS_RTEE);
	}

	// draw corners
	mvaddch(starty, startx, ACS_ULCORNER);
	mvaddch(starty, endx, ACS_URCORNER);
	mvaddch(endy, startx, ACS_LLCORNER);
	mvaddch(endy, endx, ACS_LRCORNER);

	attroff(COLOR_PAIR(BOARD));
}


static void
draw_candidate(int startx, int starty, int pos, unsigned char player)
{
	draw_disc(startx, starty , pos, -1, player);
}


static void
draw_instructions(void)
{
	mvprintw(0, 1, "Player 1: xx  Player 2: xx");
	mvprintw(2, 1, "LEFT:   Move left");
	mvprintw(3, 1, "RIGHT:  Move right");
	mvprintw(4, 1, "DOWN:   Insert disc");
	mvprintw(5, 1, "SPACE:  Reset game");
	mvprintw(6, 1, "CTRL+C: Quit");
	attron(COLOR_PAIR(PLAYER_1));
	mvaddch(0, 11, ACS_BOARD);
	mvaddch(0, 12, ACS_BOARD);
	attron(COLOR_PAIR(PLAYER_2));
	mvaddch(0, 25, ACS_BOARD);
	mvaddch(0, 26, ACS_BOARD);
	attroff(COLOR_PAIR(PLAYER_2));
}


static bool
insert_disc(unsigned char *board, int pos, unsigned char player)
{
	for (int i = 5; i >= 0; i--) {
		if (board[i * NB_COLS + pos] == 0) {
			board[i * NB_COLS + pos] = player;
			return true;
		}
	}

	return false;
}


static unsigned char
check_4_inrow(unsigned char *board, int startx, int starty, int dirx, int diry)
{
	unsigned char player = board[starty * NB_COLS + startx];

	for (int i = 1; i < 4; i++) {
		if (startx + i * dirx >= NB_COLS || startx + i * dirx < 0)
			return NO_PLAYER;
		else if (starty + i * diry >= NB_LINES || starty + i * diry < 0)
			return NO_PLAYER;
		else if (board[(starty + i * diry) * NB_COLS + (startx + i * dirx)] != player)
			return NO_PLAYER;
	}

	return player;
}


static unsigned char
check_victory(unsigned char *board)
{
	unsigned char player = NO_PLAYER;

	for (int y = 0; y < NB_LINES; y++)
		for (int x = 0; x < NB_COLS; x++) {
			if (board[y * NB_COLS + x] == NO_PLAYER)
				continue;

			player =  check_4_inrow(board, x, y, 1, 0);   // Horizontal
			player += check_4_inrow(board, x, y, 0, 1);   // Vertical
			player += check_4_inrow(board, x, y, 1, 1);   // Diagonal 1
			player += check_4_inrow(board, x, y, 1, -1);  // Diagonal 2

			if (player != NO_PLAYER)
				return player;
		}

	return NO_PLAYER;
}


static void
init_game(struct game_state *state)
{
	if (state->board == NULL)
		state->board = calloc(NB_LINES * NB_COLS, sizeof(unsigned char));
	else
		memset(state->board, 0, NB_LINES * NB_COLS);

	assert(state->board != NULL);

	state->turn = PLAYER_1;
	state->winner = NO_PLAYER;
	state->candidate_pos = 0;
}


static void
update(struct game_state *state, int ch)
{
	if (ch == KEY_LEFT && state->candidate_pos > 0) {
		state->candidate_pos--;
	}
	else if (ch == KEY_RIGHT && state->candidate_pos < 6) {
		state->candidate_pos++;
	}
	else if (state->winner == NO_PLAYER && ch == KEY_DOWN) {
		// fill board
		int result = insert_disc(state->board, state->candidate_pos, state->turn);

		if (result == true) {
			// change player
			state->turn = (state->turn == PLAYER_1) ? PLAYER_2 : PLAYER_1;

			// check victory
			unsigned char winner = check_victory(state->board);
			if (winner != NO_PLAYER) {
				state->winner = winner;
			}
		}
	}
	else if (ch == ' ') {
		init_game(state);
	}
}

static void
redraw(struct game_state *state)
{
	int starty = (LINES - NB_LINES * HEIGHT) / 2;
	int startx = (COLS - NB_COLS * WIDTH) / 2;

	clear();

	draw_candidate(startx, starty, state->candidate_pos, state->turn);
	draw_board(startx, starty);
	draw_discs(startx, starty, state->board);
	draw_instructions();
	
	if (state->winner != NO_PLAYER)
		mvprintw(LINES - 2, 1, "Victory of player %d", state->winner);

	refresh();
}


// AI CODE
#define AI_MAX_DEPTH 5

#define STATE_WIN    1
#define STATE_DRAW   0
#define STATE_LOST  -1

struct ai_node {
	int move;
	int score;
	struct ai_node *childs[NB_COLS];
	int nb_childs;
};

struct ai_state {
	struct ai_node *root;
	struct game_state *state;
};

static void
game_state_clone(struct game_state *copy, struct game_state *orig)
{
	*copy = *orig;

	copy->board = calloc(NB_LINES * NB_COLS, sizeof(unsigned char));
	assert(copy->board != NULL);

	memcpy(copy->board, orig->board, NB_LINES * NB_COLS);
}

static void
game_do_move(struct game_state *state)
{

}

static void
ai_add_child(struct ai_node *n, int move, int score)
{
	struct ai_node *new_node = malloc(sizeof(struct ai_node));

	new_node->move = move;
	new_node->score = score;
	new_node->nb_childs = 0;

	n->childs[n->nb_childs++];
}

static void
ai_del_child(struct ai_node *n)
{

}

static void
ai_build_tree(struct ai_state *ai)
{
	// for all seven possible moves
	for (int i = 0; i < NB_COLS; i++) {
		ai->root->childs[i] = malloc(sizeof(struct ai_node));
	}
}

static int
ai_minimax(struct ai_state *ai, int depth, int maximizing_player)
{
	if (depth == 0)
		return STATE_DRAW;

	unsigned char player = check_victory(ai->state->board);
	if (player != NO_PLAYER)
		return (player == maximizing_player) ? STATE_WIN : STATE_LOST;

	int best_value = 0;
	if (ai->state->turn == maximizing_player) {
		best_value = INT_MIN;

	}
	else {
		best_value = INT_MAX;

	}
	return best_value;
}

static int
ai_play(struct ai_state *ai)
{
	ai_build_tree(ai);
	int best_move = ai_minimax(ai, AI_MAX_DEPTH, PLAYER_2);
	return best_move;
}


//////////////////////

static void
sig_handler(int sig)
{
	endwin();
	printf("bye!\n");
	exit(0);
}


int
main(int argc, char **argv)
{
	signal(SIGINT, sig_handler);

	initscr();
	if (has_colors() == false)
	{
		endwin();
		printf("Your terminal does not support colors\n");
		exit(1);
	}

	clear();   // empty screen
	noecho();  // no text output
	curs_set(0);  // cursor invisible
	cbreak();  // no line buffering, pass every key pressed
	keypad(stdscr, true);  // keypad enabled

	start_color();
	init_pair(PLAYER_1, COLOR_PLAYER_1, COLOR_BLACK);
	init_pair(PLAYER_2, COLOR_PLAYER_2, COLOR_BLACK);
	init_pair(BOARD, COLOR_BOARD, COLOR_BLACK);

	struct game_state state = {0};
	init_game(&state);

	// game loop
	int ch = 0;
	while (true) {
		update(&state, ch);
		redraw(&state);
		ch = getch();
	}

	return 0;
}
