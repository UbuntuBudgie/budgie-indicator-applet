/**
 * Copyright (C) 2016-2018 David Mohammed <fossfreedom@ubuntu.com>
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

#include <config.h>

#include "applet.h"
#include <assert.h>
#include <budgie-desktop/plugin.h>
#include <gobject/gobject.h>

#if HAVE_UBUNTU_INDICATOR && HAVE_UBUNTU_INDICATOR_NG
#include <libido/libido.h>
#endif

#if HAVE_AYATANA_INDICATOR && HAVE_AYATANA_INDICATOR_NG
#include <libayatana-ido/libayatana-ido.h>
#endif

void load_modules(AppIndicatorApplet *applet, GtkWidget *menubar, gint *indicators_loaded);
void load_indicators_from_indicator_files(AppIndicatorApplet *applet, GtkWidget *menubar, gint *indicators_loaded);
void update_panel_size(AppIndicatorApplet *applet, int panel_size, int icon_size, int small_icon_size);

#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"

extern GtkPackDirection packdirection;
extern BudgiePanelPosition orient;

static gboolean swap_orient_cb(GtkWidget *item, gpointer data)
{
        GtkWidget *from = (GtkWidget *)data;
        GtkWidget *to = (GtkWidget *)g_object_get_data(G_OBJECT(from), "to");
        g_object_ref(G_OBJECT(item));
        gtk_container_remove(GTK_CONTAINER(from), item);
        if (GTK_IS_LABEL(item)) {
                switch (packdirection) {
                case GTK_PACK_DIRECTION_LTR:
                        gtk_label_set_angle(GTK_LABEL(item), 0.0);
                        break;
                case GTK_PACK_DIRECTION_TTB:
                        gtk_label_set_angle(GTK_LABEL(item),
                                            (orient == BUDGIE_PANEL_POSITION_LEFT) ? 270.0 : 90.0);
                        break;
                default:
                        break;
                }
        }
        gtk_box_pack_start(GTK_BOX(to), item, FALSE, FALSE, 0);
        return TRUE;
}

static gboolean reorient_box_cb(GtkWidget *menuitem, gpointer data)
{
        GtkWidget *from = g_object_get_data(G_OBJECT(menuitem), "box");
        GtkWidget *to = (packdirection == GTK_PACK_DIRECTION_LTR)
                            ? gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0)
                            : gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        g_object_set_data(G_OBJECT(from), "to", to);
        gtk_container_foreach(GTK_CONTAINER(from), (GtkCallback)swap_orient_cb, from);
        gtk_container_remove(GTK_CONTAINER(menuitem), from);
        gtk_container_add(GTK_CONTAINER(menuitem), to);
        g_object_set_data(G_OBJECT(menuitem), "box", to);
        gtk_widget_show_all(menuitem);
        return TRUE;
}

static void native_applet_real_panel_position_changed(BudgieApplet *base,
                                                      BudgiePanelPosition position)
{
        AppIndicatorApplet *self;
        self = (AppIndicatorApplet *)base;

        GtkWidget *menubar = self->menubar;

        switch (position) {
        case BUDGIE_PANEL_POSITION_NONE:
                g_debug("zzz changed none");

                break;
        case BUDGIE_PANEL_POSITION_LEFT:
        case BUDGIE_PANEL_POSITION_RIGHT:
                orient = position;
                packdirection = GTK_PACK_DIRECTION_TTB;
                g_debug("zzz changed left/right");

                break;
        default:
                g_debug("zzz changed horizontal");
                orient = position;
                packdirection = GTK_PACK_DIRECTION_LTR;
        }

        if (orient != BUDGIE_PANEL_POSITION_NONE) {
                gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar), packdirection);

                gtk_container_foreach(GTK_CONTAINER(menubar), (GtkCallback)reorient_box_cb, NULL);
        }
}

static void native_applet_real_panel_size_changed(BudgieApplet *base,
                                                      int panel_size,
                                                      int icon_size,
                                                      int small_icon_size)
{
        AppIndicatorApplet *self;
        self = (AppIndicatorApplet *)base;
        update_panel_size(self, panel_size, icon_size, small_icon_size);
}

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

        ((BudgieAppletClass *)klazz)->panel_position_changed =
            (void (*)(BudgieApplet *,
                      BudgiePanelPosition))native_applet_real_panel_position_changed;
        ((BudgieAppletClass *)klazz)->panel_size_changed =
            (void (*)(BudgieApplet *,
                      int, int, int))native_applet_real_panel_size_changed;
}

/**
 * We have no cleaning ourselves to do
 */
