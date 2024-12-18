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


#include <locale.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "config.h"
#include "dbus-interface.h"
#include "obsession.h"

/* Command parameters. */
static char * prompt = NULL;
static char * banner_side = NULL;
static char * banner_path = NULL;

static GOptionEntry opt_entries[] =
{
	{ "prompt", 'p', 0, G_OPTION_ARG_STRING, &prompt, N_("Custom message to show on the dialog"), N_("message") },
	{ "banner", 'b', 0, G_OPTION_ARG_STRING, &banner_path, N_("Banner to show on the dialog"), N_("image file") },
	{ "side", 's', 0, G_OPTION_ARG_STRING, &banner_side, N_("Position of the banner"), "top|left|right|bottom" },
	{ NULL }
};


static void logout_clicked(GtkButton * button, HandlerContext * handler_context);
static void shutdown_clicked(GtkButton * button, HandlerContext * handler_context);
static void reboot_clicked(GtkButton * button, HandlerContext * handler_context);
static void suspend_clicked(GtkButton * button, HandlerContext * handler_context);
static void hibernate_clicked(GtkButton * button, HandlerContext * handler_context);
static void switch_user_clicked(GtkButton * button, HandlerContext * handler_context);
static void cancel_clicked(GtkButton * button, gpointer user_data);
static GtkPositionType get_banner_position(void);
gboolean expose_event(GtkWidget * widget, GdkEventExpose * event, GdkPixbuf * pixbuf);


/* Handler for "clicked" signal on Logout button. */
static void logout_clicked(GtkButton * button, HandlerContext * handler_context)
{
	/* kill(handler_context->lxsession_pid, SIGTERM); */
	g_spawn_command_line_async(handler_context->logout_cmd, NULL);
	gtk_main_quit();
}

/* Handler for "clicked" signal on Shutdown button. */
static void shutdown_clicked(GtkButton * button, HandlerContext * handler_context)
{
	GError *err = NULL;
	gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);

	system_poweroff (handler_context, err);

	if (err)
	{
		gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
		g_error_free (err);
	}
	else gtk_main_quit();
}

/* Handler for "clicked" signal on Reboot button. */
static void reboot_clicked(GtkButton * button, HandlerContext * handler_context)
{
	GError *err = NULL;
	gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);

	system_reboot (handler_context, err);

	if (err)
	{
		gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
		g_error_free (err);
	}
	else gtk_main_quit();
}

/* Handler for "clicked" signal on Suspend button. */
static void suspend_clicked(GtkButton * button, HandlerContext * handler_context)
{
	GError *err = NULL;
	gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);

	system_suspend (handler_context, err);

	if (err)
	{
		gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
		g_error_free (err);
	}
	else gtk_main_quit();
}

/* Handler for "clicked" signal on Hibernate button. */
static void hibernate_clicked(GtkButton * button, HandlerContext * handler_context)
{
	GError *err = NULL;
	gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);

	system_hibernate (handler_context, err);

	if (err)
	{
		gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
		g_error_free (err);
	}
	else gtk_main_quit();
}

/* Handler for "clicked" signal on Switch User button. */
static void switch_user_clicked(GtkButton * button, HandlerContext * handler_context)
{
	gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
	system_user_switch (handler_context);
	gtk_main_quit();
}

/* Handler for "clicked" signal on Cancel button. */
static void cancel_clicked(GtkButton * button, gpointer user_data)
{
	gtk_main_quit();
}

/* Handler for "Escape" key pressed.
 * https://stackoverflow.com/questions/17740771/how-to-program-window-to-close-with-escape-key */
static gboolean check_escape(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  if (event->keyval == GDK_KEY_Escape) {
    gtk_main_quit();
    return TRUE;
  }
  return FALSE;
}

/* Convert the --side parameter to a GtkPositionType. */
static GtkPositionType get_banner_position(void)
{
	if (banner_side != NULL)
	{
		if (strcmp(banner_side, "right") == 0)
			return GTK_POS_RIGHT;
		if (strcmp(banner_side, "top") == 0)
			return GTK_POS_TOP;
		if (strcmp(banner_side, "bottom") == 0)
			return GTK_POS_BOTTOM;
	}
	return GTK_POS_LEFT;
}



