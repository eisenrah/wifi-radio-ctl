#ifndef _TASTER_H_
#define _TASTER_H_

/* Wert fuer taster, wenn nichts gedrückt wurde */
#define NO_TASTER (-1)

/* Maximale Anzahl der Taster */
#define NUM_TASTER 1

/* 0 --> Taster sind low-aktiv */
/* 1 --> Taster sind high-aktiv */
#define TASTER_LEVEL 0

/* Dieser Offset wird zur Tasten-Nummer addiert, */
/* wenn eine Taste lange gedrückt wurde */
#define TASTER_LONG 16

/* Zeitverzögerung (in Ticks), bis zum Beginn von auto-repeat */
#define TASTER_REPEAT_DELAY (60)

/* Zeitverzögerung (in Ticks), bis zum nächsten auto-repeat */
#define TASTER_REPEAT       (15)

/* Ab dieser Dauer wird der Tastendruck 'lange' */
#define TASTER_DELAY_LONG   (80)

typedef struct
{
	/* private */
	const unsigned char delay, old;

	/* Mode des Tasters aus: TM_SHORT, TM_LONG, TM_REPEAT */
	unsigned char mode;
} taste_t;

extern taste_t tasten[];

/* In dieser Variable kann abgefragt werden, welche Taste gedrückt wurde.
   --> NO_TASTER:
       es wurde nichts gedrückt
   --> 0..NUM_TASTER-1:
       Taster Numero 'taster' wurde (kurz) gedrückt
   --> TASTER_LONG ... TASTER_LONG + NUM_TASTER-1:
       Taster Numero 'taster-TASTER_LONG' wurde lange gedrückt
 */
extern volatile signed char taster;

extern void get_taster (const unsigned char num, unsigned char tast);

enum
{
	TM_SHORT,
	TM_LONG,
	TM_REPEAT
};

#endif /* _TASTER_H_ */
