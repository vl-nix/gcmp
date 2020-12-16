/*
* Copyright 2020 Stepan Perun
* This program is free software.
*
* License: Gnu General Public License GPL-3
* file:///usr/share/common-licenses/GPL-3
* http://www.gnu.org/licenses/gpl-3.0.html
*/

#include "gcmp-app.h"

int main ( void )
{
	GcmpApp *app = gcmp_app_new ();

	int status = g_application_run ( G_APPLICATION ( app ), 0, NULL );

	g_object_unref ( app );

	return status;
}
