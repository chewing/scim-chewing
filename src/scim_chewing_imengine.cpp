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

#include <scim.h>
#include <chewing/chewing.h>

#include "scim_chewing_imengine.h"
#include "scim_chewing_config_entry.h"

/**
 * LIBCHEWING_ENCODING is a new macro introduced in libchewing 0.3.
 * In stable release (0.2.x) doesn't provide this macro, and still
 * uses Big5 as its internal encoding.
 */
#ifndef LIBCHEWING_ENCODING
#define LIBCHEWING_ENCODING "BIG5"
#endif

using namespace scim;

static IMEngineFactoryPointer _scim_chewing_factory( 0 );
static ConfigPointer _scim_config( 0 );

extern "C" {
	void scim_module_init()
	{
	}

	void scim_module_exit()
	{
		_scim_config.reset();
	}

	unsigned int scim_imengine_module_init( const ConfigPointer& config )
	{
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
	set_languages( "zh_TW,zh_HK,zh_SG" );
	m_valid = init();
}

bool ChewingIMEngineFactory::init()
{
	char prefix[] = CHEWING_DATADIR;
	char hash_postfix[] = "/.chewing/";

	/*
	 * XXX: awkful initialization routines performed on libchewing.
	 * The routines will be cleaned up in libchewing-0.3.x.
	 */
	ReadTree( prefix );
	if ( InitChar( prefix ) == 0 ) {
		SCIM_DEBUG_IMENGINE( 1 ) << 
			"Dictionary file corrupted!\n";
		return false;
	}
	InitDict( prefix );
	// NOTE: we can use scim_get_home_dir() for getting $HOME
	if ( ReadHash( (char *)(scim_get_home_dir() + hash_postfix).c_str() ) == 0 ) {
		SCIM_DEBUG_IMENGINE( 1 ) << 
			"User Phrase Library load failed!\n";
		return false;
	}
	return true;
}

ChewingIMEngineFactory::~ChewingIMEngineFactory()
{
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
	return utf8_mbstowcs( _( "SCIM-chewing Developers." ) );
}

WideString ChewingIMEngineFactory::get_credits() const
{
	return utf8_mbstowcs( _( "Here should put some credits" ) );
}

WideString ChewingIMEngineFactory::get_help() const
{
	return utf8_mbstowcs( _( "Here should put some help" ) );
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
	reload_config( m_factory->m_config );

	m_iconv.set_encoding( LIBCHEWING_ENCODING );
	m_lookup_table.init();

	m_reload_signal_connection =
		m_factory->m_config->signal_connect_reload(
			slot( this, &ChewingIMEngineInstance::reload_config ) );
}

void ChewingIMEngineInstance::reload_config( const ConfigPointer& scim_config )
{
	char default_selectionKeys[] = "1234567890";
	String default_KeyboardType = m_factory->m_config->read (
			String( SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE ),
			String( "KB_DEFAULT" ));
	String str;

	/* Configure Keyboard Type */
	cf.kb_type = KBStr2Num(
		(char *) m_factory->m_config->read( String(
			  SCIM_CONFIG_IMENGINE_CHEWING_USER_KB_TYPE ),
			default_KeyboardType ).c_str() );
	
	/*
	 * XXX: The legacy code doesn't make sense, and will be removed in
	 * 0.3.x version of libchewing.
	 */
	cf.inp_cname = (char *) strdup( "Chewing" );
	cf.inp_ename = (char *) strdup( "Chewing" );

	InitChewing( &da, &cf );
	config.selectAreaLen = 55;
	config.maxChiSymbolLen = 16;

	/* Configure selection keys definition */
	m_factory->m_config->read(
			String( SCIM_CONFIG_IMENGINE_CHEWING_USER_SELECTION_KEYS ),
			default_selectionKeys );
	default_selectionKeys[ SCIM_CHEWING_SELECTION_KEYS_NUM ] = '\0';
	for ( int i = 0; i < SCIM_CHEWING_SELECTION_KEYS_NUM; i++ ) {
		config.selKey[ i ] = default_selectionKeys[ i ];
	}


	// add chi/eng toggle key
	str = m_factory->m_config->read (String (SCIM_CONFIG_IMENGINE_CHEWING_CHI_ENG_KEY),
			String ("Shift+Shift_L+KeyRelease"));
	scim_string_to_key (m_chi_eng_key, str);

	/* Configure the direction for user's phrase addition */
	// SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD
	str = m_factory->m_config->read (String (SCIM_CONFIG_IMENGINE_CHEWING_ADD_PHRASE_FORWARD),
			String ("false"));
	config.bAddPhraseForward = (str == "false") ? 1 : 0;

	SetConfig( &da, &config );
}


ChewingIMEngineInstance::~ChewingIMEngineInstance()
{
	m_reload_signal_connection.disconnect();
}

bool ChewingIMEngineInstance::process_key_event( const KeyEvent& key )
{
	/* 
	 * This is a workaround against the known issue in OpenOffice with 
	 * GTK+ im module hanlding key pressed/released events.
	 */
	if ( key.is_key_release() )
		return true;
	
	if (m_chi_eng_key.code == key.code) {
		OnKeyCapslock( &da, &gOut );
		return commit( &gOut );
	}

	if (
		key.mask == SCIM_KEY_NullMask || 
		key.mask == SCIM_KEY_ShiftMask || 
		key.mask == SCIM_KEY_CapsLockMask ) {
		switch ( key.code ) {
			case SCIM_KEY_Left:
				OnKeyLeft( &da, &gOut );
				break;
			case SCIM_KEY_Right:
				OnKeyRight( &da, &gOut );
				break;
			case SCIM_KEY_Up:
				OnKeyUp( &da, &gOut );
				break;
			case SCIM_KEY_Down:
				OnKeyDown( &da, &gOut );
				break;
			case SCIM_KEY_space:
				OnKeySpace( &da, &gOut );
				break;
			case SCIM_KEY_Return:
				OnKeyEnter( &da, &gOut );
				break;
			case SCIM_KEY_BackSpace:
				OnKeyBackspace( &da, &gOut );
				break;
			case SCIM_KEY_Escape:
				OnKeyEsc( &da, &gOut );
				break;
			case SCIM_KEY_Delete:
				OnKeyDel( &da, &gOut );
				break;
			case SCIM_KEY_Home:
				OnKeyHome( &da, &gOut );
				break;
			case SCIM_KEY_End:
				OnKeyEnd( &da, &gOut );
				break;
			case SCIM_KEY_Tab:
				OnKeyTab( &da, &gOut );
				break;
			default:
				OnKeyDefault(
					&da, 
					key.get_ascii_code(), 
					&gOut );
		}
		return commit( &gOut );
	}
	if ( key.mask == SCIM_KEY_ControlMask ) {
		if ( 
			key.get_ascii_code() <= '9' && 
			key.get_ascii_code() >= '0' ) {
			OnKeyCtrlNum( &da, key.get_ascii_code(), &gOut );
			return commit( &gOut );
		}
	}
	return false;
}

void ChewingIMEngineInstance::move_preedit_caret( unsigned int pos )
{
}

void ChewingIMEngineInstance::select_candidate ( unsigned int index )
{
}

void ChewingIMEngineInstance::update_lookup_table_page_size(
		unsigned int page_size )
{
	da.config.selectAreaLen = page_size * 5 + 5;
}

void ChewingIMEngineInstance::lookup_table_page_up()
{
	OnKeySpace( &da, &gOut );
	commit( &gOut );
}

void ChewingIMEngineInstance::lookup_table_page_down()
{
	OnKeySpace( &da, &gOut );
	commit( &gOut );
}

void ChewingIMEngineInstance::reset()
{
}

void ChewingIMEngineInstance::focus_in()
{
}

void ChewingIMEngineInstance::focus_out()
{
	OnKeyEnter( &da, &gOut );
	commit( &gOut );
}

void ChewingIMEngineInstance::trigger_property( const String& property )
{
}

bool ChewingIMEngineInstance::commit( ChewingOutput *pgo )
{
	// commit string
	m_commit_string = WideString();
	if ( pgo->keystrokeRtn & KEYSTROKE_COMMIT ) {
		for ( int i = 0; i < pgo->nCommitStr; i++ ) {
			m_iconv.convert(
					m_converted_string, 
					(char *) pgo->commitStr[ i ].s, 2 );
			m_commit_string += m_converted_string;
		}
		commit_string( m_commit_string );
	}
	m_preedit_string = WideString();
	// preedit string
	// XXX show Interval
	for ( int i = 0; i < pgo->chiSymbolBufLen; i++ ) {
		m_iconv.convert( 
			m_converted_string, 
			(char *) pgo->chiSymbolBuf[ i ].s, 2 );
		m_preedit_string += m_converted_string;
	}
	// zuin string
	for ( int i = 0; i < ZUIN_SIZE; i++ ) {
		if ( pgo->zuinBuf[ i ].s[ 0 ] != '\0' ) {
			 m_iconv.convert(
				m_converted_string, 
				(char *) pgo->zuinBuf[ i ].s, 2 );
			 m_preedit_string += m_converted_string;
		}
	}
	update_preedit_string( m_preedit_string, AttributeList() );
	update_preedit_caret( pgo->chiSymbolCursor );

	// show preedit string
	if ( m_preedit_string.empty() ) {
		hide_preedit_string();
	} else {
		show_preedit_string();
	}
	
	// show lookup table
	if ( pgo->pci->nPage != 0 ) {
		m_lookup_table.update( pgo->pci );
		update_lookup_table( m_lookup_table );
		show_lookup_table();
	} else {
		hide_lookup_table();
	}
	
	// show aux string
	m_aux_string = WideString();
	if ( pgo->bShowMsg ) {
		for ( int i = 0; i < pgo->showMsgLen; i++ ) {
			m_iconv.convert(
				m_converted_string,
				(char *) pgo->showMsg[ i ].s, 2 );
			m_aux_string += m_converted_string;
		}
		update_aux_string( m_aux_string );
		show_aux_string();
		pgo->showMsgLen = 0;
	} else {
		hide_aux_string();
	}
	if ( pgo->keystrokeRtn & KEYSTROKE_ABSORB )
		return true;
	if ( pgo->keystrokeRtn & KEYSTROKE_IGNORE )
		return false;
	return true;
}

ChewingLookupTable::ChewingLookupTable()
	: LookupTable( SCIM_CHEWING_SELECTION_KEYS_NUM )
{
}

ChewingLookupTable::~ChewingLookupTable()
{
}

WideString ChewingLookupTable::get_candidate( int index ) const
{
	WideString m_converted_string;
	int no = pci->pageNo * pci->nChoicePerPage;
	m_iconv.convert(
		m_converted_string, 
		(char *) pci->totalChoiceStr[ no + index ], 
		(int) strlen( (char *) pci->totalChoiceStr[ no + index ] ) );
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

void ChewingLookupTable::init()
{
	std::vector< WideString > labels;
	m_iconv.set_encoding( LIBCHEWING_ENCODING );
	char buf[ 2 ] = { 0, 0 };
	for ( int i = 0; i < (SCIM_CHEWING_SELECTION_KEYS_NUM); ++i ) {
		buf[ 0 ] = '1' + i;
		labels.push_back( utf8_mbstowcs( buf ) );
	}
	buf[ 0 ] = '0';
	labels.push_back( utf8_mbstowcs( buf ) );
	set_candidate_labels( labels );
}

void ChewingLookupTable::update( ChoiceInfo *ci )
{
	pci = ci;
}

