/**
 * Copyright (c) 2011-2013 Fabrice THIROUX <fabrice.thiroux@free.fr> (GPL-3+).
 * Copyright (c) 2010-2011 Hong Jen Yee (aka PCMan) <pcman.tw@gmail.com>
 * (GPL-2+).
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

#include <glib.h>
#include <string.h>

#include "config.h"
#include "obsession.h"
#include "dbus-interface.h"

const gchar *provider(int id)
{
	switch (id)
	{
		case UPOWER:
			return "UPower";
		case CONSOLEKIT:
			return "Consolkit";
		case SYSTEMD:
			return "systemd";
		case GDM:
			return "GDM";
		case KDM:
			return "KDM";
		case LIGHTDM:
			return "LightDM";
		case LXDM:
			return "LXDM";
		default:
			return "Unknown";
	}
}

void get_capabilities (HandlerContext* handler_context)
{
	g_print ("Capabilities:\n");
	if (handler_context->poweroff != NONE)
	{
		g_print ("  Shutdown : %s\n", provider(handler_context->poweroff));
	}

	if (handler_context->reboot != NONE)
	{
		g_print ("  Reboot: %s\n", provider(handler_context->reboot));
	}

	if (handler_context->suspend != NONE)
	{
		g_print ("  Suspend: %s\n", provider(handler_context->suspend));
	}

	if (handler_context->hibernate != NONE)
	{
		g_print ("  Hibernate: %s\n", provider(handler_context->hibernate));
	}

	if (handler_context->switch_user != NONE)
	{
		g_print ("  User switch: %s\n", provider(handler_context->switch_user));
	}

	g_print ("Lock command: '%s'\n", handler_context->lock_cmd);
	g_print ("Logout command: '%s'\n", handler_context->logout_cmd);
}


int main(int argc, char* argv[])
{
	HandlerContext handler_context;
	GError *err = NULL;
	gboolean poweroff = FALSE;
	gboolean suspend = FALSE;
	gboolean hibernate = FALSE;
	gboolean reboot = FALSE;
	gboolean capabilities = FALSE;

	GOptionEntry opt_entries[] = {
		{ "poweroff",     'p', 0, G_OPTION_ARG_NONE, &poweroff,     "Shutdown the computer", NULL },
		{ "suspend",      's', 0, G_OPTION_ARG_NONE, &suspend,      "Suspend the computer", NULL },
		{ "hibernate",    'H', 0, G_OPTION_ARG_NONE, &hibernate,    "Go to Hibernation", NULL },
		{ "reboot",       'r', 0, G_OPTION_ARG_NONE, &reboot,       "Restart the computer", NULL },
		{ "capabilities", 'c', 0, G_OPTION_ARG_NONE, &capabilities, "List power capabilities", NULL },
		{ NULL }
	};

#if GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 36
		g_type_init ();
#endif

	initialize_context (&handler_context);

	GOptionContext * context = g_option_context_new ("");
	g_option_context_add_main_entries (context, opt_entries, PACKAGE " " PACKAGE_VERSION);
	g_option_context_set_help_enabled (context, TRUE);
	if ( !g_option_context_parse (context, &argc, &argv, NULL) ||
	    (!poweroff && !suspend && !hibernate && !reboot && !capabilities))
	{
		g_print ("%s", g_option_context_get_help (context, TRUE, NULL));
		return 1;
	}
	g_option_context_free (context);

	if (capabilities)
	{
		get_capabilities (&handler_context);
	}
	else if (hibernate)
	{
		system_hibernate (&handler_context, err);
		if (err)
			goto _error;
	}
	else 	if (poweroff)
	{
		system_poweroff (&handler_context, err);
		if (err)
			goto _error;
	}
	else if (suspend)
	{
		system_suspend (&handler_context, err);
		if (err)
			goto _error;
	}
	else if (reboot)
	{
		system_reboot (&handler_context, err);
		if (err)
			goto _error;
	}

	/* We have done with it */
	free_context (&handler_context);
	return 0;

_error:
	g_print ("error %s\n", err->message);
	g_error_free (err);
	free_context (&handler_context);
	return 1;
}
