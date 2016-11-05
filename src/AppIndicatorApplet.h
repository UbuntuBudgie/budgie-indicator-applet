/**
 * Copyright (C) 2016 David Mohammed
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

//#include <budgie-desktop/plugin.h>
#include <config.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <gobject/gobject.h>
#include <gtk/gtk.h>
//#include <stdlib.h>
//#include <string.h>
#include <budgie-desktop/applet.h>  // for BudgieApplet
//#include "libindicator3-0.4/libindicator/indicator-object.h"

#define __budgie_unused__ __attribute__((unused))

G_BEGIN_DECLS

/**
 * Allow the main entry to get this plugin to register its types
 */
void appindicator_panel_applet_init_gtype(GTypeModule *module);
/**
 * Construct a new "AppIndicatorPanelApplet" instance
 */
BudgieApplet *appindicator_panel_applet_new(void);

G_END_DECLS
