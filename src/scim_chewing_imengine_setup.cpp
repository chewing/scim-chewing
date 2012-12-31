/** @file scim_chewing_imengine_setup.cpp
 * implementation of SetupUI Module of chewing imengine module.
 */

/*
 * SCIM-chewing -
 *	Intelligent Chinese Phonetic IM Engine for SCIM.
 *
 * Copyright (c) 2005, 2006, 2008, 2012
 *	SCIM-chewing Developers. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(GETTEXT_PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#include <scim.h>
#include <gtk/scimkeyselection.h>
#include <chewing/chewing.h>
#include "scim_chewing_config_entry.h"
#include "scim_color_button.h"

#include <cstring>

#if !GTK_CHECK_VERSION(2, 12, 0)
    #define SCIM_TABLES_USE_GTK_TOOLTIPS
#endif

#if !GTK_CHECK_VERSION(2, 22, 0)
    #define SCIM_TABLES_USE_GTK_DIALOG_SEPARATOR
#endif

#if GTK_CHECK_VERSION(2, 14, 0)
    #define SCIM_TABLES_USE_GTK_DIALOG_GET_CONTENT_AREA
    #define SCIM_TABLES_USE_GTK_DIALOG_GET_ACTION_AREA
#endif

#if GTK_CHECK_VERSION(2, 18, 0)
    #define SCIM_TABLES_USE_GTK_WIDGET_GET_CAN_DEFAULT
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
    #define SCIM_TABLES_USE_GTK_BOX
    #define SCIM_TABLES_USE_GTK_FILE_CHOOSER
#endif

using namespace scim;

#define scim_module_init chewing_imengine_setup_LTX_scim_module_init
#define scim_module_exit chewing_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       chewing_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    chewing_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        chewing_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description chewing_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     chewing_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     chewing_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   chewing_imengine_setup_LTX_scim_setup_module_query_changed

static GtkWidget * create_setup_window();
static void load_config( const ConfigPointer &config );
static void save_config( const ConfigPointer &config );
static bool query_changed();

// Module Interface.
extern "C" {

	void scim_module_init()
	{
		bindtextdomain( GETTEXT_PACKAGE, SCIM_CHEWING_LOCALEDIR );
		bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
	}

	void scim_module_exit()
	{
	}

	GtkWidget * scim_setup_module_create_ui()
	{
		return create_setup_window();
	}

	String scim_setup_module_get_category()
	{
		return String( "IMEngine" );
	}

	String scim_setup_module_get_name()
	{
		return String( _( "Chewing" ) );
	}

	String scim_setup_module_get_description()
	{
		return String( _( "An Intelligent Chinese Phonetic IMEngine Module." ) );
	}

	void scim_setup_module_load_config( const ConfigPointer &config )
	{
		load_config( config );
	}

	void scim_setup_module_save_config( const ConfigPointer &config )
	{
		save_config( config );
	}

	bool scim_setup_module_query_changed()
	{
		return query_changed();
	}
} // extern "C"

// Internal data structure
struct KeyboardConfigData {
	const char *key;
	const char *label;
	const char *title;
	const char *tooltip;
	GtkWidget  *entry;
	GtkWidget  *button;
	String      data;
};

struct ColorConfigData {
	const char *fg_key;
	String      fg_value;
	String      fg_default_value;
	const char *bg_key;
	String      bg_value;
	String      bg_default_valuel;
	const char *label;
	const char *title;
	void       *widget;
	bool        changed;
};

// Internal data declaration.
// static bool __config_use_capslock          = true;
static bool __config_add_phrase_forward = false;
static bool __config_phrase_choice_rearward = true;
static bool __config_auto_shift_cursor = true;
static bool __config_esc_clean_all_buffer = false;
static bool __config_space_as_selection = true;
// static bool __config_show_candidate_comment= true;
static String __config_kb_type_data;
static String __config_kb_type_data_translated;
static String __config_selKey_type_data;
static String __config_selKey_num_data;
static String __config_chieng_mode_data;
static bool __have_changed                 = false;

// static GtkWidget    * __widget_use_capslock          = 0;
static GtkWidget    * __widget_add_phrase_forward = 0;
static GtkWidget    * __widget_phrase_choice_rearward = 0;
static GtkWidget    * __widget_auto_shift_cursor = 0;
static GtkWidget    * __widget_esc_clean_all_buffer = 0;
static GtkWidget    * __widget_space_as_selection = 0;
static GtkWidget    * __widget_kb_type = 0;
static GtkWidget    * __radio_builtin = 0;
static GtkWidget    * __radio_external = 0;
static GtkWidget    * __file_btn = 0;
static GList *kb_type_list = 0;
static GtkWidget    * __widget_selKey_type = 0;
static GtkWidget    * __widget_selKey_num = 0;
static GtkWidget    * __widget_chieng_mode = 0;
static GList *selKey_type_list = 0;
static GList *selKey_num_list = 0;
static GList *chieng_mode_list = 0;
// static GtkWidget    * __widget_show_candidate_comment= 0;
#ifdef SCIM_TABLES_USE_GTK_TOOLTIPS
static GtkTooltips  * __widget_tooltips              = 0;
#endif

static KeyboardConfigData __config_keyboards[] =
{
    {
        // key
        SCIM_CONFIG_IMENGINE_CHEWING_TRIGGER_KEY,
        // label
        _( "Trigger keys:" ),
        // title
        _( "Select trigger keys" ),
        // tooltip
        _( "The key events to switch Chewing input mode. "
            "Click on the button on the right to edit it." ),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Control+space"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_KEY,
        // label
        _("Chewing CHI/ENG keys:"),
        // title
        _("Select CHI/ENG keys"),
        // tooltip
        _("The key events to switch English and Chinese mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Shift+Shift_L+KeyRelease"
    },
    {
        // key
        NULL,
        // label
        NULL,
        // title
        NULL,
        // tooltip
        NULL,
        // entry
        NULL,
        // button
        NULL,
        // data
        ""
    },
};

/* XXX: It should be a pair of fg/bg colors */
#define FG_COLOR_DEFAULT "#000000"
#define FG_COLOR ""

