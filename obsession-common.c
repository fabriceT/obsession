/**
 * Copyright (c) 2011-2013 Fabrice THIROUX <fabrice.thiroux@free.fr> (GPL-3+).
 * Copyright (c) 2010-2011 Hong Jen Yee (aka PCMan) <pcman.tw@gmail.com>
 * (GPL-2+).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or any
 * later version. See http://www.gnu.org/copyleft/gpl.html the full text
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
 #include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "obsession.h"
#include "dbus-interface.h"

static GQuark OBSESSION_ERROR;

/*
 * Set up a context containing informations about how
 * poweroff, suspend, hibernate and reboot are handled
 * and which graphic login manager is currently used.
 */
void initialize_context (HandlerContext* handler_context)
{
	memset(handler_context, 0, sizeof(HandlerContext));

	OBSESSION_ERROR = g_quark_from_string ("__obsession_error__");

	/* Is poweroff controlled by systemd or ConsolKit? */
	if (dbus_ConsoleKit_CanStop())
	{
		handler_context->poweroff = CONSOLEKIT;
	}
	else if (dbus_systemd_CanPowerOff())
	{
		handler_context->poweroff = SYSTEMD;
	}
	else
		handler_context->poweroff = NONE;

	/* Is reboot controlled by systemd or ConsolKit? */
	if (dbus_systemd_CanReboot())
	{
		handler_context->reboot = SYSTEMD;
	}
	else if (dbus_ConsoleKit_CanRestart())
	{
		handler_context->reboot = CONSOLEKIT;
	}
	else
	{
		handler_context->reboot = NONE;
	}

	/* Is suspend controlled by systemd or UPower? */
	if (dbus_UPower_CanSuspend())
	{
		handler_context->suspend = UPOWER;
	}
	else if (dbus_systemd_CanSuspend())
	{
		handler_context->suspend = SYSTEMD;
	}
	else
	{
		handler_context->suspend = NONE;
	}

	/* Is hibernation controlled by systemd or UPower? */
	if (dbus_UPower_CanHibernate())
	{
		handler_context->hibernate = UPOWER;
	}
	else if (dbus_systemd_CanHibernate())
	{
		handler_context->hibernate = SYSTEMD;
	}
	else
	{
		handler_context->hibernate = NONE;
	}

	/* If we are under LXDM, its "Switch User" is available. */
	if (verify_running("lxdm", "lxdm"))
	{
		handler_context->switch_user = LXDM;
	}
	/* If we are under GDM, its "Switch User" is available. */
	else if (verify_running("gdm", "gdmflexiserver"))
	{
		handler_context->switch_user = GDM;
	}
	/* If we are under KDM, its "Switch User" is available. */
	else if (verify_running("kdm", "kdmctl"))
	{
		handler_context->switch_user = KDM;
	}
	/* If we are under LightDM, its "Switch User" is available. */
	else if (verify_running("lightdm", "dm-tool"))
	{
		handler_context->switch_user = LIGHTDM;
	}
	else
		handler_context->switch_user = NONE;

	load_config (handler_context);
}

/* Free allocated memory from handler context */
void free_context (HandlerContext* handler_context)
{
	g_free (handler_context->logout_cmd);
	g_free (handler_context->lock_cmd);
}

/* Try to run xlock command in order to lock the screen, return TRUE on
 * success, FALSE if command execution failed
 */
