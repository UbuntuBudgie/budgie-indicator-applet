/*
A small wrapper utility to load indicators and put them as menu items
into the mate-panel using it's applet interface.

Copyright 2009-2010 Canonical Ltd., 2016-2018 David Mohammed

Authors:
    Ted Gould <ted@canonical.com>
    David Mohammed <fossfreedom@ubuntu.com>

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License version 3, as published
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranties of
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <budgie-desktop/plugin.h>

#if HAVE_UBUNTU_INDICATOR && HAVE_UBUNTU_INDICATOR_NG
#include <libindicator/indicator-ng.h>

#define INDICATOR_SERVICE_DIR "/usr/share/unity/indicators"
#endif

#if HAVE_AYATANA_INDICATOR && HAVE_AYATANA_INDICATOR_NG
#include <libayatana-indicator/indicator-ng.h>

#define INDICATOR_SERVICE_DIR "/usr/share/ayatana/indicators"
#endif

#if HAVE_UBUNTU_INDICATOR

#define INDICATOR_SERVICE_APPMENU      "libappmenu.so"
#define INDICATOR_SERVICE_ME           "libme.so"
#define INDICATOR_SERVICE_DATETIME     "libdatetime.so"

#define INDICATOR_SERVICE_APPMENU_NG   "com.canonical.indicator.appmenu"
#define INDICATOR_SERVICE_ME_NG        "com.canonical.indicator.me"
#define INDICATOR_SERVICE_DATETIME_NG  "com.canonical.indicator.datetime"

#include <libindicator/indicator-object.h>

static gchar *indicator_order[] = { "libapplication.so", "libmessaging.so", "libsoundmenu.so",
                                    "libdatetime.so",    "libsession.so",   NULL };
#endif

#if HAVE_AYATANA_INDICATOR

#define INDICATOR_SERVICE_APPMENU      "libayatana-appmenu.so"
#define INDICATOR_SERVICE_ME           "libayatana-me.so"
#define INDICATOR_SERVICE_DATETIME     "libayatana-datetime.so"

#define INDICATOR_SERVICE_APPMENU_NG   "org.ayatana.indicator.appmenu"
#define INDICATOR_SERVICE_ME_NG        "org.ayatana.indicator.me"
#define INDICATOR_SERVICE_DATETIME_NG  "org.ayatana.indicator.datetime"

#include <libayatana-indicator/indicator-object.h>

static gchar *indicator_order[] = { "libayatana-application.so", "libayatana-messaging.so", "libayatana-soundmenu.so",
                                    "libayatana-datetime.so",    "libayatana-session.so",   NULL };
#endif

static gchar *blacklist_applets[] = { "nm-applet", "chrome_status_icon_1", "chrome_status_icon_2",  0 };

void calc_default_icon_size(void);

BudgiePanelPosition orient = BUDGIE_PANEL_POSITION_NONE;
static int current_icon_size;
static int panel_size;
static int icon_size;
static int small_icon_size;

GtkPackDirection packdirection = (GtkPackDirection)GTK_ORIENTATION_HORIZONTAL;

#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"

void calc_default_icon_size() {
        int small_panel_size = 37;
        current_icon_size = 22; //appindicator spec size

        if (panel_size < small_panel_size ) {
                current_icon_size = current_icon_size + (panel_size - current_icon_size - 15);

                if (current_icon_size < 16) current_icon_size = 16;
        }
}

static void resize_image(GtkImage *image, __attribute__((unused)) gpointer user_data)
{
        GdkPixbuf *pixbuf;
        GdkPixbuf *scaled_pixbuf;
        int pixbuf_height;
        int pixbuf_width;
        int pixbuf_size;
        g_debug("zzz resize_image");

        pixbuf = gtk_image_get_pixbuf (image);

        if (pixbuf == NULL) return;

        pixbuf_height = gdk_pixbuf_get_height(pixbuf);
        if ( pixbuf_height !=0 && pixbuf_height != (current_icon_size-2)) {
                pixbuf_width = gdk_pixbuf_get_width(pixbuf);
                pixbuf_size = (int)((double)(current_icon_size-2) / pixbuf_height * pixbuf_width );
                if (pixbuf_width <= 0 || (current_icon_size-2) <= 0 || pixbuf_size <= 0) return;

                scaled_pixbuf = gdk_pixbuf_scale_simple( pixbuf,
                        pixbuf_size,
                        (current_icon_size-2), GDK_INTERP_HYPER
                        );

                gtk_image_set_from_pixbuf (image, scaled_pixbuf);
        }

}

static gboolean
entry_resized (__attribute__((unused)) AppIndicatorApplet *applet, __attribute__((unused)) int panel_size, __attribute__((unused)) int icon_size, __attribute__((unused)) int small_icon_size, gpointer data)
{
	IndicatorObject *io = (IndicatorObject *)data;

	calc_default_icon_size();

	/* Work on the entries */
        if (io == NULL) return FALSE;
	GList * entries = indicator_object_get_entries(io);
        if (entries == NULL) return FALSE;
	GList * entry = NULL;

	for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
		IndicatorObjectEntry * entrydata = (IndicatorObjectEntry *)entry->data;
		if (entrydata->image != NULL) {
			/* Resize to fit panel */

                        if (gtk_image_get_storage_type(entrydata->image) == GTK_IMAGE_PIXBUF) {
                                resize_image(entrydata->image, NULL);
                        }
			gtk_image_set_pixel_size (entrydata->image, current_icon_size);
		}
	}

	return FALSE;
}