static ColorConfigData config_color_common[] = {
    {
        FG_COLOR,
        FG_COLOR_DEFAULT,
        FG_COLOR_DEFAULT,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_1",
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_1,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_1,
        _("Color #1"),
        _("The color of preediting text"),
        NULL,
        false
    },

    {
        FG_COLOR,
        FG_COLOR_DEFAULT,
        FG_COLOR_DEFAULT,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_2",
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_2,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_2,
        _("Color #2"),
        _("The color of preediting text"),
        NULL,
        false
    },

    {
        FG_COLOR,
        FG_COLOR_DEFAULT,
        FG_COLOR_DEFAULT,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_3",
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_3,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_3,
        _("Color #3"),
        _("The color of preediting text"),
        NULL,
        false
    },

    {
        FG_COLOR,
        FG_COLOR_DEFAULT,
        FG_COLOR_DEFAULT,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_4",
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_4,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_4,
        _("Color #4"),
        _("The color of preediting text"),
        NULL,
        false
    },

    {
        FG_COLOR,
        FG_COLOR_DEFAULT,
        FG_COLOR_DEFAULT,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_5",
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_5,
        SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_5,
        _("Color #5"),
        _("The color of preediting text"),
        NULL,
        false
    }
};

// Declaration of internal functions.
static void on_default_editable_changed(
	GtkEditable *editable,
	gpointer user_data );

static void on_default_toggle_button_toggled(
	GtkToggleButton *togglebutton,
	gpointer user_data );

static void on_default_key_selection_clicked(
	GtkButton *button,
	gpointer user_data );

static GtkWidget *create_color_button (const char *config_key);

static void on_color_button_changed(
	ScimChewingColorButton *button,
	gpointer user_data );

static void setup_widget_value();

static GtkWidget *create_options_page();

static GtkWidget *create_keyboard_page();

// Function implementations.
static GtkWidget *create_options_page()
{
	GtkWidget *vbox;

#ifdef SCIM_TABLES_USE_GTK_BOX
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
#else
	vbox = gtk_vbox_new (FALSE, 0);
#endif
	gtk_widget_show (vbox);

	__widget_add_phrase_forward =
		gtk_check_button_new_with_mnemonic( _( "Add _Phrase forward" ) );
	gtk_widget_show( __widget_add_phrase_forward );
	gtk_box_pack_start( GTK_BOX( vbox ), __widget_add_phrase_forward, FALSE, FALSE, 4 );
	gtk_container_set_border_width( GTK_CONTAINER( __widget_add_phrase_forward ), 4 );

	g_signal_connect(
			(gpointer) __widget_add_phrase_forward, "toggled",
			G_CALLBACK( on_default_toggle_button_toggled ),
			&__config_add_phrase_forward );

	const gchar *show_add_phrase_forward_tooltip =
		_( "Whether to add Phrase forward or not." );
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			__widget_add_phrase_forward,
			show_add_phrase_forward_tooltip);
#else
	gtk_tooltips_set_tip(
			__widget_tooltips, __widget_add_phrase_forward,
			show_add_phrase_forward_tooltip, NULL);
#endif

	__widget_phrase_choice_rearward =
		gtk_check_button_new_with_mnemonic( _( "_Rearward phrase choice" ) );
	gtk_widget_show( __widget_phrase_choice_rearward );
	gtk_box_pack_start( GTK_BOX( vbox ), __widget_phrase_choice_rearward, FALSE, FALSE, 4 );
	gtk_container_set_border_width( GTK_CONTAINER( __widget_phrase_choice_rearward ), 4 );

	g_signal_connect(
			(gpointer) __widget_phrase_choice_rearward, "toggled",
			G_CALLBACK( on_default_toggle_button_toggled ),
			&__config_phrase_choice_rearward );

	const gchar *show_phrase_choice_rearward_tooltip =
		_( "The behavior for phrase choice to be rearward or not." );
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			__widget_phrase_choice_rearward,
			show_phrase_choice_rearward_tooltip);
#else
	gtk_tooltips_set_tip(
			__widget_tooltips, __widget_phrase_choice_rearward,
			show_phrase_choice_rearward_tooltip, NULL);
#endif

	__widget_auto_shift_cursor =
		gtk_check_button_new_with_mnemonic( _( "_Automatically shift cursor" ) );
	gtk_widget_show( __widget_auto_shift_cursor );
	gtk_box_pack_start( GTK_BOX( vbox ), __widget_auto_shift_cursor, FALSE, FALSE, 4 );
	gtk_container_set_border_width( GTK_CONTAINER( __widget_auto_shift_cursor ), 4 );

	g_signal_connect(
			(gpointer) __widget_auto_shift_cursor, "toggled",
			G_CALLBACK( on_default_toggle_button_toggled ),
			&__config_auto_shift_cursor );

	const gchar *show_auto_shift_cursor_tooltip =
		( "Automatically shift cursor after selection." );
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			__widget_auto_shift_cursor,
			show_auto_shift_cursor_tooltip);
