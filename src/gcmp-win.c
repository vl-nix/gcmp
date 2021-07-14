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
#include "gcmp-entry.h"

#include <locale.h>
#include <uchar.h>

/* Precision ( maximum characters ) */
#define MAX_DIGITS 1000

struct _GcmpWin
{
	GtkWindow  parent_instance;

	GcmpEntry *entry;
	GtkPopover *popover;

	GcmpTool *tool;
	GcmpToolExt *tool_ext;

	uint8_t base;
	uint8_t deg_rad;
	uint16_t digits;

	gboolean debug;
};

G_DEFINE_TYPE ( GcmpWin, gcmp_win, GTK_TYPE_WINDOW )

static void gcmp_win_about ( GcmpWin *win )
{
	GtkAboutDialog *dialog = (GtkAboutDialog *)gtk_about_dialog_new ();
	gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), GTK_WINDOW ( win ) );

	gtk_about_dialog_set_logo_icon_name ( dialog, "gnome-calculator" );
	gtk_window_set_icon_name ( GTK_WINDOW ( dialog ), "gnome-calculator" );

	const char *authors[] = { "Stepan Perun", " ", NULL };

	gtk_about_dialog_set_program_name ( dialog, "Gcmp" );
	gtk_about_dialog_set_version ( dialog, VERSION );
	gtk_about_dialog_set_license_type ( dialog, GTK_LICENSE_GPL_3_0 );
	gtk_about_dialog_set_authors ( dialog, authors );
	gtk_about_dialog_set_website ( dialog,   "https://github.com/vl-nix/gcmp" );
	gtk_about_dialog_set_copyright ( dialog, "Copyright 2021 Gcmp" );
	gtk_about_dialog_set_comments  ( dialog, "Calculator \nMulti-Precision arithmetic ( MPFR )" );

	gtk_dialog_run ( GTK_DIALOG (dialog) );
	gtk_widget_destroy ( GTK_WIDGET (dialog) );
}

static enum math gcmp_win_parse_get_op ( const uint32_t sym )
{
	enum math mtf = UNF;

	if ( sym == '+' ) mtf = ADD;
	if ( sym == '-' ) mtf = SUB;
	if ( sym == '/' ) mtf = DIV;
	if ( sym == '*' ) mtf = MUL;
	if ( sym == '%' ) mtf = PRC;
	if ( sym == '^' ) mtf = POW;
	if ( sym == 'm' ) mtf = MOD;
	if ( sym == U'√' ) mtf = RUT; // √ : 0XE2889A; '\342\210\232'; ( multi-character )

	return mtf;
}

static uint8_t gcmp_win_parse_space ( const char *str )
{
	uint8_t ret = 0, j = 0;

	for ( j = 0; j < 255; j++ ) if ( str[j] == ' ' ) ret++; else break;

	return ret;
}

static void gcmp_win_parse_sct ( const char *str, char *out, GcmpWin *win )
{
	if ( g_str_has_prefix ( str, "sin" ) ) gcmp_mpfr_all_ext ( SIN, str + 3, win->digits, 0, win->base, out, win->deg_rad );
	if ( g_str_has_prefix ( str, "cos" ) ) gcmp_mpfr_all_ext ( COS, str + 3, win->digits, 0, win->base, out, win->deg_rad );
	if ( g_str_has_prefix ( str, "tan" ) ) gcmp_mpfr_all_ext ( TAN, str + 3, win->digits, 0, win->base, out, win->deg_rad );

	g_signal_emit_by_name ( win->entry, "entry-set-text", out, FALSE );
}

static void gcmp_win_parse_log ( const char *str, char *out, GcmpWin *win )
{
	if ( g_str_has_prefix ( str, "ln"  ) ) gcmp_mpfr_all_ext ( LGN, str + 2, win->digits, 0, win->base, out, win->deg_rad );
	if ( g_str_has_prefix ( str, "log" ) ) gcmp_mpfr_all_ext ( LOG, str + 3, win->digits, 0, win->base, out, win->deg_rad );

	g_signal_emit_by_name ( win->entry, "entry-set-text", out, FALSE );
}

