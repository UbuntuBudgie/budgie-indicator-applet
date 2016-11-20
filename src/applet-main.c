/*
A small wrapper utility to load indicators and put them as menu items
into the mate-panel using it's applet interface.

Copyright 2009-2010 Canonical Ltd., 2016 David Mohammed

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

//#include <libindicator/indicator-ng.h>
#include <libindicator/indicator-object.h>

static gchar *indicator_order[] = { "libapplication.so", "libmessaging.so", "libsoundmenu.so",
                                    "libdatetime.so",    "libsession.so",   NULL };

#define MENU_DATA_INDICATOR_OBJECT "indicator-object"
#define MENU_DATA_INDICATOR_ENTRY "indicator-entry"

#define IO_DATA_ORDER_NUMBER "indicator-order-number"

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

static void something_shown(GtkWidget *widget, gpointer user_data)
{
        GtkWidget *menuitem = GTK_WIDGET(user_data);
        gtk_widget_show(menuitem);
}

static void something_hidden(GtkWidget *widget, gpointer user_data)
{
        GtkWidget *menuitem = GTK_WIDGET(user_data);
        gtk_widget_hide(menuitem);
}

static void sensitive_cb(GObject *obj, GParamSpec *pspec, gpointer user_data)
{
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

        return indicator_object_entry_activate(io,
                                               (IndicatorObjectEntry *)user_data,
                                               gtk_get_current_event_time());
}

static gboolean entry_scrolled(GtkWidget *menuitem, GdkEventScroll *event, gpointer data)
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

static void accessible_desc_update_cb(GtkWidget *widget, gpointer userdata)
{
        gpointer data = g_object_get_data(G_OBJECT(widget), MENU_DATA_INDICATOR_ENTRY);

        if (data != userdata) {
                return;
        }

        IndicatorObjectEntry *entry = (IndicatorObjectEntry *)data;
        update_accessible_desc(entry, widget);
}

static void accessible_desc_update(IndicatorObject *io, IndicatorObjectEntry *entry,
                                   GtkWidget *menubar)
{
        gtk_container_foreach(GTK_CONTAINER(menubar), accessible_desc_update_cb, entry);
        return;
}

static void entry_added(IndicatorObject *io, IndicatorObjectEntry *entry, GtkWidget *menubar)
{
        g_debug("zzz Signal: Entry Added");
        gboolean something_visible = FALSE;
        gboolean something_sensitive = FALSE;
        GtkStyleContext *context;
        GtkCssProvider *css_provider = NULL;
        GSettings *settings = NULL;

        /*
         * we don't want to have the nm-applet being displayed
         * budgie-desktop provides this
         */
        if (entry->name_hint != NULL) {
            if (strstr(entry->name_hint, "nm-applet") != NULL) {
                    return;
            }
            g_debug("zzz %s", entry->name_hint);
        }
        else {
            g_debug("zzz no name_hint");
        }
        
        GtkWidget *menuitem = gtk_menu_item_new();
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

        /* Allows indicators to receive mouse scroll event in GTK+3 */
        gtk_widget_add_events(GTK_WIDGET(menuitem), GDK_SCROLL_MASK);

        g_object_set_data(G_OBJECT(menuitem), "indicator", io);
        g_object_set_data(G_OBJECT(menuitem), "box", box);

        g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(entry_activated), entry);
        g_signal_connect(G_OBJECT(menuitem), "scroll-event", G_CALLBACK(entry_scrolled), entry);

        if (entry->image != NULL) {
                g_debug("zzz have an image");
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
                gtk_label_set_angle(GTK_LABEL(entry->label), 0.0);
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

        /*
         * theme each indicator correctly
         * this at the moment duplicates what happens in applet.c when the user changes the
         * theme - need to cleanup the code to do stuff only in one place
         */
        settings = g_settings_new_with_path("com.solus-project.budgie-panel", "/com/solus-project/budgie-panel/");
        if (g_settings_get_boolean(settings, "builtin-theme")) {
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
            gtk_style_context_add_class(context, "raven");
            g_debug("zzz adding raven");
        }
        else {
            context = gtk_widget_get_style_context(GTK_WIDGET(menubar));
            gtk_style_context_remove_class(context, "menubar");
            g_debug("zzz removing menubar");
        }
        
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

static void menu_show(IndicatorObject *io, IndicatorObjectEntry *entry, guint32 timestamp,
                      gpointer user_data)
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

static void load_indicator(GtkWidget *menubar, IndicatorObject *io, const gchar *name)
{
        /* Set the environment it's in */
        indicator_object_set_environment(io, (const GStrv)indicator_env);
        g_debug("zzz load_indicator %s", name);
        /* Attach the 'name' to the object */
        int pos = name2order(name);

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

        /* Work on the entries */
        GList *entries = indicator_object_get_entries(io);
        GList *entry = NULL;

        for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
                IndicatorObjectEntry *entrydata = (IndicatorObjectEntry *)entry->data;
                entry_added(io, entrydata, menubar);
        }

        g_list_free(entries);
}

/*
#define INDICATOR_SERVICE_DIR "/usr/share/unity/indicators"

void
load_indicators_from_indicator_files (GtkWidget *menubar, gint *indicators_loaded)
{
	GDir *dir;
	const gchar *name;
	GError *error = NULL;

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

*#ifdef INDICATOR_APPLET_APPMENU
		if (g_strcmp0(name, "com.canonical.indicator.appmenu")) {
			continue;
		}
#else
		if (!g_strcmp0(name, "com.canonical.indicator.appmenu")) {
			continue;
		}
#endif
#ifdef INDICATOR_APPLET
		if (!g_strcmp0(name, "com.canonical.indicator.me")) {
			continue;
		}
		if (!g_strcmp0(name, "com.canonical.indicator.datetime")) {
			continue;
		}
#endif
*
		if (indicator) {
			load_indicator(menubar, INDICATOR_OBJECT (indicator), name);
			count++;
		}else{
			g_warning ("unable to load '%s': %s", name, error->message);
			g_clear_error (&error);
		}
	}

	*indicators_loaded += count;

	g_dir_close (dir);
}
*/

static gboolean load_module(const gchar *name, GtkWidget *menubar)
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

        load_indicator(menubar, io, name);

        return TRUE;
}

void load_modules(GtkWidget *menubar, gint *indicators_loaded)
{
        if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
                GDir *dir = g_dir_open(INDICATOR_DIR, 0, NULL);

                const gchar *name;
                gint count = 0;
                while ((name = g_dir_read_name(dir)) != NULL) {
                        if (!g_strcmp0(name, "libappmenu.so")) {
                                continue;
                        }
                        if (!g_strcmp0(name, "libme.so")) {
                                continue;
                        }
                        if (!g_strcmp0(name, "libdatetime.so")) {
                                continue;
                        }
                        g_debug("zzz a: %s", name);
                        if (load_module(name, menubar)) {
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
                gtk_paint_focus(gtk_widget_get_style(widget),
                                cr,
                                gtk_widget_get_state(menubar),
                                widget,
                                "menubar-applet",
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
