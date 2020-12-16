/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#pragma once

#include <stdlib.h>
#include <stdint.h>

enum math 
{
	ADD,
	SUB,
	MUL,
	DIV,
	RUT,
	POW,
	MOD,
	PRC,
	UNF
};

enum math_ext 
{
	PW2,
	PW3,	
	RT2,
	RT3,
	D1R,
	D1X,
	LGN,
	LOG,
	FAC,
	SIN,
	COS,
	TAN,
	CPI,
	CEU,
	UND
};

void gcmp_mpfr_all ( enum math, const char *, const char *, uint16_t, uint8_t, uint8_t, char * );

void gcmp_mpfr_all_ext ( enum math_ext, const char *, uint16_t, uint8_t, uint8_t, char *, uint8_t );

