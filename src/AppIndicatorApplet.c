/**
 * Copyright (C) 2016 Ikey Doherty
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <budgie-desktop/plugin.h>
#include <gobject/gobject.h>

#include "AppIndicatorApplet.h"

G_BEGIN_DECLS

/**
 * Define our loader cruft
 */
#define NATIVE_TYPE_APPLET appindicator_applet_get_type ()
G_DECLARE_FINAL_TYPE(AppIndicatorApplet, appindicator_applet, NATIVE, APPLET, GObject)

G_END_DECLS

/**
 * Found in AppIndicatorPanelApplet.c
 */
extern void native_panel_applet_register_type(GTypeModule *module);

/**
 * This is apparently our instance
 */
struct _AppIndicatorApplet
{
        GObject parent;
};

/**
 * Return a new panel widget
 */
static BudgieApplet *appindicator_applet_get_panel_widget(__budgie_unused__ BudgiePlugin *self,
                                                    __budgie_unused__ gchar *uuid)
{
        return native_panel_applet_new();
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

G_DEFINE_DYNAMIC_TYPE_EXTENDED(AppIndicatorApplet, appindicator_applet, G_TYPE_OBJECT,
                               0, G_IMPLEMENT_INTERFACE_DYNAMIC (BUDGIE_TYPE_PLUGIN,
                                                                 appindicator_applet_iface_init))

/**
 * Export the types back to peas. Note how we extern'd the function above
 * to make use of native_panel_applet_register_type, ensuring everything is
 * exported to the type system.
 */
G_MODULE_EXPORT void
peas_register_types(PeasObjectModule *module)
{
        appindicator_applet_register_type(G_TYPE_MODULE(module));
        native_panel_applet_init_gtype(G_TYPE_MODULE(module));

        peas_object_module_register_extension_type(module,
                                                   BUDGIE_TYPE_PLUGIN,
                                                   NATIVE_TYPE_APPLET);
}
