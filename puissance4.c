#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ncurses.h>
#include <assert.h>


#define HEIGHT     2
#define WIDTH      3
#define NB_LINES   6
#define NB_COLS    7


static void
draw_disc(int startx, int starty, int posx, int posy, int color)
{
	int x = posx * WIDTH + 1;
	int y = posy * HEIGHT + 1;

	attron(COLOR_PAIR(color));

	mvaddch(starty + y, startx + x, ACS_BOARD);
	mvaddch(starty + y, startx + x + 1, ACS_BOARD);

	attroff(COLOR_PAIR(color));
}


static void
draw_discs(int startx, int starty, char *board)
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

	attron(COLOR_PAIR(3));

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

	attroff(COLOR_PAIR(3));
}


static void
draw_arrows(int startx, int starty, int pos, int color)
{
	attron(COLOR_PAIR(color));

	mvaddch(starty - 1, pos * WIDTH + startx + 1, ACS_BOARD);
	mvaddch(starty - 1, pos * WIDTH + startx + 2, ACS_BOARD);

	attroff(COLOR_PAIR(color));
}


static bool
insert_disc(char *board, int pos, int color)
{
	for (int i = 5; i >= 0; i--) {
		if (board[i * NB_COLS + pos] == 0) {
			board[i * NB_COLS + pos] = color;
			return true;
		}
	}

	return false;
}


static void
update(int ch)
{
	static int pos = 0;
	static int color = 1;
	static char *board = NULL;

	if (board == NULL)
		board = calloc(NB_LINES * NB_COLS, sizeof(char));

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
		int result = insert_disc(board, pos, color);

		// change player
		if (result == true)
			color = (color == 1) ? 2 : 1;
	}

	// redraw
	clear();
	draw_arrows(startx, starty, pos, color);
	draw_board(startx, starty);
	draw_discs(startx, starty, board);
	refresh();

	// check victory
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
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);

	// game loop
	int ch = 0;
	while (true) {
		update(ch);
		ch = getch();
	}

	return 0;
}
