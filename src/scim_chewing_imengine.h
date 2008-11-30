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

#ifndef SCIM_CHEWING_H
#define SCIM_CHEWING_H

using namespace scim;

class ChewingLookupTable : public LookupTable
{
	public:
		ChewingLookupTable();
		virtual ~ChewingLookupTable();
		virtual WideString get_candidate( int index ) const;
		virtual AttributeList get_attributes ( int index ) const;
		virtual unsigned int number_of_candidates() const;
		virtual void clear();
		void init( String s, int num );
		void update( ChewingContext* ctx );
	private:
		ChewingContext* ctx;
};

class ChewingIMEngineFactory : public IMEngineFactoryBase
{
public:
	ChewingIMEngineFactory( const ConfigPointer& config );
	virtual ~ChewingIMEngineFactory();

	virtual WideString get_name() const;
	virtual String get_uuid() const;
	virtual String get_icon_file() const;
	virtual WideString get_authors() const;
	virtual WideString get_credits() const;
	virtual WideString get_help() const;

	virtual bool validate_encoding( const String& encoding ) const;
	virtual bool validate_locale( const String& locale ) const;

	virtual IMEngineInstancePointer create_instance(
		const String& encoding, int id = -1 );
	bool valid() const { return m_valid; }
	ConfigPointer m_config;

private:
	bool init();
	bool m_valid;
	void reload_config( const ConfigPointer &scim_config );
	Connection m_reload_signal_connection;

	KeyEventList m_chi_eng_keys;
	String m_KeyboardType;
	int m_PinYinType;
	String m_ExternPinYinPath;
	String m_selection_keys;
	String m_input_mode;
	int m_selection_keys_num;
	bool m_add_phrase_forward;
	bool m_phrase_choice_rearward;
	bool m_auto_shift_cursor;
	bool m_space_as_selection;
	bool m_esc_clean_all_buffer;

	/* A series of background colors used in preedit area */
	unsigned int m_preedit_bgcolor[5];

	friend class ChewingIMEngineInstance;
};

class ChewingIMEngineInstance : public IMEngineInstanceBase
{
public:
	ChewingIMEngineInstance(
		ChewingIMEngineFactory *factory, 
		const String& encoding, int id = -1 );

	virtual ~ChewingIMEngineInstance();

	virtual bool process_key_event( const KeyEvent& key );
	virtual void move_preedit_caret( unsigned int pos );
	virtual void select_candidate( unsigned int index );
	virtual void update_lookup_table_page_size( unsigned int page_size );
	virtual void lookup_table_page_up();
	virtual void lookup_table_page_down();
	virtual void reset();
	virtual void focus_in();
	virtual void focus_out();
	virtual void trigger_property( const String& property );
private:
	bool commit( ChewingContext* ctx );
	void reload_config( const ConfigPointer &scim_config );
	bool match_key_event( const KeyEventList &keylist, const KeyEvent &key );

	void initialize_all_properties ();
	void refresh_all_properties ();
	void refresh_chieng_property ();
	void refresh_letter_property ();
	void refresh_kbtype_property ();

	Connection m_reload_signal_connection;
	KeyEvent m_prev_key;
	ChewingIMEngineFactory *m_factory;
	ChewingLookupTable m_lookup_table;
	ChewingContext *ctx;
	bool have_input;
};

#endif

