/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-tool.h"

struct _GcmpTool
{
	GtkBox parent_instance;

	GtkButton *button[NUM_BUTTONS];
};

G_DEFINE_TYPE ( GcmpTool, gcmp_tool, GTK_TYPE_BOX )

static const char *b_name[NUM_BUTTONS] = 
{
	"⌨", "±", "←", "C",
	"7", "8", "9", "/",
	"4", "5", "6", "*",
	"1", "2", "3", "-",
	"0", ".", "%", "+"
};

static const char *b_ext_name[NUM_BUTTONS] = 
{
	"e+", "e-", "π", "⚒",
	"x²", "x³", "xⁿ", "n!",
	"√", "³√", "ⁿ√", "⅟√",
	"ln", "log", "mod", "⅟x",
	"sin", "cos", "tan", "deg"
};

static void gcmp_tool_signal_handler_num ( GtkButton *button, GcmpTool *tool )
{
	const char *label = gtk_button_get_label ( button );
	const char *name  = gtk_widget_get_name ( GTK_WIDGET ( button ) );

	uint8_t num = ( uint8_t )( atoi ( name ) );

	g_signal_emit_by_name ( tool, "buttons-click-num-label", num, label );
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

		tool->button[i] = (GtkButton *)gtk_button_new_with_label ( b_name[i] );
		gtk_widget_set_size_request ( GTK_WIDGET ( tool->button[i] ), BUTTON_SIZE, -1 );

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
	GtkBox *box = GTK_BOX ( tool );
	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( box ), GTK_ORIENTATION_VERTICAL );

	gtk_box_set_spacing ( box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( box ), TRUE );

	gcmp_tool_create ( tool );
}

static void gcmp_tool_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_tool_parent_class)->finalize (object);
}

static void gcmp_tool_class_init ( GcmpToolClass *class )
{
	GObjectClass *oclass = G_OBJECT_CLASS (class);

	oclass->finalize = gcmp_tool_finalize;

	g_signal_new ( "buttons-click-num-label", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING );
}

GcmpTool * gcmp_tool_new ( void )
{
	return g_object_new ( GCMP_TYPE_TOOL, NULL );
}



struct _GcmpToolExt
{
	GtkBox parent_instance;

	GtkButton *button[NUM_BUTTONS];
};

G_DEFINE_TYPE ( GcmpToolExt, gcmp_tool_ext, GTK_TYPE_BOX )

static GObject * gcmp_tool_ext_get_button ( GcmpToolExt *tool )
{
	return G_OBJECT ( tool->button[BPF] );
}

static void gcmp_tool_ext_signal_handler_num ( GtkButton *button, GcmpToolExt *tool )
{
	const char *label = gtk_button_get_label ( button );
	const char *name  = gtk_widget_get_name ( GTK_WIDGET ( button ) );

	uint8_t num = ( uint8_t )( atoi ( name ) );

	g_signal_emit_by_name ( tool, "buttons-click-num-label", num, label );
}

static void gcmp_tool_ext_set_label ( GcmpToolExt *tool, uint8_t num, const char *label )
{
	gtk_button_set_label ( tool->button[num], label );
}

static void gcmp_tool_ext_create ( GcmpToolExt *tool )
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

		tool->button[i] = (GtkButton *)gtk_button_new_with_label ( b_ext_name[i] );
		gtk_widget_set_size_request ( GTK_WIDGET ( tool->button[i] ), BUTTON_SIZE, -1 );

		char buf[20];
		sprintf ( buf, "%d", i );
		gtk_widget_set_name ( GTK_WIDGET ( tool->button[i] ), buf );

		g_signal_connect ( tool->button[i], "clicked", G_CALLBACK ( gcmp_tool_ext_signal_handler_num ), tool );

		gtk_widget_set_visible ( GTK_WIDGET ( tool->button[i] ), TRUE );
		gtk_box_pack_start ( h_box, GTK_WIDGET ( tool->button[i] ), TRUE, TRUE, 0 );
	}
}

static void gcmp_tool_ext_init ( GcmpToolExt *tool )
{
	GtkBox *box = GTK_BOX ( tool );
	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( box ), GTK_ORIENTATION_VERTICAL );

	gtk_box_set_spacing ( box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( box ), TRUE );

	gcmp_tool_ext_create ( tool );
	g_signal_connect ( tool, "toolext-set-label",  G_CALLBACK ( gcmp_tool_ext_set_label  ), NULL );
	g_signal_connect ( tool, "toolext-get-button", G_CALLBACK ( gcmp_tool_ext_get_button ), NULL );
}

static void gcmp_tool_ext_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_tool_ext_parent_class)->finalize (object);
}

static void gcmp_tool_ext_class_init ( GcmpToolExtClass *class )
{
	GObjectClass *oclass = G_OBJECT_CLASS (class);

	oclass->finalize = gcmp_tool_ext_finalize;

	g_signal_new ( "toolext-get-button", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_OBJECT, 0 );

	g_signal_new ( "toolext-set-label", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING );

	g_signal_new ( "buttons-click-num-label", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING );
}

GcmpToolExt * gcmp_tool_ext_new ( void )
{
	return g_object_new ( GCMP_TYPE_TOOLEXT, NULL );
}


