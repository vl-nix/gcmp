/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-win.h"
#include "gcmp-mpfr.h"
#include "gcmp-tool.h"

#include <locale.h>
#include <uchar.h>
#include <glib/gprintf.h>

enum pref 
{
	DARK,
	INFO,
	DIGITS,
	OPACITY,
	SIZE_BUTTON
};

enum cols
{
	COL_DATA,
	COL_RESL,
	NUM_COLS
};

struct _GcmpWin
{
	GtkWindow  parent_instance;

	GtkEntry *entry;
	GtkHeaderBar *header_bar;
	GtkPopover *popover;
	GtkPopover *popover_edit;
	GtkTreeView *treeview;

	GcmpTool *tool;

	uint8_t base;
	uint8_t deg_rad;
	uint16_t digits;
	uint16_t opacity;

	gboolean dark;
	uint button_size;

	gboolean debug;
};

G_DEFINE_TYPE ( GcmpWin, gcmp_win, GTK_TYPE_WINDOW )

typedef void ( *fp ) ( GcmpWin *win );

static void gcmp_win_treeview_append ( const char *data, const char *res, GcmpWin *win );

static void gcmp_win_about ( GcmpWin *win )
{
	GtkAboutDialog *dialog = (GtkAboutDialog *)gtk_about_dialog_new ();
	gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( win ) );

	gtk_about_dialog_set_logo_icon_name ( dialog, "gcmp" );
	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "gcmp" );
	gtk_widget_set_opacity ( GTK_WIDGET ( dialog ), gtk_widget_get_opacity ( GTK_WIDGET ( win ) ) );

	const char *authors[] = { "Stepan Perun", " ", NULL };

	gtk_about_dialog_set_program_name ( dialog, "Gcmp" );
	gtk_about_dialog_set_version ( dialog, VERSION );
	gtk_about_dialog_set_license_type ( dialog, GTK_LICENSE_GPL_3_0 );
	gtk_about_dialog_set_authors ( dialog, authors );
	gtk_about_dialog_set_website ( dialog,   "https://github.com/vl-nix/gcmp" );
	gtk_about_dialog_set_copyright ( dialog, "Copyright 2020 Gcmp" );
	gtk_about_dialog_set_comments  ( dialog, "Calculator \nMulti-Precision arithmetic ( MPFR )" );

	gtk_dialog_run ( GTK_DIALOG (dialog) );
	gtk_widget_destroy ( GTK_WIDGET (dialog) );
}

static void gcmp_win_ext ( const char *str, const enum math_ext mt, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s: math_ext %d | str %s \n", __func__, mt, str );

	char out_str[win->digits+1]; out_str[0] = ' '; out_str[1] = '\0';

	g_autofree char *data = g_strdup ( str );

	gcmp_mpfr_all_ext ( mt, str, win->digits, 0, win->base, out_str, win->deg_rad );

	gtk_entry_set_text ( win->entry, out_str );

	gcmp_win_treeview_append ( data, gtk_entry_get_text ( win->entry ), win );
}

