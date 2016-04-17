/**
 * Copyright (c) 2011-2013 Fabrice THIROUX <fabrice.thiroux@free.fr> (GPL-3+).
 * Copyright (c) 2010-2011 Hong Jen Yee (aka PCMan) <pcman.tw@gmail.com>
 * (GPL-2+)
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

#ifndef _DBUS_INTERFACE_H
#define _DBUS_INTERFACE_H

#include <glib.h>

/* ConsoleKit Interface */
extern gboolean dbus_ConsoleKit_CanPowerOff(void);
extern gboolean dbus_ConsoleKit_CanReboot(void);
extern gboolean dbus_ConsoleKit_CanHibernate(void);
extern gboolean dbus_ConsoleKit_CanSuspend(void);

extern void dbus_ConsoleKit_PowerOff(GError **);
extern void dbus_ConsoleKit_Reboot(GError **);
extern void dbus_ConsoleKit_Suspend(GError **);
extern void dbus_ConsoleKit_Hibernate(GError **);

/* UPower Interface */
extern gboolean dbus_UPower_CanSuspend(void);
extern gboolean dbus_UPower_CanHibernate(void);

extern gboolean dbus_UPower_Suspend(GError **);
extern gboolean dbus_UPower_Hibernate(GError **);

/* SystemD Interface */
extern gboolean dbus_systemd_CanPowerOff(void);
extern gboolean dbus_systemd_CanReboot(void);
extern gboolean dbus_systemd_CanSuspend(void);
extern gboolean dbus_systemd_CanHibernate(void);

extern void dbus_systemd_PowerOff(GError **);
extern void dbus_systemd_Reboot(GError **);
extern void dbus_systemd_Suspend(GError **);
extern void dbus_systemd_Hibernate(GError **);

#endif
