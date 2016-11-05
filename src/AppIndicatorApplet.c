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

#include <budgie-desktop/plugin.h>
#include <gobject/gobject.h>

#include "AppIndicatorApplet.h"

G_BEGIN_DECLS

/**
 * Define our loader cruft
 */
#define APPINDICATOR_TYPE_APPLET appindicator_applet_get_type()
G_DECLARE_FINAL_TYPE(AppIndicatorApplet, appindicator_applet, NATIVE, APPLET, GObject)

G_END_DECLS

/**
 * Found in AppIndicatorPanelApplet.c
 */
extern void appindicator_panel_applet_register_type(GTypeModule *module);

/**
 * This is apparently our instance
 */
struct _AppIndicatorApplet {
        GObject parent;
};

/**
 * Return a new panel widget
 */
static BudgieApplet *appindicator_applet_get_panel_widget(__budgie_unused__ BudgiePlugin *self,
                                                          __budgie_unused__ gchar *uuid)
{
        return appindicator_panel_applet_new();
}

/**
 * Implement interface and override methods
 */
static void appindicator_applet_iface_init(BudgiePluginIface *iface)
{
        iface->get_panel_widget = appindicator_applet_get_panel_widget;
}

/**
 * Override anything you need here like ->destroy, etc.
 */
static void appindicator_applet_class_init(__budgie_unused__ AppIndicatorAppletClass *cl)
{
}

/**
 * Unused function
 */
static void appindicator_applet_init(__budgie_unused__ AppIndicatorApplet *self)
{
}
static void appindicator_applet_class_finalize(__budgie_unused__ AppIndicatorAppletClass *cls)
{
}

/**
 * This is us now doing the implementation chain ups..
 */

G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorApplet, appindicator_applet, G_TYPE_OBJECT, 0,
                               G_IMPLEMENT_INTERFACE_DYNAMIC(BUDGIE_TYPE_PLUGIN,
                                                             appindicator_applet_iface_init))

/**
 * Export the types back to peas. Note how we extern'd the function above
 * to make use of appindicator_panel_applet_register_type, ensuring everything is
 * exported to the type system.
 */
G_MODULE_EXPORT void peas_register_types(PeasObjectModule *module)
{
        appindicator_applet_register_type(G_TYPE_MODULE(module));
        appindicator_panel_applet_init_gtype(G_TYPE_MODULE(module));

        peas_object_module_register_extension_type(module,
                                                   BUDGIE_TYPE_PLUGIN,
                                                   APPINDICATOR_TYPE_APPLET);
}