static void gcmp_win_parse ( const char *str, uint16_t len, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s:: string: %s \n", __func__, str );

	enum math mtf = UNF;
	gboolean first = FALSE;

	if ( len > MAX_DIGITS ) len = MAX_DIGITS;

	char base[len+1]; base[0] = ' '; base[1] = '\0';
	char out_str[win->digits+1]; out_str[0] = ' '; out_str[1] = '\0';

	char tmp_str[len+1];
	g_sprintf ( tmp_str, "%.*s", len, str );

	char *pattern = tmp_str;

	// √ : 0XE2889A; '\342\210\232'; ( multi-character )
	const char32_t c32 = U'√';

	gunichar c = g_utf8_get_char  ( pattern );
	pattern    = g_utf8_next_char ( pattern );

	if ( c == '-' )
	{
		c       = g_utf8_get_char  ( pattern );
		pattern = g_utf8_next_char ( pattern );
	}

	do
	{
		// g_message ( "%s:: unichar: %c ( %d ) \n", __func__, c, c );
		// g_message ( "%s:: pattern: %s \n", __func__, pattern );

		if ( c == 'e' )
		{
			c       = g_utf8_get_char  ( pattern );
			pattern = g_utf8_next_char ( pattern );

			if ( c == '+' || c == '-' )
			{
				c       = g_utf8_get_char  ( pattern );
				pattern = g_utf8_next_char ( pattern );
			}
		}

		if ( c == '+' /*|| c == 0x002B*/ ) mtf = ADD;
		if ( c == '-' /*|| c == 0x002D*/ ) mtf = SUB;
		if ( c == '/' /*|| c == 0x002F*/ ) mtf = DIV;
		if ( c == '*' /*|| c == 0x002A*/ ) mtf = MUL;
		if ( c == '%' /*|| c == 0x0025*/ ) mtf = PRC;
		if ( c == '^' /*|| c == 0x005E*/ ) mtf = POW;
		if ( c == 'm' /*|| c == 0x006D*/ ) mtf = MOD;
		if ( c == c32 /*|| c == 0x221A*/ ) mtf = RUT;

		if ( mtf != UNF )
		{
			if ( first )
			{
				base[0] = ' '; base[1] = '\0';
				g_sprintf ( base, "%.*s", win->digits + 1, out_str );

				gcmp_mpfr_all ( mtf, base, pattern, win->digits, 0, win->base, out_str );
			}
			else
				{ gcmp_mpfr_all ( mtf, str, pattern, win->digits, 0, win->base, out_str ); first = TRUE; }

			gtk_entry_set_text ( win->entry, out_str );

			if ( win->debug ) g_message ( "%s: set %s \n", __func__, out_str );

			mtf = UNF;

			c       = g_utf8_get_char  ( pattern );
			pattern = g_utf8_next_char ( pattern );

			if ( c == ' ' || c == '-' )
			{
				if ( c == ' ' )
				{
					c       = g_utf8_get_char  ( pattern );
					pattern = g_utf8_next_char ( pattern );
				}

				if ( c == '-' )
				{
					c       = g_utf8_get_char  ( pattern );
					pattern = g_utf8_next_char ( pattern );
				}

				continue;
			}
		}

		c       = g_utf8_get_char  ( pattern );
		pattern = g_utf8_next_char ( pattern );

	} while (c);
}

static void gcmp_win_equal ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	const char *text = gtk_entry_get_text ( win->entry );

	g_autofree char *data = g_strdup ( text );

	gcmp_win_parse ( text, (uint16_t)strlen ( text ), win );

	gcmp_win_treeview_append ( data, gtk_entry_get_text ( win->entry ), win );
}

