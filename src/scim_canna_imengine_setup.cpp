/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2004 Hiroyuki Ikezoe
 *  Copyright (C) 2004 - 2005 Takuro Ashie
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Based on scim-hangul.
 * Copyright (c) 2004 James Su <suzhe@turbolinux.com.cn>
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_EVENT

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include "scim_canna_prefs.h"
#include "intl.h"

using namespace scim;

#define scim_module_init canna_imengine_setup_LTX_scim_module_init
#define scim_module_exit canna_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       canna_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    canna_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        canna_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description canna_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     canna_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     canna_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   canna_imengine_setup_LTX_scim_setup_module_query_changed

#define DATA_POINTER_KEY "scim-canna::ConfigPointer"

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_CANNA_LOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        return create_setup_window ();
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("Canna"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("An Canna IMEngine Module."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"


// Internal data structure
struct BoolConfigData
{
    const char *key;
    bool        value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct StringConfigData
{
    const char *key;
    String      value;
    const char *label;
    const char *title;
    const char *tooltip;
    GtkWidget  *widget;
    bool        changed;
};

struct KeyboardConfigPage
{
    const char       *label;
    StringConfigData *data;
};

struct ComboConfigData
{
    const char *label;
    const char *data;
};

struct ComboConfigCandidate
{
    const char *label;
    const char *data;
};

enum {
    COLUMN_LABEL = 0,
    COLUMN_VALUE = 1,
    COLUMN_DESC  = 2,
    COLUMN_DATA  = 3,
    N_COLUMNS    = 4,
};

// Internal data declaration.
static bool __have_changed    = true;

static GtkWidget   * __widget_key_categories_menu = NULL;
static GtkWidget   * __widget_key_filter          = NULL;
static GtkWidget   * __widget_key_filter_button   = NULL;
static GtkWidget   * __widget_key_list_view       = NULL;
static GtkTooltips * __widget_tooltips = 0;

static BoolConfigData __config_bool_common [] =
{
    {
        SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME,
        SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME_DEFAULT,
        N_("Specify Canna initialize file"),
        NULL,
        NULL,
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME,
        SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME_DEFAULT,
        N_("Specify Canna server name"),
        NULL,
        NULL,
        NULL,
        false,
    },
};
static unsigned int __config_bool_common_num = sizeof (__config_bool_common) / sizeof (BoolConfigData);

static StringConfigData __config_string_common [] =
{
    {
        SCIM_CANNA_CONFIG_INIT_FILE_NAME,
        SCIM_CANNA_CONFIG_INIT_FILE_NAME_DEFAULT,
        NULL,
        NULL,
        N_("The Canna initialize file name to use."),
        NULL,
        false,
    },
    {
        SCIM_CANNA_CONFIG_SERVER_NAME,
        SCIM_CANNA_CONFIG_SERVER_NAME_DEFAULT,
        NULL,
        NULL,
        N_("The Canna host name to connect."),
        NULL,
        false,
    },
};
static unsigned int __config_string_common_num = sizeof (__config_string_common) / sizeof (StringConfigData);

static StringConfigData __config_keyboards_common [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static StringConfigData __config_keyboards_mode [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static StringConfigData __config_keyboards_caret [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static StringConfigData __config_keyboards_segments [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static StringConfigData __config_keyboards_candidates [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static StringConfigData __config_keyboards_direct_select_candidate [] =
{
    {
        NULL,
        "",
        NULL,
        NULL,
        NULL,
        NULL,
    },
};

static struct KeyboardConfigPage __key_conf_pages[] =
{
    {N_("Common keys"),        __config_keyboards_common},
    {N_("Mode keys"),          __config_keyboards_mode},
    {N_("Caret keys"),         __config_keyboards_caret},
    {N_("Segments keys"),      __config_keyboards_segments},
    {N_("Candidates keys"),    __config_keyboards_candidates},
    {N_("Direct select keys"), __config_keyboards_direct_select_candidate},
};
static unsigned int __key_conf_pages_num = sizeof (__key_conf_pages) / sizeof (KeyboardConfigPage);

const int INDEX_SEARCH_BY_KEY = __key_conf_pages_num;
const int INDEX_ALL           = __key_conf_pages_num + 1;


static void on_default_editable_changed       (GtkEditable     *editable,
                                               gpointer         user_data);
static void on_default_toggle_button_toggled  (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
static void on_toggle_button_toggled_set_sensitive
                                              (GtkToggleButton *togglebutton,
                                               gpointer         user_data);
#if 0
static void on_default_key_selection_clicked  (GtkButton       *button,
                                               gpointer         user_data);
static void on_default_combo_changed          (GtkEditable     *editable,
                                               gpointer         user_data);
#endif
static void     on_key_filter_selection_clicked   (GtkButton       *button,
                                                   gpointer         user_data);
static void     on_key_category_menu_changed      (GtkOptionMenu   *omenu,
                                                   gpointer         user_data);
static gboolean on_key_list_view_key_press        (GtkWidget       *widget,
                                                   GdkEventKey     *event,
                                                   gpointer         user_data);
static gboolean on_key_list_view_button_press     (GtkWidget       *widget,
                                                   GdkEventButton  *event,
                                                   gpointer         user_data);
static void setup_widget_value ();


static BoolConfigData *
find_bool_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData *entry = &__config_bool_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static StringConfigData *
find_string_config_entry (const char *config_key)
{
    if (!config_key)
        return NULL;

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData *entry = &__config_string_common[i];
        if (entry->key && !strcmp (entry->key, config_key))
            return entry;
    }

    return NULL;
}

static GtkWidget *
create_check_button (const char *config_key)
{
    BoolConfigData *entry = find_bool_config_entry (config_key);
    if (!entry)
        return NULL;

    entry->widget = gtk_check_button_new_with_mnemonic (_(entry->label));
    gtk_container_set_border_width (GTK_CONTAINER (entry->widget), 4);
    g_signal_connect (G_OBJECT (entry->widget), "toggled",
                      G_CALLBACK (on_default_toggle_button_toggled),
                      entry);
    gtk_widget_show (entry->widget);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();
    if (entry->tooltip)
        gtk_tooltips_set_tip (__widget_tooltips, entry->widget,
                              _(entry->tooltip), NULL);

    return entry->widget;
}

static bool
match_key_event (const KeyEventList &list, const KeyEvent &key)
{
    KeyEventList::const_iterator kit;

    for (kit = list.begin (); kit != list.end (); ++kit) {
        if (key.code == kit->code && key.mask == kit->mask)
             return true;
    }
    return false;
}

#if 0
static GtkWidget *
create_combo_widget (const char *label_text, GtkWidget **widget,
                     gpointer data_p, gpointer candidates_p)
{
    GtkWidget *hbox, *label;

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);

    label = gtk_label_new (label_text);
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 4);

    *widget = gtk_combo_new ();
    gtk_combo_set_value_in_list (GTK_COMBO (*widget), TRUE, FALSE);
    gtk_combo_set_case_sensitive (GTK_COMBO (*widget), TRUE);
    gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (*widget)->entry), FALSE);
    gtk_widget_show (*widget);
    gtk_box_pack_start (GTK_BOX (hbox), *widget, FALSE, FALSE, 4);
    g_object_set_data (G_OBJECT (GTK_COMBO (*widget)->entry), DATA_POINTER_KEY,
                       (gpointer) candidates_p);

    g_signal_connect ((gpointer) GTK_COMBO (*widget)->entry, "changed",
                      G_CALLBACK (on_default_combo_changed),
                      data_p);

    return hbox;
}
#endif

#define APPEND_ENTRY(data, i)                                                  \
{                                                                              \
    GtkWidget *label;                                                          \
    (data)->widget = gtk_entry_new ();                                         \
    if ((data)->label) {                                                       \
        label = gtk_label_new (NULL);                                          \
        gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _((data)->label));\
        gtk_widget_show (label);                                               \
        gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                   \
        gtk_misc_set_padding (GTK_MISC (label), 4, 0);                         \
        gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,              \
                          (GtkAttachOptions) (GTK_FILL),                       \
                          (GtkAttachOptions) (GTK_FILL), 4, 4);                \
        gtk_label_set_mnemonic_widget (GTK_LABEL (label), (data)->widget);     \
    }                                                                          \
    g_signal_connect ((gpointer) (data)->widget, "changed",                    \
                      G_CALLBACK (on_default_editable_changed),                \
                      (data));                                                 \
    gtk_widget_show ((data)->widget);                                          \
    gtk_table_attach (GTK_TABLE (table), (data)->widget, 1, 2, i, i+1,         \
                      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),                \
                      (GtkAttachOptions) (GTK_FILL), 4, 4);                    \
                                                                               \
    if (!__widget_tooltips)                                                    \
        __widget_tooltips = gtk_tooltips_new();                                \
    if ((data)->tooltip)                                                       \
        gtk_tooltips_set_tip (__widget_tooltips, (data)->widget,               \
                              _((data)->tooltip), NULL);                       \
}

static void
append_key_bindings (GtkTreeView *treeview, gint idx, const gchar *filter)
{
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));
    KeyEventList keys1, keys2;
    
    if (filter && *filter)
        scim_string_to_key_list (keys1, filter);

    if (idx < 0 || idx >= (gint) __key_conf_pages_num)
        return;

    for (unsigned int i = 0; __key_conf_pages[idx].data[i].key; i++) {
        if (filter && *filter) {
            scim_string_to_key_list (keys2, __key_conf_pages[idx].data[i].value.c_str());
            KeyEventList::const_iterator kit;
            bool found = true;
            for (kit = keys1.begin (); kit != keys1.end (); ++kit) {
                if (!match_key_event (keys2, *kit)) {
                    found = false;
                    break;
                }
            }
            if (!found)
                continue;
        }

        GtkTreeIter iter;
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_LABEL, _(__key_conf_pages[idx].data[i].label),
                            COLUMN_VALUE, __key_conf_pages[idx].data[i].value.c_str (),
                            COLUMN_DESC,  _(__key_conf_pages[idx].data[i].tooltip),
                            COLUMN_DATA, &__key_conf_pages[idx].data[i],
                            -1);
    }
}

