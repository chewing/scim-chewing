/*
 * SCIM-chewing -
 *	Intelligent Chinese Phonetic IM Engine for SCIM.
 *
 * Copyright (c) 2004, 2005, 2006
 *	SCIM-chewing Developers. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_ICONV
#define Uses_SCIM_DEBUG
#define Uses_SCIM_C_STRING

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GETTEXT
  #include <libintl.h>
  #define _(String) dgettext(PACKAGE,String)
  #define N_(String) (String)
#else
  #define _(String) (String)
  #define N_(String) (String)
  #define bindtextdomain(Package,Directory)
  #define textdomain(domain)
  #define bind_textdomain_codeset(domain,codeset)
#endif

#define SCIM_PROP_CHIENG \
	"/IMEngine/Chinese/Chewing/ChiEngMode"
#define SCIM_PROP_LETTER \
	"/IMEngine/Chinese/Chewing/FullHalfLetter"
#define SCIM_PROP_KBTYPE \
	"/IMEngine/Chinese/Chewing/KeyboardType"

#define SCIM_CHEWING_SELECTION_KEYS_NUM_DEF 9
static int _selection_keys_num;

#include <scim.h>
#include <chewing/chewing.h>

#include "scim_chewing_imengine.h"
#include "scim_chewing_config_entry.h"

using namespace scim;

static IMEngineFactoryPointer _scim_chewing_factory( 0 );
static ConfigPointer _scim_config( 0 );

static Property _chieng_property (SCIM_PROP_CHIENG, "");
static Property _letter_property (SCIM_PROP_LETTER, "");
static Property _kbtype_property (SCIM_PROP_KBTYPE, "");
//static Property _punct_property  (SCIM_PROP_PUNCT, _("Full/Half Punct"));

extern "C" {
	void scim_module_init()
	{
		bindtextdomain (GETTEXT_PACKAGE, SCIM_CHEWING_LOCALEDIR);
		bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	}

	void scim_module_exit()
	{
		_scim_config.reset();
		/* New API introduced in libchewing 0.2.7 */
		chewing_Terminate();
	}

	unsigned int scim_imengine_module_init( const ConfigPointer& config )
	{
		_chieng_property.set_tip (_("The status of the current input method. Click to change it."));
		_chieng_property.set_label (_("Eng"));
		_letter_property.set_label (_("Half"));
		_kbtype_property.set_label (_("Default"));
		_scim_config = config;
		return 1;
	}

	IMEngineFactoryPointer scim_imengine_module_create_factory(
			uint32 engine )
	{
		if ( engine != 0 )
			return IMEngineFactoryPointer( 0 );

		if ( _scim_chewing_factory.null() ) {
			ChewingIMEngineFactory *factory =
				new ChewingIMEngineFactory( _scim_config );
			if ( factory && factory->valid() )
				_scim_chewing_factory = factory;
			else
				delete factory;
		}
		return _scim_chewing_factory;
	}
}

/**
 * Implementation of ChewingIMEngineFactory.
 */

ChewingIMEngineFactory::ChewingIMEngineFactory( const ConfigPointer& config )
	: m_config( config ),
	  m_valid( false )
{
	reload_config( config );
	set_languages( "zh_TW,zh_HK,zh_SG" );
	m_valid = init();
	m_reload_signal_connection = m_config->signal_connect_reload (
			slot (this, &ChewingIMEngineFactory::reload_config));
}

bool ChewingIMEngineFactory::init()
{
	char prefix[] = CHEWING_DATADIR;
	char hash_postfix[] = "/.chewing/";

	chewing_Init(prefix, (char *)(scim_get_home_dir() + hash_postfix).c_str() );
	return true;
}

static char *chewing_preedit_bgcolor[] = {
	SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_1,
	SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_2,
	SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_3,
	SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_4,
	SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_DEF_5
};

