#if 0 /* jserv: still hacking */

/** @file scim_chewing_imengine_setup.cpp
 * implementation of Setup Module of chewing imengine module.
 */

/*
 * SCIM-chewing -
 *	Intelligent Chinese Phonetic IM Engine for SCIM.
 *
 * Copyright (c) 2004
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


#define SCIM_CONFIG_IMENGINE_HANGUL_USE_CAPSLOCK                "/IMEngine/Chewing/UseCapslock"
#define SCIM_CONFIG_IMENGINE_HANGUL_USE_DVORAK                  "/IMEngine/Chewing/UseDvorak"
#define SCIM_CONFIG_IMENGINE_HANGUL_SHOW_CANDIDATE_COMMENT      "/IMEngine/Chewing/ShowCandidateComment"
#define SCIM_CONFIG_IMENGINE_HANGUL_TRIGGER_KEY                 "/IMEngine/Chewing/TriggerKey"
#define SCIM_CONFIG_IMENGINE_HANGUL_HANGUL_HANJA_KEY            "/IMEngine/Chewing/HangulHanjaKey"
#define SCIM_CONFIG_IMENGINE_HANGUL_MANUAL_MODE_KEY             "/IMEngine/Chewing/ManualModeKey"


static GtkWidget * create_setup_window();
static void load_config( const ConfigPointer &config );
static void save_config( const ConfigPointer &config );
static bool query_changed();

// Module Interface.
extern "C" {

	void scim_module_init()
	{
		bindtextdomain( GETTEXT_PACKAGE, SCIM_HANGUL_LOCALEDIR );
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
		return String( _( "A Intelligent Chinese Phonetic IMEngine Module." ) );
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

// Internal data declaration.
static bool __config_use_capslock          = true;
static bool __config_use_dvorak            = false;
static bool __config_show_candidate_comment= true;

static bool __have_changed                 = false;

static GtkWidget    * __widget_use_capslock          = 0;
static GtkWidget    * __widget_use_dvorak            = 0;
static GtkWidget    * __widget_show_candidate_comment= 0;
static GtkTooltips  * __widget_tooltips              = 0;

static KeyboardConfigData __config_keyboards[] =
{
    {
        // key
        SCIM_CONFIG_IMENGINE_CHEWING_TRIGGER_KEY,
        // label
        N_( "Trigger keys:" ),
        // title
        N_( "Select trigger keys" ),
        // tooltip
        N_( "The key events to switch Chewing input mode. "
            "Click on the button on the right to edit it." ),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Ctrl+space"
    },
    {
        // key
        SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_KEY,
        // label
        N_("Hangul to Hanja keys:"),
        // title
        N_("Select Hangul to Hanja keys"),
        // tooltip
        N_("The key events to switch English and Chinese mode. "
           "Click on the button on the right to edit it."),
        // entry
        NULL,
        // button
        NULL,
        // data
        "Tab"
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

static void setup_widget_value();

static GtkWidget *create_options_page();

static GtkWidget *create_keyboard_page();

// Function implementations.
static GtkWidget *create_options_page()
{
	GtkWidget *vbox;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	__widget_use_capslock = gtk_check_button_new_with_mnemonic (_("Use _CapsLock"));
	gtk_widget_show (__widget_use_capslock);
	gtk_box_pack_start (GTK_BOX (vbox), __widget_use_capslock, FALSE, FALSE, 4);
	gtk_container_set_border_width (GTK_CONTAINER (__widget_use_capslock), 4);

	__widget_use_dvorak = gtk_check_button_new_with_mnemonic (_("Use _Dvorak keyboard"));
	gtk_widget_show (__widget_use_dvorak);
	gtk_box_pack_start (GTK_BOX (vbox), __widget_use_dvorak, FALSE, FALSE, 4);
	gtk_container_set_border_width (GTK_CONTAINER (__widget_use_dvorak), 4);

	__widget_show_candidate_comment = gtk_check_button_new_with_mnemonic (_("_Show candidate comment"));
	gtk_widget_show (__widget_show_candidate_comment);
	gtk_box_pack_start (GTK_BOX (vbox), __widget_show_candidate_comment, FALSE, FALSE, 4);
	gtk_container_set_border_width (GTK_CONTAINER (__widget_show_candidate_comment), 4);

	// Connect all signals.
	g_signal_connect ((gpointer) __widget_use_capslock, "toggled",
			G_CALLBACK (on_default_toggle_button_toggled),
			&__config_use_capslock);
	g_signal_connect ((gpointer) __widget_use_dvorak, "toggled",
			G_CALLBACK (on_default_toggle_button_toggled),
			&__config_use_dvorak);
	g_signal_connect ((gpointer) __widget_show_candidate_comment, "toggled",
			G_CALLBACK (on_default_toggle_button_toggled),
			&__config_show_candidate_comment);

	// Set all tooltips.
	gtk_tooltips_set_tip (__widget_tooltips, __widget_use_capslock,
			_("Whether to use Caps Lock key for changing chewing output mode to Jamo or not."), NULL);

	gtk_tooltips_set_tip (__widget_tooltips, __widget_use_dvorak,
			_("Whether to use dvorak keyboard or not."), NULL);

	gtk_tooltips_set_tip (__widget_tooltips, __widget_show_candidate_comment,
			_("Whether to show the comment of candidates or not."), NULL);

	return vbox;
}

static GtkWidget *create_keyboard_page()
{
	GtkWidget *table;
	GtkWidget *label;

	int i;

	table = gtk_table_new (3, 3, FALSE);
	gtk_widget_show (table);

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
		gtk_entry_set_editable (GTK_ENTRY (__config_keyboards[i].entry), FALSE);

		__config_keyboards[i].button = gtk_button_new_with_label ("...");
		gtk_widget_show (__config_keyboards[i].button);
		gtk_table_attach (GTK_TABLE (table), __config_keyboards[i].button, 2, 3, i, i+1,
				(GtkAttachOptions) (GTK_FILL),
				(GtkAttachOptions) (GTK_FILL), 4, 4);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), __config_keyboards[i].button);
	}

	for (i = 0; __config_keyboards [i].key; ++ i) {
		g_signal_connect ((gpointer) __config_keyboards [i].button, "clicked",
				G_CALLBACK (on_default_key_selection_clicked),
				&(__config_keyboards [i]));
		g_signal_connect ((gpointer) __config_keyboards [i].entry, "changed",
				G_CALLBACK (on_default_editable_changed),
				&(__config_keyboards [i].data));
	}

	for (i = 0; __config_keyboards [i].key; ++ i) {
		gtk_tooltips_set_tip (__widget_tooltips, __config_keyboards [i].entry,
				_(__config_keyboards [i].tooltip), NULL);
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

		__widget_tooltips = gtk_tooltips_new ();

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

		// Append this page.
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

		window = notebook;

		setup_widget_value ();
	}

	return window;
}

void setup_widget_value()
{
	if (__widget_use_capslock) {
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (__widget_use_capslock),
				__config_use_capslock);
	}

	if (__widget_use_dvorak) {
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (__widget_use_dvorak),
				__config_use_dvorak);
	}

	if (__widget_show_candidate_comment) {
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (__widget_show_candidate_comment),
				__config_show_candidate_comment);
	}

	for (int i = 0; __config_keyboards [i].key; ++ i) {
		if (__config_keyboards [i].entry) {
			gtk_entry_set_text (
					GTK_ENTRY (__config_keyboards [i].entry),
					__config_keyboards [i].data.c_str ());
		}
	}
}

void load_config( const ConfigPointer &config )
{
	if (!config.null ()) {
		__config_use_capslock =
			config->read (String (SCIM_CONFIG_IMENGINE_HANGUL_USE_CAPSLOCK),
					__config_use_capslock);
		__config_use_dvorak =
			config->read (String (SCIM_CONFIG_IMENGINE_HANGUL_USE_DVORAK),
					__config_use_dvorak);
		__config_show_candidate_comment =
			config->read (String (SCIM_CONFIG_IMENGINE_HANGUL_SHOW_CANDIDATE_COMMENT),
					__config_show_candidate_comment);

		for (int i = 0; __config_keyboards [i].key; ++ i) {
			__config_keyboards [i].data =
				config->read (String (__config_keyboards [i].key),
						__config_keyboards [i].data);
		}

		setup_widget_value ();

		__have_changed = false;
	}
}

void save_config( const ConfigPointer &config )
{
	if (!config.null ()) {
		config->write (String (SCIM_CONFIG_IMENGINE_HANGUL_USE_CAPSLOCK),
				__config_use_capslock);
		config->write (String (SCIM_CONFIG_IMENGINE_HANGUL_USE_DVORAK),
				__config_use_dvorak);
		config->write (String (SCIM_CONFIG_IMENGINE_HANGUL_SHOW_CANDIDATE_COMMENT),
				__config_show_candidate_comment);

		for (int i = 0; __config_keyboards [i].key; ++ i) {
			config->write (String (__config_keyboards [i].key),
					__config_keyboards [i].data);
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

#endif
