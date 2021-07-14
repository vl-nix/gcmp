/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-entry.h"

enum cols
{
	COL_DATA,
	COL_RESL,
	NUM_COLS
};

struct _GcmpEntry
{
	GtkBox parent_instance;

	GtkEntry *entry;
	GtkTreeView *treeview;
	GtkPopover *popover_edit;

	ulong entry_signal_id;
};

G_DEFINE_TYPE ( GcmpEntry, gcmp_entry, GTK_TYPE_BOX )

static void gcmp_entry_treeview_append ( const char *data, const char *res, GcmpEntry *entry );

static char * gcmp_entry_get_text ( GcmpEntry *entry )
{
	return g_strdup ( gtk_entry_get_text ( entry->entry ) );
}

static void gcmp_entry_set_text ( GcmpEntry *entry, const char *text, gboolean add )
{
	const char *old_text = gtk_entry_get_text ( entry->entry );

	if ( !add )
	{
		gcmp_entry_treeview_append ( old_text, text, entry );

		gtk_entry_set_text ( entry->entry, text );
	}
	else
	{
		g_autofree char *text_set = g_strdup_printf ( "%s%s", old_text, text );

		gtk_entry_set_text ( entry->entry, text_set );
	}

	uint16_t len = gtk_entry_get_text_length ( entry->entry );
	g_signal_emit_by_name ( entry->entry, "move-cursor", GTK_MOVEMENT_WORDS, len, FALSE, NULL );
}

static void gcmp_entry_clr ( GcmpEntry *entry )
{
	gtk_entry_set_text ( entry->entry, "" );
}

static void gcmp_entry_dec ( GcmpEntry *entry )
{
	uint16_t len = gtk_entry_get_text_length ( entry->entry );

	if ( len == 0 ) return;

	const char *text = gtk_entry_get_text ( entry->entry );

	g_autofree char *text_set = g_strndup ( text, len - 1 );

	gtk_entry_set_text ( entry->entry, text_set );
}

static void gcmp_entry_sgn ( GcmpEntry *entry )
{
	const char *text = gtk_entry_get_text ( entry->entry );

	char text_set[UINT16_MAX];
	sprintf ( text_set, "%s%s", text, "-" );

	g_signal_handler_block   ( entry->entry, entry->entry_signal_id );

		gtk_entry_set_text ( entry->entry, text_set );

	g_signal_handler_unblock ( entry->entry, entry->entry_signal_id );
}

static void gcmp_entry_check_op ( GcmpEntry *entry )
{
	const char *op_n[] = { "+", "-", "*", "/", "%", "^", "√", "m" };
	const char *text  = gtk_entry_get_text ( entry->entry );

	uint8_t x = 0, y = 0;
	for ( x = 0; x < G_N_ELEMENTS ( op_n ); x++ )
	{
		for ( y = 0; y < G_N_ELEMENTS ( op_n ); y++ )
		{
			char c1_text[8];
			sprintf ( c1_text, "%s%s", op_n[x], op_n[y] );

			char c2_text[8];
			sprintf ( c2_text, "%s %s", op_n[x], op_n[y] );

			if ( g_str_has_suffix ( text, c1_text ) || g_str_has_suffix ( text, c2_text ) ) gcmp_entry_dec ( entry );
		}
	}
}

static void gcmp_entry_check_epm ( GcmpEntry *entry )
{
	const char *op_n[] = { "e+", "e-" };
	const char *text  = gtk_entry_get_text ( entry->entry );

	uint8_t x = 0, y = 0;
	for ( x = 0; x < G_N_ELEMENTS ( op_n ); x++ )
	{
		for ( y = 0; y < G_N_ELEMENTS ( op_n ); y++ )
		{
			char c1_text[8];
			sprintf ( c1_text, "%s%s", op_n[x], op_n[y] );

			char c2_text[8];
			sprintf ( c2_text, "%s %s", op_n[x], op_n[y] );

			if ( g_str_has_suffix ( text, c1_text ) || g_str_has_suffix ( text, c2_text ) ) gcmp_entry_dec ( entry );
		}
	}

	if ( g_str_has_prefix ( text, "e+" ) || g_str_has_prefix ( text, "e-" ) ) gcmp_entry_dec ( entry );
}

static void gcmp_entry_check_sp ( GcmpEntry *entry )
{
	const char *sp_n[] = { "~", "!", "@", "#", "$", "&", "(", ")", "_", "=" };
	const char *text  = gtk_entry_get_text ( entry->entry );

	uint8_t x = 0;
	for ( x = 0; x < G_N_ELEMENTS ( sp_n ); x++ )
	{
		if ( g_str_has_suffix ( text, sp_n[x] ) ) gcmp_entry_dec ( entry );
	}
}

