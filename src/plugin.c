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

#include <budgie-desktop/plugin.h>

#include "applet.h"
#include "plugin.h"

static void appindicator_native_plugin_iface_init(BudgiePluginIface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorNativePlugin, appindicator_native_plugin, G_TYPE_OBJECT,
                               0,
                               G_IMPLEMENT_INTERFACE_DYNAMIC(BUDGIE_TYPE_PLUGIN,
                                                             appindicator_native_plugin_iface_init))

/**
 * Return a new panel widget
 */
static BudgieApplet *native_applet_get_panel_widget(__budgie_unused__ BudgiePlugin *self,
                                                    __budgie_unused__ gchar *uuid)
{
        return appindicator_applet_new();
}

/**
 * Handle cleanup
 */
static void appindicator_native_plugin_dispose(GObject *object)
{
        G_OBJECT_CLASS(appindicator_native_plugin_parent_class)->dispose(object);
}

/**
 * Class initialisation
 */
static void appindicator_native_plugin_class_init(AppIndicatorNativePluginClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = appindicator_native_plugin_dispose;
}

/**
 * Implement the BudgiePlugin interface, i.e the factory method get_panel_widget
 */
static void appindicator_native_plugin_iface_init(BudgiePluginIface *iface)
{
        iface->get_panel_widget = native_applet_get_panel_widget;
}

/**
 * No-op, just skips compiler errors
 */
static void appindicator_native_plugin_init(__budgie_unused__ AppIndicatorNativePlugin *self)
{
}

/**
 * We have no cleaning ourselves to do
 */
static void appindicator_native_plugin_class_finalize(
    __budgie_unused__ AppIndicatorNativePluginClass *klazz)
{
}

/**
 * Export the types to the gobject type system
 */
G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
        appindicator_native_plugin_register_type(G_TYPE_MODULE(module));

        /* Register the actual dynamic types contained in the resulting plugin */
        appindicator_applet_init_gtype(G_TYPE_MODULE(module));

        peas_object_module_register_extension_type(module,
                                                   BUDGIE_TYPE_PLUGIN,
                                                   APPINDICATOR_TYPE_NATIVE_PLUGIN);
}