#else
	gtk_tooltips_set_tip(
			__widget_tooltips, __widget_auto_shift_cursor,
			show_auto_shift_cursor_tooltip, NULL);
#endif

	__widget_esc_clean_all_buffer =
		gtk_check_button_new_with_mnemonic(_( "_Esc key to clean all buffer" ) );
	gtk_widget_show( __widget_esc_clean_all_buffer );
	gtk_box_pack_start( GTK_BOX( vbox ), __widget_esc_clean_all_buffer, FALSE, FALSE, 4 );
	gtk_container_set_border_width( GTK_CONTAINER( __widget_esc_clean_all_buffer ), 4);
	
	g_signal_connect(
			(gpointer) __widget_esc_clean_all_buffer, "toggled",
			G_CALLBACK( on_default_toggle_button_toggled ),
			&__config_esc_clean_all_buffer );

	const gchar *show_esc_clean_all_buffer_tooltip =
		_( "Assign Esc key to clean all keyboard buffer or not." );
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			__widget_esc_clean_all_buffer,
			show_esc_clean_all_buffer_tooltip);
#else
	gtk_tooltips_set_tip(
			__widget_tooltips, __widget_esc_clean_all_buffer,
			show_esc_clean_all_buffer_tooltip, NULL);
#endif

	__widget_space_as_selection = 
		gtk_check_button_new_with_mnemonic( _( "_SpaceKey as selection key" ) );
	gtk_widget_show( __widget_space_as_selection );
	gtk_box_pack_start( GTK_BOX( vbox ), __widget_space_as_selection, FALSE, FALSE, 4 );
	gtk_container_set_border_width( GTK_CONTAINER( __widget_space_as_selection ), 4 );

	g_signal_connect(
			(gpointer) __widget_space_as_selection, "toggled",
			G_CALLBACK( on_default_toggle_button_toggled ),
			&__config_space_as_selection );

	const gchar *show_space_as_selection_tooltip =
		_( "Whether SpaceKey is used as selection key or not." );
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			__widget_space_as_selection,
			show_space_as_selection_tooltip);
#else
	gtk_tooltips_set_tip(
			__widget_tooltips, __widget_space_as_selection,
			show_space_as_selection_tooltip, NULL);
#endif

	return vbox;
}

struct _builtin_keymap {
	const char *entry;
	String translated_name;
} builtin_keymaps[] = {
		{ 
			"KB_DEFAULT",
			String( _( "Default Keyboard" ) ) },
		{       
			"KB_HSU",
			String( _( "Hsu's Keyboard" ) ) },
		{
			"KB_IBM",
			String( _( "IBM Keyboard" ) ) },
		{       
			"KB_GIN_YEIH",
			String( _( "Gin-Yieh Keyboard" ) ) },
		{
			"KB_ET",
			String( _( "ETen Keyboard" ) ) },
		{
			"KB_ET26",
			String( _( "ETen 26-key Keyboard" ) ) },
		{
			"KB_DVORAK",
			String( _( "Dvorak Keyboard" ) ) },
		{
			"KB_DVORAK_HSU",
			String( _( "Dvorak Keyboard with Hsu's support" ) ) },
		{
			"KB_DACHEN_CP26",
			String( _( "DACHEN_CP26 Keyboard") ) },
		{
			"KB_HANYU_PINYIN",
			String( _( "Han-Yu PinYin Keyboard" ) ) },
		{
			"KB_THL",
			String( _( "Taiwan Huayu Luomapinyin Keyboard" ) ) },
		{
			"KB_MPS2",
			String( _( "MPS2 PinYin Keyboard"  ) ) },
};

static const char *builtin_selectkeys[] = {
	SCIM_CONFIG_IMENGINE_CHEWING_SELECTION_KEYS,
	"asdfghjkl;",
	"asdfzxcv89",
	"asdfjkl789",
	"aoeuhtn789",
	"1234qweras",
};

static const char *builtin_selectkeys_num[] = {
	"10",
	"9",
	"8",
	"7",
	"6",
	"5"
};

static const struct _builtin_chieng_mode {
	const char *entry;
	const char *translated_name;
} builtin_chieng_mode[] = {
	{ "Chi", _( "Chi" ) },
	{ "Eng", _( "Eng" ) }
};

