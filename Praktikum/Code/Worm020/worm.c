// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//

#include <curses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// ********************************************************************************************
// Constants, data structures
// ********************************************************************************************

enum ResCodes { RES_OK, RES_FAILED };

// Dimensions and bounds
#define NAP_TIME 100 // Time in milliseconds to sleep between updates of display
#define MIN_NUMBER_OF_ROWS                                                     \
  3 // The guaranteed number of rows available for the board
#define MIN_NUMBER_OF_COLS                                                     \
  10 // The guaranteed number of columns available for the board

// maximum length
#define WORM_MAX_LEN 20

#define UNUSED_POS_ELEM -1

// Numbers for color pairs used by curses macro COLOR_PAIR
enum ColorPairs { COLP_USER_WORM = 1, COLP_FREE_CELL };

// Symbols to display
#define SYMBOL_WORM_INNER_ELEMENT '0'
#define SYMBOL_WORM_FREE_CELL ' '

// Game state codes
enum GameStates {
  WORM_GAME_ONGOING,
  // worm left screen
  WORM_OUT_OF_BOUNDS,
  // user initiated quit
  WORM_GAME_QUIT,

  // Worm crossing itself
  WORM_CROSSING
};

// Directions for the worm
enum WormHeading {
  WORM_UP,
  WORM_DOWN,
  WORM_LEFT,
  WORM_RIGHT,
  WORM_DIA_UL,
  WORM_DIA_DL,
  WORM_DIA_UR,
  WORM_DIA_DR
};

// ********************************************************************************************
// Global variables
// ********************************************************************************************

// Data defining the worm

// storing body
int theworm_wormpos_y[WORM_MAX_LEN];
int theworm_wormpos_x[WORM_MAX_LEN];
// last usable index in the body arrays
int theworm_max_index;
// location of the worms head
int theworm_head_index;

// The current heading of the worm
// These are offsets from the set {-1,0,+1}
int theworm_dx;
int theworm_dy;

enum ColorPairs theworm_wcolor;

// ********************************************************************************************
// Forward declarations of functions
// ********************************************************************************************
// This avoids problems with the sequence of function declarations inside the
// code. Note: this kind of problem is solved by header files later on!

// Management of the game
void initializeColors();
void readUserInput(enum GameStates *agame_state);
enum ResCodes doLevel();

// Standard curses initialization and cleanup
void initializeCursesApplication();
void cleanupCursesApp(void);

// Placing and removing items from the game board
// Check boundaries of game board
void placeItem(int y, int x, chtype symbol, enum ColorPairs color_pair);
int getLastRow();
int getLastCol();

// Functions concerning the management of the worm data
enum ResCodes initializeWorm(int max_len, int headpos_y, int headpos_x,
                             enum WormHeading dir, enum ColorPairs color);
void showWorm();
void cleanWormTail();
void moveWorm(enum GameStates *agame_state);
void setWormHeading(enum WormHeading dir);
bool isInUseByWorm(int pos_x, int pos_y);

// ********************************************************************************************
// Functions
// ********************************************************************************************

// ************************************
// Management of the game
// ************************************

// Initialize colors of the game
void initializeColors() {
  // Define colors of the game
  start_color();
  init_pair(COLP_USER_WORM, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLP_FREE_CELL, COLOR_BLACK, COLOR_BLACK);
}

void readUserInput(enum GameStates *agame_state) {
  int ch; // For storing the key codes

  if ((ch = getch()) > 0) {
    // Is there some user input?
    // Blocking or non-blocking depends of config of getch
    switch (ch) {
    case 'q': // User wants to end the show
      *agame_state = WORM_GAME_QUIT;
      break;
    case KEY_UP: // User wants up
      setWormHeading(WORM_UP);
      break;
    case KEY_DOWN: // User wants down
      setWormHeading(WORM_DOWN);
      break;
    case KEY_LEFT: // User wants left
      setWormHeading(WORM_LEFT);
      break;
    case KEY_RIGHT: // User wants right
      setWormHeading(WORM_RIGHT);
      break;
    case '9': // User wants right
      setWormHeading(WORM_DIA_UR);
      break;
    case '7': // User wants right
      setWormHeading(WORM_DIA_UL);
      break;
    case '1': // User wants right
      setWormHeading(WORM_DIA_DL);
      break;
    case '3': // User wants right
      setWormHeading(WORM_DIA_DR);
      break;

    case 's': // User wants single step
              // make `getch` blocking
      nodelay(stdscr, false);
      break;
    case ' ': // Terminate single step; make getch non-blocking again
      nodelay(stdscr, true);
      break;
    }
  }
  return;
}

enum ResCodes doLevel() {
  enum GameStates game_state; // The current game_state

  int res_code;       // Result code from functions
  int end_level_loop; // Indicates whether we should leave the main loop

