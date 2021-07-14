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

#define COL 4
#define ROW 5
#define NUM_BUTTONS (COL*ROW)

#define BUTTON_SIZE 80

enum b_num 
{
	BMN, BSG, BDS, BCL,
	BN7, BN8, BN9, BDV,
	BN4, BN5, BN6, BML,
	BN1, BN2, BN3, BSB,
	BN0, BDT, BPR, BAD
};

enum b_ext_num 
{
	BEP, BEM, BPI, BPF,
	BP2, BP3, BPN, BFC,
	BR2, BR3, BRN, B1R,
	BLN, BLG, BMD, B1X,
	BSN, BCS, BTN, BDR
};

#define GCMP_TYPE_TOOL gcmp_tool_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpTool, gcmp_tool, GCMP, TOOL, GtkBox )

GcmpTool * gcmp_tool_new ( void );

#define GCMP_TYPE_TOOLEXT gcmp_tool_ext_get_type ()

G_DECLARE_FINAL_TYPE ( GcmpToolExt, gcmp_tool_ext, GCMP, TOOLEXT, GtkBox )

GcmpToolExt * gcmp_tool_ext_new ( void );