static GtkWidget *create_keyboard_page()
{
	GtkWidget *table;
	GtkWidget *label;

	table = gtk_table_new (6, 3, FALSE);
	gtk_widget_show (table);

	size_t i;
	// Create keyboard setting.
	for (i = 0; __config_keyboards [i].key; ++ i) {
		label = gtk_label_new (NULL);
		gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _(__config_keyboards[i].label));
		gtk_widget_show (label);
		gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
		gtk_misc_set_padding (GTK_MISC (label), 4, 0);
		gtk_table_attach (GTK_TABLE (table), label, 0, 1, i, i+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (GTK_FILL), 4, 4);

		__config_keyboards [i].entry = gtk_entry_new ();
		gtk_widget_show (__config_keyboards [i].entry);
		gtk_table_attach (GTK_TABLE (table), __config_keyboards [i].entry, 1, 2, i, i+1,
				(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
				(GtkAttachOptions) (GTK_FILL), 4, 4);
		gtk_editable_set_editable (GTK_EDITABLE (__config_keyboards[i].entry), FALSE);

		__config_keyboards[i].button = gtk_button_new_with_label ("...");
		gtk_widget_show (__config_keyboards[i].button);
		gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].button, 2, 3, i, i+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (GTK_FILL), 4, 4);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_keyboards[i].button);
	}

	// keyboard: trigger keys
	for (i = 0; __config_keyboards [i].key; ++ i) {
		g_signal_connect ((gpointer) __config_keyboards [i].button, "clicked",
				G_CALLBACK (on_default_key_selection_clicked),
				&(__config_keyboards [i]));
		g_signal_connect ((gpointer) __config_keyboards [i].entry, "changed",
				G_CALLBACK (on_default_editable_changed),
				&(__config_keyboards [i].data));
	}

	for (i = 0; __config_keyboards [i].key; ++ i) {
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
		gtk_widget_set_tooltip_text(__config_keyboards [i].entry,
				_(__config_keyboards [i].tooltip));
#else
		gtk_tooltips_set_tip (__widget_tooltips, __config_keyboards [i].entry,
				_(__config_keyboards [i].tooltip), NULL);
#endif
	}

#if GTK_CHECK_VERSION(2, 4, 0)
	__widget_chieng_mode = gtk_combo_box_text_new_with_entry();
#else
	// Setup chieng_mode combo box
	__widget_chieng_mode = gtk_combo_new();
#endif
	gtk_widget_show (__widget_chieng_mode);

	for (i = 0; 
			i < (sizeof(builtin_chieng_mode) / sizeof(builtin_chieng_mode[0])); 
			i++) {
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(__widget_chieng_mode),
				builtin_chieng_mode[ i ].translated_name );
#else
		chieng_mode_list = g_list_append(
				chieng_mode_list,
				(void *) builtin_chieng_mode[ i ].translated_name );
#endif
	}

#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_editable_set_editable (
		GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (__widget_chieng_mode))),
		FALSE);
#else
	gtk_combo_set_popdown_strings (GTK_COMBO (__widget_chieng_mode), chieng_mode_list);
	g_list_free(chieng_mode_list);
	gtk_combo_set_use_arrows (GTK_COMBO (__widget_chieng_mode), TRUE);
	gtk_editable_set_editable (GTK_EDITABLE (GTK_ENTRY (GTK_COMBO (__widget_chieng_mode)->entry)), FALSE);
#endif
	label = gtk_label_new (_("Initial trigger Chinese/English mode:"));
	gtk_widget_show (label);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 4, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 4, 4);
	gtk_table_attach (GTK_TABLE (table), __widget_chieng_mode, 1, 2, 2, 3,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (GTK_FILL), 4, 4);

	const gchar *show_chieng_mode_tooltip =
		_("Change the default Chinese/English mode on every trigger");
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(
			gtk_bin_get_child (GTK_BIN (__widget_chieng_mode)),
			show_chieng_mode_tooltip);
#else
	gtk_tooltips_set_tip (__widget_tooltips, GTK_COMBO (__widget_chieng_mode)->entry,
			show_chieng_mode_tooltip, NULL);
#endif
	g_signal_connect(
#if GTK_CHECK_VERSION(2, 4, 0)
			G_OBJECT (gtk_bin_get_child (GTK_BIN (__widget_chieng_mode))),
#else
			(gpointer) GTK_ENTRY(GTK_COMBO(__widget_chieng_mode)->entry), 
#endif
			"changed",
			G_CALLBACK (on_default_editable_changed),
			&(__config_chieng_mode_data));

#if GTK_CHECK_VERSION(2, 4, 0)
	__widget_selKey_num = gtk_combo_box_text_new_with_entry();
#else
	// Setup selKey_num combo box
	__widget_selKey_num = gtk_combo_new();
#endif
	gtk_widget_show (__widget_selKey_num);

	for (i = 0; 
	     i < (sizeof(builtin_selectkeys_num) / sizeof(builtin_selectkeys_num[0])); 
	     i++) {
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(__widget_selKey_num),
				builtin_selectkeys_num[ i ] );
#else
		selKey_num_list = g_list_append(
				selKey_num_list,
				(void *) builtin_selectkeys_num[ i ] );
#endif
	}

#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_editable_set_editable (
		GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (__widget_selKey_num))),
		FALSE);
#else
	gtk_combo_set_popdown_strings (GTK_COMBO (__widget_selKey_num), selKey_num_list);
	g_list_free(selKey_num_list);
	gtk_combo_set_use_arrows (GTK_COMBO (__widget_selKey_num), TRUE);
	gtk_editable_set_editable (GTK_EDITABLE (GTK_ENTRY (GTK_COMBO (__widget_selKey_num)->entry)), FALSE);
#endif
	label = gtk_label_new (_("Number of Selection Keys :"));
	gtk_widget_show (label);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 4, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 4, 4);
	gtk_table_attach (GTK_TABLE (table), __widget_selKey_num, 1, 2, 3, 4,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (GTK_FILL), 4, 4);

	const gchar *show_selKey_num_tooltip =
		_("Change the default number of selection keys");
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(gtk_bin_get_child (GTK_BIN (__widget_selKey_num)),
			show_selKey_num_tooltip);
