// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The board model
#include "board_model.h"
#include "worm.h"
#include <curses.h>

void placeItem(int y, int x, chtype symbol, enum ColorPairs color_pair) {

  //  Store item on the display (symbol code)
  move(y, x);
  attron(COLOR_PAIR(color_pair));
  addch(symbol);
  attroff(COLOR_PAIR(color_pair));
}
// Getters

// Get the last usable row on the display
int getLastRow() { return LINES - 1; }

// Get the last usable column on the display
int getLastCol() { return COLS - 1; }