static void
key_list_view_popup_key_selection (GtkTreeView *treeview)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreePath *treepath = NULL;
    GtkTreeIter iter;

    gtk_tree_view_get_cursor (treeview, &treepath, NULL);
    if (!treepath) return;
    gtk_tree_model_get_iter (model, &iter, treepath);
    gtk_tree_path_free (treepath);

    StringConfigData *data;
    gtk_tree_model_get (model, &iter,
                        COLUMN_DATA, &data,
                        -1);
    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys
            (SCIM_KEY_SELECTION_DIALOG (dialog),
             data->value.c_str());

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys
                (SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, data->value.c_str())) {
                data->value = keys;
                gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                                    COLUMN_VALUE, data->value.c_str(),
                                    -1);
                data->changed = true;
                __have_changed = true;
            }
        }

        gtk_widget_destroy (dialog);
    }
}

static GtkWidget *
create_options_page ()
{
    GtkWidget *vbox, *frame, *table, *widget;
    StringConfigData *entry;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    if (!__widget_tooltips)
        __widget_tooltips = gtk_tooltips_new();

    /* specify initialize file name */
    frame = gtk_frame_new ("");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 4);
    gtk_widget_show (frame);

    widget = create_check_button (SCIM_CANNA_CONFIG_SPECIFY_INIT_FILE_NAME);
     gtk_frame_set_label_widget (GTK_FRAME (frame), widget);

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);
    entry = find_string_config_entry (SCIM_CANNA_CONFIG_INIT_FILE_NAME);
    APPEND_ENTRY(entry, 0);

    g_signal_connect (G_OBJECT (widget), "toggled",
                      G_CALLBACK (on_toggle_button_toggled_set_sensitive),
                      entry->widget);
    gtk_widget_set_sensitive (entry->widget, FALSE);

    /* specify server name */
    frame = gtk_frame_new ("");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
    gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 4);
    gtk_widget_show (frame);

    widget = create_check_button (SCIM_CANNA_CONFIG_SPECIFY_SERVER_NAME);
    gtk_frame_set_label_widget (GTK_FRAME (frame), widget);

    table = gtk_table_new (2, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_widget_show (table);
    entry = find_string_config_entry (SCIM_CANNA_CONFIG_SERVER_NAME);
    APPEND_ENTRY(entry, 0);

    g_signal_connect (G_OBJECT (widget), "toggled",
                      G_CALLBACK (on_toggle_button_toggled_set_sensitive),
                      entry->widget);
    gtk_widget_set_sensitive (entry->widget, FALSE);

    return vbox;
}

