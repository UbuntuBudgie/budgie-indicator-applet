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

//#include <budgie-desktop/plugin.h>
//#include <gobject/gobject.h>
//#include <glib-object.h>
#include <budgie-desktop/applet.h>  // for BUDGIE_APPLET, BUDGIE_TYPE_APPLET
#include "AppIndicatorApplet.h"     // for __budgie_unused__

G_BEGIN_DECLS

void load_modules(GtkWidget *menubar, gint *indicators_loaded);

#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"

/**
 * Define our type, which is a BudgieApplet extension
 */
#define APPINDICATOR_TYPE_PANEL_APPLET appindicator_panel_applet_get_type()
G_DECLARE_FINAL_TYPE(AppIndicatorPanelApplet, appindicator_panel_applet, NATIVE, PANEL_APPLET,
                     BudgieApplet)

G_END_DECLS

/**
 * Pass properties here too if you wish
 */
BudgieApplet *appindicator_panel_applet_new(void)
{
        return BUDGIE_APPLET(g_object_new(APPINDICATOR_TYPE_PANEL_APPLET, NULL));
}

/**
 * Simple instance tracking
 */
struct _AppIndicatorPanelApplet {
        BudgieApplet parent;
};

/**
 * Initialise this applet instance. For now just throw on a label :)
 */
static void appindicator_panel_applet_init(AppIndicatorPanelApplet *self)
{
        GtkWidget *eventbox = NULL;
        GtkWidget *menubar = NULL;

        gint indicators_loaded = 0;

        menubar = gtk_menu_bar_new();
        eventbox = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(self), eventbox);
        gtk_widget_show(eventbox);

        gtk_container_set_border_width(GTK_CONTAINER(menubar), 1);
    
        gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
                                          INDICATOR_ICONS_DIR);
        
        load_modules(menubar, &indicators_loaded);
        
        if (indicators_loaded == 0) {
                /* A label to allow for click through */
                GtkWidget *item = gtk_label_new(_("No Indicators"));
                gtk_container_add(GTK_CONTAINER(eventbox), item);
                gtk_widget_show(item);
        } else {
                gtk_container_add(GTK_CONTAINER(eventbox), menubar);
                gtk_widget_show(menubar);
        }

        /* Show all of our things. */
        gtk_widget_show_all(GTK_WIDGET(self));
}

/**
 * Unused in our implementation. Feel free to override class methods of
 * BudgieApplet here.
 */
static void appindicator_panel_applet_class_init(__budgie_unused__ AppIndicatorPanelAppletClass *cls)
{
}

static void appindicator_panel_applet_class_finalize(__budgie_unused__ AppIndicatorPanelAppletClass *cls)
{
}

/**
 * This is us now doing the implementation chain ups..
 */

G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorPanelApplet, appindicator_panel_applet, BUDGIE_TYPE_APPLET,
                               0, )

/**
 * Work around the register types issue.
 */
void appindicator_panel_applet_init_gtype(GTypeModule *module)
{
        appindicator_panel_applet_register_type(module);
}