static void gcmp_win_set_pi_eu ( enum math mt, GcmpWin *win )
{
	g_autofree char *text_a = g_strdup ( gtk_entry_get_text ( win->entry ) );

	gcmp_win_ext ( "", mt, win );

	const char *text_b = gtk_entry_get_text ( win->entry );

	g_autofree char *text_set = g_strdup_printf ( "%s %s", text_a, text_b );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_cpi ( GcmpWin *win )
{
	gcmp_win_set_pi_eu ( CPI, win );
}

static void gcmp_win_set_ceu ( GcmpWin *win )
{
	gcmp_win_set_pi_eu ( CEU, win );
}

static void gcmp_win_set_ep ( const char *label, GcmpWin *win )
{
	if ( gtk_entry_get_text_length (win->entry) == 0 ) return;

	const char *text = gtk_entry_get_text ( win->entry );

	if ( text && ( g_str_has_suffix ( text, "e+" ) || g_str_has_suffix ( text, "e-" ) ) ) return;

	g_autofree char *text_set = g_strdup_printf ( "%s%s", text, label );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_epp ( GcmpWin *win )
{
	gcmp_win_set_ep ( "e+", win );
}

static void gcmp_win_set_epm ( GcmpWin *win )
{
	gcmp_win_set_ep ( "e-", win );
}

static void gcmp_win_set_prm ( const char *sym, GcmpWin *win )
{
	if ( gtk_entry_get_text_length (win->entry) == 0 ) return;

	const char *text = gtk_entry_get_text ( win->entry );

	if ( text && ( g_str_has_suffix ( text, " ^ " ) || g_str_has_suffix ( text, " √ " ) || g_str_has_suffix ( text, " m " ) ) ) return;

	g_autofree char *text_set = g_strdup_printf ( "%s%s", text, sym );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_pwn ( GcmpWin *win )
{
	gcmp_win_set_prm ( " ^ ", win );
}

static void gcmp_win_set_rtn ( GcmpWin *win )
{
	gcmp_win_set_prm ( " √ ", win );
}

static void gcmp_win_set_mod ( GcmpWin *win )
{
	gcmp_win_set_prm ( " m ", win );
}

static void gcmp_win_set_dec ( GcmpWin *win )
{
	if ( gtk_entry_get_text_length (win->entry) == 0 ) return;

	const char *text = gtk_entry_get_text ( win->entry );

	uint8_t num = 1;
	if ( text && ( g_str_has_suffix ( text, " " ) || g_str_has_suffix ( text, "e+" ) || g_str_has_suffix ( text, "e-" ) ) ) num = 2;

	g_autofree char *text_set = g_strndup ( text, strlen ( text ) - num );
	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_sgn ( GcmpWin *win )
{
	const char *text = gtk_entry_get_text ( win->entry );

	if ( text && ( g_str_has_suffix ( text, "-" ) ) ) { gcmp_win_set_dec ( win ); return; }

	g_autofree char *text_set = g_strdup_printf ( "%s%s", text, "-" );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_dot ( GcmpWin *win )
{
	const char *text = gtk_entry_get_text ( win->entry );

	if ( text && ( g_str_has_suffix ( text, "." ) ) ) return;

	g_autofree char *text_set = g_strdup_printf ( "%s%s", text, "." );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_clr ( GcmpWin *win )
{
	gtk_entry_set_text ( win->entry, "" ); return;
}

static void gcmp_win_set_drd ( GcmpTool *tool, GcmpWin *win )
{
	win->deg_rad = ( win->deg_rad ) ? 0 : 1;

	const char *new_label = ( win->deg_rad ) ? "deg" : "rad";

	GtkButton *button = gcmp_tool_get_button ( BDR, tool );
	gtk_button_set_label  ( button, new_label );
}


static void gcmp_win_set_min ( GcmpTool *tool, GcmpWin *win )
{
	gcmp_tool_set_mini ( tool );
	gtk_window_resize ( GTK_WINDOW ( win ), 100, 100 );
}

static void gcmp_win_button_clicked_handler_num ( GcmpTool *tool, uint num, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s: num  %d \n", __func__, num );

	enum bt_num b_n[] = { BSG, BEP, BEM, BPI, BEU, BDS, BCL, BPN, BRN, BMD, BDT };
	fp funcs[] = { gcmp_win_set_sgn, gcmp_win_set_epp, gcmp_win_set_epm, gcmp_win_set_cpi, gcmp_win_set_ceu, gcmp_win_set_dec, gcmp_win_set_clr, 
		gcmp_win_set_pwn, gcmp_win_set_rtn, gcmp_win_set_mod, gcmp_win_set_dot };

	uint8_t j = 0, set = 0, elm_n = G_N_ELEMENTS ( b_n );
	for ( j = 0; j < elm_n; j++ ) { if ( num == b_n[j] ) { funcs[j]( win ); return; } }

	if ( num == BMN ) { gcmp_win_set_min ( tool, win ); return; }
	if ( num == BDR ) { gcmp_win_set_drd ( tool, win ); return; }

	const char *text = gtk_entry_get_text ( win->entry );

	enum bt_num   bt_n[] = { BAD, BSB, BML, BDV, BPR };
	const char  *sys_n[] = { "+", "-", "*", "/", "%" };
	const char  *sym_n[] = { " + ", " - ", " * ", " / ", " % " };

	set = 0, elm_n = G_N_ELEMENTS ( sym_n );
	for ( j = 0; j < elm_n; j++ ) if ( num == bt_n[j] ) set = 1;
	if ( set ) { for ( j = 0; j < elm_n; j++ ) if ( g_str_has_suffix ( text, sym_n[j] ) || g_str_has_suffix ( text, sys_n[j] ) ) return; }

	enum bt_num   bte_n[] = { BP2, BP3, BR2, BR3, B1R, B1X, BLN, BLG, BFC, BSN, BCS, BTN };
	enum math_ext mte_n[] = { PW2, PW3, RT2, RT3, D1R, D1X, LGN, LOG, FAC, SIN, COS, TAN };

	elm_n = G_N_ELEMENTS ( bte_n );
	for ( j = 0; j < elm_n; j++ ) if ( num == bte_n[j] ) { gcmp_win_ext ( text, mte_n[j], win ); return; }

	GtkButton *button = gcmp_tool_get_button ( num, tool );
	const char *label = gtk_button_get_label ( button );

	g_autofree char *text_set = g_strdup_printf ( "%s%s", text, label );

	gtk_entry_set_text ( win->entry, text_set );
}

static void gcmp_win_set_dark ( GcmpWin *win )
{
	gboolean dark = FALSE;
	g_object_get ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", &dark, NULL );
	g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", !dark, NULL );
	win->dark = !dark;
}

static void gcmp_win_set_opacity ( GcmpWin *win )
{
	gtk_widget_set_opacity ( GTK_WIDGET ( GTK_WINDOW ( win ) ), ( (float)win->opacity / 100 ) );
}

static void gcmp_win_menu_about ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_widget_set_visible ( GTK_WIDGET ( win->popover ), FALSE );
	gcmp_win_about ( win );
}

static void gcmp_win_menu_dark ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_widget_set_visible ( GTK_WIDGET ( win->popover ), FALSE );
	gcmp_win_set_dark ( win );
}

static void gcmp_win_menu_quit ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_widget_destroy ( GTK_WIDGET ( win ) );
}

static void gcmp_win_pref_changed_digits ( GtkSpinButton *button, GcmpWin *win )
{
	gtk_spin_button_update ( button );
	win->digits = (uint16_t)gtk_spin_button_get_value_as_int ( button );
}

static void gcmp_win_pref_changed_opacity ( GtkSpinButton *button, GcmpWin *win )
{
	gtk_spin_button_update ( button );
	win->opacity = (uint16_t)gtk_spin_button_get_value_as_int ( button );

	gcmp_win_set_opacity ( win );
}

static void gcmp_win_pref_changed_butn_sz ( GtkSpinButton *button, GcmpWin *win )
{
	gtk_spin_button_update ( button );
	win->button_size = (uint16_t)gtk_spin_button_get_value_as_int ( button );

	g_object_set ( win->tool, "button-size", win->button_size, NULL );

	gtk_window_resize ( GTK_WINDOW ( win ), 100, 100 );
}

static GtkSpinButton * gcmp_win_pref_create_spinbutton ( uint16_t val, uint16_t min, uint16_t max, uint16_t step, const char *text )
{
	GtkSpinButton *spinbutton = (GtkSpinButton *)gtk_spin_button_new_with_range ( min, max, step );
	gtk_spin_button_set_value ( spinbutton, val );

	gtk_entry_set_icon_from_icon_name ( GTK_ENTRY ( spinbutton ), GTK_ENTRY_ICON_PRIMARY, "gcmp-info" );
	gtk_entry_set_icon_tooltip_text   ( GTK_ENTRY ( spinbutton ), GTK_ENTRY_ICON_PRIMARY, text );

	return spinbutton;
}

static GtkSpinButton * gcmp_win_pref_create_spin ( enum pref p, GcmpWin *win )
{
	GtkSpinButton *spinbutton;

	if ( p == DIGITS      ) spinbutton = gcmp_win_pref_create_spinbutton ( win->digits,      1,  MAX_DIGITS, 1, "Precision" );
	if ( p == OPACITY     ) spinbutton = gcmp_win_pref_create_spinbutton ( win->opacity,     40, 100,  1, "Opacity" );
	if ( p == SIZE_BUTTON ) spinbutton = gcmp_win_pref_create_spinbutton ( (uint16_t)win->button_size, 24, 1000, 1, "Button size" );

	if ( p == DIGITS      ) g_signal_connect ( spinbutton, "changed", G_CALLBACK ( gcmp_win_pref_changed_digits  ), win );
	if ( p == OPACITY     ) g_signal_connect ( spinbutton, "changed", G_CALLBACK ( gcmp_win_pref_changed_opacity ), win );
	if ( p == SIZE_BUTTON ) g_signal_connect ( spinbutton, "changed", G_CALLBACK ( gcmp_win_pref_changed_butn_sz ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( spinbutton ), TRUE );

	return spinbutton;
}

static void gcmp_win_pref_set_theme ( GtkFileChooserButton *button, GcmpWin *win )
{
	gtk_widget_set_visible ( GTK_WIDGET ( win->popover ), FALSE );

	g_autofree char *path = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( button ) );

	if ( path == NULL ) return;

	g_autofree char *name = g_path_get_basename ( path );

	g_object_set ( gtk_settings_get_default (), "gtk-theme-name", name, NULL );
}

static GtkFileChooserButton * gcmp_win_pref_create_theme ( const char *path, GcmpWin *win )
{
	GtkFileChooserButton *button = (GtkFileChooserButton *)gtk_file_chooser_button_new ( "Theme", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER );
	gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( button ), path );
	g_signal_connect ( button, "file-set", G_CALLBACK ( gcmp_win_pref_set_theme ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( button ), TRUE );

	return button;
}

static GtkButton * gcmp_win_pref_create_button ( const char *icon_name, void ( *f )( GtkButton *, GcmpWin * ), GcmpWin *win )
{
	GtkButton *button = (GtkButton *)gtk_button_new_from_icon_name ( icon_name, GTK_ICON_SIZE_MENU );
	g_signal_connect ( button, "clicked", G_CALLBACK ( f ), win );
	gtk_widget_set_visible ( GTK_WIDGET ( button ), TRUE );

	return button;
}

static GtkPopover * gcmp_win_popover_hbar ( GcmpWin *win )
{
	GtkPopover *popover = (GtkPopover *)gtk_popover_new ( NULL );
	gtk_popover_set_position ( popover, GTK_POS_TOP );
	gtk_container_set_border_width ( GTK_CONTAINER ( popover ), 2 );

	GtkBox *vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 2 );
	gtk_box_set_spacing ( vbox, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( vbox ), TRUE );

	gtk_box_pack_start ( vbox, GTK_WIDGET ( gcmp_win_pref_create_spin  ( DIGITS,      win ) ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( vbox, GTK_WIDGET ( gcmp_win_pref_create_spin  ( OPACITY,     win ) ), FALSE, FALSE, 0 );
	gtk_box_pack_start ( vbox, GTK_WIDGET ( gcmp_win_pref_create_spin  ( SIZE_BUTTON, win ) ), FALSE, FALSE, 0 );

	gtk_box_pack_start ( vbox, GTK_WIDGET ( gcmp_win_pref_create_theme ( "/usr/share/themes/", win ) ), FALSE, FALSE, 0 );

	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( hbox, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( hbox ), TRUE );

	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "gcmp-dark", gcmp_win_menu_dark,  win ) ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "gcmp-info", gcmp_win_menu_about, win ) ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "gcmp-quit", gcmp_win_menu_quit,  win ) ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( vbox, GTK_WIDGET ( hbox ), FALSE, FALSE, 0 );

	gtk_container_add ( GTK_CONTAINER ( popover ), GTK_WIDGET ( vbox ) );

	return popover;
}

static void gcm_clic_popover ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_popover_popup ( win->popover );
}