#if 0
static GtkWidget *
create_toolbar_page ()
{
    GtkWidget *vbox, *hbox, *label;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    return vbox;
}
#endif

#if 0
static GtkWidget *
create_dict_page (void)
{
    GtkWidget *table;
    GtkWidget *label;

    table = gtk_table_new (3, 3, FALSE);
    gtk_widget_show (table);

    return table;
}
#endif

static GtkWidget *
create_keyboard_page (void)
{
    GtkWidget *vbox, *hbox;

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    // category menu
    GtkWidget *omenu = gtk_option_menu_new ();
    __widget_key_categories_menu = omenu;
    gtk_box_pack_start (GTK_BOX (hbox), omenu, FALSE, FALSE, 2);
    gtk_widget_show (omenu);

    GtkWidget *menu = gtk_menu_new ();

    GtkWidget *menuitem;

    for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
        menuitem = gtk_menu_item_new_with_label (_(__key_conf_pages[i].label));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        gtk_widget_show (menuitem);
    }

    menuitem = gtk_menu_item_new_with_label (_("Search by key"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    menuitem = gtk_menu_item_new_with_label (_("all"));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    gtk_widget_show (menuitem);

    gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
    gtk_widget_show (menu);

    GtkWidget *entry = gtk_entry_new ();
    __widget_key_filter = entry;
    gtk_entry_set_editable (GTK_ENTRY (entry), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 2);
    gtk_widget_show(entry);

    GtkWidget *button = gtk_button_new_with_label ("...");
    __widget_key_filter_button = button;
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (on_key_filter_selection_clicked), entry);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
    gtk_widget_show (button);

    // key bindings view
    GtkWidget *scrwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
                                         GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (scrwin), 4);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrwin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), scrwin, TRUE, TRUE, 2);
    gtk_widget_show (scrwin);

    GtkListStore *store = gtk_list_store_new (N_COLUMNS,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_STRING,
                                              G_TYPE_POINTER);
    GtkWidget *treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
    __widget_key_list_view = treeview;
    gtk_container_add (GTK_CONTAINER (scrwin), treeview);
    gtk_widget_show (treeview);

    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Feature"), cell,
                                                       "text", COLUMN_LABEL,
                                                       NULL);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width (column, 120);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Key bindings"), cell,
                                                       "text", COLUMN_VALUE,
                                                       NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    cell = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Description"), cell,
                                                       "text", COLUMN_DESC,
                                                       NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    // connect signals
    g_signal_connect (G_OBJECT (omenu), "changed",
                      G_CALLBACK (on_key_category_menu_changed), treeview);
    g_signal_connect (G_OBJECT (treeview), "key-press-event",
                      G_CALLBACK (on_key_list_view_key_press), NULL);
    g_signal_connect (G_OBJECT (treeview), "button-press-event",
                      G_CALLBACK (on_key_list_view_button_press), NULL);

    return vbox;
}