gboolean lock_screen(gchar *cmd)
{
	g_return_val_if_fail (cmd != NULL, FALSE);

	if (!g_spawn_command_line_async(cmd, NULL))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* Verify that a program is running and that an executable is available. */
gboolean verify_running(const char * display_manager, const char * executable)
{
	/* See if the executable we need to run is in the path. */
	gchar * full_path = g_find_program_in_path(executable);
	if (full_path != NULL)
	{
		g_free(full_path);

		/* Form the filespec of the pid file for the display manager. */
		char buffer[PATH_MAX];
		sprintf(buffer, "/run/%s.pid", display_manager);

		/* Open the pid file. */
		int fd = open(buffer, O_RDONLY);
		if (fd >= 0)
		{
			/* Pid file exists.  Read it. */
			ssize_t length = read(fd, buffer, sizeof(buffer));
			close(fd);
			if (length > 0)
			{
				/* Null terminate the buffer and convert the pid. */
				buffer[length] = '\0';
				pid_t pid = atoi(buffer);
				if (pid > 0)
				{
					/* Form the filespec of the command line file under /proc.
					 * This is Linux specific.  Should be conditionalized to the appropriate /proc layout for
					 * other systems.  Your humble developer has no way to test on other systems. */
					sprintf(buffer, "/proc/%d/cmdline", pid);

					/* Open the file. */
					int fd = open(buffer, O_RDONLY);
					if (fd >= 0)
					{
						/* Read the command line. */
						ssize_t length = read(fd, buffer, sizeof(buffer));
						close(fd);
						if (length > 0)
						{
							/* Null terminate the buffer and look for the display manager name in the command.
							 * If found, return success. */
							buffer[length] = '\0';
							if (strstr(buffer, display_manager) != NULL)
								return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

void system_suspend (HandlerContext* handler_context, GError *err)
{
	switch (handler_context->suspend)
	{
		case SYSTEMD:
			lock_screen(handler_context->lock_cmd);
			dbus_systemd_Suspend (&err);
			break;

		case UPOWER:
			lock_screen(handler_context->lock_cmd);
			dbus_UPower_Suspend (&err);
			break;

		default:
			err = g_error_new (OBSESSION_ERROR, SUSPEND_ERROR, "Don't know how to suspend");
			break;
	}
}

void system_hibernate (HandlerContext* handler_context, GError *err)
{
	switch (handler_context->hibernate)
	{
		case SYSTEMD:
			lock_screen(handler_context->lock_cmd);
			dbus_systemd_Hibernate (&err);
			break;

		case UPOWER:
			lock_screen(handler_context->lock_cmd);
			dbus_UPower_Hibernate (&err);
			break;

		default:
			err = g_error_new (OBSESSION_ERROR, HIBERNATE_ERROR, "Don't know how to hibernate");
			break;
	}
}

void system_reboot (HandlerContext* handler_context, GError *err)
{
	switch (handler_context->reboot)
	{
		case SYSTEMD:
			dbus_systemd_Reboot (&err);
			break;

		case CONSOLEKIT:
			dbus_ConsoleKit_Restart (&err);
			break;

		default:
			err = g_error_new (OBSESSION_ERROR, REBOOT_ERROR, "Don't know how to reboot");
			break;
	}
}

void system_poweroff (HandlerContext* handler_context, GError *err)
{
	switch (handler_context->poweroff)
	{
		case SYSTEMD:
			dbus_systemd_PowerOff (&err);
			break;

		case CONSOLEKIT:
			dbus_ConsoleKit_Stop (&err);
			break;

		default:
			err = g_error_new (OBSESSION_ERROR, POWEROFF_ERROR, "Don't know how to shutdown");
			break;
	}
}

void system_user_switch (HandlerContext* handler_context)
{
	switch (handler_context->switch_user)
	{
		case LXDM:
			g_spawn_command_line_sync("lxdm -c USER_SWITCH", NULL, NULL, NULL, NULL);
			break;

		case GDM:
			lock_screen(handler_context->lock_cmd);
			g_spawn_command_line_sync("gdmflexiserver --startnew", NULL, NULL, NULL, NULL);
			break;

		case KDM:
			lock_screen(handler_context->lock_cmd);
			g_spawn_command_line_sync("kdmctl reserve", NULL, NULL, NULL, NULL);
			break;

		case LIGHTDM:
			lock_screen(handler_context->lock_cmd);
			g_spawn_command_line_sync("dm-tool switch-to-greeter", NULL, NULL, NULL, NULL);
			break;

		default:
			break;
	}
}


gchar *get_default_lock_cmd (void)
{
	return g_strdup ("xlock -mode blank");
}


gchar *get_default_logout_cmd (void)
{
	return g_strdup ("openbox --exit");
}


void load_config (HandlerContext* handler_context)
{
	GError *error = NULL;
	gchar *pathname = g_build_filename (g_get_user_config_dir(), "obsession.conf", NULL);

	GKeyFile *kf = g_key_file_new ();
	if (g_key_file_load_from_file (kf, pathname, G_KEY_FILE_KEEP_COMMENTS, &error))
	{
		handler_context->lock_cmd = g_key_file_get_string (kf, "Session", "screenlock", NULL);
		handler_context->logout_cmd = g_key_file_get_string (kf, "Session", "logout", NULL);

		/* Set default value if any */
		if (handler_context->lock_cmd == NULL)
			handler_context->lock_cmd = get_default_lock_cmd ();

		if (handler_context->logout_cmd == NULL)
			handler_context->logout_cmd = get_default_logout_cmd ();
	}
	else
	{
		// get default configuration.
		handler_context->lock_cmd = get_default_lock_cmd ();
		handler_context->logout_cmd = get_default_logout_cmd ();

		// The config file doesn't exist. We create it.
		if (error != NULL && error->code == G_FILE_ERROR_NOENT)
		{
			g_key_file_set_string (kf, "Session", "screenlock", handler_context->lock_cmd);
			g_key_file_set_string (kf, "Session", "logout", handler_context->logout_cmd);
			gchar *content = g_key_file_to_data (kf, NULL, NULL);
			g_file_set_contents (pathname, content, -1, NULL);
			g_free (content);
		}
	}

	/* free error if needed */
	if (error)
		g_error_free (error);

	g_key_file_free (kf);
	g_free (pathname);
}
