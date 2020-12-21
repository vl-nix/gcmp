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

#define COL 8
#define ROW 5
#define NUM_BUTTONS (COL*ROW)

#define BUTTON_SIZE 80

enum bt_num 
{
	BMN, BSG, BEP, BEM, BPI, BEU, BDS, BCL,
	BN7, BN8, BN9, BDV, BP2, BP3, BPN, BFC,
	BN4, BN5, BN6, BML, BR2, BR3, BRN, B1R,
	BN1, BN2, BN3, BSB, BLN, BLG, BMD, B1X,
	BN0, BDT, BPR, BAD, BSN, BCS, BTN, BDR
};

#define GCMP_TYPE_TOOL gcmp_tool_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpTool, gcmp_tool, GCMP, TOOL, GtkBox )

GcmpTool * gcmp_tool_new ( uint );

void gcmp_tool_set_mini ( GcmpTool * );

GtkButton * gcmp_tool_get_button ( enum bt_num, GcmpTool * );

