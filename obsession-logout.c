#include <locale.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gi18n.h>
#include "config.h"
#include "dbus-interface.h"
#include "obsession.h"

static char *prompt = NULL;
static char *banner_side = NULL;
static char *banner_path = NULL;

static GOptionEntry opt_entries[] = {
    {"prompt", 'p', 0, G_OPTION_ARG_STRING, &prompt, N_("Custom message to show on the dialog"), N_("message")},
    {"banner", 'b', 0, G_OPTION_ARG_STRING, &banner_path, N_("Banner to show on the dialog"), N_("image file")},
    {"side", 's', 0, G_OPTION_ARG_STRING, &banner_side, N_("Position of the banner"), "top|left|right|bottom"},
    {NULL}
};

static void logout_clicked(GtkButton *button, HandlerContext *handler_context);
static void shutdown_clicked(GtkButton *button, HandlerContext *handler_context);
static void reboot_clicked(GtkButton *button, HandlerContext *handler_context);
static void suspend_clicked(GtkButton *button, HandlerContext *handler_context);
static void hibernate_clicked(GtkButton *button, HandlerContext *handler_context);
static void switch_user_clicked(GtkButton *button, HandlerContext *handler_context);
static void cancel_clicked(GtkButton *button, gpointer user_data);
static GtkPositionType get_banner_position(void);
static void draw_banner(GtkDrawingArea *area, cairo_t *cr, int width, int height, GdkPixbuf *pixbuf);

static void logout_clicked(GtkButton *button, HandlerContext *handler_context) {
    g_spawn_command_line_async(handler_context->logout_cmd, NULL);
    gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
}

static void shutdown_clicked(GtkButton *button, HandlerContext *handler_context) {
    GError *err = NULL;
    gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
    system_poweroff(handler_context, err);
    if (err) {
        gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
        g_error_free(err);
    } else {
        gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
    }
}

static void reboot_clicked(GtkButton *button, HandlerContext *handler_context) {
    GError *err = NULL;
    gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
    system_reboot(handler_context, err);
    if (err) {
        gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
        g_error_free(err);
    } else {
        gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
    }
}

static void suspend_clicked(GtkButton *button, HandlerContext *handler_context) {
    GError *err = NULL;
    gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
    system_suspend(handler_context, err);
    if (err) {
        gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
        g_error_free(err);
    } else {
        gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
    }
}

static void hibernate_clicked(GtkButton *button, HandlerContext *handler_context) {
    GError *err = NULL;
    gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
    system_hibernate(handler_context, err);
    if (err) {
        gtk_label_set_text(GTK_LABEL(handler_context->error_label), err->message);
        g_error_free(err);
    } else {
        gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
    }
}

static void switch_user_clicked(GtkButton *button, HandlerContext *handler_context) {
    gtk_label_set_text(GTK_LABEL(handler_context->error_label), NULL);
    system_user_switch(handler_context);
    gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
}

static void cancel_clicked(GtkButton *button, gpointer user_data) {
    gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(button)));
}

static gboolean check_escape(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer data) {
    if (keyval == GDK_KEY_Escape) {
        gtk_window_destroy(GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(data))));
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

static GtkPositionType get_banner_position(void) {
    if (banner_side != NULL) {
        if (strcmp(banner_side, "right") == 0)
            return GTK_POS_RIGHT;
        if (strcmp(banner_side, "top") == 0)
            return GTK_POS_TOP;
        if (strcmp(banner_side, "bottom") == 0)
            return GTK_POS_BOTTOM;
    }
    return GTK_POS_LEFT;
}

static void draw_banner(GtkDrawingArea *area, cairo_t *cr, int width, int height, GdkPixbuf *pixbuf) {
    if (pixbuf != NULL) {
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
    }
}