static void update_accessible_desc(IndicatorObjectEntry *entry, GtkWidget *menuitem);

/*************
 * main
 * ***********/

/*************
 * log files
 * ***********/
#define LOG_FILE_NAME "indicator-applet.log"
GOutputStream *log_file = NULL;

/********************
 * Environment Names
 * *******************/
#define INDICATOR_SPECIFIC_ENV "indicator-applet-original"

static const gchar *indicator_env[] = { "indicator-applet", INDICATOR_SPECIFIC_ENV, NULL };

/*************
 * init function
 * ***********/

static gint name2order(const gchar *name)
{
        int i;

        for (i = 0; indicator_order[i] != NULL; i++) {
                if (g_strcmp0(name, indicator_order[i]) == 0) {
                        return i;
                }
        }

        return -1;
}

typedef struct _incoming_position_t incoming_position_t;
struct _incoming_position_t {
        gint objposition;
        gint entryposition;
        gint menupos;
        gboolean found;
};

/* This function helps by determining where in the menu list
   this new entry should be placed.  It compares the objects
   that they're on, and then the individual entries.  Each
   is progressively more expensive. */
static void place_in_menu(GtkWidget *widget, gpointer user_data)
{
        incoming_position_t *position = (incoming_position_t *)user_data;
        if (position->found) {
                /* We've already been placed, just finish the foreach */
                return;
        }

        IndicatorObject *io =
            INDICATOR_OBJECT(g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_OBJECT));
        g_assert(io != NULL);

        gint objposition = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
        /* We've already passed it, well, then this is where
           we should be be.  Stop! */
        if (objposition > position->objposition) {
                position->found = TRUE;
                return;
        }

        /* The objects don't match yet, keep looking */
        if (objposition < position->objposition) {
                position->menupos++;
                return;
        }

        /* The objects are the same, let's start looking at entries. */
        IndicatorObjectEntry *entry =
            (IndicatorObjectEntry *)g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);
        gint entryposition = indicator_object_get_location(io, entry);

        if (entryposition > position->entryposition) {
                position->found = TRUE;
                return;
        }

        if (entryposition < position->entryposition) {
                position->menupos++;
                return;
        }

        /* We've got the same object and the same entry.  Well,
           let's just put it right here then. */
        position->found = TRUE;
        return;
}

static void something_shown(__attribute__((unused)) GtkWidget *widget, gpointer user_data)
{
        g_debug("zzz something shown");
        GtkWidget *menuitem = GTK_WIDGET(user_data);
        gtk_widget_show(menuitem);
}

static void something_hidden(__attribute__((unused)) GtkWidget *widget, gpointer user_data)
{
        g_debug("zzz something hidden");
        GtkWidget *menuitem = GTK_WIDGET(user_data);
        gtk_widget_hide(menuitem);
}

