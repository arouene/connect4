#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <assert.h>


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
		for (int j = 0; j < NB_COLS; j++)
			if (board[i * NB_COLS + j] != 0)
				draw_disc(startx, starty, j, i, board[i * NB_COLS + j]);
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
	attron(COLOR_PAIR(player));

	mvaddch(starty - 1, pos * WIDTH + startx + 1, ACS_BOARD);
	mvaddch(starty - 1, pos * WIDTH + startx + 2, ACS_BOARD);

	attroff(COLOR_PAIR(player));
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

	if (player == NO_PLAYER)
		return NO_PLAYER;

	for (int i = 1; i < 4; i++) {
		if (startx + i * dirx > NB_COLS || startx + i * dirx < 0)
			return NO_PLAYER;
		else if (starty + i * diry > NB_LINES || starty + i * diry < 0)
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
update(int ch)
{
	static int pos = 0;
	static unsigned char player = PLAYER_1;
	static unsigned char *board = NULL;

	if (board == NULL)
		board = calloc(NB_LINES * NB_COLS, sizeof(unsigned char));

	assert(board != NULL);

	int starty = (LINES - NB_LINES * HEIGHT) / 2;
	int startx = (COLS - NB_COLS * WIDTH) / 2;

	if (ch == KEY_LEFT && pos > 0) {
		pos--;
	}
	else if (ch == KEY_RIGHT && pos < 6) {
		pos++;
	}
	else if (ch == KEY_ENTER || ch == KEY_DOWN) {
		// fill board
		int result = insert_disc(board, pos, player);

		// change player
		if (result == true)
			player = (player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
	}

	// redraw
	clear();
	draw_candidate(startx, starty, pos, player);
	draw_board(startx, starty);
	draw_discs(startx, starty, board);
	refresh();

	// check victory
	if (check_victory(board))
		printw("Victory");
}


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

	// game loop
	int ch = 0;
	while (true) {
		update(ch);
		ch = getch();
	}

	return 0;
}
