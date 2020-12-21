/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-app.h"
#include "gcmp-win.h"

struct _GcmpApp
{
	GtkApplication  parent_instance;
};

G_DEFINE_TYPE ( GcmpApp, gcmp_app, GTK_TYPE_APPLICATION )

static void gcmp_new_win ( GApplication *app )
{
	G_GNUC_UNUSED GcmpWin *win = gcmp_win_new ( GCMP_APP ( app ) );
}

static void gcmp_app_activate ( GApplication *app )
{
	gcmp_new_win ( app );
}

static void gcmp_app_init ( G_GNUC_UNUSED GcmpApp *gcmp_app )
{
	
}

static void gcmp_app_finalize ( GObject *object )
{
	G_OBJECT_CLASS (gcmp_app_parent_class)->finalize (object);
}

static void gcmp_app_class_init ( GcmpAppClass *class )
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	G_APPLICATION_CLASS (class)->activate = gcmp_app_activate;

	object_class->finalize = gcmp_app_finalize;
}

GcmpApp * gcmp_app_new ( void )
{
	return g_object_new ( GCMP_TYPE_APP, "application-id", "org.gtk.gcmp", NULL );
}