#else
	gtk_tooltips_set_tip (__widget_tooltips, GTK_COMBO (__widget_selKey_num)->entry,
			show_selKey_num_tooltip, NULL);
#endif
	g_signal_connect(
#if GTK_CHECK_VERSION(2, 4, 0)
			G_OBJECT(gtk_bin_get_child(GTK_BIN(__widget_selKey_num))),
#else
			(gpointer) GTK_ENTRY(GTK_COMBO(__widget_selKey_num)->entry), 
#endif
			"changed",
			G_CALLBACK (on_default_editable_changed),
			&(__config_selKey_num_data));

#if GTK_CHECK_VERSION(2, 4, 0)
	__widget_selKey_type = gtk_combo_box_text_new_with_entry();
#else
	// Setup selKey combo box
	__widget_selKey_type = gtk_combo_new();
#endif
	gtk_widget_show (__widget_selKey_type);

	for (i = 0;
	     i < (sizeof(builtin_selectkeys) / sizeof(builtin_selectkeys[0]));
	     i++) {
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(__widget_selKey_type),
				builtin_selectkeys[ i ] );
#else
		selKey_type_list = g_list_append(
				selKey_type_list,
				(void *) builtin_selectkeys[ i ] );
#endif
	}

#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_editable_set_editable (GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (__widget_selKey_type))), FALSE);
#else
	gtk_combo_set_popdown_strings (GTK_COMBO (__widget_selKey_type), selKey_type_list);
	g_list_free(selKey_type_list);
	gtk_combo_set_use_arrows (GTK_COMBO (__widget_selKey_type), TRUE);
	gtk_editable_set_editable (GTK_EDITABLE (GTK_ENTRY (GTK_COMBO (__widget_selKey_type)->entry)), FALSE);
#endif
	label = gtk_label_new (_("Customized Selection Keys:"));
	gtk_widget_show (label);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 4, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 4, 4);
	gtk_table_attach (GTK_TABLE (table), __widget_selKey_type, 1, 2, 4, 5,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (GTK_FILL), 4, 4);

	const gchar *show_selKey_type_tooltip =
		_("Change the default selection keys");
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(gtk_bin_get_child (GTK_BIN (__widget_selKey_type)),
			show_selKey_type_tooltip);
#else
	gtk_tooltips_set_tip (__widget_tooltips, GTK_COMBO (__widget_selKey_type)->entry,
			show_selKey_type_tooltip, NULL);
#endif
	g_signal_connect(
#if GTK_CHECK_VERSION(2, 4, 0)
			G_OBJECT(gtk_bin_get_child(GTK_BIN(__widget_selKey_type))),
#else
			(gpointer) GTK_ENTRY(GTK_COMBO(__widget_selKey_type)->entry),
#endif
			"changed",
			G_CALLBACK (on_default_editable_changed),
			&(__config_selKey_type_data));

#if GTK_CHECK_VERSION(2, 4, 0)
	__widget_kb_type = gtk_combo_box_text_new_with_entry();
#else
	// Setup KB_TYPE combo box
	__widget_kb_type = gtk_combo_new();
#endif
	gtk_widget_show (__widget_kb_type);

	for (i = 0;
	     i < (int) (sizeof(builtin_keymaps) / sizeof(_builtin_keymap));
	     i++) {
#if GTK_CHECK_VERSION(2, 4, 0)
		gtk_combo_box_text_append_text(
				GTK_COMBO_BOX_TEXT(__widget_kb_type), 
				builtin_keymaps[ i ].translated_name.c_str() );
#else
		kb_type_list = g_list_append(
				kb_type_list,
				(void *) builtin_keymaps[ i ].translated_name.c_str() );
#endif
	}

#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_editable_set_editable (GTK_EDITABLE (gtk_bin_get_child (GTK_BIN (__widget_kb_type))), FALSE);
#else
	gtk_combo_set_popdown_strings (GTK_COMBO (__widget_kb_type), kb_type_list);
	g_list_free(kb_type_list);
	gtk_combo_set_use_arrows (GTK_COMBO (__widget_kb_type), TRUE);
	gtk_editable_set_editable (GTK_EDITABLE (GTK_ENTRY (GTK_COMBO (__widget_kb_type)->entry)), FALSE);
#endif

	label = gtk_label_new (_("Use keyboard type:"));
	gtk_widget_show (label);
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
	gtk_misc_set_padding (GTK_MISC (label), 4, 0);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 5, 6,
			(GtkAttachOptions) (GTK_FILL),
			(GtkAttachOptions) (GTK_FILL), 4, 4);
	gtk_table_attach (GTK_TABLE (table), __widget_kb_type, 1, 2, 5, 6,
			(GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			(GtkAttachOptions) (GTK_FILL), 4, 4);

	const gchar *show_kb_type_tooltip =
		_("Change the default keyboard layout type");
#ifndef SCIM_TABLES_USE_GTK_TOOLTIPS
	gtk_widget_set_tooltip_text(gtk_bin_get_child (GTK_BIN (__widget_kb_type)),
			show_kb_type_tooltip);
#else
	gtk_tooltips_set_tip (__widget_tooltips, GTK_COMBO (__widget_kb_type)->entry,
			show_kb_type_tooltip, NULL);
#endif
	g_signal_connect(
#if GTK_CHECK_VERSION(2, 4, 0)
			G_OBJECT(gtk_bin_get_child(GTK_BIN(__widget_kb_type))),
#else
			(gpointer) GTK_ENTRY(GTK_COMBO(__widget_kb_type)->entry),
#endif
			"changed",
			G_CALLBACK (on_default_editable_changed),
			&(__config_kb_type_data_translated));

	return table;
}

