/**
 * Copyright (c) 2011-2013 Fabrice THIROUX <fabrice.thiroux@free.fr> (GPL-3+).
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
#ifndef OBSESSION_H
#define OBSESSION_H

#include <gtk/gtk.h>

enum {
	NONE = 0,
	UPOWER,
	CONSOLEKIT,
	SYSTEMD,
	GDM,
	KDM,
	LIGHTDM,
	LXDM
};

enum {
	POWEROFF_ERROR,
	REBOOT_ERROR,
	HIBERNATE_ERROR,
	SUSPEND_ERROR,
	SWITCH_USER_ERROR
};


typedef struct {
	GtkWidget *error_label;		/* Text of an error, if we get one */
	int poweroff;
	int reboot;
	int hibernate;
	int suspend;
	int switch_user;
	char *logout_cmd;
	char *lock_cmd;
} HandlerContext;

void initialize_context (HandlerContext *);
void free_context (HandlerContext *);
void load_config (HandlerContext *);
gboolean lock_screen(gchar *);
gboolean verify_running(const char *, const char *);

void system_suspend (HandlerContext *, GError *);
void system_hibernate (HandlerContext *, GError *);
void system_reboot (HandlerContext *, GError *);
void system_poweroff (HandlerContext *, GError *);
void system_user_switch (HandlerContext *);

#endif /* !OBSESSION_H */