  int bottomLeft_y, bottomLeft_x; // Start positions of the worm

  // At the beginnung of the level, we still have a chance to win
  game_state = WORM_GAME_ONGOING;

  // There is always an initialized user worm.
  // Initialize the userworm with its size, position, heading.
  bottomLeft_y = getLastRow();
  bottomLeft_x = 0;

  res_code = initializeWorm(WORM_MAX_LEN, bottomLeft_y, bottomLeft_x,
                            WORM_RIGHT, COLP_USER_WORM);
  if (res_code != RES_OK) {
    return res_code;
  }

  // Show worm at its initial position
  showWorm();

  // Display all what we have set up until now
  refresh();

  // Start the loop for this level
  end_level_loop = false; // Flag for controlling the main loop
  while (end_level_loop == false) {
    // Process optional user input
    readUserInput(&game_state);
    if (game_state == WORM_GAME_QUIT) {
      printf("quit worm game");
      end_level_loop = true;
      continue; // Go to beginning of the loop's block and check loop condition
    }

    // Process userworm
    // Now move the worm for one step
    moveWorm(&game_state);
    // Bail out of the loop if something bad happened
    if (game_state != WORM_GAME_ONGOING) {
      printf("worm game not ongoing: %d", game_state);
      end_level_loop = true;
      continue; // Go to beginning of the loop's block and check loop condition
    }
    // Show the worm at its new position
    showWorm();
    // END process userworm

    // Sleep a bit before we show the updated window
    napms(NAP_TIME);

    // Display all the updates
    refresh();

    // Start next iteration
  }

  // Preset res_code for rest of the function
  res_code = RES_OK;

  // For some reason we left the control loop of the current level.
  // However, in this version we do not yet check for the reason.
  // There is no user feedback at the moment!

  // Normal exit point
  return res_code;
}

// *********************************************
// Standard curses initialization and cleanup
// *********************************************

// Initialize application with respect to curses settings
void initializeCursesApplication() {
  initscr(); // Initialize the curses screen

  // Note:
  // The call to initscr() defines various global variables of the curses
  // framework. stdscr, LINES, COLS, TRUE, FALSE

  noecho(); // Characters typed ar not echoed
  cbreak(); // No buffering of stdin
  nonl();   // Do not translate 'return key' on keyboard to newline character
  keypad(stdscr, true); // Enable the keypad
  curs_set(0);          // Make cursor invisible
  // Begin in non-single-step mode (getch will not block)
  nodelay(stdscr, true); // make getch to be a non-blocking call
}

// Reset display to normale state and terminate curses application
void cleanupCursesApp(void) {
  standend();  // Turn off all attributes
  refresh();   // Write changes to terminal
  curs_set(1); // Set cursor state to normal visibility
  endwin();    // Terminate curses application
}

// *************************************************
// Placing and removing items from the game board
// Check boundaries of game board
// *************************************************

// Place an item onto the curses display.
void placeItem(int y, int x, chtype symbol, enum ColorPairs color_pair) {

  //  Store item on the display (symbol code)
  move(y, x);                      // Move cursor to (y,x)
  attron(COLOR_PAIR(color_pair));  // Start writing in selected color
  addch(symbol);                   // Store symbol on the virtual display
  attroff(COLOR_PAIR(color_pair)); // Stop writing in selected color
}

// Getters

// Get the last usable row on the display
int getLastRow() { return LINES - 1; }

// Get the last usable column on the display
int getLastCol() { return COLS - 1; }

// *****************************************************
// Functions concerning the management of the worm data
// *****************************************************

// START WORM_DETAIL
// The following functions all depend on the model of the worm

// Initialize the worm
enum ResCodes initializeWorm(int max_len, int headpos_y, int headpos_x,
                             enum WormHeading dir, enum ColorPairs color) {
  theworm_max_index = max_len - 1;

  theworm_head_index = 0;

  int i;

  for (i = 1; i <= WORM_MAX_LEN; i++) {
    theworm_wormpos_x[i] = UNUSED_POS_ELEM;
    theworm_wormpos_y[i] = UNUSED_POS_ELEM;
  }

  theworm_wormpos_x[theworm_head_index] = headpos_x;
  theworm_wormpos_y[theworm_head_index] = headpos_y;

  // Initialize the heading of the worm
  setWormHeading(dir);
  // Initialze color of the worm
  theworm_wcolor = color;

  return RES_OK;
}

// Show the worms's elements on the display
// Simple version
void showWorm() {
  // Due to our encoding we just need to show the head element
  // All other elements are already displayed
  placeItem(theworm_wormpos_y[theworm_head_index],
            theworm_wormpos_x[theworm_head_index], SYMBOL_WORM_INNER_ELEMENT,
            theworm_wcolor);
}

