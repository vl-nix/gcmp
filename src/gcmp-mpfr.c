/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-mpfr.h"

#include <mpfr.h>

typedef unsigned long ulong;

static void mpfr_prc ( mpfr_t res, mpfr_t a, mpfr_t b, uint16_t digits, mpfr_rnd_t rnd )
{
	mpfr_t c;
	mpfr_init2 ( c, digits * 4 );
	mpfr_set_d ( c, 100.0, MPFR_RNDD );

	mpfr_mul ( res, a, b, rnd );
	mpfr_div ( res, res, c, rnd );

	mpfr_clear ( c );
}

static void gcmp_mpfr_init_str ( mpfr_t res, mpfr_t a, mpfr_t b, const char *a_str, const char *b_str, uint16_t digits, uint8_t base )
{
	mpfr_init2 ( a, digits * 4 );
	mpfr_set_str ( a, a_str, base, MPFR_RNDN );

	mpfr_init2 ( b, digits * 4 );
	mpfr_set_str ( b, b_str, base, MPFR_RNDN );

	mpfr_init2 ( res, digits * 4 );
	mpfr_set_d ( res, 0.0, MPFR_RNDD );
}

static void gcmp_mpfr_clear_all ( mpfr_t a, mpfr_t b, mpfr_t res )
{
	mpfr_clear ( a );
	mpfr_clear ( b );
	mpfr_clear ( res );
}

static void gcmp_mpfr_get_str ( mpfr_t res, uint16_t digits, uint8_t out_fm, char *out_str )
{
	if ( out_fm == 0 ) mpfr_sprintf ( out_str, "%.*Rg", digits, res );
	if ( out_fm == 1 ) mpfr_sprintf ( out_str, "%.*Re", digits, res );
	if ( out_fm == 2 ) mpfr_sprintf ( out_str, "%.*Rf", digits, res );
}

void gcmp_mpfr_all ( enum math mt, const char *a_str, const char *b_str, uint16_t digits, uint8_t out_fm, uint8_t base, char *out_str )
{
	mpfr_t a, b, res;

	gcmp_mpfr_init_str ( res, a, b, a_str, b_str, digits, base );

	if ( mt == ADD ) mpfr_add ( res, a, b, MPFR_RNDD );
	if ( mt == SUB ) mpfr_sub ( res, a, b, MPFR_RNDD );
	if ( mt == MUL ) mpfr_mul ( res, a, b, MPFR_RNDD );
	if ( mt == DIV ) mpfr_div ( res, a, b, MPFR_RNDD );

	if ( mt == RUT ) mpfr_rootn_ui ( res, a, (ulong)atol ( b_str ), MPFR_RNDD );
	if ( mt == POW ) mpfr_pow  ( res, a, b, MPFR_RNDD );
	if ( mt == MOD ) mpfr_fmod ( res, a, b, MPFR_RNDD );
	if ( mt == PRC ) mpfr_prc  ( res, a, b, digits, MPFR_RNDD );

	gcmp_mpfr_get_str ( res, digits, out_fm, out_str );

	gcmp_mpfr_clear_all ( a, b, res );
}

static void gcmp_mpfr_init_str_one ( mpfr_t a, const char *a_str, uint16_t digits, uint8_t base )
{
	mpfr_init2 ( a, digits*4 );
	mpfr_set_str ( a, a_str, base, MPFR_RNDN );
}

static void mpfr_sct ( enum math_ext mt, mpfr_t res, mpfr_t a, const char *a_str, uint16_t digits, uint8_t base, uint8_t deg_rad )
{
	mpfr_t grd, pi;

	mpfr_init2 ( pi,  digits*4 );
	mpfr_const_pi ( pi, MPFR_RNDN );

	gcmp_mpfr_init_str_one ( grd, "180", digits, base );

	if ( deg_rad )
	{
		mpfr_div ( grd, pi, grd, MPFR_RNDD );
		mpfr_mul ( grd,  a, grd, MPFR_RNDD );
	}
	else
		mpfr_set_str ( grd, a_str, base, MPFR_RNDN );

	if ( mt == SIN ) mpfr_sin ( res, grd, MPFR_RNDD );
	if ( mt == COS ) mpfr_cos ( res, grd, MPFR_RNDD );
	if ( mt == TAN ) mpfr_tan ( res, grd, MPFR_RNDD );

	mpfr_clear ( grd );
	mpfr_clear ( pi  );
}

void gcmp_mpfr_all_ext ( enum math_ext mt, const char *a_str, uint16_t digits, uint8_t out_fm, uint8_t base, char *out_str, uint8_t deg_rad )
{
	mpfr_t a, b, res;
	mpfr_t one, two, tre;

	gcmp_mpfr_init_str ( res, a, b, a_str, "0", digits, base );

	gcmp_mpfr_init_str_one ( one, "1", digits, base );
	gcmp_mpfr_init_str_one ( two, "2", digits, base );
	gcmp_mpfr_init_str_one ( tre, "3", digits, base );

	if ( mt == RT2 ) mpfr_sqrt ( res, a, MPFR_RNDD );
	if ( mt == RT3 ) mpfr_cbrt ( res, a, MPFR_RNDD );

	if ( mt == D1R ) mpfr_rec_sqrt ( res, a, MPFR_RNDD );
	if ( mt == D1X ) mpfr_div      ( res, one, a, MPFR_RNDD );

	if ( mt == PW2 ) mpfr_pow ( res, a, two, MPFR_RNDD );
	if ( mt == PW3 ) mpfr_pow ( res, a, tre, MPFR_RNDD );

	if ( mt == LGN ) mpfr_log   ( res, a, MPFR_RNDD );
	if ( mt == LOG ) mpfr_log10 ( res, a, MPFR_RNDD );

	if ( mt == FAC ) { if ( atol ( a_str ) > 1 ) mpfr_fac_ui ( res, (ulong)atol ( a_str ), MPFR_RNDD ); }

	if ( mt == CPI ) mpfr_const_pi    ( res, MPFR_RNDN );
	if ( mt == CEU ) mpfr_const_euler ( res, MPFR_RNDN );

	if ( mt == SIN || mt == COS || mt == TAN ) mpfr_sct ( mt, res, a, a_str, digits, base, deg_rad );

	gcmp_mpfr_get_str ( res, digits, out_fm, out_str );

	gcmp_mpfr_clear_all ( a, b, res );
	gcmp_mpfr_clear_all ( one, two, tre );
	mpfr_free_cache ();
}