static GtkHeaderBar * gcmp_win_header_bar ( GcmpWin *win )
{
	GtkHeaderBar *header_bar = (GtkHeaderBar *)gtk_header_bar_new ();
	gtk_header_bar_set_show_close_button ( header_bar, TRUE );
	// gtk_header_bar_set_title ( header_bar, "Gcmp" );

	win->popover = gcmp_win_popover_hbar ( win );

	GtkButton *button = (GtkButton *)gtk_button_new_from_icon_name ( "gcmp-header", GTK_ICON_SIZE_MENU );
	gtk_widget_set_visible ( GTK_WIDGET ( button ), TRUE );
	g_signal_connect ( button, "clicked", G_CALLBACK ( gcm_clic_popover ), win );

	gtk_popover_set_relative_to ( win->popover, GTK_WIDGET ( button ) );

	gtk_header_bar_pack_end ( header_bar, GTK_WIDGET ( button ) );

	return header_bar;
}

static void gcmp_win_treeview_append ( const char *data, const char *res, GcmpWin *win )
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( win->treeview );

	gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
	gtk_list_store_set    ( GTK_LIST_STORE ( model ), &iter, COL_DATA, data, COL_RESL, res, -1 );
}

static void gcmp_win_treeview_create_columns ( GtkTreeView *tree_view, int column_id )
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();

	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ( "", renderer, "text", column_id, NULL );
	gtk_tree_view_append_column ( tree_view, column );
}

