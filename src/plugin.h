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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _AppIndicatorNativePlugin AppIndicatorNativePlugin;
typedef struct _AppIndicatorNativePluginClass AppIndicatorNativePluginClass;

#define APPINDICATOR_TYPE_NATIVE_PLUGIN appindicator_native_plugin_get_type()
#define APPINDICATOR_NATIVE_PLUGIN(o)                                                                   \
        (G_TYPE_CHECK_INSTANCE_CAST((o), APPINDICATOR_TYPE_NATIVE_PLUGIN, AppIndicatorNativePlugin))
#define APPINDICATOR_IS_NATIVE_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), APPINDICATOR_TYPE_NATIVE_PLUGIN))
#define APPINDICATOR_NATIVE_PLUGIN_CLASS(o)                                                             \
        (G_TYPE_CHECK_CLASS_CAST((o), APPINDICATOR_TYPE_NATIVE_PLUGIN, AppIndicatorNativePluginClass))
#define APPINDICATOR_IS_NATIVE_PLUGIN_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), APPINDICATOR_TYPE_NATIVE_PLUGIN))
#define APPINDICATOR_NATIVE_PLUGIN_GET_CLASS(o)                                                         \
        (G_TYPE_INSTANCE_GET_CLASS((o), APPINDICATOR_TYPE_NATIVE_PLUGIN, AppIndicatorNativePluginClass))

struct _AppIndicatorNativePluginClass {
        GObjectClass parent_class;
};

struct _AppIndicatorNativePlugin {
        GObject parent;
};

GType appindicator_native_plugin_get_type(void);

G_END_DECLS