void ChewingIMEngineFactory::reload_config( const ConfigPointer &scim_config )
{
	String str;
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "ReloadConfig\n";
	// Load Chi/Eng mode keys
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Load Chi/Eng mode keys\n";
	str = m_config->read(
			String( SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_KEY ),
			String( "ShiftShift_LKeyRelease" ) +
			String( "ShiftShift_RKeyRelease" ) );
	scim_string_to_key_list( m_chi_eng_keys, str );

	// Load keyboard type
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Load keyboard type\n";
	m_KeyboardType = m_config->read (
		String( SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE ),
		String( "KB_DEFAULT" ));

	// Load PinYin method Type
	m_PinYinType = m_config->read (
		String( SCIM_CONFIG_IMENGINE_CHEWING_PINYIN_METHD),
		PINYIN_HANYU);

	m_ExternPinYinPath = m_config->read (
		String( SCIM_CONFIG_IMENGINE_CHEWING_EXTERNAL_PINYIN_PATH),
		String(""));

	// SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS
	m_selection_keys = m_config->read(
		String( SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS ),
		String( SCIM_CONFIG_IMENGINE_CHEWING_SELECTION_KEYS ) );

	// SCIM_CHEWING_SELECTION_KEYS_NUM
	m_selection_keys_num = _selection_keys_num = m_config->read(
			String( SCIM_CHEWING_SELECTION_KEYS_NUM ),
			9);

	// SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD
	m_add_phrase_forward = m_config->read(
			String( SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD ),
			false);
	
	// SCIM_CONFIG_IMENGINE_CHEWING_ESC_CLEAN_ALL_BUFFER
	m_esc_clean_all_buffer = m_config->read(
			String( SCIM_CONFIG_IMENGINE_CHEWING_ESC_CLEAN_ALL_BUFFER ),
			false);

	// SCIM_CONFIG_IMENGINE_CHEWING_SPACE_AS_SELECTION
	m_space_as_selection = m_config->read(
			String( SCIM_CONFIG_IMENGINE_CHEWING_SPACE_AS_SELECTION ),
			true);

	// SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_
	for (int i = 0; i < SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_NUM; i++) {
		int red, green, blue;
		char bgcolor_str[64];
		String str;
		sprintf(bgcolor_str, SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_ "_%d", i + 1);
		str = m_config->read(
			String(bgcolor_str),
			String(chewing_preedit_bgcolor[i] ));
		sscanf (str.c_str (), "#%02X%02X%02X", &red, &green, &blue);
		m_preedit_bgcolor[i] = SCIM_RGB_COLOR (red, green, blue);
	}
}

ChewingIMEngineFactory::~ChewingIMEngineFactory()
{
	m_reload_signal_connection.disconnect ();
}

WideString ChewingIMEngineFactory::get_name() const
{
	return utf8_mbstowcs( _( "Chewing" ) );
}

String ChewingIMEngineFactory::get_uuid() const
{
	return String( "fcff66b6-4d3e-4cf2-833c-01ef66ac6025" );
}

String ChewingIMEngineFactory::get_icon_file() const
{
	return String( SCIM_CHEWING_ICON_FILE );
}

WideString ChewingIMEngineFactory::get_authors() const
{
	return utf8_mbstowcs( _( "Chewing core team <http://chewing.csie.net>" ) );
}

WideString ChewingIMEngineFactory::get_credits() const
{
	return WideString();
}

WideString ChewingIMEngineFactory::get_help() const
{
	String help;
	String chi_eng_mode_switch;

	scim_key_list_to_string( chi_eng_mode_switch, m_chi_eng_keys );


	help =	String( _( "Hot Keys:" ) ) +
		String( "\n\n  " )  chi_eng_mode_switch  String( ":\n" ) 
		String( _( "    Switch between English/Chinese mode." ) ) 
		String( _( "\n\n  Space:\n"
			   "    Use space key to select candidate phrases."
			   "\n\n  Tab:\n"
			   "    Use tab key to dispart or connect a phrase."
			   "\n\n  Ctrl + [number]:\n"
			   "    Use Ctrl + number key to add a user-defined phrase.\n"
			   "    (Here number stands for the length of the user-defined phrase.)"
			   "\n\n  Ctrl + 0:\n"
			   "    Use Ctrl + 0 to specify symbolic input."
			   "\n\n j / k:\n"
			   "    While selecting candidate phrases, it could invoke \n"
			   "    switching between the previous and the next one."
		) );

	return utf8_mbstowcs( help );
}

IMEngineInstancePointer ChewingIMEngineFactory::create_instance(
		const String& encoding, int id )
{
	return new ChewingIMEngineInstance( this, encoding, id );
}

bool ChewingIMEngineFactory::validate_encoding( const String& encoding ) const
{
	return IMEngineFactoryBase::validate_encoding( encoding );
}

bool ChewingIMEngineFactory::validate_locale( const String& locale ) const
{
	return IMEngineFactoryBase::validate_locale( locale );
}

/**
 * Implementation of ChewingIMEngineInstance
 */