static void gcmp_win_treeview_add_columns ( GtkTreeView *tree_view )
{
	uint8_t c = 0; for ( c = 0; c < NUM_COLS; c++ )
		gcmp_win_treeview_create_columns ( tree_view, c );
}

static void gcmp_win_treeview_row_activated ( GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, GcmpWin *win )
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

		const char *text = gtk_entry_get_text ( win->entry );

		g_autofree char *text_set = g_strdup_printf ( "%s %s", text, data );

		gtk_entry_set_text ( win->entry, text_set );
	}
}

static GtkTreeView * gcmp_win_treeview_new ( GcmpWin *win )
{
	GtkListStore *store = (GtkListStore *)gtk_list_store_new ( 2, G_TYPE_STRING, G_TYPE_STRING );

	GtkTreeView *treeview = (GtkTreeView *)gtk_tree_view_new_with_model ( GTK_TREE_MODEL ( store ) );
	gtk_tree_view_set_headers_visible ( treeview, FALSE );

	gcmp_win_treeview_add_columns ( treeview );
	g_object_set ( treeview, "activate-on-single-click", TRUE, NULL );

	g_signal_connect ( treeview, "row-activated", G_CALLBACK ( gcmp_win_treeview_row_activated ), win );

	return treeview;
}

static GtkScrolledWindow * gcmp_win_create_scroll ( GtkTreeView *tree_view )
{
	GtkScrolledWindow *scroll = (GtkScrolledWindow *)gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_widget_set_size_request ( GTK_WIDGET ( scroll ), 200, 100 );
	gtk_container_add ( GTK_CONTAINER ( scroll ), GTK_WIDGET ( tree_view ) );

	gtk_widget_set_visible ( GTK_WIDGET ( scroll ), TRUE );

	return scroll;
}

