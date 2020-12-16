/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#pragma once

#include <gtk/gtk.h>

/* Precision ( maximum characters ) */
#define MAX_DIGITS 1000

#define GCMP_APP_TYPE_APPLICATION gcmp_app_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpApp, gcmp_app, GCMP, APP, GtkApplication )

GcmpApp * gcmp_app_new ( void );