ChewingIMEngineInstance::ChewingIMEngineInstance(
		ChewingIMEngineFactory *factory, 
		const String& encoding, int id )
	: IMEngineInstanceBase( factory, encoding, id ),
	  m_factory( factory )
{
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Create IMEngineInstance\n";
	ctx = chewing_new();
	reload_config( m_factory->m_config );
	m_lookup_table.init( m_factory->m_selection_keys,
			     m_factory->m_selection_keys_num );

	m_reload_signal_connection =
		m_factory->m_config->signal_connect_reload(
			slot( this, &ChewingIMEngineInstance::reload_config ) );
}

void ChewingIMEngineInstance::reload_config( const ConfigPointer& scim_config )
{
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "IMEngine Instance ReloadConfig\n";
	// Reset all data.
	reset();

	config.candPerPage = m_factory->m_selection_keys_num * 2;

	config.maxChiSymbolLen = 16;

	// SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD
	config.bAddPhraseForward = m_factory->m_add_phrase_forward ? 0 : 1;

	// SCIM_CONFIG_IMENGINE_CHEWING_SPACE_AS_SELECTION
	config.bSpaceAsSelection = m_factory->m_space_as_selection ? 1 : 0;

	// SCIM_CONFIG_IMENGINE_CHEWING_ESC_CLEAN_ALL_BUFFER
	config.bEscCleanAllBuf = m_factory->m_esc_clean_all_buffer ? 1 : 0;

	//SetConfig( &da, &config );
	chewing_Configure( ctx, &config );
}


ChewingIMEngineInstance::~ChewingIMEngineInstance()
{
	chewing_free( ctx );
	m_reload_signal_connection.disconnect();
}

bool ChewingIMEngineInstance::process_key_event( const KeyEvent& key )
{
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Process Key Event\n";

	if ( match_key_event( m_factory->m_chi_eng_keys, key ) ) {
		m_prev_key = key;
		trigger_property( SCIM_PROP_CHIENG );
		SCIM_DEBUG_IMENGINE( 2 ) <<
			"Matcg Chi/Eng Key, End Process\n";
		return true;
	}
	m_prev_key = key;

	/* 
	 * This is a workaround against the known issue in OpenOffice with 
	 * GTK+ im module hanlding key pressed/released events.
	 */
	if ( key.is_key_release() ) {
        SCIM_DEBUG_IMENGINE( 2 ) <<
            "Key Release, End Process Key\n";
		return true;
    }

	if ( key.mask == 0 ) {
		switch ( key.code ) {
			case SCIM_KEY_Left:
				chewing_handle_Left( ctx );
				break;
			case SCIM_KEY_Right:
				chewing_handle_Right( ctx );
				break;
			case SCIM_KEY_Up:
				chewing_handle_Up( ctx );
				break;
			case SCIM_KEY_Down:
				chewing_handle_Down( ctx );
				break;
			case SCIM_KEY_space:
				chewing_handle_Space( ctx );
				break;
			case SCIM_KEY_Return:
				chewing_handle_Enter( ctx );
				break;
			case SCIM_KEY_BackSpace:
				chewing_handle_Backspace( ctx );
				break;
			case SCIM_KEY_Escape:
				chewing_handle_Esc( ctx );
				break;
			case SCIM_KEY_Delete:
				chewing_handle_Del( ctx );
				break;
			case SCIM_KEY_Home:
				chewing_handle_Home( ctx );
				break;
			case SCIM_KEY_End:
				chewing_handle_End( ctx );
				break;
			case SCIM_KEY_Tab:
				chewing_handle_Tab( ctx );
				break;
			case SCIM_KEY_Caps_Lock:
				chewing_handle_Capslock( ctx );
				break;
			case SCIM_KEY_Page_Up:
				chewing_handle_PageUp( ctx );
				break;
			case SCIM_KEY_Page_Down:
				chewing_handle_PageDown( ctx );
				break;
			case SCIM_KEY_Shift_L:
			case SCIM_KEY_Shift_R:
			case SCIM_KEY_Control_L:
			case SCIM_KEY_Control_R:
			case SCIM_KEY_Alt_L:
			case SCIM_KEY_Alt_R:
                SCIM_DEBUG_IMENGINE( 2 ) <<
                    "Unused keys, End Process Key\n";
				return true;
			default:
                SCIM_DEBUG_IMENGINE( 2 ) <<
                    "Begin OnKeyDefault\n";
				chewing_handle_Default( ctx, key.get_ascii_code() );
                SCIM_DEBUG_IMENGINE( 2 ) <<
                    "End OnKeyDefault\n";
				break;
		}
	}
	else if ( key.mask == SCIM_KEY_ShiftMask ) {
		switch ( key.code ) {
			case SCIM_KEY_Left:
				chewing_handle_ShiftLeft( ctx );
				break;
			case SCIM_KEY_Right:
				chewing_handle_ShiftRight( ctx );
				break;
			case SCIM_KEY_space:
				chewing_handle_ShiftSpace( ctx );
				chewing_set_ShapeMode( ctx, !chewing_get_ShapeMode( ctx ) );
				refresh_letter_property();
				break;
			/* Workaround with shift */
			case SCIM_KEY_Home:
			case SCIM_KEY_Page_Up:
			case SCIM_KEY_Page_Down:
			case SCIM_KEY_End:
			case SCIM_KEY_Begin:
			case SCIM_KEY_Insert:
			case SCIM_KEY_Delete:
				break;
			default:
				chewing_handle_Default( ctx, key.get_ascii_code() );
				break;
		}
	}
	else if ( key.mask == SCIM_KEY_ControlMask ) {
		if ( 
			key.code <= SCIM_KEY_9 && 
			key.code >= SCIM_KEY_0 ) {
			chewing_handle_CtrlNum( ctx, key.get_ascii_code() );
		} else {
			// chewing_handle_CtrlOption( ctx, key.get_ascii_code() );
			return false;
		}
	}
	else {
		return false;
	}
	have_input = true;
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "End Process Key\n";
	return commit( ctx->output );
}