static GtkBox * gcmp_win_create_history ( GcmpWin *win )
{
	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );

	win->treeview = gcmp_win_treeview_new ( win );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( gcmp_win_create_scroll ( win->treeview ) ), TRUE, TRUE, 0 );

	gtk_widget_set_visible ( GTK_WIDGET ( win->treeview ), TRUE );
	gtk_widget_set_visible ( GTK_WIDGET ( m_box ), TRUE );

	return m_box;
}

static void entry_icon_press ( GtkEntry *entry, GtkEntryIconPosition icon_pos, G_GNUC_UNUSED GdkEvent *event, GcmpWin *win )
{
	cairo_rectangle_int_t rect;

	gtk_entry_get_icon_area ( entry, icon_pos, &rect );
	gtk_popover_set_pointing_to ( win->popover_edit, &rect );
	gtk_popover_popup ( win->popover_edit );

	g_object_set_data ( G_OBJECT (entry), "popover-icon-pos", GUINT_TO_POINTER (icon_pos) );
}

static void gcmp_win_create_entry ( GcmpWin *win )
{
	win->entry = (GtkEntry *)gtk_entry_new ();
	gtk_entry_set_text ( win->entry, "" );
	gtk_entry_set_icon_from_icon_name ( win->entry, GTK_ENTRY_ICON_SECONDARY, "gcmp-copy" );

	win->popover_edit = (GtkPopover *)gtk_popover_new ( GTK_WIDGET ( win->entry ) );
	gtk_popover_set_position ( win->popover_edit, GTK_POS_BOTTOM );
	gtk_container_add ( GTK_CONTAINER (win->popover_edit), GTK_WIDGET ( gcmp_win_create_history ( win ) ) );
	gtk_container_set_border_width ( GTK_CONTAINER ( win->popover_edit ), 2 );

	g_signal_connect ( win->entry, "icon-press", G_CALLBACK (entry_icon_press), win );

	gtk_widget_set_visible ( GTK_WIDGET ( win->entry ), TRUE );
}