static GtkWidget *create_color_button_page()
{               
	GtkWidget *widget;
	GtkWidget *hbox;
	GtkWidget *table;
	char color_button_name_string[64] = { 0 };
	table = gtk_table_new (4, 5, FALSE);
	gtk_widget_show (table);

	for (int i = 0; i < SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_NUM; i++) {
#ifdef SCIM_TABLES_USE_GTK_BOX
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
		hbox = gtk_hbox_new (FALSE, 0);
#endif
		gtk_widget_show (hbox);
		sprintf(color_button_name_string, 
			SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_%d", i + 1);
		widget = create_color_button (color_button_name_string);
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		gtk_table_attach (GTK_TABLE (table), hbox, 4, 5, i, i + 1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (GTK_FILL), 5, 5);
	}
	
	return table;
}

static GtkWidget *create_setup_window()
{
	static GtkWidget *window = 0;

	if (!window) {
		GtkWidget *notebook;
		GtkWidget *label;
		GtkWidget *page;

#ifdef SCIM_TABLES_USE_GTK_TOOLTIPS
		__widget_tooltips = gtk_tooltips_new ();
#endif

		// Create the Notebook.
		notebook = gtk_notebook_new ();
		gtk_widget_show (notebook);

		// Create the first page.
		page = create_options_page ();
		gtk_container_add (GTK_CONTAINER (notebook), page);

		// Create the label for this note page.
		label = gtk_label_new (_("Options"));
		gtk_widget_show (label);
		gtk_notebook_set_tab_label(
			GTK_NOTEBOOK( notebook ),
			gtk_notebook_get_nth_page( GTK_NOTEBOOK( notebook ), 0 ), 
			label );

		// Create the second page.
		page = create_keyboard_page ();

		// Create the label for this note page.
		label = gtk_label_new (_("Keyboard"));
		gtk_widget_show (label);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		// Create the third page.
		page = create_color_button_page ();

		// Create the label for this note page.
		label = gtk_label_new (_("Decorative Color"));
		gtk_widget_show (label);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		window = notebook;

		setup_widget_value ();
	}

	return window;
}

void setup_widget_value()
{
	if ( __widget_add_phrase_forward ) {
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON( __widget_add_phrase_forward ),
				__config_add_phrase_forward);
	}

	if ( __widget_phrase_choice_rearward ) {
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON( __widget_phrase_choice_rearward ),
				__config_phrase_choice_rearward );
	}

	if ( __widget_auto_shift_cursor ) {
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON( __widget_auto_shift_cursor ),
				__config_auto_shift_cursor );
	}

	if ( __widget_space_as_selection ) {
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON( __widget_space_as_selection ),
				__config_space_as_selection );
	}

	if ( __widget_esc_clean_all_buffer ) {
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON( __widget_esc_clean_all_buffer ),
				__config_esc_clean_all_buffer );
	}

	
	for (int i = 0; __config_keyboards [i].key; ++ i) {
		if (__config_keyboards [i].entry) {
			gtk_entry_set_text (
					GTK_ENTRY (__config_keyboards [i].entry),
					__config_keyboards [i].data.c_str ());
		}
	}

	for (unsigned int i = 0;
	     i < (sizeof(config_color_common) / sizeof((config_color_common)[0])); i++) {
		ColorConfigData &entry = config_color_common[i];
		if (entry.widget) {
			scim_color_button_set_colors (
				SCIM_COLOR_BUTTON (entry.widget),
				entry.fg_value, 
				entry.bg_value);
		}
	}

	/* KB_TYPE */
	int index_keymap = (sizeof(builtin_keymaps) / sizeof(_builtin_keymap)) - 1;
	for ( ; index_keymap >= 0;  index_keymap--) {
		if ( __config_kb_type_data == builtin_keymaps[index_keymap].entry ) {
			break;
		}
	}
	if (index_keymap < 0)
		index_keymap = 0;
	
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_combo_box_set_active(GTK_COMBO_BOX(__widget_kb_type), index_keymap);
#else
	gtk_entry_set_text (
			GTK_ENTRY(GTK_COMBO(__widget_kb_type)->entry),
			builtin_keymaps[index_keymap].translated_name.c_str()
	);
#endif

	/* selKey */
	int index_selectkeys = sizeof(builtin_selectkeys) / sizeof(builtin_selectkeys[0]) - 1;
	for ( ; index_selectkeys >= 0;  index_selectkeys--) {
		if ( __config_selKey_type_data ==
		     builtin_selectkeys[index_selectkeys]) {
			break;
		}
	}
	if (index_selectkeys < 0)
		index_selectkeys = 0;
	
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_combo_box_set_active(GTK_COMBO_BOX(__widget_selKey_type), index_selectkeys);
#else
	gtk_entry_set_text (
		GTK_ENTRY(GTK_COMBO(__widget_selKey_type)->entry),
		builtin_selectkeys[index_selectkeys]
	);