static void gcmp_win_parse ( const char *str, uint16_t len, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s:: string: %s ", __func__, str );

	const char *pattern = str;

	enum math mtf = UNF;
	gboolean first = FALSE;

	if ( len > MAX_DIGITS ) len = MAX_DIGITS+8;

	char base[len+1]; base[0] = '\0';
	char out_str[win->digits+1]; out_str[0] = '\0';
	char out_str_one_a[win->digits+1]; out_str_one_a[0] = '\0';
	char out_str_one_b[win->digits+1]; out_str_one_b[0] = '\0';

	uint32_t c = g_utf8_get_char ( pattern );

	uint8_t space_n = gcmp_win_parse_space ( pattern );
	if ( space_n ) pattern += space_n;

	// c = g_utf8_get_char  ( pattern );
	// if ( c == '-' ) pattern++;

	if ( g_str_has_prefix ( pattern, "sin" ) || g_str_has_prefix ( pattern, "cos" ) || g_str_has_prefix ( pattern, "tan" ) )
		{ gcmp_win_parse_sct ( pattern, out_str_one_a, win ); pattern += 3; }

	if ( g_str_has_prefix ( pattern, "ln" ) || g_str_has_prefix ( pattern, "log" ) )
		{ gcmp_win_parse_log ( pattern, out_str_one_a, win ); pattern += ( g_str_has_prefix ( pattern, "log" ) ) ? 3 : 2; }

	space_n = gcmp_win_parse_space ( pattern );
	if ( space_n ) pattern += space_n;

	c = g_utf8_get_char  ( pattern );
	if ( c == '-' ) pattern++;

	c = g_utf8_get_char  ( pattern );

	pattern = g_utf8_next_char ( pattern );

	while ( c )
	{
		if ( c == 'e' )
		{
			c = g_utf8_get_char ( pattern ); pattern = g_utf8_next_char ( pattern );

			if ( c == '+' || c == '-' ) { c = g_utf8_get_char ( pattern ); pattern = g_utf8_next_char ( pattern ); }
		}

		mtf = gcmp_win_parse_get_op ( c );

		if ( mtf != UNF )
		{
			space_n = gcmp_win_parse_space ( pattern );
			if ( space_n ) pattern += space_n;

			// c = g_utf8_get_char  ( pattern );
			// if ( c == '-' ) pattern++;

			if ( g_str_has_prefix ( pattern, "sin" ) || g_str_has_prefix ( pattern, "cos" ) || g_str_has_prefix ( pattern, "tan" ) )
				{ gcmp_win_parse_sct ( pattern, out_str_one_b, win ); pattern += 3; }

			if ( g_str_has_prefix ( pattern, "ln" ) || g_str_has_prefix ( pattern, "log" ) )
				{ gcmp_win_parse_log ( pattern, out_str_one_b, win ); pattern += ( g_str_has_prefix ( pattern, "log" ) ) ? 3 : 2; }

			if ( strlen ( out_str ) ) sprintf ( base, "%.*s", win->digits + 1, out_str );

			if ( strlen ( out_str_one_a ) || strlen ( out_str_one_b ) )
			{
				if ( strlen ( out_str_one_a ) && strlen ( out_str_one_b ) )
					gcmp_mpfr_all ( mtf, out_str_one_a, out_str_one_b, win->digits, 0, win->base, out_str );
				else if ( strlen ( out_str_one_a ) )
					gcmp_mpfr_all ( mtf, out_str_one_a, pattern, win->digits, 0, win->base, out_str );
				else
					gcmp_mpfr_all ( mtf, ( first ) ? base : str, out_str_one_b, win->digits, 0, win->base, out_str );
			}
			else
				gcmp_mpfr_all ( mtf, ( first ) ? base : str, pattern, win->digits, 0, win->base, out_str );

			if ( win->debug ) g_message ( "%s: set %s ", __func__, out_str );

			mtf = UNF;
			first = TRUE;
			out_str_one_a[0] = '\0';
			out_str_one_b[0] = '\0';

			space_n = gcmp_win_parse_space ( pattern );
			if ( space_n ) pattern += space_n;

			c = g_utf8_get_char  ( pattern );
			if ( c == '-' ) pattern++;
		}

		c = g_utf8_get_char ( pattern ); pattern = g_utf8_next_char ( pattern );
	}

	if ( strlen ( out_str ) ) g_signal_emit_by_name ( win->entry, "entry-set-text", out_str, FALSE );
}