static gboolean gcmp_win_get_settings ( const char *schema_id )
{
	GSettingsSchemaSource *schemasrc = g_settings_schema_source_get_default ();
	GSettingsSchema *schema = g_settings_schema_source_lookup ( schemasrc, schema_id, FALSE );

	if ( schema == NULL ) g_critical ( "%s:: schema: %s - not installed.", __func__, schema_id );

	if ( schema == NULL ) return FALSE;

	return TRUE;
}

static void gcmp_win_load_pref ( GcmpWin *win )
{
	const char *schema_id = "org.gtk.gcmp";
	if ( !gcmp_win_get_settings ( schema_id ) ) return;

	GSettings *settings = g_settings_new ( schema_id );

	win->dark        = g_settings_get_boolean ( settings, "dark" );
	win->digits      = (uint16_t)g_settings_get_uint ( settings, "digits" );
	win->opacity     = (uint16_t)g_settings_get_uint ( settings, "opacity" );
	win->button_size = g_settings_get_uint ( settings, "button-size" );

	g_autofree char *theme = g_settings_get_string  ( settings, "theme" );

	g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", win->dark, NULL );

	if ( theme && !g_str_has_prefix ( theme, "none" ) ) g_object_set ( gtk_settings_get_default (), "gtk-theme-name", theme, NULL );

	g_object_unref ( settings );
}

static void gcmp_win_save_pref ( GcmpWin *win )
{
	const char *schema_id = "org.gtk.gcmp";
	if ( !gcmp_win_get_settings ( schema_id ) ) return;

	GSettings *settings = g_settings_new ( schema_id );

	g_settings_set_boolean ( settings, "dark",    win->dark );
	g_settings_set_uint    ( settings, "digits",  win->digits );
	g_settings_set_uint    ( settings, "opacity", win->opacity );
	g_settings_set_uint    ( settings, "button-size", win->button_size );

	g_autofree char *data = NULL;
	g_object_get ( gtk_settings_get_default (), "gtk-theme-name", &data, NULL );

	g_settings_set_string  ( settings, "theme", data );

	g_object_unref ( settings );
}

