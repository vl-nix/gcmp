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

#define GCMP_TYPE_ENTRY gcmp_entry_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpEntry, gcmp_entry, GCMP, ENTRY, GtkBox )

GcmpEntry * gcmp_entry_new ( void );