static void gcmp_win_equal ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	g_autofree char *text = NULL;
	g_signal_emit_by_name ( win->entry, "entry-get-text", &text );

	uint16_t len = ( uint16_t )strlen ( text );

	gcmp_win_parse ( text, len, win );
}

static void gcmp_win_equal_ext ( enum math_ext mt, GcmpWin *win )
{
	char out_str[win->digits+1]; out_str[0] = '\0';

	g_autofree char *text = NULL;
	g_signal_emit_by_name ( win->entry, "entry-get-text", &text );

	gcmp_mpfr_all_ext ( mt, text, win->digits, 0, win->base, out_str, win->deg_rad );

	g_signal_emit_by_name ( win->entry, "entry-set-text", out_str, FALSE );
}

static void gcmp_win_ext ( GcmpWin *win )
{
	gboolean vis = gtk_widget_get_visible ( GTK_WIDGET ( win->tool_ext ) );

	gtk_widget_set_visible ( GTK_WIDGET ( win->tool_ext ), !vis );

	gtk_window_resize ( GTK_WINDOW ( win ), 100, 100 );
}

static void gcmp_win_control ( uint8_t num, GcmpWin *win )
{
	if ( num == BMN ) gcmp_win_ext ( win );
	if ( num == BSG ) g_signal_emit_by_name ( win->entry, "entry-sgn" );
	if ( num == BDS ) g_signal_emit_by_name ( win->entry, "entry-dec" );
	if ( num == BCL ) g_signal_emit_by_name ( win->entry, "entry-clr" );
}

static void gcmp_win_buttons_click_handler ( G_GNUC_UNUSED GcmpTool *tool, uint8_t num, const char *label, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s: num = %d | label = %s ", __func__, num, label );

	if ( num == BMN || num == BSG || num == BDS || num == BCL ) { gcmp_win_control ( num, win );  return; }

	g_signal_emit_by_name ( win->entry, "entry-set-text", label, TRUE );
}

static void gcmp_win_pi ( GcmpWin *win )
{
	gcmp_win_equal_ext ( CPI, win );
}

static void gcmp_win_grd ( GcmpWin *win )
{
	win->deg_rad = ( win->deg_rad ) ? 0 : 1;

	const char *new_label = ( win->deg_rad ) ? "deg" : "rad";

	g_signal_emit_by_name ( win->tool_ext, "toolext-set-label", BDR, new_label );
}

static void gcmp_win_pref ( GcmpWin *win )
{
	gtk_popover_popup ( win->popover );
}

