/**
 * Copyright (c) 2011-2012 Fabrice THIROUX <fabrice.thiroux@free.fr>.
 * Copyright (c) 2010 LxDE Developers, see AUTHORS file.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or any
 * later version. See http://www.gnu.org/copyleft/lgpl.html the full text
 * of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <config.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>

#include "dbus-interface.h"

enum {
	NONE = 0,
	UPOWER = 1,
	CONSOLEKIT = 2,
	SYSTEMD = 4,
};

typedef struct {
	int shutdown;
	int reboot;
	int hibernate;
	int suspend;
	int switch_user;
} HandlerContext;

static gboolean shutdown;
static gboolean suspend;
static gboolean hibernate;
static gboolean reboot;
static gboolean capabilities;

static GOptionEntry opt_entries[] =
{
    { "shutdown", 's', 0, G_OPTION_ARG_NONE, &shutdown, "Shutdown the computer", NULL },
    { "suspend", 'S', 0, G_OPTION_ARG_NONE, &suspend, "Suspend the computer", NULL },
    { "hibernate", 'H', 0, G_OPTION_ARG_NONE, &hibernate, "Go to Hibernation", NULL },
    { "reboot", 'r', 0, G_OPTION_ARG_NONE, &reboot, "Restart the computer", NULL },
    { "capabilities", 'c', 0, G_OPTION_ARG_NONE, &capabilities, "List power capabilities", NULL },
    { NULL }
};

void initialize_context (HandlerContext* handler_context)
{
	memset (handler_context, 0, sizeof(handler_context));

	if (dbus_systemd_CanPowerOff ())
	{
		handler_context->shutdown = SYSTEMD;
	}
	else if (dbus_ConsoleKit_CanStop ())
	{
		handler_context->shutdown = CONSOLEKIT;
	}
	else
		handler_context->shutdown = NONE;


	if (dbus_systemd_CanReboot ())
	{
		handler_context->reboot = SYSTEMD;
	}
	else if (dbus_ConsoleKit_CanRestart ())
	{
		handler_context->reboot = CONSOLEKIT;
	}
	else
	{
		handler_context->reboot = NONE;
	}

	if (dbus_systemd_CanSuspend ())
	{
		handler_context->suspend = SYSTEMD;
	}
	else if (dbus_UPower_CanSuspend ())
	{
		handler_context->suspend = UPOWER;
	}
	else
	{
		handler_context->suspend = NONE;
	}


	if (dbus_systemd_CanHibernate ())
	{
		handler_context->hibernate = SYSTEMD;
	}
	else if (dbus_UPower_CanHibernate ())
	{
		handler_context->hibernate = UPOWER;
	}
	else
	{
		handler_context->hibernate = NONE;
	}
}

void get_capabilities (HandlerContext* handler_context)
{
	g_print ("Capabilities:\n");
	if (handler_context->shutdown != NONE)
	{
		g_print ("  Shutdown\n");
	}

	if (handler_context->reboot != NONE)
	{
		g_print ("  Reboot\n");
	}

	if (handler_context->suspend != NONE)
	{
		g_print ("  Suspend\n");

	}

	if (handler_context->hibernate != NONE)
	{
		g_print ("  Hibernate\n");

	}
}

/* Try to run xlock command in order to lock the screen, return TRUE on
 * success, FALSE if command execution failed
 */
static gboolean lock_screen(void)
{
	if (!g_spawn_command_line_async("xlock -mode blank", NULL))
	{
		return TRUE;
	}
	return FALSE;
}

int main(int argc, char* argv[])
{
	HandlerContext handler_context;
	GError *err = NULL;

	g_type_init ();
	initialize_context (&handler_context);

	GOptionContext * context = g_option_context_new ("");
	g_option_context_add_main_entries (context, opt_entries, PACKAGE " " PACKAGE_VERSION);
	g_option_context_set_help_enabled (context, TRUE);
	if ( !g_option_context_parse (context, &argc, &argv, NULL) ||
	    (!shutdown && !suspend && !hibernate && !reboot && !capabilities))
	{
		g_print ("%s", g_option_context_get_help (context, TRUE, NULL));
		return 1;
	}
	g_option_context_free (context);

	if (capabilities)
	{
		get_capabilities (&handler_context);
		return 0;
	}

	if (hibernate)
	{
		lock_screen ();
		if (handler_context.hibernate == SYSTEMD)
			dbus_systemd_Hibernate (&err);
		else if (handler_context.hibernate == UPOWER)
			dbus_UPower_Hibernate (&err);

		if (err) {
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (shutdown)
	{
		if (handler_context.shutdown == SYSTEMD)
			dbus_systemd_PowerOff (&err);
		else if (handler_context.shutdown == CONSOLEKIT)
			dbus_ConsoleKit_Stop (&err);

		if (err) {
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (suspend)
	{
		lock_screen ();
		if (handler_context.suspend == SYSTEMD)
			dbus_systemd_Suspend (&err);
		else if (handler_context.suspend == UPOWER)
			dbus_UPower_Suspend (&err);

		if (err) {
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (reboot)
	{
		if (handler_context.reboot == SYSTEMD)
			dbus_systemd_Reboot (&err);
		else if (handler_context.reboot == CONSOLEKIT)
			dbus_ConsoleKit_Restart (&err);

		if (err) {
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	return 1;
}