#endif

	/* selKey_num */
	int index_selectkeys_num =
		sizeof(builtin_selectkeys_num) / sizeof(builtin_selectkeys_num[0]) - 1;
	for ( ; index_selectkeys_num >= 0;  index_selectkeys_num--) {
		if ( __config_selKey_num_data ==
			builtin_selectkeys_num[index_selectkeys_num]) {
			break;
		}
	}
	if (index_selectkeys_num < 0)
		index_selectkeys_num = 0;
	
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_combo_box_set_active(GTK_COMBO_BOX(__widget_selKey_num), index_selectkeys_num);
#else
	gtk_entry_set_text (
		GTK_ENTRY(GTK_COMBO(__widget_selKey_num)->entry),
		builtin_selectkeys_num[index_selectkeys_num]
	);
#endif

	/* chieng_mode */
	int index_chieng_mode =
		sizeof(builtin_chieng_mode) / sizeof(builtin_chieng_mode[0]) - 1;
	for ( ; index_chieng_mode >= 0;  index_chieng_mode--) {
		if ( __config_chieng_mode_data ==
			builtin_chieng_mode[index_chieng_mode].entry) {
			break;
		}
	}
	if (index_chieng_mode < 0)
		index_chieng_mode = 0;
	
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_combo_box_set_active(GTK_COMBO_BOX(__widget_chieng_mode), index_chieng_mode);
#else
	gtk_entry_set_text (
		GTK_ENTRY(GTK_COMBO(__widget_chieng_mode)->entry),
		builtin_chieng_mode[index_chieng_mode].translated_name
	);
#endif
}

void load_config( const ConfigPointer &config )
{
	if (!config.null ()) {
		__config_add_phrase_forward =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD ),
				__config_add_phrase_forward );

		__config_phrase_choice_rearward =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_PHRASE_CHOICE_REARWARD ),
				__config_phrase_choice_rearward );

		__config_auto_shift_cursor =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_AUTO_SHIFT_CURSOR ),
				__config_auto_shift_cursor );

		__config_esc_clean_all_buffer =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_ESC_CLEAN_ALL_BUFFER ),
				__config_esc_clean_all_buffer );

		__config_space_as_selection =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_SPACE_AS_SELECTION ),
				__config_space_as_selection );

		__config_kb_type_data = 
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE ),
				__config_kb_type_data);

		__config_selKey_type_data =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS ),
					__config_selKey_type_data);

		__config_selKey_num_data =
			config->read( String( SCIM_CHEWING_SELECTION_KEYS_NUM ),
					__config_selKey_num_data);

		__config_chieng_mode_data =
			config->read( String( SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_MODE ),
					__config_chieng_mode_data);


		for (int i = 0; __config_keyboards[ i ].key; ++ i) {
			__config_keyboards[ i ].data =
				config->read( String( __config_keyboards [ i ].key ),
						__config_keyboards[ i ].data);
		}

		for (unsigned int i = 0;
		     i < (sizeof(config_color_common) / sizeof((config_color_common)[0])); i++) {
			ColorConfigData &entry = config_color_common[i];
			entry.bg_value = config->read (String (entry.bg_key), entry.bg_value);
		}

		setup_widget_value ();

		__have_changed = false;
	}
}

void save_config( const ConfigPointer &config )
{
	if ( ! config.null() ) {
		config->write( String( SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD ),
				__config_add_phrase_forward );

		config->write( String( SCIM_CONFIG_IMENGINE_CHEWING_PHRASE_CHOICE_REARWARD ),
				__config_phrase_choice_rearward );

		config->write( String( SCIM_CONFIG_IMENGINE_CHEWING_AUTO_SHIFT_CURSOR ),
				__config_auto_shift_cursor );

		config->write( String( SCIM_CONFIG_IMENGINE_CHEWING_ESC_CLEAN_ALL_BUFFER ),
				__config_esc_clean_all_buffer );

		config->write( String( SCIM_CONFIG_IMENGINE_CHEWING_SPACE_AS_SELECTION ),
				__config_space_as_selection );

		// SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE
		int index_keymap = 
			(sizeof(builtin_keymaps) / sizeof(_builtin_keymap)) - 1;
		for ( ; index_keymap >= 0;  index_keymap--) {
			if (__config_kb_type_data_translated == 
				builtin_keymaps[index_keymap].translated_name ) {
				break;
			}
		}
		if (index_keymap < 0)
			index_keymap = 0;
		__config_kb_type_data = builtin_keymaps[index_keymap].entry;

		config->write (String (SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE),
			__config_kb_type_data);

		// SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS
		int index_selectkeys =
			sizeof(builtin_selectkeys) / sizeof(builtin_selectkeys[0]) - 1;
		for ( ; index_selectkeys >= 0; index_selectkeys--) {
			if (__config_selKey_type_data ==
			    builtin_selectkeys[index_selectkeys]) {
				break;
			}
		}
		if (index_selectkeys < 0)
			index_selectkeys = 0;
		__config_selKey_type_data =
			builtin_selectkeys[index_selectkeys];

		config->write (String (SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS),
				__config_selKey_type_data);

		// SCIM_CHEWING_SELECTION_KEYS_NUM
		int index_selectkeys_num =
			sizeof(builtin_selectkeys_num) / sizeof(builtin_selectkeys_num[0]) - 1;
		for ( ; index_selectkeys_num >= 0; index_selectkeys_num--) {
			if (__config_selKey_num_data ==
			    builtin_selectkeys_num[index_selectkeys_num]) {
				break;
			}
		}
		if (index_selectkeys_num < 0)
			index_selectkeys_num = 0;
		__config_selKey_num_data =
			builtin_selectkeys_num[index_selectkeys_num];

		config->write (String (SCIM_CHEWING_SELECTION_KEYS_NUM),
		               __config_selKey_num_data);

		// SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_MODE
		int index_chieng_mode =
			sizeof(builtin_chieng_mode) / sizeof(builtin_chieng_mode[0]) - 1;
		for ( ; index_chieng_mode >= 0; index_chieng_mode--) {
			if (__config_chieng_mode_data ==
			    builtin_chieng_mode[index_chieng_mode].entry ||
			    __config_chieng_mode_data ==
			    builtin_chieng_mode[index_chieng_mode].translated_name
			    ) {
				break;
			}
		}
		if (index_chieng_mode < 0)
			index_chieng_mode = 0;
		__config_chieng_mode_data =
			builtin_chieng_mode[index_chieng_mode].entry;

		config->write (String (SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_MODE)		               , __config_chieng_mode_data);

		for (int i = 0; __config_keyboards [i].key; ++ i) {
			config->write (String (__config_keyboards [i].key),
					__config_keyboards [i].data);
		}

		for (unsigned int i = 0;
		     i < (sizeof(config_color_common) / sizeof((config_color_common)[0])); i++) {
			ColorConfigData &entry = config_color_common[i];
			if (entry.changed) {
				entry.bg_value = config->write (String (entry.bg_key),
						entry.bg_value);
			}
			entry.changed = false;
		}


		__have_changed = false;
	}
}