static void sensitive_cb(GObject *obj, __attribute__((unused)) GParamSpec *pspec,
                         gpointer user_data)
{
        g_debug("zzz something made sensitive");
        g_return_if_fail(GTK_IS_WIDGET(obj));
        g_return_if_fail(GTK_IS_WIDGET(user_data));

        gtk_widget_set_sensitive(GTK_WIDGET(user_data), gtk_widget_get_sensitive(GTK_WIDGET(obj)));
        return;
}


static void entry_activated(GtkWidget *widget, gpointer user_data)
{
        g_return_if_fail(GTK_IS_WIDGET(widget));
        gpointer pio = g_object_get_data(G_OBJECT(widget), "indicator");
        g_return_if_fail(INDICATOR_IS_OBJECT(pio));
        IndicatorObject *io = INDICATOR_OBJECT(pio);

        indicator_object_entry_activate(io,
                                        (IndicatorObjectEntry *)user_data,
                                        gtk_get_current_event_time());
}

static gboolean entry_scrolled(GtkWidget *menuitem, GdkEventScroll *event,
                               __attribute__((unused)) gpointer data)
{
        IndicatorObject *io = g_object_get_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_OBJECT);
        IndicatorObjectEntry *entry =
            g_object_get_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_ENTRY);

        g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);

        g_signal_emit_by_name(io, "scroll", 1, event->direction);
        g_signal_emit_by_name(io, "scroll-entry", entry, 1, event->direction);
        g_signal_emit_by_name(io,
                              INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED,
                              entry,
                              1,
                              event->direction);

        return FALSE;
}

static gboolean
entry_pressed (GtkWidget *menuitem, GdkEvent *event, gpointer data)
{
       g_return_val_if_fail(GTK_IS_MENU_ITEM(menuitem), FALSE);

       if (((GdkEventButton*)event)->button == 2) /* middle button */
       {
               gtk_widget_grab_focus(menuitem);

               return TRUE;
       }

       return FALSE;
}

static gboolean
entry_released (GtkWidget *menuitem, GdkEvent *event, gpointer data)
{
       g_return_val_if_fail(GTK_IS_MENU_ITEM(menuitem), FALSE);

       if (((GdkEventButton*)event)->button == 2) /* middle button */
       {
               IndicatorObject *io = g_object_get_data (G_OBJECT (menuitem), MENU_DATA_INDICATOR_OBJECT);
               IndicatorObjectEntry *entry = g_object_get_data (G_OBJECT (menuitem), MENU_DATA_INDICATOR_ENTRY);

               g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);

               g_signal_emit_by_name (io, INDICATOR_OBJECT_SIGNAL_SECONDARY_ACTIVATE, entry,
                       ((GdkEventButton*)event)->time);

               return TRUE;
       }

       return FALSE;
}

static void accessible_desc_update_cb(GtkWidget *widget, gpointer userdata)
{
        gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

        if (data != userdata) {
                return;
        }

        IndicatorObjectEntry *entry = (IndicatorObjectEntry *)data;
        update_accessible_desc(entry, widget);
}

static void accessible_desc_update(__attribute__((unused)) IndicatorObject *io,
                                   IndicatorObjectEntry *entry, GtkWidget *menubar)
{
        gtk_container_foreach(GTK_CONTAINER(menubar), accessible_desc_update_cb, entry);
        return;
}

GtkCssProvider *css_provider = NULL;

