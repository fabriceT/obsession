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
	LIGHTDM
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
} HandlerContext;

void initialize_context (HandlerContext *);
gboolean lock_screen(void);
gboolean verify_running(const char *, const char *);

void system_suspend (HandlerContext *, GError *);
void system_hibernate (HandlerContext *, GError *);
void system_reboot (HandlerContext *, GError *);
void system_poweroff (HandlerContext *, GError *);
void system_user_switch (HandlerContext *);


#endif /* !OBSESSION_H */
