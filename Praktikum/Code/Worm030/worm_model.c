// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The worm model
#include "worm_model.h"
#include "board_model.h"
#include "worm.h"
#include <curses.h>

int theworm_maxindex;
int theworm_headindex;
int theworm_wormpos_x[WORM_LENGTH];
int theworm_wormpos_y[WORM_LENGTH];
int theworm_dx;
int theworm_dy;
enum ColorPairs theworm_wcolor;

// Initialize the worm
extern enum ResCodes initializeWorm(int len_max, int headpos_y, int headpos_x,
                                    enum WormHeading dir,
                                    enum ColorPairs color) {
  // Local variables for loops etc.
  int i;                          // @001
                                  // Initialize last usable index to len_max -1
                                  // theworm_maxindex
  theworm_maxindex = len_max - 1; //@002
                                  // Initialize headindex
  theworm_headindex = 0;

  // Mark all elements as unused in the arrays of positions
  // theworm_wormpos_y[] and theworm_wormpos_x[]
  // An unused position in the array is marked
  // with code UNUSED_POS_ELEM
  for (i = 0; i <= theworm_maxindex; i++) {
    theworm_wormpos_y[i] = UNUSED_POS_ELEM; //@004
    theworm_wormpos_x[i] = UNUSED_POS_ELEM; //@004
  }
  // Initialize position of worms head
  theworm_wormpos_x[theworm_headindex] = headpos_x; //@005
  theworm_wormpos_y[theworm_headindex] =
      headpos_y; //@005
                 // Initialize the heading of the worm
  setWormHeading(dir);
  // Initialize color of the worm
  theworm_wcolor = color;

  return RES_OK;
}

// Show the worms's elements on the display
// Simple version
extern void showWorm() {
  int index = (theworm_headindex - 1);
  int i;
  // Due to our encoding we just need to show the head element
  // All other elements are already displayed
  placeItem(theworm_wormpos_y[theworm_headindex],
            theworm_wormpos_x[theworm_headindex], SYMBOL_WORM_HEAD,
            theworm_wcolor); //@007
  if (index == -1) {
    index = theworm_maxindex;
  }
  placeItem(theworm_wormpos_y[index], theworm_wormpos_x[index],
            SYMBOL_WORM_INNER_ELEMENT,
            theworm_wcolor); //??? differentiates between Head and inner element
  for (i = theworm_headindex;
       i == ((theworm_headindex + 1) % (theworm_maxindex + 1)); i--) {
  }
}
extern void cleanWormTail() {
  int tailindex; //  @006
  // Compute tailindex
  tailindex = (theworm_headindex + 1) %
              (theworm_maxindex +
               1); //
                   // Check the array of worm elements.
                   // Is the array element at tailindex already in use?
                   // Checking either array theworm_wormpos_y
                   // or theworm_wormpos_x is enough.
  if (theworm_wormpos_x[tailindex] != UNUSED_POS_ELEM) {
    // YES: place a SYMBOL_FREE_CELL at the tail's position
    placeItem(theworm_wormpos_y[tailindex], theworm_wormpos_x[tailindex],
              SYMBOL_FREE_CELL, COLP_FREE_CELL);
  }
}
// The following functions all depend on the model of the worm

extern void moveWorm(enum GameStates *agame_state) {
  int headpos_x;
  int headpos_y; //@010
                 // Get the current position of the worm's head element and
                 // compute the new head position according to current heading.
                 // Do not store the new head position in the array of
                 // positions, yet.
  headpos_x = theworm_wormpos_x[theworm_headindex] + theworm_dx; //@011
  headpos_y = theworm_wormpos_y[theworm_headindex] +
              theworm_dy; //@012
                          // Check if we would hit something (for good or bad)
                          // or are going to leave the display if we move the
                          // worm's head according to worm's last direction. We
                          // are not allowed to leave the display's window.

  if (headpos_x < 0) {
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (headpos_x > getLastCol()) {
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (headpos_y < 0) {
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (headpos_y > getLastRow()) {
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else {
    // We will stay within bounds.
    // Check if the worm's head will collide with itself at the new position
    if (isInUseByWorm(headpos_y, headpos_x)) {
      // That's bad: stop game
      *agame_state = WORM_CROSSING;
    }
  }
  // Check the status of *agame_state
  // Go on if nothing bad happened
  if (*agame_state == WORM_GAME_ONGOING) {
    // So all is well: we did not hit anything bad and did not leave the
    // window. --> Update the worm structure.
    // Increment theworm_headindex
    // Go round if end of worm is reached (ring buffer)
    theworm_headindex = (theworm_headindex + 1) % (theworm_maxindex + 1);
    // Store new coordinates of head element in worm structure
    theworm_wormpos_x[theworm_headindex] = headpos_x;
    theworm_wormpos_y[theworm_headindex] = headpos_y;
  }
}

// A simple collision detection
extern bool isInUseByWorm(int new_headpos_y, int new_headpos_x) {
  int i;
  bool collision = false;
  i = theworm_headindex;
  do {
    // Compare the position of the current worm element with the new_headpos
    if (theworm_wormpos_x[i] == new_headpos_x) {
      if (theworm_wormpos_y[i] == new_headpos_y) {
        collision = true;
        break;
      }
    }
    i = (i + theworm_maxindex) % (theworm_maxindex + 1);
  } while (i != theworm_headindex && theworm_wormpos_x[i] != UNUSED_POS_ELEM);
  // Return what we found out.
  return collision;
}
// Setters
extern void setWormHeading(enum WormHeading dir) {
  switch (dir) {
  case WORM_UP: // User wants up, @005
    theworm_dx = 0;
    theworm_dy = -1;
    break;
  case WORM_DOWN: // User wants down
    theworm_dx = 0;
    theworm_dy = 1;
    break;
  case WORM_LEFT: // User wants left
    theworm_dx = -1;
    theworm_dy = 0;
    break;
  case WORM_RIGHT: // User wants right
    theworm_dx = 1;
    theworm_dy = 0;
    break;
  }
}