static void gcmp_window_quit ( G_GNUC_UNUSED GtkWindow *window, GcmpWin *win )
{
	gcmp_win_save_pref ( win );
}

static void gcmp_win_set_icon_window ( GtkWindow *window )
{
	GdkPixbuf *pixbuf = gtk_icon_theme_load_icon ( gtk_icon_theme_get_default (),
		"gcmp", 48, GTK_ICON_LOOKUP_USE_BUILTIN, NULL );

	if ( pixbuf )
	{
		gtk_window_set_icon ( window, pixbuf );
		g_object_unref ( pixbuf );
	}
	else
		gtk_window_set_icon_name ( window, "gnome-calculator" );
}

static void gcmp_win_create ( GcmpWin *win )
{
	GtkWindow *window = GTK_WINDOW ( win );

	setlocale ( LC_NUMERIC, "C" );
	gtk_icon_theme_add_resource_path ( gtk_icon_theme_get_default (), "/gcmp" );

	// gtk_window_set_title ( window, "Gcmp" );
	gcmp_win_set_icon_window ( window );
	g_signal_connect ( window, "destroy", G_CALLBACK ( gcmp_window_quit ), win );

	GtkAccelGroup *accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group ( window, accel_group );

	win->header_bar = gcmp_win_header_bar ( win );
	gtk_widget_set_visible ( GTK_WIDGET ( win->header_bar ), TRUE );
	gtk_window_set_titlebar ( window, GTK_WIDGET ( win->header_bar ) );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( m_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( m_box ), TRUE );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );

	gcmp_win_create_entry ( win );

	gtk_box_pack_start ( h_box, GTK_WIDGET ( win->entry ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );

	win->tool = gcmp_tool_new ( win->button_size );
	g_signal_connect ( win->tool, "button-click-num", G_CALLBACK ( gcmp_win_button_clicked_handler_num ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( win->tool ), TRUE );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( win->tool ), TRUE, TRUE, 0 );

	GtkButton *button_equal = (GtkButton *)gtk_button_new_with_label ( "=" );
	gtk_widget_add_accelerator ( GTK_WIDGET ( button_equal ), "activate", accel_group, GDK_KEY_Return, 0, 0 );
	gtk_widget_add_accelerator ( GTK_WIDGET ( button_equal ), "activate", accel_group, GDK_KEY_KP_Enter, 0, 0 );
	g_signal_connect ( button_equal, "clicked", G_CALLBACK ( gcmp_win_equal ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( button_equal ), TRUE );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_equal ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );
}

static void gcmp_win_init ( GcmpWin *win )
{
	win->digits = 24;
	win->base   = 10;
	win->deg_rad = 1;
	win->opacity = 100;
	win->button_size = BUTTON_SIZE;

	win->debug = ( g_getenv ( "GCMP_DEBUG" ) ) ? TRUE : FALSE;

	gcmp_win_load_pref ( win );

	gcmp_win_create ( win );
}

static void gcmp_win_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_win_parent_class)->finalize (object);
}

static void gcmp_win_class_init ( GcmpWinClass *class )
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	object_class->finalize = gcmp_win_finalize;

	g_signal_new ( "run-new-win", G_TYPE_FROM_CLASS ( class ), G_SIGNAL_RUN_LAST,
		0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING );
}

GcmpWin * gcmp_win_new ( GcmpApp *app )
{
	GcmpWin *win = g_object_new ( GCMP_TYPE_WIN, "application", app, NULL );

	gtk_window_present ( GTK_WINDOW ( win ) );

	gcmp_win_set_opacity ( win );
	gcmp_win_set_min ( win->tool, win );

	return win;
}
