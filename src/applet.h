/**
 * Copyright (C) 2016-2017 David Mohammed
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

#include <budgie-desktop/applet.h>
#include <gtk/gtk.h>

#define __budgie_unused__ __attribute__((unused))

G_BEGIN_DECLS

typedef struct _AppIndicatorApplet AppIndicatorApplet;
typedef struct _AppIndicatorAppletClass AppIndicatorAppletClass;

#define APPINDICATOR_TYPE_NATIVE_APPLET appindicator_applet_get_type()
#define APPINDICATOR_NATIVE_APPLET(o)                                                              \
        (G_TYPE_CHECK_INSTANCE_CAST((o), APPINDICATOR_TYPE_NATIVE_APPLET, AppIndicatorApplet))
#define APPINDICATOR_IS_NATIVE_APPLET(o)                                                           \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), APPINDICATOR_TYPE_NATIVE_APPLET))
#define APPINDICATOR_NATIVE_APPLET_CLASS(o)                                                        \
        (G_TYPE_CHECK_CLASS_CAST((o), APPINDICATOR_TYPE_NATIVE_APPLET, AppIndicatorAppletClass))
#define APPINDICATOR_IS_NATIVE_APPLET_CLASS(o)                                                     \
        (G_TYPE_CHECK_CLASS_TYPE((o), APPINDICATOR_TYPE_NATIVE_APPLET))
#define APPINDICATOR_NATIVE_APPLET_GET_CLASS(o)                                                    \
        (G_TYPE_INSTANCE_GET_CLASS((o), APPINDICATOR_TYPE_NATIVE_APPLET, AppIndicatorAppletClass))

struct _AppIndicatorAppletClass {
        BudgieAppletClass parent_class;
};

struct _AppIndicatorApplet {
        BudgieApplet parent;
        GSettings *settings;
        GtkWidget *menubar;
};

GType appindicator_applet_get_type(void);

/**
 * Public for the plugin to allow registration of types
 */
void appindicator_applet_init_gtype(GTypeModule *module);

/**
 * Construct a new AppIndicatorApplet
 */
BudgieApplet *appindicator_applet_new(void);

G_END_DECLS
