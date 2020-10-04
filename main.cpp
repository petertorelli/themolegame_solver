#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <set>
#include <string>

#include <vector>

using namespace std;

enum fill
{
	open = '0',
	wall,
	dirt,
	food,
	rock,
	mole
};

char chars[] = { ' ', ' ', 'X', '.', '`', 'O', '>' };

#define ROWS 7
#define COLS 10
#define SIZE (ROWS * COLS)

char state0[SIZE+1] = {
	wall, wall, wall, wall, rock, wall, wall, wall, wall, wall,
	wall, open, dirt, dirt, rock, wall, open, open, rock, wall,
	wall, open, dirt, dirt, rock, open, open, open, rock, wall,
	wall, open, rock, rock, rock, dirt, wall, open, dirt, open,
	wall, rock, dirt, dirt, food, rock, wall, open, open, wall,
	mole, dirt, rock, open, open, food, dirt, open, open, wall,
	wall, wall, wall, wall, wall, wall, wall, wall, wall, wall,
	0 /* null term so this becomes a state string of chars for set detect */
};

enum dir
{
	up = 0,
	right,
	down,
	left,
};

char move_names[] = { 'u', 'r', 'd', 'l' };

#define xlate(r,c) (((r) * COLS) + c)
#define is_real(r,c) (r >= 0 && c >= 0 && r < ROWS && c < COLS)
#define is_digg(r,c,b) (is_real(r,c) && (b[xlate(r,c)] == open) || (b[xlate(r,c)] == dirt) || (b[xlate(r,c)] == food))
#define is_open(r,c,b) (is_real(r,c) && (b[xlate(r,c)] == open))
#define is_rock(r,c,b) (is_real(r,c) && (b[xlate(r,c)] == rock))

bool
can_move(int r, int c, enum dir d, char b[])
{
	switch (d)
	{
		case up:
			return (is_digg(r-1,c,b) || (is_rock(r-1,c,b) && is_open(r-2,c,b)));
		case right:
			return (is_digg(r,c+1,b) || (is_rock(r,c+1,b) && is_open(r,c+2,b)));
		case down:
			return (is_digg(r+1,c,b) || (is_rock(r+1,c,b) && is_open(r+2,c,b))) && !is_rock(r-1,c,b);
		case left:
			return (is_digg(r,c-1,b) || (is_rock(r,c-1,b) && is_open(r,c-2,b)));
		default:
			break;
	}
	return false;
}

bool
food_all_gone(char b[])
{
	int cnt = 0;
	for (int i=0; i<SIZE; ++i)
	{
		if (b[i] == food)
		{
			cnt++;
		}
	}
	return cnt == 0;
}

void
print_board(char b[])
{
	for (int r=0; r<ROWS; ++r)
	{
		for (int c=0; c<COLS; ++c)
		{
			printf("%c", chars[b[xlate(r,c)] - '0' + 1]);
		}
		printf("\n");
	}
	printf("\n");
}

bool
settle_one_rock(char b[])
{
	for (int r=0; r<ROWS; ++r)
	{
		for (int c=0; c<COLS; ++c)
		{
			if (b[xlate(r,c)] == rock && b[xlate(r+1,c)] == open)
			{
				b[xlate(r,c)] = open;
				b[xlate(r+1,c)] = rock;
				return true;
			}
		}
	}
	return false;
}

void
modify_board(int r1, int c1, int r2, int c2, enum dir d, char b[])
{
	/*
	if (b[xlate(r2,c2)] == food)
	{
		printf("MUNCH!\n");
	}
	if (b[xlate(r2,c2)] == dirt)
	{
		printf("DIG!\n");
	}
	*/
	if (b[xlate(r2,c2)] == rock)
	{
	//	printf("ROLL!\n");
		switch (d)
		{
			case up    : b[xlate(r2-1,c2)] = rock; break;
			case right : b[xlate(r2,c2+1)] = rock; break;
			case down  : b[xlate(r2+1,c2)] = rock; break;
			case left  : b[xlate(r2,c2-1)] = rock; break;
			default:
				break;
		}
	}
	b[xlate(r1,c1)] = open;
	b[xlate(r2,c2)] = mole;

	// check all the rocks that need to fall
	while (settle_one_rock(b))
	{
	}
}

set<string> boards;

bool
seen_board(char b[])
{
	string board(b);
	if (boards.find(board) != boards.end())
	{
		return true;
	}
	boards.insert(board);
	return false;
}

void
turn(int depth, vector<char> m, int r, int c, char b[])
{
	int moves[4] = {0, 0, 0, 0};
	int save_board[SIZE];
	enum dir dirs[] = { up, right, down, left };
	if (depth >= 200)
	{
		printf("Too deep!\n");
		exit(1);
	}

	if (seen_board(b))
	{
		return;
	}

//	printf("Current Board: at depth %d (%d, %d)\n", depth, r, c);
//	print_board(b);
	if (food_all_gone(b) && r == 3 && c == 9)
	{
//		printf("Done (%lu)\n", m.size());
		if (m.size() <= 27ul)
		{
			printf("%lu ", m.size());
			for (size_t i=0; i < m.size(); ++i)
			{
				printf("%c", m[i]);
			}
			printf("\n");
			print_board(b);
		}
//		exit(1);
	}
	for (size_t i=0; i<4; ++i)
	{
		if (can_move(r, c, dirs[i], b))
		{
			memcpy(save_board, b, SIZE);
			int r2 = r;
			int c2 = c;
			switch (dirs[i])
			{
				case up    : --r2; break;
				case right : ++c2; break;
				case down  : ++r2; break;
				case left  : --c2; break;
				default    : exit(1); break;
			}
			modify_board(r, c, r2, c2, dirs[i], b);

			m.push_back(move_names[dirs[i]]);
			turn(depth + 1, m, r2, c2, b);
			m.pop_back();
			memcpy(b, save_board, SIZE);
		}
	}
}

int
main(int argc, char *argv[])
{
	int locr = 5;
	int locc = 0;
	vector<char> moves;
	turn(1, moves, locr, locc, state0);
	return 0;
}