static void entry_added(IndicatorObject *io, IndicatorObjectEntry *entry, GtkWidget *menubar)
{
        g_debug("zzz Signal: Entry Added");
        gboolean something_visible = FALSE;
        gboolean something_sensitive = FALSE;
        GtkStyleContext *context;
        GtkCssProvider *css_provider = NULL;

        /*
         * we don't want to have the nm-applet being displayed
         * budgie-desktop provides this
         */
        if (entry->name_hint != NULL) {
                int loop = 0;
                while (blacklist_applets[loop]) {
                        if (strstr(entry->name_hint, blacklist_applets[loop]) != NULL) {
                                return;
                        }
                        loop++;
                }
                g_debug("zzz %s", entry->name_hint);
        } else {
                g_debug("zzz no name_hint");
        }

        GtkWidget *menuitem = gtk_menu_item_new();
        GtkWidget *box =
            (orient == BUDGIE_PANEL_POSITION_TOP || orient == BUDGIE_PANEL_POSITION_BOTTOM)
                ? gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3)
                : gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

        /* Allows indicators to receive mouse scroll event in GTK+3 */
        gtk_widget_add_events(GTK_WIDGET(menuitem), GDK_SCROLL_MASK);
        gtk_widget_add_events(GTK_WIDGET(menuitem), GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(GTK_WIDGET(menuitem), GDK_BUTTON_RELEASE_MASK);

        g_object_set_data(G_OBJECT(menuitem), "indicator", io);
        g_object_set_data(G_OBJECT(menuitem), "box", box);

        g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(entry_activated), entry);
        g_signal_connect(G_OBJECT(menuitem), "scroll-event", G_CALLBACK(entry_scrolled), entry);
        g_signal_connect(G_OBJECT(menuitem), "button-press-event", G_CALLBACK(entry_pressed), entry);
	g_signal_connect(G_OBJECT(menuitem), "button-release-event", G_CALLBACK(entry_released), entry);

        if (entry->image != NULL) {
                g_debug("zzz have an image");
                if (gtk_image_get_storage_type(entry->image) == GTK_IMAGE_PIXBUF) {
                        g_debug("zzz have a pixbuf based image");
                        g_signal_connect(G_OBJECT(entry->image),
                                 "notify::pixbuf",
                                 G_CALLBACK(resize_image),
                                 NULL);
                        resize_image(entry->image, NULL);
                }
                gtk_image_set_pixel_size(entry->image, current_icon_size);
                gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->image), FALSE, FALSE, 1);
                if (gtk_widget_get_visible(GTK_WIDGET(entry->image))) {
                        g_debug("zzz and is visible");
                        something_visible = TRUE;
                }

                if (gtk_widget_get_sensitive(GTK_WIDGET(entry->image))) {
                        something_sensitive = TRUE;
                }

                g_signal_connect(G_OBJECT(entry->image),
                                 "show",
                                 G_CALLBACK(something_shown),
                                 menuitem);
                g_signal_connect(G_OBJECT(entry->image),
                                 "hide",
                                 G_CALLBACK(something_hidden),
                                 menuitem);

                g_signal_connect(G_OBJECT(entry->image),
                                 "notify::sensitive",
                                 G_CALLBACK(sensitive_cb),
                                 menuitem);
        }
        if (entry->label != NULL) {
                g_debug("zzz have a label");
                switch (orient) {
                case BUDGIE_PANEL_POSITION_LEFT:
                        gtk_label_set_angle(GTK_LABEL(entry->label), 270.0);
                        break;
                case BUDGIE_PANEL_POSITION_RIGHT:
                        gtk_label_set_angle(GTK_LABEL(entry->label), 90.0);
                        break;
                default:
                        // g_assert(1==2);
                        gtk_label_set_angle(GTK_LABEL(entry->label), 0.0);
                }
                gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(entry->label), FALSE, FALSE, 1);

                if (gtk_widget_get_visible(GTK_WIDGET(entry->label))) {
                        g_debug("zzz and is visible");
                        something_visible = TRUE;
                }

                if (gtk_widget_get_sensitive(GTK_WIDGET(entry->label))) {
                        something_sensitive = TRUE;
                }

                g_signal_connect(G_OBJECT(entry->label),
                                 "show",
                                 G_CALLBACK(something_shown),
                                 menuitem);
                g_signal_connect(G_OBJECT(entry->label),
                                 "hide",
                                 G_CALLBACK(something_hidden),
                                 menuitem);

                g_signal_connect(G_OBJECT(entry->label),
                                 "notify::sensitive",
                                 G_CALLBACK(sensitive_cb),
                                 menuitem);
        }

        /* for the appindicator (menuitem) we need to style it with budgie-menubar
         * otherwise
         * all submenus are transparent for the system theme
        */
        context = gtk_widget_get_style_context(GTK_WIDGET(menuitem));
        gtk_style_context_add_class(context, "budgie-menubar");
        context = gtk_widget_get_style_context(GTK_WIDGET(menubar));
        gtk_style_context_remove_class(context, "menubar");

        css_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(css_provider,
                                        ".budgie-menubar { \n"
                                        "    padding-left: 2px; \n"
                                        "    padding-right: 2px; \n"
                                        "} \n",
                                        -1,
                                        NULL);
        gtk_style_context_add_provider(GTK_STYLE_CONTEXT(
                                           gtk_widget_get_style_context(GTK_WIDGET(menuitem))),
                                       GTK_STYLE_PROVIDER(css_provider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        g_debug("zzz adding budgie-menubar");

        gtk_container_add(GTK_CONTAINER(menuitem), box);
        gtk_widget_show(box);

        if (entry->menu != NULL) {
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));
        }

        incoming_position_t position;
        position.objposition =
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
        position.entryposition = indicator_object_get_location(io, entry);
        position.menupos = 0;
        position.found = FALSE;

        gtk_container_foreach(GTK_CONTAINER(menubar), place_in_menu, &position);

        gtk_menu_shell_insert(GTK_MENU_SHELL(menubar), menuitem, position.menupos);

        g_debug("zzz just about there");
        if (something_visible) {
                if (entry->accessible_desc != NULL) {
                        update_accessible_desc(entry, menuitem);
                }
                g_debug("zzz final show");
                gtk_widget_show(menuitem);
        }
        gtk_widget_set_sensitive(menuitem, something_sensitive);

        g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_ENTRY, entry);
        g_object_set_data(G_OBJECT(menuitem), MENU_DATA_INDICATOR_OBJECT, io);

        return;
}