static GtkWidget *
create_setup_window ()
{
    static GtkWidget *window = NULL;

    if (!window) {
        GtkWidget *notebook = gtk_notebook_new();
        gtk_widget_show (notebook);
        window = notebook;
        gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);

        // Create the first page.
        GtkWidget *page = create_options_page ();
        GtkWidget *label = gtk_label_new (_("Options"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

#if 0
        // Create the second page.
        page = create_toolbar_page ();
        label = gtk_label_new (_("Toolbar"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

        // Create the third page.
        page = create_dict_page ();
        label = gtk_label_new (_("Dictionary"));
        gtk_widget_show (label);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
#endif

#if 0
        // Create the key bind pages.
        for (unsigned int i = 0; i < __key_conf_pages_num; i++) {
            page = create_keyboard_page (i);
            label = gtk_label_new (_(__key_conf_pages[i].label));
            gtk_widget_show (label);
            gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
        }
#endif

        // for preventing enabling left arrow.
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 0);

        setup_widget_value ();
    }

    return window;
}

static void
setup_combo_value (GtkCombo *combo, const String & str)
{
    GList *list = NULL;
    const char *defval = NULL;

    ComboConfigCandidate *data
        = static_cast<ComboConfigCandidate*>
        (g_object_get_data (G_OBJECT (GTK_COMBO(combo)->entry),
                            DATA_POINTER_KEY));

    for (unsigned int i = 0; data[i].label; i++) {
        list = g_list_append (list, (gpointer) _(data[i].label));
        if (!strcmp (data[i].data, str.c_str ()))
            defval = _(data[i].label);
    }

    gtk_combo_set_popdown_strings (combo, list);
    g_list_free (list);

    if (defval)
        gtk_entry_set_text (GTK_ENTRY (combo->entry), defval);
}

static void
setup_widget_value ()
{
    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.widget)
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (entry.widget),
                                          entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.widget && GTK_IS_COMBO (entry.widget))
            setup_combo_value (GTK_COMBO (entry.widget), entry.value);
        else if (entry.widget && GTK_IS_ENTRY (entry.widget))
            gtk_entry_set_text (GTK_ENTRY (entry.widget),
                                entry.value.c_str ());
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].widget) {
                gtk_entry_set_text (
                    GTK_ENTRY (__key_conf_pages[j].data[i].widget),
                    __key_conf_pages[j].data[i].value.c_str ());
            }
        }
    }

    if (__widget_key_categories_menu) {
        gtk_option_menu_set_history (GTK_OPTION_MENU (__widget_key_categories_menu), 0);
        gtk_widget_set_sensitive (__widget_key_filter, FALSE);
        gtk_widget_set_sensitive (__widget_key_filter_button, FALSE);
        GtkTreeModel *model;
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
        gtk_list_store_clear (GTK_LIST_STORE (model));
        append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view), 0, NULL);
    }
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        entry.value = config->read (String (entry.key), entry.value);
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; ++ j) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            __key_conf_pages[j].data[i].value =
                config->read (String (__key_conf_pages[j].data[i].key),
                              __key_conf_pages[j].data[i].value);
        }
    }

    setup_widget_value ();

    for (unsigned int i = 0; i < __config_bool_common_num; i++)
        __config_bool_common[i].changed = false;

    for (unsigned int i = 0; i < __config_string_common_num; i++)
        __config_string_common[i].changed = false;

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i)
            __key_conf_pages[j].data[i].changed = false;
    }

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null ())
        return;

    for (unsigned int i = 0; i < __config_bool_common_num; i++) {
        BoolConfigData &entry = __config_bool_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int i = 0; i < __config_string_common_num; i++) {
        StringConfigData &entry = __config_string_common[i];
        if (entry.changed)
            entry.value = config->write (String (entry.key), entry.value);
        entry.changed = false;
    }

    for (unsigned int j = 0; j < __key_conf_pages_num; j++) {
        for (unsigned int i = 0; __key_conf_pages[j].data[i].key; ++ i) {
            if (__key_conf_pages[j].data[i].changed)
                config->write (String (__key_conf_pages[j].data[i].key),
                               __key_conf_pages[j].data[i].value);
            __key_conf_pages[j].data[i].changed = false;
        }
    }

    __have_changed = false;
}

