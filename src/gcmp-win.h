/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#pragma once

#include "gcmp-app.h"

#define GCMP_WIN_TYPE_WINDOW gcmp_win_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpWin, gcmp_win, GCMP, WIN, GtkWindow )

GcmpWin * gcmp_win_new ( GcmpApp * );