void cleanWormTail() {
  int tailindex;

  tailindex = (theworm_head_index + 1) % (theworm_max_index + 1);

  // check whether elem at tailindex is already in use.
  // if yes, clean it up
  if (theworm_wormpos_x[tailindex] != UNUSED_POS_ELEM) {
    placeItem(theworm_wormpos_y[tailindex], theworm_wormpos_x[tailindex],
              SYMBOL_WORM_FREE_CELL, COLP_FREE_CELL);
  }
}

void moveWorm(enum GameStates *agame_state) {
  // Compute and store new head position according to current heading.

  int pos_x = (theworm_wormpos_x[theworm_head_index] + theworm_dx);
  int pos_y = (theworm_wormpos_y[theworm_head_index] + theworm_dy);

  // Check if we would leave the display if we move the worm's head according
  // to worm's last direction.
  // We are not allowed to leave the display's window.
  printf("theworm_wormpos_x[theworm_head_index]: %d  |  ",
         theworm_wormpos_x[theworm_head_index]);
  printf("theworm_wormpos_y[theworm_head_index]: %d  |  ",
         theworm_wormpos_y[theworm_head_index]);
  if (theworm_wormpos_x[theworm_head_index] < 0) {
    printf("head_x < 0  |  ");
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (theworm_wormpos_x[theworm_head_index] > getLastCol()) {
    printf("head_x > lastCol  |  ");
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (theworm_wormpos_y[theworm_head_index] < 0) {
    printf("head_y < 0  |  ");
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else if (theworm_wormpos_y[theworm_head_index] > getLastRow()) {
    printf("head_y > lastRow  |  ");
    *agame_state = WORM_OUT_OF_BOUNDS;
  } else {
    // We will stay within bounds.
    // So all is well
    // Do nothing

    if (isInUseByWorm(pos_x, pos_y)) {
      // thats bad, we need to break immediately
      *agame_state = WORM_CROSSING;
    }
  }

  if (*agame_state == WORM_GAME_ONGOING) {

    // start loop at 1, 0 refers to the head
    // dont go down to 0, we don't want to overwrite the head
    theworm_head_index = (theworm_head_index + 1) % (theworm_max_index + 1);

    // set new head pos
    theworm_wormpos_y[theworm_head_index] += theworm_dy;
    theworm_wormpos_x[theworm_head_index] += theworm_dx;
  }
}

// checks if a given coordinate is in use by this worm
bool isInUseByWorm(int pos_x, int pos_y) {
  int i;
  bool collision = false;
  i = theworm_head_index + 1;
  do {
    // Compare the position of the current worm element with the new_headpos
    if (theworm_wormpos_x[i] == pos_x) {
      if (theworm_wormpos_y[i] == pos_y) {
        collision = true;
        break;
      }
    }
    i = (i + theworm_max_index) % (theworm_max_index + 1);
  } while (i != theworm_head_index && theworm_wormpos_x[i] != UNUSED_POS_ELEM);
  // Return what we found out.
  return collision;
}

// Setters
void setWormHeading(enum WormHeading dir) {
  switch (dir) {
  case WORM_UP: // User wants up
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
  case WORM_DIA_UL: // User wants right
    theworm_dx = -1;
    theworm_dy = -1;
    break;
  case WORM_DIA_DL: // User wants right
    theworm_dx = -1;
    theworm_dy = 1;
    break;
  case WORM_DIA_DR: // User wants right
    theworm_dx = 1;
    theworm_dy = 1;
    break;
  case WORM_DIA_UR: // User wants right
    theworm_dx = 1;
    theworm_dy = -1;
    break;
  }
}

// END WORM_DETAIL
// ********************************************************************************************

// ********************************************************************************************
// MAIN
// ********************************************************************************************

int main(void) {
  enum ResCodes res_code; // Result code from functions

  // Here we start
  initializeCursesApplication(); // Init various settings of our application
  initializeColors();            // Init colors used in the game

  // Maximal LINES and COLS are set by curses for the current window size.
  // Note: we do not cope with resizing in this simple examples!

  // Check if the window is large enough to display messages in the message area
  // a has space for at least one line for the worm
  if (LINES < MIN_NUMBER_OF_ROWS || COLS < MIN_NUMBER_OF_COLS) {
    // Since we not even have the space for displaying messages
    // we print a conventional error message via printf after
    // the call of cleanupCursesApp()
    cleanupCursesApp();
    printf("Das Fenster ist zu klein: wir brauchen mindestens %dx%d\n",
           MIN_NUMBER_OF_COLS, MIN_NUMBER_OF_ROWS);
    res_code = RES_FAILED;
  } else {
    res_code = doLevel();
    cleanupCursesApp();
  }

  return res_code;
}