static void gcmp_win_buttons_ext_click_handler ( G_GNUC_UNUSED GcmpTool *tool, uint8_t num, const char *label, GcmpWin *win )
{
	if ( win->debug ) g_message ( "%s: num = %d | label = %s ", __func__, num, label );

	enum b_ext_num bte_n[] = { BP2, BP3, BR2, BR3, B1R, B1X, BLN, BLG, BFC, BSN, BCS, BTN };
	enum math_ext  mte_n[] = { PW2, PW3, RT2, RT3, D1R, D1X, LGN, LOG, FAC, SIN, COS, TAN };

	uint8_t j = 0, elm_n = G_N_ELEMENTS ( bte_n );
	for ( j = 0; j < elm_n; j++ ) if ( num == bte_n[j] ) { gcmp_win_equal_ext ( mte_n[j], win ); return; }

	if ( num == BPI ) gcmp_win_pi   ( win );
	if ( num == BDR ) gcmp_win_grd  ( win );
	if ( num == BPF ) gcmp_win_pref ( win );

	if ( num == BPF || num == BPI || num == BDR ) return;

	if ( num == BPN ) g_signal_emit_by_name ( win->entry, "entry-set-text", "^", TRUE );
	if ( num == BRN ) g_signal_emit_by_name ( win->entry, "entry-set-text", "√", TRUE );
	if ( num == BMD ) g_signal_emit_by_name ( win->entry, "entry-set-text", "m", TRUE );

	if ( num == BPN || num == BRN || num == BMD ) return;

	if ( num == BEP || num == BEM ) g_signal_emit_by_name ( win->entry, "entry-set-text", label, TRUE );
}

static void gcmp_win_menu_about ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_widget_set_visible ( GTK_WIDGET ( win->popover ), FALSE );

	gcmp_win_about ( win );
}

static void gcmp_win_menu_dark ( G_GNUC_UNUSED GtkButton *button, GcmpWin *win )
{
	gtk_widget_set_visible ( GTK_WIDGET ( win->popover ), FALSE );

	gboolean dark = FALSE;
	g_object_get ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", &dark, NULL );
	g_object_set ( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", !dark, NULL );

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

static GtkSpinButton * gcmp_win_pref_create_spinbutton ( uint16_t val, uint16_t min, uint16_t max, uint16_t step, const char *text )
{
	GtkSpinButton *spinbutton = (GtkSpinButton *)gtk_spin_button_new_with_range ( min, max, step );
	gtk_spin_button_set_value ( spinbutton, val );

	gtk_entry_set_icon_from_icon_name ( GTK_ENTRY ( spinbutton ), GTK_ENTRY_ICON_PRIMARY, "info" );
	gtk_entry_set_icon_tooltip_text   ( GTK_ENTRY ( spinbutton ), GTK_ENTRY_ICON_PRIMARY, text );

	return spinbutton;
}

static GtkSpinButton * gcmp_win_pref_create_spin ( GcmpWin *win )
{
	GtkSpinButton *spinbutton;

	spinbutton = gcmp_win_pref_create_spinbutton ( win->digits, 1, MAX_DIGITS, 1, "Precision" );
	g_signal_connect ( spinbutton, "changed", G_CALLBACK ( gcmp_win_pref_changed_digits ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( spinbutton ), TRUE );

	return spinbutton;
}

static GtkButton * gcmp_win_pref_create_button ( const char *icon_name, void ( *f )( GtkButton *, GcmpWin * ), GcmpWin *win )
{
	GtkButton *button = (GtkButton *)gtk_button_new_from_icon_name ( icon_name, GTK_ICON_SIZE_MENU );
	gtk_widget_set_size_request ( GTK_WIDGET ( button ), 50, -1 );
	gtk_widget_set_visible ( GTK_WIDGET ( button ), TRUE );

	g_signal_connect ( button, "clicked", G_CALLBACK ( f ), win );

	return button;
}

static GtkPopover * gcmp_win_popover_pref ( GcmpWin *win )
{
	GtkPopover *popover = (GtkPopover *)gtk_popover_new ( NULL );
	gtk_popover_set_position ( popover, GTK_POS_TOP );
	gtk_container_set_border_width ( GTK_CONTAINER ( popover ), 2 );

	GtkBox *vbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 2 );
	gtk_box_set_spacing ( vbox, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( vbox ), TRUE );

	gtk_box_pack_start ( vbox, GTK_WIDGET ( gcmp_win_pref_create_spin ( win ) ), FALSE, FALSE, 0 );

	GtkBox *hbox = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( hbox, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( hbox ), TRUE );

	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "weather-clear-night", gcmp_win_menu_dark,  win ) ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "gtk-info", gcmp_win_menu_about, win ) ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( hbox, GTK_WIDGET ( gcmp_win_pref_create_button ( "gtk-quit", gcmp_win_menu_quit,  win ) ), TRUE, TRUE, 0 );

	gtk_box_pack_start ( vbox, GTK_WIDGET ( hbox ), FALSE, FALSE, 0 );

	gtk_container_add ( GTK_CONTAINER ( popover ), GTK_WIDGET ( vbox ) );

	return popover;
}

