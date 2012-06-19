#include "taster.h"

volatile signed char taster = NO_TASTER;

taste_t tasten[NUM_TASTER];

/*
 * Aktualisiert 'taster', falls taster==NO_TASTER
 * num:  Nummer des Tasters von 0...NUM_TASTER-1
 * TASTER_LEVEL=1:
 *    tast: ==0 falls der Taster gerade nicht gedrueckt wird
 *    tast: !=0 falls der Taster gerade gedrueckt wird
 * TASTER_LEVEL=0:
 *    tast: !=0 falls der Taster gerade nicht gedrueckt wird
 *    tast: ==0 falls der Taster gerade gedrueckt wird
 */
void get_taster (const unsigned char num, unsigned char tast)
{
	const taste_t * const ptast = &tasten[num];
	const unsigned char taster_old = ptast->old;
	unsigned char pressed, press, release, mode, delay;

#if TASTER_LEVEL
	tast = !!tast;
#else
	tast = !tast;
#endif

	/* Was wurde gedrueckt/losgelassen...? */

	/* Taster bleibt gedrueckt */
	pressed =  taster_old &  tast;
	/* Taster neu gedrueckt */
	press   = ~taster_old &  tast;
	/* Taster losgelassen */
	release =  taster_old & ~tast;

	/* ptast->old = tast;
	 * Der Cast dient zum Wegwerfen des 'const' Qualifiers.
	 * Die Komponente wurde als 'const' qualifiziert,
	 * damit es einen Fehler gibt, wenn versucht wird,
	 * ihren Wert von aussen zu aendern (private). */
	*((unsigned char *) &ptast->old) = tast;

	tast = NO_TASTER;

	mode  = ptast->mode;
	delay = ptast->delay;

	if (press)
	{
		if (mode != TM_LONG)
			tast = num;

		delay = 0;
	}
	else if (pressed)
	{
		if (delay < 0xfe)
			delay++;
	}
	else if (release)
	{
		if (mode == TM_LONG && delay != 0xff)
			tast = num;
	}

	if (mode == TM_LONG)
	{
		if (delay == TASTER_DELAY_LONG)
		{
			tast = TASTER_LONG + num;
			delay = 0xff;
		}
	}
	else if (mode == TM_REPEAT)
	{
		if (delay == TASTER_REPEAT_DELAY)
		{
			tast = num;
			delay = TASTER_REPEAT_DELAY - TASTER_REPEAT;
		}
	}

	if (taster == NO_TASTER)
		taster = tast;

	/* siehe oben */
	*((unsigned char *) &ptast->delay) = delay;
}