int main(int argc, char *argv[]) {
#ifdef ENABLE_NLS
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif

    GOptionContext *context = g_option_context_new("");
    g_option_context_add_main_entries(context, opt_entries, GETTEXT_PACKAGE);
    g_option_context_add_group(context, gtk_get_option_group(TRUE));
    GError *err = NULL;
    if (!g_option_context_parse(context, &argc, &argv, &err)) {
        g_print(_("Error: %s\n"), err->message);
        g_error_free(err);
        return 1;
    }
    g_option_context_free(context);

    HandlerContext handler_context;
    initialize_context(&handler_context);

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_for_display(gdk_display_get_default()), PACKAGE_DATA_DIR "/obsession/images");

    GtkWidget *window = gtk_window_new();
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_start(main_box, 12);
    gtk_widget_set_margin_end(main_box, 12);
    gtk_widget_set_margin_top(main_box, 12);
    gtk_widget_set_margin_bottom(main_box, 12);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    if (banner_path != NULL) {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(banner_path, NULL);
        GtkWidget *drawing_area = gtk_drawing_area_new();
        gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), (GtkDrawingAreaDrawFunc)draw_banner, pixbuf, NULL);
        GtkPositionType banner_position = get_banner_position();
        GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

        if (banner_position == GTK_POS_LEFT || banner_position == GTK_POS_RIGHT) {
            GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
            if (banner_position == GTK_POS_LEFT) {
                gtk_box_append(GTK_BOX(hbox), drawing_area);
                gtk_box_append(GTK_BOX(hbox), separator);
                gtk_box_append(GTK_BOX(hbox), controls);
            } else {
                gtk_box_append(GTK_BOX(hbox), controls);
                gtk_box_append(GTK_BOX(hbox), separator);
                gtk_box_append(GTK_BOX(hbox), drawing_area);
            }
            gtk_box_append(GTK_BOX(main_box), hbox);
        } else {
            if (banner_position == GTK_POS_TOP) {
                gtk_box_append(GTK_BOX(controls), drawing_area);
                gtk_box_append(GTK_BOX(controls), separator);
            }
            gtk_box_append(GTK_BOX(main_box), controls);
            if (banner_position == GTK_POS_BOTTOM) {
                gtk_box_append(GTK_BOX(controls), separator);
                gtk_box_append(GTK_BOX(controls), drawing_area);
            }
        }
    } else {
        gtk_box_append(GTK_BOX(main_box), controls);
    }

    prompt = g_strdup_printf("<b><big>Log out of %s?</big></b>", session_get_name());
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), prompt);
    gtk_box_append(GTK_BOX(controls), label);

    if (handler_context.poweroff) {
        GtkWidget *shutdown_button = gtk_button_new_with_mnemonic(_("Sh_utdown"));
        GtkWidget *image = gtk_image_new_from_icon_name("system-shutdown");
        gtk_button_set_child(GTK_BUTTON(shutdown_button), image);
        gtk_button_set_alignment(GTK_BUTTON(shutdown_button), 0.0);
        g_signal_connect(shutdown_button, "clicked", G_CALLBACK(shutdown_clicked), &handler_context);
        gtk_box_append(GTK_BOX(controls), shutdown_button);
    }

    if (handler_context.reboot) {
        GtkWidget *reboot_button = gtk_button_new_with_mnemonic(_("_Reboot"));
        GtkWidget *image = gtk_image_new_from_icon_name("system-restart");
        gtk_button_set_child(GTK_BUTTON(reboot_button), image);
        gtk_button_set_alignment(GTK_BUTTON(reboot_button), 0.0);
        g_signal_connect(reboot_button, "clicked", G_CALLBACK(reboot_clicked), &handler_context);
        gtk_box_append(GTK_BOX(controls), reboot_button);
    }

    if (handler_context.suspend) {
        GtkWidget *suspend_button = gtk_button_new_with_mnemonic(_("_Suspend"));
        GtkWidget *image = gtk_image_new_from_icon_name("system-suspend");
        gtk_button_set_child(GTK_BUTTON(suspend_button), image);
        gtk_button_set_alignment(GTK_BUTTON(suspend_button), 0.0);
        g_signal_connect(suspend_button, "clicked", G_CALLBACK(suspend_clicked), &handler_context);
        gtk_box_append(GTK_BOX(controls), suspend_button);
    }

    if (handler_context.hibernate) {
        GtkWidget *hibernate_button = gtk_button_new_with_mnemonic(_("_Hibernate"));
        GtkWidget *image = gtk_image_new_from_icon_name("system-hibernate");
        gtk_button_set_child(GTK_BUTTON(hibernate_button), image);
        gtk_button_set_alignment(GTK_BUTTON(hibernate_button), 0.0);
        g_signal_connect(hibernate_button, "clicked", G_CALLBACK(hibernate_clicked), &handler_context);
        gtk_box_append(GTK_BOX(controls), hibernate_button);
    }

    if (handler_context.switch_user) {
        GtkWidget *switch_user_button = gtk_button_new_with_mnemonic(_("S_witch User"));
        GtkWidget *image = gtk_image_new_from_icon_name("system-switch-user");
        gtk_button_set_child(GTK_BUTTON(switch_user_button), image);
        gtk_button_set_alignment(GTK_BUTTON(switch_user_button), 0.0);
        g_signal_connect(switch_user_button, "clicked", G_CALLBACK(switch_user_clicked), &handler_context);
        gtk_box_append(GTK_BOX(controls), switch_user_button);
    }

    GtkWidget *logout_button = gtk_button_new_with_mnemonic(_("_Logout"));
    GtkWidget *image = gtk_image_new_from_icon_name("system-log-out");
    gtk_button_set_child(GTK_BUTTON(logout_button), image);
    gtk_button_set_alignment(GTK_BUTTON(logout_button), 0.0);
    g_signal_connect(logout_button, "clicked", G_CALLBACK(logout_clicked), &handler_context);
    gtk_box_append(GTK_BOX(controls), logout_button);

    GtkWidget *cancel_button = gtk_button_new_with_mnemonic(_("_Cancel"));
    gtk_button_set_alignment(GTK_BUTTON(cancel_button), 0.0);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(cancel_clicked), NULL);
    gtk_box_append(GTK_BOX(controls), cancel_button);

    handler_context.error_label = gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL(handler_context.error_label), GTK_JUSTIFY_CENTER);
    gtk_box_append(GTK_BOX(controls), handler_context.error_label);

    GtkEventController *key_controller = gtk_event_controller_key_new();
    g_signal_connect(key_controller, "key-pressed", G_CALLBACK(check_escape), window);
    gtk_widget_add_controller(window, key_controller);

    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(context);
    return 0;
}