void ChewingIMEngineInstance::move_preedit_caret( unsigned int pos )
{
}

void ChewingIMEngineInstance::select_candidate ( unsigned int index )
{
	chewing_handle_Default( ctx, '1' + index );
	commit( ctx->output );
}

void ChewingIMEngineInstance::update_lookup_table_page_size(
		unsigned int page_size )
{
	//XXX should not directly access data member.
	ctx->data->config.candPerPage = page_size * 2;
	m_lookup_table.set_page_size (page_size);
}

void ChewingIMEngineInstance::lookup_table_page_up()
{
	chewing_handle_Space( ctx );
	commit( ctx->output );
}

void ChewingIMEngineInstance::lookup_table_page_down()
{
	chewing_handle_Space( ctx );
	commit( ctx->output );
}

void ChewingIMEngineInstance::reset()
{
	chewing_Reset( ctx );

	/* Configure PinYin input method.
	   if the PinYin method type is a built-in type,
	   the second parameter will be ignored.
	 */
	chewing_set_PinYinMethod(static_cast<PinYinMethodType>(m_factory->m_PinYinType),
		m_factory->m_ExternPinYinPath.c_str());
	
	/* Configure Keyboard Type */
	chewing_set_KBType( ctx, chewing_KBStr2Num( 
				(char *) m_factory->m_KeyboardType.c_str() ));
	
	/* Configure selection keys definition */
	int i = 0;
	for (; m_factory->m_selection_keys[i] &&
	       i <= m_factory->m_selection_keys_num; i++) {
		config.selKey[i] = m_factory->m_selection_keys[i];
	}
	config.selKey[i] = '\0';
	m_lookup_table.init( m_factory->m_selection_keys,
	                     m_factory->m_selection_keys_num );

	/* Re-focus to clear preedit, to avoid gedit crash when select-all. */
	focus_out();
	focus_in();
}

void ChewingIMEngineInstance::focus_in()
{
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Focus In\n";
	initialize_all_properties ();
}

void ChewingIMEngineInstance::focus_out()
{
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "Focus Out\n";
    if (have_input == true) {
        chewing_handle_Enter( ctx );
        commit( ctx->output );
        chewing_handle_Esc( ctx );
        have_input = false;
    }
}

void ChewingIMEngineInstance::trigger_property( const String& property )
{
	if ( property == SCIM_PROP_CHIENG ) {
		chewing_handle_Capslock( ctx );
		commit( ctx->output );
	} else if ( property == SCIM_PROP_LETTER ) {
		chewing_set_ShapeMode( ctx, !chewing_get_ShapeMode( ctx ) );
	} else if ( property == SCIM_PROP_KBTYPE ) {
		/* loop through the keyboard type array */
		if ( chewing_get_KBType( ctx ) == LAST_KBTYPE )
 	                chewing_set_KBType( ctx, FIRST_KBTYPE );
		else
			chewing_set_KBType( ctx, chewing_get_KBType( ctx )  1 );
	}
	refresh_all_properties ();
}

