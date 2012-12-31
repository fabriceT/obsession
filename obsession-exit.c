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

#include <glib.h>
#include <string.h>

#include "config.h"
#include "obsession.h"
#include "dbus-interface.h"


void get_capabilities (HandlerContext* handler_context)
{
	g_print ("Capabilities:\n");
	if (handler_context->poweroff != NONE)
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


int main(int argc, char* argv[])
{
	HandlerContext handler_context;
	GError *err = NULL;
	gboolean poweroff;
	gboolean suspend;
	gboolean hibernate;
	gboolean reboot;
	gboolean capabilities;

	GOptionEntry opt_entries[] = {
		{ "poweroff",     'p', 0, G_OPTION_ARG_NONE, &poweroff,     "Shutdown the computer", NULL },
		{ "suspend",      's', 0, G_OPTION_ARG_NONE, &suspend,      "Suspend the computer", NULL },
		{ "hibernate",    'H', 0, G_OPTION_ARG_NONE, &hibernate,    "Go to Hibernation", NULL },
		{ "reboot",       'r', 0, G_OPTION_ARG_NONE, &reboot,       "Restart the computer", NULL },
		{ "capabilities", 'c', 0, G_OPTION_ARG_NONE, &capabilities, "List power capabilities", NULL },
		{ NULL }
	};

	g_type_init ();
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
		return 0;
	}

	if (hibernate)
	{
		system_hibernate (&handler_context, err);
		if (err)
		{
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (poweroff)
	{
		system_poweroff (&handler_context, err);
		if (err)
		{
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (suspend)
	{
		system_suspend (&handler_context, err);
		if (err)
		{
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	if (reboot)
	{
		system_reboot (&handler_context, err);
		if (err)
		{
			g_print ("error %s\n", err->message);
			g_error_free (err);
			return 1;
		}
		return 0;
	}

	return 1;
}