static bool
query_changed ()
{
    return __have_changed;
}


static void
on_default_toggle_button_toggled (GtkToggleButton *togglebutton,
                                  gpointer         user_data)
{
    BoolConfigData *entry = static_cast<BoolConfigData*> (user_data);

    if (entry) {
        entry->value = gtk_toggle_button_get_active (togglebutton);
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_default_editable_changed (GtkEditable *editable,
                             gpointer     user_data)
{
    StringConfigData *entry = static_cast <StringConfigData*> (user_data);

    if (entry) {
        entry->value = String (gtk_entry_get_text (GTK_ENTRY (editable)));
        entry->changed = true;
        __have_changed = true;
    }
}

static void
on_toggle_button_toggled_set_sensitive (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
    GtkWidget *widget = GTK_WIDGET (user_data);

    if (widget) {
        bool active = gtk_toggle_button_get_active (togglebutton);
        gtk_widget_set_sensitive (widget, active);
    }
}

static void
on_key_category_menu_changed (GtkOptionMenu *omenu, gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (user_data);
    GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (treeview));

    gtk_list_store_clear (store);

    gint idx = gtk_option_menu_get_history (omenu);

    bool use_filter = false;

    if (idx >= 0 && idx < (gint) __key_conf_pages_num) {
        append_key_bindings (treeview, idx, NULL);

    } else if (idx == INDEX_SEARCH_BY_KEY) {
        // search by key
        use_filter = true;
        const char *str = gtk_entry_get_text (GTK_ENTRY (__widget_key_filter));
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, str);

    } else if (idx == INDEX_ALL) {
        // all
        for (unsigned int i = 0; i < __key_conf_pages_num; i++)
            append_key_bindings (treeview, i, NULL);
    }

    gtk_widget_set_sensitive (__widget_key_filter,        use_filter);
    gtk_widget_set_sensitive (__widget_key_filter_button, use_filter);
}