static void entry_removed_cb(GtkWidget *widget, gpointer userdata)
{
        gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

        if (data != userdata) {
                return;
        }

        IndicatorObjectEntry *entry = (IndicatorObjectEntry *)data;
        if (entry->label != NULL) {
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->label),
                                                     G_CALLBACK(something_shown),
                                                     widget);
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->label),
                                                     G_CALLBACK(something_hidden),
                                                     widget);
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->label),
                                                     G_CALLBACK(sensitive_cb),
                                                     widget);
        }
        if (entry->image != NULL) {
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->image),
                                                     G_CALLBACK(something_shown),
                                                     widget);
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->image),
                                                     G_CALLBACK(something_hidden),
                                                     widget);
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->image),
                                                     G_CALLBACK(sensitive_cb),
                                                     widget);
                g_signal_handlers_disconnect_by_func(G_OBJECT(entry->image),
                                                     G_CALLBACK(resize_image),
                                                     widget);
        }

        gtk_widget_destroy(widget);
        return;
}

static void entry_removed(IndicatorObject *io G_GNUC_UNUSED, IndicatorObjectEntry *entry,
                          gpointer user_data)
{
        g_debug("Signal: Entry Removed");

        gtk_container_foreach(GTK_CONTAINER(user_data), entry_removed_cb, entry);

        return;
}

static void entry_moved_find_cb(GtkWidget *widget, gpointer userdata)
{
        gpointer *array = (gpointer *)userdata;
        if (array[1] != NULL) {
                return;
        }

        gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

        if (data != array[0]) {
                return;
        }

        array[1] = widget;
        return;
}

/* Gets called when an entry for an object was moved. */
static void entry_moved(IndicatorObject *io, IndicatorObjectEntry *entry, gint old G_GNUC_UNUSED,
                        gint new G_GNUC_UNUSED, gpointer user_data)
{
        GtkWidget *menubar = GTK_WIDGET(user_data);

        gpointer array[2];
        array[0] = entry;
        array[1] = NULL;

        gtk_container_foreach(GTK_CONTAINER(menubar), entry_moved_find_cb, array);
        if (array[1] == NULL) {
                g_warning("Moving an entry that isn't in our menus.");
                return;
        }

        GtkWidget *mi = GTK_WIDGET(array[1]);
        g_object_ref(G_OBJECT(mi));
        gtk_container_remove(GTK_CONTAINER(menubar), mi);

        incoming_position_t position;
        position.objposition =
            GPOINTER_TO_INT(g_object_get_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER));
        position.entryposition = indicator_object_get_location(io, entry);
        position.menupos = 0;
        position.found = FALSE;

        gtk_container_foreach(GTK_CONTAINER(menubar), place_in_menu, &position);

        gtk_menu_shell_insert(GTK_MENU_SHELL(menubar), mi, position.menupos);
        g_object_unref(G_OBJECT(mi));

        return;
}

