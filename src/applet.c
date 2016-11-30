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

#define _GNU_SOURCE

#include <budgie-desktop/plugin.h>
#include <gobject/gobject.h>
#include "applet.h"

void load_modules(GtkWidget *menubar, gint *indicators_loaded);
void
load_indicators_from_indicator_files (GtkWidget *menubar, gint *indicators_loaded);


#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"


G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorApplet, appindicator_applet, BUDGIE_TYPE_APPLET, 0, )

/**
 * Handle cleanup
 */
static void appindicator_applet_dispose(GObject *object)
{
        G_OBJECT_CLASS(appindicator_applet_parent_class)->dispose(object);
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

static void style_in_menu(GtkWidget *menuitem, gpointer user_data) {
    GtkCssProvider *css_provider = NULL;
    GtkStyleContext *context;

    /*
     * override menuitem so that the background color of the applet is the same as the panel
     */
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
                                    ".menuitem {\n"
                                    "    -GtkMenuItem-horizontal-padding: 0;\n"
                                    "    background: transparent;\n"
                                    "    border-radius: 0;\n"
                                    "    padding: 1px 1px 1px 1px;"
                                    "    text-shadow: none;}",
                                    -1,
                                    NULL);
    gtk_style_context_add_provider(GTK_STYLE_CONTEXT(
                                       gtk_widget_get_style_context(GTK_WIDGET(menuitem))),
                                   GTK_STYLE_PROVIDER(css_provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref (css_provider);
    /* for the appindicator (menuitem) we need to style it with raven otherwise
     * all submenus are transparent
    */
    context = gtk_widget_get_style_context(GTK_WIDGET(menuitem));
    gtk_style_context_add_class(context, "budgie-polkit-dialog");
    
}

static void builtin_theme_changed(gpointer user_data, gchar *key, GSettings *settings )
{
        GtkWidget *menubar = (GtkWidget *)user_data;
        GtkStyleContext *context;
        
        g_debug("zzz builtin_theme");
        g_debug("zzz builtin_theme_changed %s", key);
        
        if (g_settings_get_boolean(settings, key)) {
            context = gtk_widget_get_style_context(GTK_WIDGET(menubar));
            gtk_style_context_add_class(context, "menubar");
            g_debug("zzz adding menubar");
            
            
            gtk_container_foreach(GTK_CONTAINER(menubar), style_in_menu, NULL);
            
            g_debug("zzz set");
        }
        else {
            g_debug("zzz notset");
            
            context = gtk_widget_get_style_context(GTK_WIDGET(menubar));
            gtk_style_context_remove_class(context, "menubar");
            g_debug("zzz removing menubar");
        }
}

/**
 * Initialisation of basic UI layout and such
 */
static void appindicator_applet_init(AppIndicatorApplet *self)
{
        GtkWidget *eventbox = NULL;
        GtkWidget *menubar = NULL;
        GSettings *settings = NULL;
        
        gint indicators_loaded = 0;

        menubar = gtk_menu_bar_new();
        
        /*
         * connect to the panel schema
         */
         
        g_debug("zzz 2");
        settings = g_settings_new("com.solus-project.budgie-panel");
        g_signal_connect_swapped(settings, "changed::builtin-theme", builtin_theme_changed, menubar);
        
        eventbox = gtk_event_box_new();
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
        
        //builtin_theme_changed(menubar, "builtin-theme", settings);
}

void appindicator_applet_init_gtype(GTypeModule *module)
{
        appindicator_applet_register_type(module);
}

//#define BUDGIE_APPLET(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUDGIE_APPLET, BudgieApplet))

//struct _BudgieApplet {
//	BudgieApplet parent_instance;
//	GSettings* settings;
//};

//typedef struct _BudgieApplet BudgieApplet;


BudgieApplet* applet_construct (GType object_type, gchar* uuid) {
    BudgieApplet * self = NULL;
    
    self = (BudgieApplet*) g_object_new (object_type, "uuid", uuid, NULL);
    //budgie_applet_set_settings_schema ((AppIndicatorApplet*) self, "com.solus-project.budgie-panel.panel");
    //budgie_applet_set_settings_prefix ((AppIndicatorApplet*) self, "/com/solus-project/budgie-panel/instance/panel");
    budgie_applet_get_applet_settings (self, uuid);
    //g_signal_connect_object (self->settings, "changed", 
    //    (GCallback) builtin_theme_changed, self, 0);
        
    g_debug("zzz in construct");
    return self;
}

BudgieApplet *appindicator_applet_new(const gchar* uuid)
{
        return applet_construct(APPINDICATOR_TYPE_NATIVE_APPLET, uuid);
        //return g_object_new(APPINDICATOR_TYPE_NATIVE_APPLET, NULL);
}