bool query_changed()
{
	return __have_changed;
}

static void on_default_editable_changed(
		GtkEditable *editable,
		gpointer user_data )
{
	String *str = static_cast< String * >( user_data );

	if ( str ) {
		*str = String( gtk_entry_get_text( GTK_ENTRY( editable ) ) );
		__have_changed = true;
	}
}

static void on_default_toggle_button_toggled(
		GtkToggleButton *togglebutton,
		gpointer user_data )
{
	bool *toggle = static_cast< bool * >( user_data );

	if ( toggle ) {
		*toggle = gtk_toggle_button_get_active( togglebutton );
		__have_changed = true;
	}
}

static void on_default_key_selection_clicked(
		GtkButton *button,
		gpointer user_data )
{
	KeyboardConfigData *data = static_cast< KeyboardConfigData * >( user_data );

	if ( data ) {
		GtkWidget *dialog = scim_key_selection_dialog_new (_(data->title));
		gint result;

		scim_key_selection_dialog_set_keys (
				SCIM_KEY_SELECTION_DIALOG( dialog ),
				gtk_entry_get_text( GTK_ENTRY( data->entry ) ) );

		result = gtk_dialog_run( GTK_DIALOG( dialog ) );

		if ( result == GTK_RESPONSE_OK ) {
			const gchar *keys = scim_key_selection_dialog_get_keys (
				SCIM_KEY_SELECTION_DIALOG( dialog ) );

			if ( ! keys )
				keys = "";

			if ( strcmp( keys, gtk_entry_get_text( GTK_ENTRY( data->entry ) ) ) != 0 )
				gtk_entry_set_text( GTK_ENTRY( data->entry ), keys );
		}

		gtk_widget_destroy( dialog );
	}
}

static ColorConfigData *find_color_config_entry (const char *config_key)
{
	if (!config_key)
		return NULL;

	for (unsigned int i = 0;
	     i < (sizeof(config_color_common) / sizeof((config_color_common)[0])); i++) {
		ColorConfigData *entry = &config_color_common[i];
		if (entry->fg_key && !strcmp (entry->bg_key, config_key))
			return entry;
	}

	return NULL;
}

static GtkWidget *create_color_button (const char *config_key)
{
	ColorConfigData *entry = find_color_config_entry (config_key);
	if (!entry)
		return NULL;

#ifdef SCIM_TABLES_USE_GTK_BOX
	GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
#else
	GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
#endif
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 4);
	gtk_widget_show (hbox);

	GtkWidget *label = NULL;
	if (entry->label) {
		label = gtk_label_new_with_mnemonic (_(entry->label));
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
		gtk_widget_show (label);
	}

	entry->widget = scim_color_button_new ();
	gtk_widget_set_size_request (GTK_WIDGET (entry->widget), 32, 24);
	g_signal_connect (G_OBJECT (entry->widget), "color-changed",
			G_CALLBACK (on_color_button_changed),
			entry);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry->widget),
			FALSE, FALSE, 2);
	gtk_widget_show (GTK_WIDGET (entry->widget));

	if (label) {
		gtk_label_set_mnemonic_widget (GTK_LABEL (label),
				GTK_WIDGET (entry->widget));

#if 0	/* XXX: not functioned. */
		gtk_tooltips_set_tip(
			__widget_tooltips,
			GTK_WIDGET (label),
			_( entry->title ), NULL);
#endif
	}

	return hbox;
}

static void on_color_button_changed(
	ScimChewingColorButton *button,
	gpointer user_data)
{
	ColorConfigData *entry = static_cast <ColorConfigData*> (user_data);

	if (entry->widget) {
		scim_color_button_get_colors (button, &entry->fg_value, &entry->bg_value);
		entry->changed = true;
		__have_changed = true;
	}
}