#if 0
static void
on_default_key_selection_clicked (GtkButton *button,
                                  gpointer   user_data)
{
    StringConfigData *data = static_cast <StringConfigData*> (user_data);

    if (data) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (GTK_ENTRY (data->widget)));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (GTK_ENTRY (data->widget))))
                gtk_entry_set_text (GTK_ENTRY (data->widget), keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static void
on_default_combo_changed (GtkEditable *editable,
                          gpointer user_data)
{
    String *str = static_cast<String *> (user_data);
    ComboConfigData *data
        = static_cast<ComboConfigData *> (g_object_get_data (G_OBJECT (editable),
                                                             DATA_POINTER_KEY));

    if (!str) return;
    if (!data) return;

    const char *label =  gtk_entry_get_text (GTK_ENTRY (editable));

    for (unsigned int i = 0; data[i].label; i++) {
        if (label && !strcmp (_(data[i].label), label)) {
            *str = data[i].data;
            __have_changed = true;
            break;
        }
    }
}
#endif

static void
on_key_filter_selection_clicked (GtkButton *button,
                                 gpointer   user_data)
{
    GtkEntry *entry = static_cast <GtkEntry*> (user_data);

    if (entry) {
        GtkWidget *dialog = scim_key_selection_dialog_new (_("Set key filter"));
        gint result;

        scim_key_selection_dialog_set_keys (
            SCIM_KEY_SELECTION_DIALOG (dialog),
            gtk_entry_get_text (entry));

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (result == GTK_RESPONSE_OK) {
            const gchar *keys = scim_key_selection_dialog_get_keys (
                            SCIM_KEY_SELECTION_DIALOG (dialog));

            if (!keys) keys = "";

            if (strcmp (keys, gtk_entry_get_text (entry)))
                gtk_entry_set_text (entry, keys);

            GtkTreeModel *model;
            model = gtk_tree_view_get_model (GTK_TREE_VIEW (__widget_key_list_view));
            gtk_list_store_clear (GTK_LIST_STORE (model));
            for (unsigned int i = 0; i < __key_conf_pages_num; i++)
                append_key_bindings (GTK_TREE_VIEW (__widget_key_list_view),
                                     i, keys);
        }

        gtk_widget_destroy (dialog);
    }
}

static gboolean
on_key_list_view_key_press (GtkWidget *widget, GdkEventKey *event,
                            gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    switch (event->keyval) {
    case GDK_Return:
    case GDK_KP_Enter:
        key_list_view_popup_key_selection (treeview);
        break;
    }

    return FALSE;
}

static gboolean
on_key_list_view_button_press (GtkWidget *widget, GdkEventButton *event,
                               gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW (widget);

    if (event->type == GDK_2BUTTON_PRESS) {
        key_list_view_popup_key_selection (treeview);
        return TRUE;
    }

    return FALSE;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
