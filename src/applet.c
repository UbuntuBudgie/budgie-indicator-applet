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

#define _GNU_SOURCE

#include "applet.h"
#include <budgie-desktop/plugin.h>
#include <gobject/gobject.h>

void load_modules(GtkWidget *menubar, gint *indicators_loaded);
void load_indicators_from_indicator_files(GtkWidget *menubar, gint *indicators_loaded);

#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"

G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorApplet, appindicator_applet, BUDGIE_TYPE_APPLET, 0, )

extern GtkCssProvider *css_provider;

/**
 * Handle cleanup
 */
static void appindicator_applet_dispose(GObject *object)
{
        G_OBJECT_CLASS(appindicator_applet_parent_class)->dispose(object);
        if (css_provider != NULL) {
                g_object_unref(css_provider);
                css_provider = NULL;
        }
}

/**
 * Class initialisation
 */
static void appindicator_applet_class_init(AppIndicatorAppletClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = appindicator_applet_dispose;
}

/**
 * We have no cleaning ourselves to do
 */
static void appindicator_applet_class_finalize(__budgie_unused__ AppIndicatorAppletClass *klazz)
{
}

/**
 * Initialisation of basic UI layout and such
 */
static void appindicator_applet_init(AppIndicatorApplet *self)
{
        GtkWidget *eventbox = NULL;
        GtkWidget *menubar = NULL;
        GtkStyleContext *context;

        gint indicators_loaded = 0;

        menubar = gtk_menu_bar_new();
        eventbox = gtk_event_box_new();
        context = gtk_widget_get_style_context(GTK_WIDGET(menubar));
        gtk_style_context_add_class(context, "budgie-panel");
        
        gtk_container_add(GTK_CONTAINER(self), eventbox);
        gtk_widget_show(eventbox);

        gtk_container_set_border_width(GTK_CONTAINER(menubar), 1);

        gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), INDICATOR_ICONS_DIR);

        load_modules(menubar, &indicators_loaded);
        /*
         * leave this here - this is the entry point for indicators such as
         * indicator-messages. Currently these indicators don't display their
         * menu contents correctly - e.g. missing thunderbird from indicator-messages
         * drop-down.
         * load_indicators_from_indicator_files (menubar, &indicators_loaded);
         */

        if (indicators_loaded == 0) {
                /* A label to allow for click through */
                GtkWidget *item = gtk_label_new("No Indicators");
                gtk_container_add(GTK_CONTAINER(eventbox), item);
                gtk_widget_show(item);
        } else {
                gtk_container_add(GTK_CONTAINER(eventbox), menubar);
                gtk_widget_show(menubar);
        }

        /* Show all of our things. */
        gtk_widget_show_all(GTK_WIDGET(self));
}

void appindicator_applet_init_gtype(GTypeModule *module)
{
        appindicator_applet_register_type(module);
}

BudgieApplet *appindicator_applet_new()
{
        // return applet_construct(APPINDICATOR_TYPE_NATIVE_APPLET, uuid);
        return g_object_new(APPINDICATOR_TYPE_NATIVE_APPLET, NULL);
}