bool ChewingIMEngineInstance::commit( ChewingOutput *pgo )
{
	AttributeList attr;

    SCIM_DEBUG_IMENGINE( 2 ) <<
        "IMEngine Instance Commit\n";
	// commit string
	m_commit_string = L"";
	if ( pgo->keystrokeRtn & KEYSTROKE_COMMIT ) {
		for ( int i = 0; i < pgo->nCommitStr; i++ ) {
			m_commit_string += utf8_mbstowcs((char *)pgo->commitStr[ i ].s, 
					MAX_UTF8_SIZE);
            SCIM_DEBUG_IMENGINE( 2 ) << "Commit Add: " <<
                (char *)pgo->commitStr[ i ].s << "\n";
		}
		commit_string( m_commit_string );
	}
	m_preedit_string = L"";
	// preedit string
	// XXX show Interval
	for ( int i = 0; i < pgo->chiSymbolCursor; i++ ) {
		m_preedit_string += utf8_mbstowcs((char *)pgo->chiSymbolBuf[ i ].s, MAX_UTF8_SIZE);
		SCIM_DEBUG_IMENGINE( 2 ) << "PreEdit Add: " <<
			(char *)pgo->chiSymbolBuf[ i ].s << "\n";
	}
	// zuin string
	for ( int i = 0, j = 0; i < ZUIN_SIZE; i++ ) {
		if ( pgo->zuinBuf[ i ].s[ 0 ] != '\0' ) {
			m_preedit_string += utf8_mbstowcs((char *)pgo->zuinBuf[ i ].s, 
					MAX_UTF8_SIZE);
			attr.push_back(Attribute(pgo->chiSymbolCursor + j, 1,
					SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
			j++;
		}
	}

	for ( int i = pgo->chiSymbolCursor; i < pgo->chiSymbolBufLen; i++ ) {
        	m_preedit_string += utf8_mbstowcs((char *)pgo->chiSymbolBuf[ i ].s, 
				MAX_UTF8_SIZE);
	}

	for ( int i = 0; i < pgo->nDispInterval; i++ ) {
		if ( pgo->dispInterval[ i ].to - pgo->dispInterval[ i ].from > 1 ) {
			attr.push_back(
				Attribute(
					pgo->dispInterval[ i ].from,
					pgo->dispInterval[ i ].to - pgo->dispInterval[ i ].from,
					SCIM_ATTR_DECORATE,
					SCIM_ATTR_DECORATE_UNDERLINE));
			attr.push_back(
				Attribute(
					pgo->dispInterval[ i ].from,
					pgo->dispInterval[ i ].to - pgo->dispInterval[ i ].from,
					SCIM_ATTR_BACKGROUND,
	m_factory->m_preedit_bgcolor[i % SCIM_CONFIG_IMENGINE_CHEWING_PREEDIT_BGCOLOR_NUM] ));
		}
	}
	// cursor decoration
	if ( pgo->zuinBuf[ 0 ].s[ 0 ] == '\0' )
		attr.push_back(
			Attribute(
				pgo->chiSymbolCursor, 
				1, 
				SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));

	// update display
	update_preedit_string( m_preedit_string, attr );
	update_preedit_caret( pgo->chiSymbolCursor );

	// show preedit string
	if ( m_preedit_string.empty() ) {
		hide_preedit_string();
	}
	else {
		show_preedit_string();
	}
	
	// show lookup table
        if ( !pgo->pci )
                return true;
        if ( pgo->pci->nPage != 0 ) {
                m_lookup_table.update( pgo->pci );
                show_lookup_table();

                if ( ( pgo->pci->nTotalChoice % pgo->pci->nChoicePerPage
                != 0 ) && pgo->pci->pageNo == pgo->pci->nPage - 1 ) {
                        m_lookup_table.set_page_size(
                                pgo->pci->nTotalChoice %
                                        pgo->pci->nChoicePerPage );
                } else {
                        m_lookup_table.set_page_size(
                                pgo->pci->nChoicePerPage );
                }

                update_lookup_table( m_lookup_table );
        }
        else {
                hide_lookup_table();
        }
	
	// show aux string
	m_aux_string = L"";
	if ( pgo->bShowMsg ) {
		for ( int i = 0; i < pgo->showMsgLen; i++ ) {
            m_aux_string += utf8_mbstowcs((char *)pgo->showMsg[ i ].s, MAX_UTF8_SIZE);
		}
		update_aux_string( m_aux_string );
		show_aux_string();
		pgo->showMsgLen = 0;
	}
	else {
		hide_aux_string();
	}
	if ( pgo->keystrokeRtn & KEYSTROKE_ABSORB )
		return true;
	if ( pgo->keystrokeRtn & KEYSTROKE_IGNORE )
		return false;
	return true;
}

bool ChewingIMEngineInstance::match_key_event(
		const KeyEventList &keylist,
		const KeyEvent &key )
{
	KeyEventList::const_iterator ++kit;
	for (kit = keylist.begin(); kit != keylist.end(); kit) {
		if (key.code == kit->code && key.mask == kit->mask)
			if (key.is_key_press() || m_prev_key.code == key.code)
				return true;
	}
	return false;
}

void ChewingIMEngineInstance::initialize_all_properties ()
{
	PropertyList proplist;
	proplist.push_back (_chieng_property);
	proplist.push_back (_letter_property);
	proplist.push_back (_kbtype_property);

	register_properties (proplist);
	refresh_all_properties ();
}

void ChewingIMEngineInstance::refresh_all_properties ()
{
	refresh_chieng_property ();
	refresh_letter_property ();
	refresh_kbtype_property ();
}

void ChewingIMEngineInstance::refresh_chieng_property ()
{
	if ( chewing_get_ChiEngMode( ctx ) != CHINESE_MODE )
		_chieng_property.set_label (_("Eng"));
	else
		_chieng_property.set_label (_("Chi"));

	update_property (_chieng_property);
}

void ChewingIMEngineInstance::refresh_kbtype_property ()
{
	/* look forward to be evoluted into drop-down list */
	switch ( chewing_get_KBType( ctx ) )
	{
		case KB_DEFAULT:
			_kbtype_property.set_label (_("Default"));
			break;
		case KB_HSU:
			_kbtype_property.set_label (_("Hsu's"));
			break;
		case KB_IBM:
			_kbtype_property.set_label (_("IBM"));
			break;
		case KB_GINYIEH:
			_kbtype_property.set_label (_("Gin-Yieh"));
			break;
		case KB_ETEN:
			_kbtype_property.set_label (_("ETen"));
			break;
		case KB_ETEN26:
			_kbtype_property.set_label (_("ETen 26-key"));
			break;
		case KB_DVORAK:
			_kbtype_property.set_label (_("Dvorak"));
			break;
		case KB_DVORAKHSU:
			_kbtype_property.set_label (_("Dvorak Hsu's"));
			break;
		case KB_HANYU:
			_kbtype_property.set_label (_("Han-Yu"));
			break;
		default:
			_kbtype_property.set_label (_("Default"));
		}

	update_property (_kbtype_property);
}

void ChewingIMEngineInstance::refresh_letter_property ()
{
	if ( chewing_get_ShapeMode( ctx ) != FULLSHAPE_MODE )
		_letter_property.set_label (_("Half"));
	else
		_letter_property.set_label (_("Full"));

	update_property (_letter_property);
}

ChewingLookupTable::ChewingLookupTable()
	: LookupTable( _selection_keys_num )
{
}

ChewingLookupTable::~ChewingLookupTable()
{
}

WideString ChewingLookupTable::get_candidate( int index ) const
{
	WideString m_converted_string;
	int no = pci->pageNo * pci->nChoicePerPage;
	m_converted_string = utf8_mbstowcs(
			(char *) pci->totalChoiceStr[ no + index ],
			(int) strlen( (char *) pci->totalChoiceStr[ no + index ] ));
	return m_converted_string;
}

AttributeList ChewingLookupTable::get_attributes( int index ) const
{
	return AttributeList();
}

unsigned int ChewingLookupTable::number_of_candidates() const
{
	return pci->nTotalChoice;
}

void ChewingLookupTable::clear()
{
}

void ChewingLookupTable::init(String selection_keys, int selection_keys_num)
{
	std::vector< WideString > labels;
    
    SCIM_DEBUG_IMENGINE( 2 ) <<
        "LookupTable Init\n";
	char buf[ 2 ] = { 0, 0 };
	for ( int i = 0; i < selection_keys_num; ++i ) {
		buf[ 0 ] = selection_keys[i];
		labels.push_back( utf8_mbstowcs( buf ) );
	}
	set_candidate_labels( labels );
}

void ChewingLookupTable::update( ChoiceInfo *ci )
{
	pci = ci;
}