static gboolean gcmp_entry_check_sct ( GcmpEntry *entry )
{
	gboolean ret = FALSE;
	const char *sct_n[] = { "s", "si", "sin", "sin ", "c", "co", "cos", "cos ", "t", "ta", "tan", "tan " };
	const char *text  = gtk_entry_get_text ( entry->entry );

	uint8_t x = 0;
	for ( x = 0; x < G_N_ELEMENTS ( sct_n ); x++ )
	{
		if ( g_str_has_suffix ( text, sct_n[x] ) ) ret = TRUE;
	}

	if ( g_str_has_suffix ( text, "ss" ) || g_str_has_suffix ( text, "cc" ) || g_str_has_suffix ( text, "tt" ) ) ret = FALSE;

	return ret;
}

static gboolean gcmp_entry_check_log ( GcmpEntry *entry )
{
	gboolean ret = FALSE;
	const char *sct_n[] = { "l", "lo", "log", "log ", "ln", "ln " };
	const char *text  = gtk_entry_get_text ( entry->entry );

	uint8_t x = 0;
	for ( x = 0; x < G_N_ELEMENTS ( sct_n ); x++ )
	{
		if ( g_str_has_suffix ( text, sct_n[x] ) ) ret = TRUE;
	}

	if ( g_str_has_suffix ( text, "ll" ) ) ret = FALSE;

	return ret;
}

static void gcmp_entry_changed ( GtkEntry *entry, GcmpEntry *tool )
{
	uint16_t len = gtk_entry_get_text_length ( entry );
	const char *text = gtk_entry_get_text ( entry );

	if ( !text || len == 0 ) return;

	uint32_t c = text[len-1];

	if ( g_str_has_suffix ( text, "nan" ) || g_str_has_suffix ( text, "inf" ) ) return;

	if ( gcmp_entry_check_sct ( tool ) ) return;
	if ( gcmp_entry_check_log ( tool ) ) return;

	if ( len == 1 && !g_unichar_isdigit ( c ) ) { gcmp_entry_dec ( tool ); return; }

	if ( g_unichar_isalpha ( c ) && c != 'm'  ) { gcmp_entry_dec ( tool ); return; }

	gcmp_entry_check_op ( tool );
	gcmp_entry_check_sp ( tool );
	gcmp_entry_check_epm ( tool );

	if ( ( len == 1 && g_str_has_suffix ( text, " " ) ) || g_str_has_suffix ( text, "  " ) || g_str_has_suffix ( text, ".." ) 
		|| g_str_has_suffix ( text, " ." ) || g_str_has_suffix ( text, ". " ) ) gcmp_entry_dec ( tool );
}

static void gcmp_entry_treeview_append ( const char *data, const char *res, GcmpEntry *entry )
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( entry->treeview );

	gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
	gtk_list_store_set    ( GTK_LIST_STORE ( model ), &iter, COL_DATA, data, COL_RESL, res, -1 );
}

static void gcmp_entry_treeview_create_columns ( GtkTreeView *tree_view, int column_id )
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();

	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ( "", renderer, "text", column_id, NULL );
	gtk_tree_view_append_column ( tree_view, column );
}

static void gcmp_entry_treeview_add_columns ( GtkTreeView *tree_view )
{
	uint8_t c = 0; for ( c = 0; c < NUM_COLS; c++ )
		gcmp_entry_treeview_create_columns ( tree_view, c );
}

static void gcmp_entry_treeview_row_activated ( GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, GcmpEntry *entry )
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( GTK_TREE_VIEW ( tree_view ) );

	uint8_t c = 0, num = 0;

	for ( c = 0; c < gtk_tree_view_get_n_columns ( tree_view ); c++ )
	{
		GtkTreeViewColumn *column_f = gtk_tree_view_get_column ( tree_view, c );

		if ( column_f == column ) num = c;
	}

	if ( gtk_tree_model_get_iter ( model, &iter, path ) )
	{
		g_autofree char *data = NULL;
		gtk_tree_model_get ( model, &iter, num, &data, -1 );

		const char *text = gtk_entry_get_text ( entry->entry );
		uint16_t len = gtk_entry_get_text_length ( entry->entry );

		g_autofree char *text_set = ( len ) ? g_strdup_printf ( "%s %s", text, data ) : g_strdup_printf ( "%s", data );

		gtk_entry_set_text ( entry->entry, text_set );
	}
}

