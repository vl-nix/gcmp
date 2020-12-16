/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-tool.h"

enum prop
{
	PROP_0,
	PROP_SIZE,
	N_PROPS
};

struct _GcmpTool
{
	GtkBox parent_instance;

	GtkButton *button[NUM_BUTTONS];

	uint button_size;
};

G_DEFINE_TYPE ( GcmpTool, gcmp_tool, GTK_TYPE_BOX )

static const char *bt_name[NUM_BUTTONS] = 
{
	"⌨", "±", "e+", "e-",    "π", "ε", "←", "C",
	"7", "8", "9", " / ",    "x²", "x³", "xⁿ", "n!",
	"4", "5", "6", " * ",    "√", "³√", "ⁿ√", "⅟√",
	"1", "2", "3", " - ",    "ln", "log", "mod", "⅟x",
	"0", ".", " % ", " + ",  "sin", "cos", "tan", "deg"
};

GtkButton * gcmp_tool_get_button ( enum bt_num num, GcmpTool *tool )
{
	return tool->button[num];
}

void gcmp_tool_set_mini ( GcmpTool *tool )
{
	uint8_t z_n[] = { BEP, BEM, BPI, BEU, BP2, BP3, BPN, BFC, BR2, BR3, BRN, B1R, BLN, BLG, BMD, B1X, BSN, BCS, BTN, BDR };

	uint8_t i = 0; for ( i = 0; i < G_N_ELEMENTS ( z_n ); i++ )
	{
		gtk_widget_get_visible ( GTK_WIDGET ( tool->button[z_n[i]] ) )
			? gtk_widget_set_visible  ( GTK_WIDGET ( tool->button[z_n[i]] ), FALSE )
			: gtk_widget_set_visible  ( GTK_WIDGET ( tool->button[z_n[i]] ), TRUE  );
	}
}

static void gcmp_tool_resize ( GcmpTool *tool )
{
	int size = (int)tool->button_size;

	uint8_t i = 0; for ( i = 0; i < NUM_BUTTONS; i++ )
	{
		gtk_widget_set_size_request ( GTK_WIDGET ( tool->button[i] ), size, -1 );
	}
}

static void gcmp_tool_signal_handler_num ( GtkButton *button, GcmpTool *tool )
{
	const char *name = gtk_widget_get_name ( GTK_WIDGET ( button ) );

	uint num = ( uint )( atoi ( name ) );
	g_signal_emit_by_name ( tool, "button-click-num", num );
}

static void gcmp_tool_create ( GcmpTool *tool )
{
	uint8_t i = 0, row = 0;

	GtkBox *h_box;

	for ( i = 0; i < NUM_BUTTONS; i++ )
	{
		if ( i == COL * row )
		{
			h_box  = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
			gtk_box_set_spacing ( h_box,  5 );

			gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );
			gtk_box_pack_start ( GTK_BOX ( tool ), GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );
			row++;
		}

		if ( i == 0 )
			tool->button[i] = (GtkButton *)gtk_button_new_from_icon_name ( "gcmp-ext", GTK_ICON_SIZE_MENU );
		else
			tool->button[i] = (GtkButton *)gtk_button_new_with_label ( bt_name[i] );

		char buf[20];
		sprintf ( buf, "%d", i );
		gtk_widget_set_name ( GTK_WIDGET ( tool->button[i] ), buf );

		g_signal_connect ( tool->button[i], "clicked", G_CALLBACK ( gcmp_tool_signal_handler_num ), tool );

		gtk_widget_set_visible ( GTK_WIDGET ( tool->button[i] ), TRUE );
		gtk_box_pack_start ( h_box, GTK_WIDGET ( tool->button[i] ), TRUE, TRUE, 0 );
	}
}

static void gcmp_tool_init ( GcmpTool *tool )
{
	tool->button_size = BUTTON_SIZE;

	GtkBox *box = GTK_BOX ( tool );
	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( box ), GTK_ORIENTATION_VERTICAL );
	gtk_box_set_spacing ( box, 5 );

	gcmp_tool_create ( tool );
}

static void gcmp_tool_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_tool_parent_class)->finalize (object);
}

static void gcmp_tool_get_property ( GObject *object, uint id, GValue *value, GParamSpec *pspec )
{
	GcmpTool *tool = GCMP_TOOL (object);

	switch ( id )
	{
		case PROP_SIZE:
			g_value_set_uint ( value, tool->button_size );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, id, pspec );
			break;
	}
}

static void gcmp_tool_set_property ( GObject *object, uint id, const GValue *value, GParamSpec *pspec )
{
	GcmpTool *tool = GCMP_TOOL (object);

	switch ( id )
	{
		case PROP_SIZE:
			tool->button_size = g_value_get_uint (value);
			gcmp_tool_resize ( tool );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, id, pspec );
			break;
	}
}

static void gcmp_tool_class_init ( GcmpToolClass *class )
{
	GObjectClass *oclass = G_OBJECT_CLASS (class);

	oclass->finalize = gcmp_tool_finalize;

	g_signal_new ( "button-click-num", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT );

	oclass->get_property = gcmp_tool_get_property;
	oclass->set_property = gcmp_tool_set_property;

	g_object_class_install_property ( oclass, PROP_SIZE, g_param_spec_uint ( "button-size", NULL, NULL, 24, G_MAXUINT, BUTTON_SIZE, G_PARAM_READWRITE ) );
}

GcmpTool * gcmp_tool_new ( uint button_size )
{
	return g_object_new ( GCMP_TOOL_TYPE_BOX, "button-size", button_size, NULL );
}