static void gcmp_win_create ( GcmpWin *win )
{
	setlocale ( LC_NUMERIC, "C" );

	GtkWindow *window = GTK_WINDOW ( win );

	gtk_window_set_title ( window, "Gcmp" );
	gtk_window_set_icon_name ( window, "gnome-calculator" );

	GtkAccelGroup *accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group ( window, accel_group );

	win->popover = gcmp_win_popover_pref ( win );

	GtkBox *m_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_VERTICAL, 0 );
	gtk_box_set_spacing ( m_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( m_box ), TRUE );

	GtkBox *h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );

	win->entry = gcmp_entry_new ();
	gtk_box_pack_start ( h_box, GTK_WIDGET ( win->entry ), TRUE, TRUE, 0 );
	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );

	win->tool = gcmp_tool_new ();
	gtk_box_pack_start ( h_box, GTK_WIDGET ( win->tool ), TRUE, TRUE, 0 );
	g_signal_connect ( win->tool, "buttons-click-num-label", G_CALLBACK ( gcmp_win_buttons_click_handler ), win );

	win->tool_ext = gcmp_tool_ext_new ();
	gtk_box_pack_start ( h_box, GTK_WIDGET ( win->tool_ext ), TRUE, TRUE, 0 );
	g_signal_connect ( win->tool_ext, "buttons-click-num-label", G_CALLBACK ( gcmp_win_buttons_ext_click_handler ), win );

	gtk_box_pack_start ( m_box, GTK_WIDGET ( h_box ), TRUE, TRUE, 0 );

	h_box = (GtkBox *)gtk_box_new ( GTK_ORIENTATION_HORIZONTAL, 0 );
	gtk_box_set_spacing ( h_box, 5 );
	gtk_widget_set_visible ( GTK_WIDGET ( h_box ), TRUE );

	GtkButton *button_equal = (GtkButton *)gtk_button_new_with_label ( "=" );
	gtk_widget_add_accelerator ( GTK_WIDGET ( button_equal ), "activate", accel_group, GDK_KEY_Return, 0, 0 );
	gtk_widget_add_accelerator ( GTK_WIDGET ( button_equal ), "activate", accel_group, GDK_KEY_KP_Enter, 0, 0 );
	g_signal_connect ( button_equal, "clicked", G_CALLBACK ( gcmp_win_equal ), win );

	gtk_widget_set_visible ( GTK_WIDGET ( button_equal ), TRUE );
	gtk_box_pack_start ( h_box, GTK_WIDGET ( button_equal ), TRUE, TRUE, 0 );

	gtk_box_pack_end ( m_box, GTK_WIDGET ( h_box ), FALSE, FALSE, 0 );

	gtk_container_set_border_width ( GTK_CONTAINER ( m_box ), 10 );
	gtk_container_add ( GTK_CONTAINER ( window ), GTK_WIDGET ( m_box ) );

	gtk_window_present ( window );

	GObject *object = NULL;
	g_signal_emit_by_name ( win->tool_ext, "toolext-get-button", &object );

	gtk_popover_set_relative_to ( win->popover, GTK_WIDGET ( object ) );

	gcmp_win_ext ( win );
}

static void gcmp_win_init ( GcmpWin *win )
{
	win->base    = 10;
	win->digits  = 24;
	win->deg_rad = 1;

	win->debug = ( g_getenv ( "GCMP_DEBUG" ) ) ? TRUE : FALSE;

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
}

GcmpWin * gcmp_win_new ( GcmpApp *app )
{
	return g_object_new ( GCMP_TYPE_WIN, "application", app, NULL );
}