static void appindicator_applet_class_finalize(__budgie_unused__ AppIndicatorAppletClass *klazz)
{
}

static GtkWidget *eventbox = NULL;
static GtkWidget *menubar = NULL;

static gboolean delay_load_indicators(AppIndicatorApplet *applet)
{
        gint indicators_loaded = 0;

        gtk_menu_bar_set_pack_direction(GTK_MENU_BAR(menubar), packdirection);
        load_modules(applet, menubar, &indicators_loaded);

        if (indicators_loaded == 0) {
                /* A label to allow for click through */
                GtkWidget *item = gtk_label_new("No Indicators");
                gtk_container_add(GTK_CONTAINER(eventbox), item);
                gtk_widget_show(item);
        } else {
                gtk_container_add(GTK_CONTAINER(eventbox), menubar);
                gtk_widget_show(menubar);
        }
        return FALSE;
}

/**
* We only support left click events; prevent confusion for right
* click then left click fandango
**/
static gboolean
menubar_press (GtkWidget * widget,
                    GdkEventButton *event,
                    gpointer data G_GNUC_UNUSED)
{
	if (event->button != 1) {
		g_signal_stop_emission_by_name(widget, "button-press-event");
	}

	return FALSE;
}

/**
 * Initialisation of basic UI layout and such
 */
static void appindicator_applet_init(AppIndicatorApplet *self)
{
        GtkCssProvider *css_provider = NULL;

#if HAVE_AYATANA_INDICATOR_NG || HAVE_UBUNTU_INDICATOR_NG
        ido_init();
#endif

        gint indicators_loaded = 0;

        menubar = gtk_menu_bar_new();
        self->menubar = menubar;

        g_signal_connect(menubar, "button-press-event", G_CALLBACK(menubar_press), NULL);

        css_provider = gtk_css_provider_new();
#if GTK_CHECK_VERSION(3, 20, 0)
        gtk_css_provider_load_from_data(css_provider,
                                        "menubar { \n"
                                        "    background: transparent; } \n"
                                        ".budgie-menubar { \n"
                                        "    padding-left: 2px; \n"
                                        "    padding-right: 2px; \n"
                                        "} \n",
                                        -1,
                                        NULL);
#else
        gtk_css_provider_load_from_data(css_provider,
                                        ".menuitem { \n"
                                        "    background: transparent; } \n"
                                        ".budgie-menubar { \n"
                                        "    padding-left: 2px; \n"
                                        "    padding-right: 2px; \n"
                                        "} \n",
                                        -1,
                                        NULL);
#endif
        gtk_style_context_add_provider(GTK_STYLE_CONTEXT(
                                           gtk_widget_get_style_context(GTK_WIDGET(menubar))),
                                       GTK_STYLE_PROVIDER(css_provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        eventbox = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(self), eventbox);
        gtk_widget_show(eventbox);

        gtk_container_set_border_width(GTK_CONTAINER(menubar), 1);

        gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), INDICATOR_ICONS_DIR);

#if HAVE_AYATANA_INDICATOR_NG || HAVE_UBUNTU_INDICATOR_NG
        load_indicators_from_indicator_files (self, menubar, &indicators_loaded);
#endif

        /* Show all of our things. */
        gtk_widget_show_all(GTK_WIDGET(self));

        g_timeout_add_seconds(1, delay_load_indicators, self);
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
