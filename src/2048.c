#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void init();
void addNumToBoard(int values[4][4]);
void drawBoard(WINDOW *winArray[4][4], int values[4][4]);
bool mergeRight(int values[4][4]);
bool mergeUp(int values[4][4]);
bool mergeLeft(int values[4][4]);
bool mergeDown(int values[4][4]);
void copyArray(int toBeCopied[4][4], int copy[4][4]);
void undo(int values[4][4], int prevValues[4][4]);

// make option to continue after winning
int main(int argc, char* argv[])
{
	const char *title[7] = {
		"  _______  ________  ___   ___  ________     ",
		" /  ___  \\|\\   __  \\|\\  \\ |\\  \\|\\   __  \\    ",
		"/__/|_/  /\\ \\  \\|\\  \\ \\  \\\\_\\  \\ \\  \\|\\  \\   ",
		"|__|//  / /\\ \\  \\\\\\  \\ \\______  \\ \\   __  \\  ",
		"    /  /_/__\\ \\  \\\\\\  \\|_____|\\  \\ \\  \\|\\  \\ ",
		"   |\\________\\ \\_______\\     \\ \\__\\ \\_______\\",
		"    \\|_______|\\|_______|      \\|__|\\|_______|"
	};

	const char *win_message[6] = {
		"__  ______  __  __   _       ______  _   ____",
		"\\ \\/ / __ \\/ / / /  | |     / / __ \\/ | / / /",
		" \\  / / / / / / /   | | /| / / / / /  |/ / / ",
		" / / /_/ / /_/ /    | |/ |/ / /_/ / /|  /_/  ",
		"/_/\\____/\\____/     |__/|__/\\____/_/ |_(_)   ",
	};

	// initialize ncurses
	init();

	bool won = false;


	// array of windows to represent cells on board
	WINDOW *windowArray[4][4];
	// array of values on board
	int valueArray[4][4];
	int prevValueArray[4][4];

	// current size of terminal window
	int columns = getmaxx(stdscr);
	int lines = getmaxy(stdscr);

	// calculate padding to use for centering
	int leftPad = (columns-36)/2;
	int topPad = (lines-20)/2;

	// print title if screen is large enough
	if(columns >= 45 && lines >= 28)
	{
		attron(A_BOLD);
		int i;
		for(i=0;i<7;i++)
		{
			mvprintw(i, (columns-48)/2, title[i]);
		}
		attroff(A_BOLD);
	}

	// initialize window array
	int r, c;
	for(r = 0; r < 4; r++)
	{
		for(c = 0; c < 4; c++)
		{
			windowArray[r][c] = newwin(5, 9, topPad + 5*r, leftPad + 9*c);
			box(windowArray[r][c], 0, 0);
			refresh();
			wrefresh(windowArray[r][c]);
		}
	}
	// initialize value arrays
	for(r = 0; r < 4; r++)
	{
		for(c = 0; c < 4; c++)
		{
			valueArray[r][c] = 0;
			prevValueArray[r][c] = 0;
		}
	}

	// add two random starting numbers
	addNumToBoard(valueArray);
	addNumToBoard(valueArray);

	drawBoard(windowArray, valueArray);

	// main game logic loop
	while(!won)
	{
		// create copy for undo purposes
		int tempCopy[4][4];
		copyArray(valueArray, tempCopy);

		char input = getch();

		// arrow keys send 3 characters to getch() \033, [, and A-D identifying which arrow key it was
		if(input == '\033')
		{
			bool moved;
			getch();
			switch(getch())
			{
				// up arrow
				case 'A':
					moved = mergeUp(valueArray);
					break;
				// down arrow
				case 'B':
					moved = mergeDown(valueArray);
					break;
				// right arrow
				case 'C':
					moved = mergeRight(valueArray);
					break;
				// left arrow
				case 'D':
					moved = mergeLeft(valueArray);
					break;
			}
			// only add number and update board if a merge above caused a change
			if(moved)
			{
				copyArray(tempCopy, prevValueArray);
				addNumToBoard(valueArray);
				drawBoard(windowArray, valueArray);
				int r,c;
				for(r=0; r < 4; r++)
				{
					for(c=0; c < 4; c++)
					{
						if(valueArray[r][c] == 2048)
						{
							won = true;
						}
					}
				}
			}
		}else if(input == 'u'){
			undo(valueArray, prevValueArray);
			drawBoard(windowArray, prevValueArray);
		}
	}

	// clear main screen
	clear();

	// print win message
	int i;
	for(i=0; i < 6; i++)
	{
		mvprintw((LINES/2)-3+i, (COLS/2)-22, win_message[i]);
	}

	getch();

	// cleanly exit ncurses
	endwin();

	return 0;
}

