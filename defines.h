#ifndef DEFINES_H
#define DEFINES_H

#include <qglobal.h>

/* Strings for the configuration file */
extern const char *OP_GRP;
extern const char *OP_UMARK;
extern const char *OP_MENUBAR;
extern const char *OP_LEVEL;
extern const char *OP_CASE_SIZE;
extern const char *OP_KEYBOARD;
extern const char *OP_MOUSE_BINDINGS[3];

extern const char *HS_NAME;
extern const char *HS_MIN;
extern const char *HS_SEC;

/* States of a case (unsigned int) */
#define NOTHING    0
#define MINE       1
#define COVERED    2
#define UNCERTAIN  4
#define MARKED     8
#define UNCOVERED  16
#define EXPLODED   32
#define ERROR      64

/* Default case size */
extern const uint CASE_SIZE;
extern const uint MIN_CASE_SIZE;
extern const uint MAX_CASE_SIZE;

enum GameType    { Easy = 0, Normal, Expert, Custom, NbLevels };
enum GameState   { Stopped, Playing, Paused, GameOver };
enum MouseAction { Reveal = 0, Mark, AutoReveal, UMark };
enum MouseButton { Left = 0, Mid, Right };

struct Level {
	uint     width, height, nbMines;
	GameType type;
};
extern const Level LEVELS[NbLevels-1];
extern const char *HS_GRP[NbLevels-1];

struct Score {
	uint     sec, min;
	GameType type;
};

#endif // DEFINES_H
