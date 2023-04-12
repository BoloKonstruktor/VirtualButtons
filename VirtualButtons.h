#ifndef VIRTUAL_H
#define VIRTUAL_H
#include "Arduino.h"
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#define WebServer ESP8266WebServer
#else
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#include <WiFiClient.h>
#include <Vector.h>
#include <map>

#define FLASH(x) flash2str(F(x)).c_str()


extern unsigned int FSHlength(const __FlashStringHelper * FSHinput);
extern String flash2str( const __FlashStringHelper *text );


class VirtualButton {
		String def_color = "", act_color = "", font_color = "", group = "", name = "", caption = "";
		uint8_t font_size = 0;
		bool state = 0;
		std::map< String, std::map< String, String > > css;
		
	public:
		VirtualButton( void ){
			this->def_color = "background-image:linear-gradient(#eee,#999);";
			this->act_color = "background-image:linear-gradient(#999,#eee);";
		}
		
		void(*callback)( void ) = NULL;
		bool(*callback_res)( bool ) = NULL;
		
		void setState( bool state ){
			this->state = state;
		}
		
		void setDefColor( const char* str ){
			this->def_color = str;
		}
		
		void setActColor( const char* str ){
			this->act_color = str;
		}
		
		void setFontSize( uint8_t s ){
			this->font_size = s;
			this->css[this->getName()]["font_size"] = "font-size:"+String( font_size )+"px;";
		}
		
		void setFontColor( const char* str ){
			this->font_color = str;
			this->css[this->getName()]["font_color"] = "color:"+String( str )+";";
		}
		
		void setGroup( const char* str ){
			this->group = str;
		}
	
		
		void setName( const char* str ){
			this->name = str;
		}
		
		void setCaption( const char* str ){
			this->caption = str;
		}
		
		void setCSS( const char* param, const char* style ){
			String name = this->getName();
			this->css[name][param] = style;
		}
		
		String getDefColor( void ){
			return this->def_color;
		}
		
		String getActColor( void ){
			return this->act_color;
		}
		
		uint8_t getFontSize( void ){
			return this->font_size;
		}
		
		String getGroup( void ){
			return this->group;
		}
		
		String getName( void ){
			String gr = "";
				if( getGroup() != "" ) gr = getGroup()+"-";
			return gr+name;
		}
		
		String getCaption( void ){
			return this->caption;
		}
		
		bool getState( void ){
			return this->state;
		}
		
		String getCSS( void ){
			String name = this->getName();
			String str = "";
			
				for( auto item : this->css[name] ){
					str += item.second;
				}
				
			return str;
		}
};


class VirtualButtons {
	private:
		Stream* monitor = NULL;
		WebServer* server = NULL;
		static VirtualButtons* int_inst;
		String (*display_callback)( void ) = NULL;
		bool ext_server_inst = false, res_callback = false;
		uint16_t display_refresh_interval = 0;
		VirtualButton* storage_array[10];
		typedef Vector<VirtualButton*>Buttons;
		Buttons vector;
		String index_path = "", btnstr = "", header = "", title = "Virtual Buttons";
			
		template< typename V> void print( V v ){
					
				if( this->monitor ){
					this->monitor->print( v );
				}
		}
			
		template< typename V> void println( V v ){	
			this->print( v );
			this->print( "\n" );
		}
		
		String ip2str( const IPAddress& ip ){
			return String( ip[0]) + String(".") +\
				String(ip[1]) + String(".") +\
				String(ip[2]) + String(".") +\
				String(ip[3]); 
		}

		void sendRedirect( String uri="/" );
		void btn_state_update( void );
		String Display( void );
		String get_btn( void );
	
	public:
		VirtualButtons( Stream* monitor = NULL );
		void begin( const char* path="/", WebServer* server = NULL, const uint16_t port = 80 );
		void loop( void );
		VirtualButton* addButton( const char* name, const char* caption, void(*callback)( void ) );
		VirtualButton* addButton( const char* name, const char* caption, bool(*callback)( bool ) );
		VirtualButton* getButtonByName( const char* name, const char* group=0 );
		void createGroup( const char* group=0, const char* label=0 );
		String getButtonsForm( void );
		void setDisplay( String(*callback)( void ), uint16_t refresh_interval=1000 );
		void setHeaderText( const char* str );
		void setTitleText( const char* str );
		String getTitleText( void );
		
};
#endif