static GtkTreeView * gcmp_entry_treeview_new ( GcmpEntry *entry )
{
	GtkListStore *store = (GtkListStore *)gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_STRING );

	GtkTreeView *treeview = (GtkTreeView *)gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ) );
	gtk_tree_view_set_headers_visible ( treeview, FALSE );

	gcmp_entry_treeview_add_columns ( treeview );
	g_object_set ( treeview, "activate-on-single-click", TRUE, NULL );

	g_signal_connect ( treeview, "row-activated", G_CALLBACK ( gcmp_entry_treeview_row_activated ), entry );

	return treeview;
}

static GtkScrolledWindow * gcmp_entry_create_scroll ( GtkTreeView *tree_view )
{
	GtkScrolledWindow *scroll = (GtkScrolledWindow *)gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_widget_set_size_request ( GTK_WIDGET ( scroll ), 200, 100 );
	gtk_container_add ( GTK_CONTAINER ( scroll ), GTK_WIDGET ( tree_view ) );

	gtk_widget_set_visible ( GTK_WIDGET ( scroll ), TRUE );

	return scroll;
}

static GtkBox * gcmp_entry_create_history ( GcmpEntry *entry )
{
	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	entry->treeview = gcmp_entry_treeview_new ( entry );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( gcmp_entry_create_scroll ( entry->treeview ) ), TRUE, TRUE, 0 );

	gtk_widget_set_visible ( GTK_WIDGET ( entry->treeview ), TRUE );
	gtk_widget_set_visible ( GTK_WIDGET ( m_box ), TRUE );

	return m_box;
}

static void entry_icon_press ( GtkEntry *entry, GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, GcmpEntry *tool )
{
	cairo_rectangle_int_t rect;

	gtk_entry_get_icon_area ( entry, icon_pos, &rect );
	gtk_popover_set_pointing_to ( tool->popover_edit, &rect );
	gtk_popover_popup ( tool->popover_edit );

	g_object_set_data ( G_OBJECT (entry), "popover-icon-pos", GUINT_TO_POINTER (icon_pos) );
}

static void gcmp_entry_create ( GcmpEntry *entry )
{
	entry->entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( entry->entry, "" );
	gtk_entry_set_icon_from_icon_name ( entry->entry, GTK_ENTRY_ICON_SECONDARY, "edit-copy" );
	gtk_widget_set_visible ( GTK_WIDGET ( entry->entry ), TRUE );

	entry->popover_edit = (GtkPopover *)gtk_popover_new ( GTK_WIDGET ( entry->entry ) );
	gtk_popover_set_position ( entry->popover_edit, GTK_POS_BOTTOM );
	gtk_container_add ( GTK_CONTAINER (entry->popover_edit), GTK_WIDGET ( gcmp_entry_create_history ( entry ) ) );
	gtk_container_set_border_width ( GTK_CONTAINER ( entry->popover_edit ), 2 );

	g_signal_connect ( entry->entry, "icon-press", G_CALLBACK ( entry_icon_press ), entry );

	entry->entry_signal_id = g_signal_connect ( entry->entry, "changed", G_CALLBACK ( gcmp_entry_changed ), entry );
}

static void gcmp_entry_init ( GcmpEntry *entry )
{
	GtkBox *box = GTK_BOX ( entry );
	gtk_orientable_set_orientation ( GTK_ORIENTABLE ( box ), GTK_ORIENTATION_VERTICAL );

	gtk_box_set_spacing ( box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( box ), TRUE );

	gcmp_entry_create ( entry );
	gtk_box_pack_start ( box, GTK_WIDGET ( entry->entry ), TRUE, TRUE, 0 );

	g_signal_connect ( entry, "entry-clr", G_CALLBACK ( gcmp_entry_clr ), NULL );
	g_signal_connect ( entry, "entry-dec", G_CALLBACK ( gcmp_entry_dec ), NULL );
	g_signal_connect ( entry, "entry-sgn", G_CALLBACK ( gcmp_entry_sgn ), NULL );
	g_signal_connect ( entry, "entry-set-text", G_CALLBACK ( gcmp_entry_set_text ), NULL );
	g_signal_connect ( entry, "entry-get-text", G_CALLBACK ( gcmp_entry_get_text ), NULL );
}

static void gcmp_entry_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_entry_parent_class)->finalize (object);
}

static void gcmp_entry_class_init ( GcmpEntryClass *class )
{
	GObjectClass *oclass = G_OBJECT_CLASS (class);

	oclass->finalize = gcmp_entry_finalize;

	g_signal_new ( "entry-clr", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 0 );

	g_signal_new ( "entry-dec", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 0 );

	g_signal_new ( "entry-sgn", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 0 );

	g_signal_new ( "entry-set-text", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOOLEAN );

	g_signal_new ( "entry-get-text", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_STRING, 0 );
}

GcmpEntry * gcmp_entry_new ( void )
{
	return g_object_new ( GCMP_TYPE_ENTRY, NULL );
}