static void menu_show(IndicatorObject *io, IndicatorObjectEntry *entry,
                      __attribute__((unused)) guint32 timestamp, gpointer user_data)
{
        GtkWidget *menubar = GTK_WIDGET(user_data);

        if (entry == NULL) {
                /* Close any open menus instead of opening one */
                GList *entries = indicator_object_get_entries(io);
                GList *entry = NULL;
                for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
                        IndicatorObjectEntry *entrydata = (IndicatorObjectEntry *)entry->data;
                        gtk_menu_popdown(entrydata->menu);
                }
                g_list_free(entries);

                /* And tell the menubar to exit activation mode too */
                gtk_menu_shell_cancel(GTK_MENU_SHELL(menubar));
                return;
        }
}

static void update_accessible_desc(IndicatorObjectEntry *entry, GtkWidget *menuitem)
{
        /* We need to deal with the use case where the contents of the
           label overrides what is found in the atk object's name, or at least
           orca speaks the label instead of the atk object name.
         */
        AtkObject *menuitem_obj = gtk_widget_get_accessible(menuitem);
        if (menuitem_obj == NULL) {
                /* Should there be an error printed here? */
                return;
        }

        if (entry->accessible_desc != NULL) {
                atk_object_set_name(menuitem_obj, entry->accessible_desc);
        } else {
                atk_object_set_name(menuitem_obj, "");
        }
        return;
}

static void load_indicator(AppIndicatorApplet *applet,
                           GtkWidget *menubar,
                           IndicatorObject *io,
                           const gchar *name)
{
        /* Set the environment it's in */
        indicator_object_set_environment(io, (const GStrv)indicator_env);
        g_debug("zzz load_indicator %s", name);
        /* Attach the 'name' to the object */

#if HAVE_AYATANA_INDICATOR_NG || HAVE_UBUNTU_INDICATOR_NG
        int pos = 5000 - indicator_object_get_position(io);
        if (pos > 5000) {
                pos = name2order(name);
        }
#else
        int pos = name2order(name);
#endif
        g_object_set_data(G_OBJECT(io), IO_DATA_ORDER_NUMBER, GINT_TO_POINTER(pos));

        /* Connect to its signals */
        g_signal_connect(G_OBJECT(io),
                         INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,
                         G_CALLBACK(entry_added),
                         menubar);
        g_signal_connect(G_OBJECT(io),
                         INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED,
                         G_CALLBACK(entry_removed),
                         menubar);
        g_signal_connect(G_OBJECT(io),
                         INDICATOR_OBJECT_SIGNAL_ENTRY_MOVED,
                         G_CALLBACK(entry_moved),
                         menubar);
        g_signal_connect(G_OBJECT(io),
                         INDICATOR_OBJECT_SIGNAL_MENU_SHOW,
                         G_CALLBACK(menu_show),
                         menubar);
        g_signal_connect(G_OBJECT(io),
                         INDICATOR_OBJECT_SIGNAL_ACCESSIBLE_DESC_UPDATE,
                         G_CALLBACK(accessible_desc_update),
                         menubar);

        g_signal_connect(G_OBJECT(applet),
                         "panel-size-changed",
                         G_CALLBACK(entry_resized),
                         G_OBJECT(io));

        /* Work on the entries */
        GList *entries = indicator_object_get_entries(io);
        GList *entry = NULL;

        for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
                IndicatorObjectEntry *entrydata = (IndicatorObjectEntry *)entry->data;

                entry_added(io, entrydata, menubar);
        }

        g_list_free(entries);
}

