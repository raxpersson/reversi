/*
 *
 * Written by Rasmus A. X. Persson.
 *
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>

#define LOWEST (-10000000)

int parity; // equal to 1 if depth is even, and -1 if depth is odd

// evaluate board
int evalb(int *board, int recur, int side);
// calculate next move
int findmv(int side, int *board, int *en, int recur);
int move(int *board, int en, int side);

// what rank is square i?
inline int rank(int i)
{
	if (i < 20) return 0; if (i < 30) return 1; if (i < 40) return 2;
	if (i < 50) return 3; if (i < 60) return 4; if (i < 70) return 5;
	if (i < 80) return 6;
	return 7;
}

// what file is square i?
inline int file(int i) { return (i % 10); }

// read the x and y coordinates and convert them into the internal move
// representation
void readmv(int x, int y, int *en)
{
	int ef, er;	

	ef = (int)nearbyintf(((float)x - 50) / 50.f);
	er = (int)nearbyintf(((float)y - 50) / 50.f);

	*en = ef + 10*er + 11;
}

// print a move in coordinate notation (with "move" prefixed).
void printmv(int en)
{
	char move[3];

	move[0] = (file(en) - 1) + 97;
	move[1] = (7 - rank(en)) + 49;
	move[2] = 0;

	printf("%s\n", move);	
}

// copy board from src to dst
inline void cpboard(int *src, int *dst)
{
	unsigned int i;

	for (i = 0; i < 100; ++i) dst[i] = src[i];
}

// return a character for each piece or square
inline char pchar(int i)
{
	if (i == 1) return 'X';
	if (i == -1) return 'O';
	if (i == 2) return '|';
	return '.';
}

// highlight: animate the placement of the stone
void highlight(int en, int side)
{
	int i;

	scare_mouse();
	if (side == 1)
	{
		for (i = 0; i < 20; ++i)
		{
			// go from blackground color to black
			circlefill(screen, 50*file(en), 45 + 50*rank(en), 15,
				makecol(0, 200 - i*10, 0));
			rest(10);
		}
	}
	else
	{
		for (i = 0; i < 20; ++i)
		{
			// go from background color to white
			circlefill(screen, 50*file(en), 45 + 50*rank(en), 15,
				makecol(i*10, 200 + 55*i/20, i*10));
			rest(10);
		}
	}
	unscare_mouse();
}

// print board
void printbd(int *board)
{
	int i;

	scare_mouse();

	line(screen, 20, 20, 420, 20, makecol(0, 0, 0));
	line(screen, 20, 20, 20, 420, makecol(0, 0, 0));
	line(screen, 420, 20, 420, 420, makecol(0, 0, 0));
	line(screen, 20, 420, 420, 420, makecol(0, 0, 0));

	// vertical lines
	for (i = 0; i < 8; ++i)
		line(screen, 70 + 50*i, 20, 70 + 50*i, 420, makecol(0, 0, 0));
	
	// horizontal lines
	for (i = 0; i < 8; ++i)
		line(screen, 20, 70 + 50*i, 420, 70 + 50*i, makecol(0, 0, 0));
	
	// pieces
	for (i = 0; i < 100; ++i)
	{
		if (board[i] == 1)
		{
			circlefill(screen, 50*file(i), 45 + 50*rank(i), 15, 
						makecol(0, 0, 0));
		}
		if (board[i] == -1)
		{
			circlefill(screen, 50*file(i), 45 + 50*rank(i), 15, 
						makecol(255, 255, 255));
		}
	}
	
	unscare_mouse();
}

// print status line beneath board
void printst(int *board, int flag)
{
	int iblack = 0, iwhite = 0;
	int i;

	for (i = 0; i < 100; ++i)
	{
		if (board[i] == 1) ++iblack;
		if (board[i] == -1) ++iwhite;
	}

	scare_mouse();

	if (!flag)
		textprintf_centre_ex(screen, font, SCREEN_W/2, 440, 
							makecol(0, 0, 0), makecol(0, 200, 0),
							"Black: %i | White: %i", iblack, iwhite);
	else
		textprintf_centre_ex(screen, font, SCREEN_W/2, 440,
							makecol(0, 0, 0), makecol(0, 200, 0),
							"Black: %i [GAME OVER] White: %i", iblack, iwhite);

	unscare_mouse();
}

int main(int argc, char *argv[])
{
	int x, y, en, side, depth, score; 

	// true reversi starts with empty board, but that is not implemented yet!
	// therefore, here is the othello starting arrangement.
	int board[100] =
	{
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0,-1, 1, 0, 0, 0, 2,
		2, 0, 0, 0, 1,-1, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2
	};

	if (argc > 1)
	{
		side  = atoi(argv[1]); 
	}
	else
	{
		printf("Insufficient arguments\n\n");
		printf("Syntax: %s {computer's side} [difficulty]\n", argv[0]);
		return 0;
	}
	if (argc == 3)
	{
		// side is the color that the computer plays
		// '1' is black, '-1' white
		// depth is the difficulty, the number of plies.
		depth = atoi(argv[2]);
	}
	else
	{
		depth = 4;
	}
	if (depth % 2 == 0) parity = 1; else parity = -1;

	printf("Computer playing as %c\n", pchar(side));
	printf("Difficulty %i\n", depth);
	printf("Right-click to quit\n");

	if (allegro_init() != 0) return 1;

	install_keyboard();
	install_mouse();
	install_timer();
	if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 440, 440 + 20, 0, 0))
	{
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Unable to enter graphics mode\n%s\n", allegro_error);
		return 1;
	}
	
	set_palette(desktop_palette);
	clear_to_color(screen, makecol(0, 200, 0)); // green background

	textout_centre_ex(screen, font, "REVERSI", SCREEN_W/2, 10, 
						makecol(0, 0, 0), -1);

	printbd(board);
	printst(board, 0);
	show_mouse(screen);

	if (side == 1)
	{
		// make first move
		findmv(side, board, &en, depth);
		move(board, en, side);
		highlight(en, side);

		printbd(board);
		printst(board, 0);
	}
	// game loop
	while (1)
	{
		if (mouse_b & 1)
		{
			// the user has clicked to place a move
			x = mouse_pos >> 16;
			y = mouse_pos & 0x0000ffff;

			readmv(x, y, &en);
			
			// perform user move
			if (move(board, en, -side))
			{
				// highlight user move
				highlight(en, -side);
				printbd(board);
				printst(board, 0);
				
				// find reply
				score = findmv(side, board, &en, depth);
				if (move(board, en, side))
				{
					highlight(en, side);

					printbd(board);
					printst(board, 0);
				}
			}
			else
			{
				// check and see if user CAN move at all
				en = -1;
				score = findmv(-side, board, &en, depth);
				if (en == -1)
				{
					// computer move instead
					score = findmv(side, board, &en, depth);
					if (move(board, en, side))
					{
						highlight(en, side);

						printbd(board);
						printst(board, 0);
					}
					else
					{
						// Game over
						printst(board, 1);
					}
				}
			}
		}

		if (mouse_b & 2) return 0; // exit on right-click
	}

	return 0;
}
END_OF_MAIN() // Macro needed by Allegro.

// returns a positive number if side 'side' is winning (-1 for black).
// if recur = 0, then no recursion.
int evalb(int *board, int recur, int side)
{
	unsigned int i;
	int t, s, temp[100], val;

	val = 0;

	cpboard(board, temp); // make a safety copy
	
	// is anything dangerous about to happen?
	if (recur > 1)
	{
		// find possible replies for opposite side
		if (recur % 2 == 0) s = parity; else s = -parity;
		val = s*findmv(s*side, temp, &t, recur - 1);
	}
	else
	{
		// we've reached the end of the ply cycle
		// count material
		for (i = 0; i < 100; ++i)
			if (temp[i] != 2) val += side*temp[i];
	}

	return val;
}

// makes a search and returns the best move for side 'side' (= 1 or -1)
// the move is reported as square 'en'. it also reports the evaluation score
// expected after this move.
int findmv(int side, int *board, int *en, int recur)
{
	int i, temp[100];
	int k, best;

	cpboard(board, temp);
	best = LOWEST; 

	// find all legal moves, evaluate each one and pick the best one.
	for (i = 0; i < 100; ++i)
	{ 
		if (move(temp, i, side))
		{
			k = evalb(temp, recur, side);
			if (k > best)
			{
				*en = i;
				best = k;
			}
			cpboard(board, temp);
		}
	}

	return best;
}

// will perform the move. returns 0 for illegal move, 1 for legal move.
int move(int *board, int en, int side)
{
	int i, j, k, l, f, vec[8], vm[8];

	// the eight possible adjacent squares in relative coordinates
	vec[0] = -11; vec[1] = -10;
	vec[2] = -9; vec[3] = -1;
	vec[4] = 1; vec[5] = 9;
	vec[6] = 10; vec[7] = 11;

	// the maximum steps in each of these directions
	for (i = 0; i < 8; ++i) vm[i] = 0;
	f = 0;

	// the legal move has (1) an adjacent square of opposite color
	// and (2) a square of the same color along the direction given 
	// by (1) at most 7 squares distant.
	if (en < 0) return 0;
	if (board[en]) return 0; // illegal: square occupied

	for (i = 0; i < 8; ++i)
	{
		k = en + vec[i];
		if (k < 0 || k > 99) continue;

		if (board[k] == -side)
		{
			// search along this direction for square of same side
			l = k;
			for (j = 0; j < 7; ++j)
			{
				l += vec[i];
				if (l < 0 || l > 99 || board[l] == 2 || !board[l]) break;
				if (board[l] == side)
				{
					f = 1;
					vm[i] = j + 1;
					break;
				}
			}
		}
	}

	if (!f) return 0;	// illegal move: no pairing possible

	// now flip the pieces
	for (i = 0; i < 8; ++i)
	{
		k = en + vec[i];
		l = k;
		for (j = 0; j < vm[i]; ++j)
		{
			if (l < 0 || l > 99 || board[l] == 2) break;
			board[l] = side;
			l += vec[i];
		}
	}
	board[en] = side;

	return 1;
}