/* Handler for "expose_event" on background. */
gboolean expose_event(GtkWidget * widget, GdkEventExpose * event, GdkPixbuf * pixbuf)
{
	if (pixbuf != NULL)
	{
		/* Copy the appropriate rectangle of the root window pixmap to the drawing area.
		 * All drawing areas are immediate children of the toplevel window, so the allocation yields the source coordinates directly. */
#if GTK_CHECK_VERSION(2,14,0)
	   cairo_t * cr = gdk_cairo_create (gtk_widget_get_window(widget));
#else
	   cairo_t * cr = gdk_cairo_create (widget->window);
#endif
	   gdk_cairo_set_source_pixbuf (
		   cr,
		   pixbuf,
		   0,
		   0);

	   cairo_paint (cr);
	   cairo_destroy(cr);
	}
	return FALSE;
}

/* Main program. */
int main(int argc, char * argv[])
{
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	/* Initialize GTK (via g_option_context_parse) and parse command line arguments. */
	GOptionContext * context = g_option_context_new("");
	g_option_context_add_main_entries(context, opt_entries, GETTEXT_PACKAGE);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	GError * err = NULL;
	if ( ! g_option_context_parse(context, &argc, &argv, &err))
	{
		g_print(_("Error: %s\n"), err->message);
		g_error_free(err);
		return 1;
	}
	g_option_context_free(context);

	HandlerContext handler_context;
	initialize_context (&handler_context);



	/* Make the button images accessible. */
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), PACKAGE_DATA_DIR "/obsession/images");

	/* Create the toplevel window. */
	GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	/* Toplevel container */
	GtkWidget* alignment = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(window), alignment);

	GtkWidget* center_area = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(alignment), center_area);

	GtkWidget* center_vbox = gtk_vbox_new(FALSE, 6);
	gtk_container_set_border_width(GTK_CONTAINER(center_vbox), 12);
	gtk_container_add(GTK_CONTAINER(center_area), center_vbox);

	GtkWidget* controls = gtk_vbox_new(FALSE, 6);

	/* If specified, apply a user-specified banner image. */
	if (banner_path != NULL)
	{
		GtkWidget * banner_image = gtk_image_new_from_file(banner_path);
		GtkPositionType banner_position = get_banner_position();

		switch (banner_position)
		{
			case GTK_POS_LEFT:
			case GTK_POS_RIGHT:
				{
				/* Create a horizontal box to contain the image and the controls. */
				GtkWidget * box = gtk_hbox_new(FALSE, 2);
				gtk_box_pack_start(GTK_BOX(center_vbox), box, FALSE, FALSE, 0);

				/* Pack the image and a separator. */
				gtk_misc_set_alignment(GTK_MISC(banner_image), 0.5, 0.0);
				if (banner_position == GTK_POS_LEFT)
				{
					gtk_box_pack_start(GTK_BOX(box), banner_image, FALSE, FALSE, 2);
					gtk_box_pack_start(GTK_BOX(box), gtk_vseparator_new(), FALSE, FALSE, 2);
					gtk_box_pack_start(GTK_BOX(box), controls, FALSE, FALSE, 2);
				}
				else
				{
					gtk_box_pack_start(GTK_BOX(box), controls, FALSE, FALSE, 2);
					gtk_box_pack_end(GTK_BOX(box), gtk_vseparator_new(), FALSE, FALSE, 2);
					gtk_box_pack_end(GTK_BOX(box), banner_image, FALSE, FALSE, 2);
				}
				}
				break;

			case GTK_POS_TOP:
				gtk_box_pack_start(GTK_BOX(controls), banner_image, FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(controls), gtk_hseparator_new(), FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(center_vbox), controls, FALSE, FALSE, 0);
				break;

			case GTK_POS_BOTTOM:
				gtk_box_pack_end(GTK_BOX(controls), banner_image, FALSE, FALSE, 2);
				gtk_box_pack_end(GTK_BOX(controls), gtk_hseparator_new(), FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(center_vbox), controls, FALSE, FALSE, 0);
				break;
		}
	}
	else
		gtk_box_pack_start(GTK_BOX(center_vbox), controls, FALSE, FALSE, 0);

	/* Create the label. */
	GtkWidget * label = gtk_label_new("");

	prompt = g_strdup_printf ("<b><big>Log out of %s?</big></b>", session_get_name());

	gtk_label_set_markup(GTK_LABEL(label), prompt);
	gtk_box_pack_start(GTK_BOX(controls), label, FALSE, FALSE, 4);

	/* Create the Shutdown button. */
	if (handler_context.poweroff)
	{
		GtkWidget * shutdown_button = gtk_button_new_with_mnemonic(_("Sh_utdown"));
		GtkWidget * image = gtk_image_new_from_icon_name("system-shutdown", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(shutdown_button), image);
		gtk_button_set_alignment(GTK_BUTTON(shutdown_button), 0.0, 0.5);
		g_signal_connect(G_OBJECT(shutdown_button), "clicked", G_CALLBACK(shutdown_clicked), &handler_context);
		gtk_box_pack_start(GTK_BOX(controls), shutdown_button, FALSE, FALSE, 4);
	}

	/* Create the Reboot button. */
	if (handler_context.reboot)
	{
		GtkWidget * reboot_button = gtk_button_new_with_mnemonic(_("_Reboot"));
		GtkWidget * image = gtk_image_new_from_icon_name("system-restart", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(reboot_button), image);
		gtk_button_set_alignment(GTK_BUTTON(reboot_button), 0.0, 0.5);
		g_signal_connect(G_OBJECT(reboot_button), "clicked", G_CALLBACK(reboot_clicked), &handler_context);
		gtk_box_pack_start(GTK_BOX(controls), reboot_button, FALSE, FALSE, 4);
	}

	/* Create the Suspend button. */
	if (handler_context.suspend)
	{
		GtkWidget * suspend_button = gtk_button_new_with_mnemonic(_("_Suspend"));
		GtkWidget * image = gtk_image_new_from_icon_name("system-suspend", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(suspend_button), image);
		gtk_button_set_alignment(GTK_BUTTON(suspend_button), 0.0, 0.5);
		g_signal_connect(G_OBJECT(suspend_button), "clicked", G_CALLBACK(suspend_clicked), &handler_context);
		gtk_box_pack_start(GTK_BOX(controls), suspend_button, FALSE, FALSE, 4);
	}

	/* Create the Hibernate button. */
	if (handler_context.hibernate)
	{
		GtkWidget * hibernate_button = gtk_button_new_with_mnemonic(_("_Hibernate"));
		GtkWidget * image = gtk_image_new_from_icon_name("system-hibernate", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(hibernate_button), image);
		gtk_button_set_alignment(GTK_BUTTON(hibernate_button), 0.0, 0.5);
		g_signal_connect(G_OBJECT(hibernate_button), "clicked", G_CALLBACK(hibernate_clicked), &handler_context);
		gtk_box_pack_start(GTK_BOX(controls), hibernate_button, FALSE, FALSE, 4);
	}

	/* Create the Switch User button. */
	if (handler_context.switch_user)
	{
		GtkWidget * switch_user_button = gtk_button_new_with_mnemonic(_("S_witch User"));
		GtkWidget * image = gtk_image_new_from_icon_name("system-switch-user", GTK_ICON_SIZE_BUTTON);
		gtk_button_set_image(GTK_BUTTON(switch_user_button), image);
		gtk_button_set_alignment(GTK_BUTTON(switch_user_button), 0.0, 0.5);
		g_signal_connect(G_OBJECT(switch_user_button), "clicked", G_CALLBACK(switch_user_clicked), &handler_context);
		gtk_box_pack_start(GTK_BOX(controls), switch_user_button, FALSE, FALSE, 4);
	}

	/* Create the Logout button. */
	GtkWidget * logout_button = gtk_button_new_with_mnemonic(_("_Logout"));
	GtkWidget * image = gtk_image_new_from_icon_name("system-log-out", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(logout_button), image);
	gtk_button_set_alignment(GTK_BUTTON(logout_button), 0.0, 0.5);
	g_signal_connect(G_OBJECT(logout_button), "clicked", G_CALLBACK(logout_clicked), &handler_context);
	gtk_box_pack_start(GTK_BOX(controls), logout_button, FALSE, FALSE, 4);

	/* Create the Cancel button. */
	GtkWidget * cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	gtk_button_set_alignment(GTK_BUTTON(cancel_button), 0.0, 0.5);
	g_signal_connect(G_OBJECT(cancel_button), "clicked", G_CALLBACK(cancel_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(controls), cancel_button, FALSE, FALSE, 4);

	/* Create the error text. */
	handler_context.error_label = gtk_label_new("");
	gtk_label_set_justify(GTK_LABEL(handler_context.error_label), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(controls), handler_context.error_label, FALSE, FALSE, 4);

	g_signal_connect(window, "key_press_event", G_CALLBACK(check_escape), NULL);

	/* Show everything. */
	gtk_widget_show_all(window);

	/* Run the main event loop. */
	gtk_main();

	/* Return. */
	return 0;
}