#if HAVE_AYATANA_INDICATOR_NG || HAVE_UBUNTU_INDICATOR_NG
void
load_indicators_from_indicator_files (AppIndicatorApplet *applet, GtkWidget *menubar, gint *indicators_loaded)
{
        GDir *dir;
        const gchar *name;
        GError *error = NULL;

        if (!g_file_test(INDICATOR_SERVICE_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
                return; // directory not available
        }

        dir = g_dir_open (INDICATOR_SERVICE_DIR, 0, &error);

        if (!dir) {
                g_warning ("unable to open indicator service file directory: %s", error->message);
                g_error_free (error);

                return;
        }

        gint count = 0;
        while ((name = g_dir_read_name (dir))) {
                gchar *filename;
                IndicatorNg *indicator;

                filename = g_build_filename (INDICATOR_SERVICE_DIR, name, NULL);
                indicator = indicator_ng_new_for_profile (filename, "desktop", &error);
                g_free (filename);

                if (!g_strcmp0(name, INDICATOR_SERVICE_APPMENU_NG)) {
                        continue;
                }
                /*if (!g_strcmp0(name, INDICATOR_SERVICE_ME_NG)) {
                        continue;
                }*/
                /*if (!g_strcmp0(name,  INDICATOR_SERVICE_DATETIME_NG)) {
                        continue;
                }*/
                if (indicator) {
                        load_indicator(applet, menubar, INDICATOR_OBJECT (indicator), name);
                        count++;
                }else{
                        g_warning ("unable to load '%s': %s", name, error->message);
                        g_clear_error (&error);
                }
        }

        *indicators_loaded += count;

        g_dir_close (dir);
}
#endif  /* HAVE_AYATANA_INDICATOR_NG || HAVE_UBUNTU_INDICATOR_NG */

static gboolean load_module(const gchar *name, AppIndicatorApplet *applet, GtkWidget *menubar)
{
        g_debug("Looking at Module: %s", name);
        g_return_val_if_fail(name != NULL, FALSE);

        if (!g_str_has_suffix(name, G_MODULE_SUFFIX)) {
                return FALSE;
        }

        g_debug("Loading Module: %s", name);

        /* Build the object for the module */
        gchar *fullpath = g_build_filename(INDICATOR_DIR, name, NULL);
        IndicatorObject *io = indicator_object_new_from_file(fullpath);
        g_free(fullpath);

        load_indicator(applet, menubar, io, name);

        return TRUE;
}

void update_panel_size(__attribute__((unused)) AppIndicatorApplet *applet, int u_panel_size, int u_icon_size, int u_small_icon_size){
        g_debug("Panel size %d", u_panel_size);
        g_debug("icon size %d", u_icon_size);
        g_debug("small size %d", u_small_icon_size);
        panel_size = u_panel_size;
        icon_size = u_icon_size;
        small_icon_size = u_small_icon_size;

        calc_default_icon_size();
}

void load_modules(AppIndicatorApplet *applet, GtkWidget *menubar, gint *indicators_loaded)
{
        if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
                GDir *dir = g_dir_open(INDICATOR_DIR, 0, NULL);

                const gchar *name;
                gint count = 0;
                while ((name = g_dir_read_name(dir)) != NULL) {
                        if (!g_strcmp0(name, INDICATOR_SERVICE_APPMENU)) {
                                continue;
                        }
                        if (!g_strcmp0(name, INDICATOR_SERVICE_ME)) {
                                continue;
                        }
                        if (!g_strcmp0(name, INDICATOR_SERVICE_DATETIME)) {
                                continue;
                        }
                        g_debug("zzz a: %s", name);
                        if (load_module(name, applet, menubar)) {
                                count++;
                        }
                }

                *indicators_loaded += count;

                g_dir_close(dir);
        }
}

void hotkey_filter(char *keystring G_GNUC_UNUSED, gpointer data)
{
        g_return_if_fail(GTK_IS_MENU_SHELL(data));

        /* Oh, wow, it's us! */
        GList *children = gtk_container_get_children(GTK_CONTAINER(data));
        if (children == NULL) {
                g_debug("Menubar has no children");
                return;
        }

        gtk_menu_shell_select_item(GTK_MENU_SHELL(data), GTK_WIDGET(g_list_last(children)->data));
        g_list_free(children);
        return;
}

gboolean menubar_on_draw(GtkWidget *widget, cairo_t *cr, GtkWidget *menubar)
{
        if (gtk_widget_has_focus(menubar))
                gtk_render_focus(gtk_widget_get_style_context(widget),
                                cr,
                                0,
                                0,
                                -1,
                                -1);

        return FALSE;
}

#ifdef N_
#undef N_
#endif
#define N_(x) x
