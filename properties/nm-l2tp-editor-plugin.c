// SPDX-License-Identifier: GPL-2.0+
/***************************************************************************
 *
 * Copyright (C) 2008 Dan Williams, <dcbw@redhat.com>
 * Copyright (C) 2008 - 2011 Red Hat, Inc.
 * Based on work by David Zeuthen, <davidz@redhat.com>
 *
 */

#include "nm-default.h"

#include "nm-l2tp-editor-plugin.h"

#if (NETWORKMANAGER_COMPILATION & NM_NETWORKMANAGER_COMPILATION_WITH_LIBNM_UTIL)
#include "nm-l2tp-editor.h"
#else
#include "nm-utils/nm-vpn-plugin-utils.h"
#endif

#define L2TP_PLUGIN_NAME    _("Layer 2 Tunneling Protocol (L2TP)")
#define L2TP_PLUGIN_DESC    _("Compatible with Microsoft and other L2TP VPN servers.")

/*****************************************************************************/

static void l2tp_plugin_ui_interface_init (NMVpnEditorPluginInterface *iface_class);

G_DEFINE_TYPE_EXTENDED (L2tpPluginUi, l2tp_plugin_ui, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (NM_TYPE_VPN_EDITOR_PLUGIN,
                                               l2tp_plugin_ui_interface_init))

enum {
	PROP_0,
	PROP_NAME,
	PROP_DESC,
	PROP_SERVICE,

	LAST_PROP
};

/*****************************************************************************/

static NMConnection *
import (NMVpnEditorPlugin *iface, const char *path, GError **error)
{
	gs_free char *contents = NULL;
	gs_strfreev char **lines = NULL;
	char *ext;

	ext = strrchr (path, '.');
	if (!ext) {
		g_set_error (error,
		             NMV_EDITOR_PLUGIN_ERROR,
		             NMV_EDITOR_PLUGIN_ERROR_FILE_NOT_VPN,
		             "unknown L2TP file extension");
		return NULL;
	}

	if (strcmp (ext, ".conf") && strcmp (ext, ".cnf")) {
		g_set_error (error,
		             NMV_EDITOR_PLUGIN_ERROR,
		             NMV_EDITOR_PLUGIN_ERROR_FILE_NOT_VPN,
		             "unknown L2TP file extension");
		return NULL;
	}

	if (!g_file_get_contents (path, &contents, NULL, error))
		return NULL;

	lines = g_strsplit_set (contents, "\r\n", 0);
	if (g_strv_length (lines) <= 1) {
		g_set_error (error,
		             NMV_EDITOR_PLUGIN_ERROR,
		             NMV_EDITOR_PLUGIN_ERROR_FILE_NOT_READABLE,
		             "not a valid L2TP configuration file");
		return NULL;
	}

	g_set_error_literal (error,
	                     NMV_EDITOR_PLUGIN_ERROR,
	                     NMV_EDITOR_PLUGIN_ERROR_FAILED,
	                     "L2TP import is not implemented");
	return NULL;
}

static gboolean
export (NMVpnEditorPlugin *iface,
        const char *path,
        NMConnection *connection,
        GError **error)
{
	g_set_error_literal (error,
	                     NMV_EDITOR_PLUGIN_ERROR,
	                     NMV_EDITOR_PLUGIN_ERROR_FAILED,
	                     "L2TP export is not implemented");
	return FALSE;
}

static char *
get_suggested_filename (NMVpnEditorPlugin *iface, NMConnection *connection)
{
	NMSettingConnection *s_con;
	const char *id;

	g_return_val_if_fail (connection != NULL, NULL);

	s_con = nm_connection_get_setting_connection (connection);
	g_return_val_if_fail (s_con != NULL, NULL);

	id = nm_setting_connection_get_id (s_con);
	g_return_val_if_fail (id != NULL, NULL);

	return g_strdup_printf ("%s (l2tp).conf", id);
}

static NMVpnEditorPluginCapability
get_capabilities (NMVpnEditorPlugin *iface)
{
	return NM_VPN_EDITOR_PLUGIN_CAPABILITY_NONE;
}

#if !(NETWORKMANAGER_COMPILATION & NM_NETWORKMANAGER_COMPILATION_WITH_LIBNM_UTIL)
static NMVpnEditor *
_call_editor_factory (gpointer factory,
                      NMVpnEditorPlugin *editor_plugin,
                      NMConnection *connection,
                      gpointer user_data,
                      GError **error)
{
	return ((NMVpnEditorFactory) factory) (editor_plugin,
	                                       connection,
	                                       error);
}
#endif

static NMVpnEditor *
get_editor (NMVpnEditorPlugin *iface, NMConnection *connection, GError **error)
{
	g_return_val_if_fail (L2TP_IS_PLUGIN_UI (iface), NULL);
	g_return_val_if_fail (NM_IS_CONNECTION (connection), NULL);
	g_return_val_if_fail (!error || !*error, NULL);

	{
#if (NETWORKMANAGER_COMPILATION & NM_NETWORKMANAGER_COMPILATION_WITH_LIBNM_UTIL)
		return nm_vpn_plugin_ui_widget_interface_new (connection, error);
#else
		return nm_vpn_plugin_utils_load_editor (NM_PLUGIN_DIR"/libnm-vpn-plugin-l2tp-editor.so",
		                                        "nm_vpn_editor_factory_l2tp",
		                                        _call_editor_factory,
		                                        iface,
		                                        connection,
		                                        NULL,
		                                        error);
#endif
	}
}

static void
get_property (GObject *object, guint prop_id,
              GValue *value, GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, L2TP_PLUGIN_NAME);
		break;
	case PROP_DESC:
		g_value_set_string (value, L2TP_PLUGIN_DESC);
		break;
	case PROP_SERVICE:
		g_value_set_string (value, NM_DBUS_SERVICE_L2TP);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
l2tp_plugin_ui_class_init (L2tpPluginUiClass *req_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (req_class);

	object_class->get_property = get_property;

	g_object_class_override_property (object_class,
	                                  PROP_NAME,
	                                  NM_VPN_EDITOR_PLUGIN_NAME);

	g_object_class_override_property (object_class,
	                                  PROP_DESC,
	                                  NM_VPN_EDITOR_PLUGIN_DESCRIPTION);

	g_object_class_override_property (object_class,
	                                  PROP_SERVICE,
	                                  NM_VPN_EDITOR_PLUGIN_SERVICE);
}

static void
l2tp_plugin_ui_init (L2tpPluginUi *plugin)
{
}

static void
l2tp_plugin_ui_interface_init (NMVpnEditorPluginInterface *iface_class)
{
	iface_class->get_editor = get_editor;
	iface_class->get_capabilities = get_capabilities;
	iface_class->import_from_file = import;
	iface_class->export_to_file = export;
	iface_class->get_suggested_filename = get_suggested_filename;
}

/*****************************************************************************/

G_MODULE_EXPORT NMVpnEditorPlugin *
nm_vpn_editor_plugin_factory (GError **error)
{
	if (error)
		g_return_val_if_fail (*error == NULL, NULL);

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

	return NM_VPN_EDITOR_PLUGIN (g_object_new (L2TP_TYPE_PLUGIN_UI, NULL));
}