void init()
{
	// ncurses setup
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	//nodelay(stdscr, TRUE);

	start_color();
	use_default_colors();
	init_pair(1, COLOR_BLACK, COLOR_WHITE);
	init_pair(2, COLOR_BLACK, COLOR_CYAN);
	init_pair(3, COLOR_BLACK, COLOR_BLUE);
	init_pair(4, COLOR_BLACK, COLOR_GREEN);
	init_pair(5, COLOR_BLACK, COLOR_YELLOW);
	init_pair(6, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(7, COLOR_BLACK, COLOR_RED);
}

void addNumToBoard(int values[4][4])
{
	// seed random num generator with current time
	srand(time(NULL));

	// generate random positions 0-3
	int r = rand() % 4;
	int c = rand() % 4;

	// if theres a value in the spot already continue picking rand spots until it's empty
	while(values[r][c] != 0){
		r = rand() % 4;
		c = rand() % 4;
	}

	// make ~10% chance that new value is 4 rather than 2
	int random = rand() % 10;
	if(random == 4)
	{
		values[r][c] = 4;
	}else{
		values[r][c] = 2;
	}
}

void drawBoard(WINDOW *winArray[4][4], int values[4][4])
{
	int r, c;
	for(r = 0; r < 4; r++)
	{
		for(c = 0; c < 4; c++)
		{
			WINDOW *currentWin = winArray[r][c];
			int value = values[r][c];

			// convert into to string for printing
			char numStr[4];
			sprintf(numStr, "%d", value);
			int strLen = strlen(numStr);

			// pick color based on value
			int colorNum;
			switch(value)
			{
				case 2:
					colorNum = 1; break;
				case 4:
				case 256:
					colorNum = 2; break;
				case 8:
				case 512:
					colorNum = 3; break;
				case 16:
				case 1024:
					colorNum = 4; break;
				case 32:
				case 2048:
					colorNum = 5; break;
				case 64:
				case 4096:
					colorNum = 6; break;
				case 128:
				case 8192:
					colorNum = 7; break;
				default:
					colorNum = 0;
			}

			wattron(currentWin, COLOR_PAIR(colorNum));
				int x, y;
				// print spaces in window to make bg color
				for(y=1; y<4; y++)
				{
					for(x=1; x<8; x++)
					{
						mvwaddch(currentWin, y, x, ' ');
					}
				}
				// print value on top of bg if not zero
				if(value != 0)
				{
					mvwprintw(currentWin, 2, 4-(strLen/2), "%d", value);
				}
			wattroff(currentWin, COLOR_PAIR(colorNum));

			refresh();
			wrefresh(currentWin);
		}
	}
}

bool mergeUp(int values[4][4])
{
	bool moved = false;
	int r, c;

	// move all values up
	for(c=0; c<4; c++)	
	{
		for(r=0; r<4; r++)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=r+1; i<4; i++)
				{
					if(values[i][c] != 0)
					{
						values[r][c] = values[i][c];
						values[i][c] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	// merge all valid pairs
	for(c=0; c<4; c++)
	{
		for(r=1; r<4; r++)
		{
			if(values[r][c] == values[r-1][c] && values[r][c] != 0)
			{
				values[r-1][c] = 2*values[r][c];
				values[r][c] = 0;
				moved = true;
			}
		}
	}
	// move all values up
	for(c=0; c<4; c++)	
	{
		for(r=0; r<4; r++)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=r+1; i<4; i++)
				{
					if(values[i][c] != 0)
					{
						values[r][c] = values[i][c];
						values[i][c] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	return moved;
}

bool mergeRight(int values[4][4])
{
	bool moved = false;
	int r, c;

	// move all values right
	for(r=0; r<4; r++)
	{
		for(c=3; c>0; c--)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=c-1; i>=0; i--)
				{
					if(values[r][i] != 0)
					{
						values[r][c] = values[r][i];
						values[r][i] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	// merge all valid pairs
	for(r=0; r<4; r++)
	{
		for(c=2; c>=0; c--)
		{
			if(values[r][c] == values[r][c+1] && values[r][c] != 0)
			{
				values[r][c+1] = 2*values[r][c];
				values[r][c] = 0;
				moved = true;
			}
		}
	}
	// move all values right
	for(r=0; r<4; r++)
	{
		for(c=3; c>0; c--)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=c-1; i>=0; i--)
				{
					if(values[r][i] != 0)
					{
						values[r][c] = values[r][i];
						values[r][i] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	return moved;
}

bool mergeLeft(int values[4][4])
{
	bool moved = false;
	int r, c;

	// move all values left
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=c+1; i<4; i++)
				{
					if(values[r][i] != 0)
					{
						values[r][c] = values[r][i];
						values[r][i] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	// merge all valid pairs
	for(r=0; r<4; r++)
	{
		for(c=1; c<4; c++)
		{
			if(values[r][c] == values[r][c-1] && values[r][c] != 0)
			{
				values[r][c-1] = 2*values[r][c];
				values[r][c] = 0;
				moved = true;
			}
		}
	}
	// move all values left
	for(r=0; r<4; r++)
	{
		for(c=0; c<4; c++)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=c+1; i<4; i++)
				{
					if(values[r][i] != 0)
					{
						values[r][c] = values[r][i];
						values[r][i] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	return moved;
}

bool mergeDown(int values[4][4])
{
	bool moved = false;
	int r, c;

	// move all values down
	for(c=0; c<4; c++)	
	{
		for(r=3; r>=0; r--)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=r-1; i>=0; i--)
				{
					if(values[i][c] != 0)
					{
						values[r][c] = values[i][c];
						values[i][c] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	// merge all valid pairs
	for(c=0; c<4; c++)
	{
		for(r=2; r>=0; r--)
		{
			if(values[r][c] == values[r+1][c] && values[r][c] != 0)
			{
				values[r+1][c] = 2*values[r][c];
				values[r][c] = 0;
				moved = true;
			}
		}
	}
	// move all values down
	for(c=0; c<4; c++)	
	{
		for(r=3; r>=0; r--)
		{
			if(values[r][c] == 0)
			{
				int i;
				for(i=r-1; i>=0; i--)
				{
					if(values[i][c] != 0)
					{
						values[r][c] = values[i][c];
						values[i][c] = 0;
						moved = true;
						break;
					}
				}
			}
		}
	}
	return moved;
}

void copyArray(int toBeCopied[4][4], int copy[4][4])
{
	int r, c;
	for(r=0; r < 4; r++)
	{
		for(c=0; c < 4; c++)
		{
			copy[r][c] = toBeCopied[r][c];
		}
	}
}

void undo(int values[4][4], int prevValues[4][4])
{
	int r, c;
	for(r=0; r < 4; r++)
	{
		for(c=0; c < 4; c++)
		{
			values[r][c] = prevValues[r][c];
		}
	}
}
