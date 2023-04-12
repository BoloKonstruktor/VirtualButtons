#include "VirtualButtons.h"

static const char HTML[] PROGMEM = {
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8'>"
"<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/>"
"<meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'/>"
"<meta http-equiv='Pragma' content='no-cache'/>"
"<meta http-equiv='Expires' content='0'/>"
"<style>"
"body{"
"text-align:center;"
"background:radial-gradient(circle, rgba(255,255,255,1) 0%,rgba(221,221,221,1) 90%);"
"}"
"#main{"
"width:420px;"
"height:100%;"
"margin:auto;"
"text-align:left;"
"}"
"h2{"
"font-family:Arial;"
"margin:0 0 30px 0;"
"padding:0px;"
"text-shadow:1px 1px 1px #FFF;"
"}"
"#display{"
"font-family:Courier;"
"font-size:18px;"
"margin:25px 0 30px 0;"
"padding:5px;"
"border-radius:5px;"
"background-color:#fff;"
"}"
"#display p{"
"margin:5px 0 5px 0;"
"padding:0;"
"overflow:hidden;"
"text-overflow:clip;"
"white-space:nowrap;"
"}"
"#display p b{"
"font-size:24px;"
"}"
"form div,form fieldset{"
"font-family:Arial;"
"margin:25px 0 25px 0;"
"font-family:Arial;"
"font-size:18px;"
"border-radius:5px;"
"text-align:center;"
"}"
"legend{"
"text-align:left;"
"text-shadow:1px 1px 1px #FFF;"
"}"
"fieldset{"
"border:1px solid #CCC;"
"}"
"button{"
"margin:8px;"
"cursor:pointer;"
"font-size:18px;"
"width:140px;"
"height:60px;"
//"text-shadow:1px 1px 1px #FFF;"
"border-radius:5px;"
"border:1px solid #999;"
"}"
"@media (orientation: portrait){"
"#main{"
"width:100%;"
"}"
"}"
"</style>"
"<title>%TITLE%</title>"
"</head>"
"<body>"
"<div id='main'>"
"%BTN_FORM%"
"</div>"
"</body>"
"</html>"
};

VirtualButtons* VirtualButtons::int_inst = NULL;

unsigned int FSHlength(const __FlashStringHelper * FSHinput) {
  PGM_P FSHinputPointer = reinterpret_cast<PGM_P>(FSHinput);
  unsigned int stringLength = 0;
  while (pgm_read_byte(FSHinputPointer++)) {
    stringLength++;
  }
  return stringLength;
}

String flash2str( const __FlashStringHelper *text ) {
  const byte textLength = FSHlength(text);
  char buffer[textLength+1];
  memcpy_P( buffer, text, textLength+1 );
  return buffer;
}

void VirtualButtons::sendRedirect( String uri ){
  this->server->sendHeader( "Location", uri, true );
  this->server->send( 302, "text/plain", "" );
  this->server->client().stop();
}

void VirtualButtons::btn_state_update( void ){
		
		for( auto btn : this->vector ){
			String name = btn->getName();
									
				if( btn->callback_res ){
					String prev_css = btn->getCSS();
										
						if( btn->callback_res( true ) ) btn->setCSS( "bg", btn->getActColor().c_str() );
						else btn->setCSS( "bg", btn->getDefColor().c_str() );
						
					String s = "<button style='"+prev_css+"' name='"+name+"'";
					String r = "<button style='"+btn->getCSS()+"' name='"+name+"'";
					this->btnstr.replace( s, r );
				
				} else btn->setCSS( "bg", btn->getDefColor().c_str() );
		}	
}

String VirtualButtons::get_btn( void ){
	return this->btnstr;
}

String VirtualButtons::Display( void ){
	String str = "";
	
		if( this->display_callback ){
			str += "<div id='display'>";
			str += this->display_callback();
			str += "</div>";
		}
	
	return str;	
}

//Metody publiczne
VirtualButtons::VirtualButtons( Stream* monitor ){
	this->int_inst = this;
	this->monitor = monitor;
	this->vector.setStorage( this->storage_array );
}

void VirtualButtons::begin( const char* path, WebServer* server, const uint16_t port ){
	
		if( WiFi.waitForConnectResult() == WL_CONNECTED ){
				
				if( server ){
					this->server = server;
					this->ext_server_inst = true;
				} else {
					this->server = new WebServer( port );
					this->server->begin();
					this->ext_server_inst = false;
				}
				
			this->index_path = path;
			this->println( "[VirtualButtons] Ready" );
			
			
			this->server->on( path, HTTP_GET, [](){
				int_inst->btn_state_update();
				String html = HTML;
				String form = int_inst->getButtonsForm();
				
					if( int_inst->display_callback && int_inst->display_refresh_interval ){
						String script = "<script type='text/javascript'>";
						script += "function u(){";
						script += "const url=new URL(location.href);";
						script += "var p=url.port!=''?':'+url.port:'';";
						script += "var r=new XMLHttpRequest();";
						script += "r.open('GET','http://'+url.hostname+p+'/display_update',true);";
						script += "r.onreadystatechange=function(){";
						script += "if(this.readyState==4 && this.status==200){";
						script += "document.getElementById('display').innerHTML=this.responseText;";
						script += "}";
						script += "}\n";
						script += "r.send();";
						script += "setTimeout(function(){u();},"+String(int_inst->display_refresh_interval)+");";
						script += "} ";
						script += "u();";		
						script += "</script>";
						form += script;
					}
					
				html.replace( "%BTN_FORM%", form );
				html.replace( "%TITLE%", int_inst->getTitleText() );
				int_inst->server->send( 200, "text/html", html.c_str() );
			});

			this->server->on( "/click", HTTP_GET, [](){
				
					for( auto& btn : int_inst->vector ){
						
						String name = btn->getName();
							
							if( int_inst->server->hasArg( name ) ){
								
									if( btn->callback ){
										btn->callback();
									}
									
									if( btn->callback_res ){
										btn->callback_res( false );
									}
							
								int_inst->sendRedirect( int_inst->index_path );
							}
					}
					
				int_inst->sendRedirect( int_inst->index_path );
			});
			
			this->server->on( "/display_update", HTTP_GET, [](){
				String str  = int_inst->display_callback ? int_inst->display_callback() : "";
				int_inst->server->send( 200, "text/html", str );
			});
		}
}

void VirtualButtons::loop( void ) {
	
		if( this->server ) {	
				if( !this->ext_server_inst ) this->server->handleClient();
		}
}

VirtualButton* VirtualButtons::addButton( const char* name, const char* caption, void(*callback)( void ) ){
	VirtualButton* btn = new VirtualButton;
	btn->callback = callback;
	btn->setName( name );
	btn->setCaption( caption );
	vector.push_back( btn );
	return btn;
}

VirtualButton* VirtualButtons::addButton( const char* name, const char* caption, bool(*callback)( bool ) ){
	VirtualButton* btn = new VirtualButton;
	btn->callback_res = callback;
	btn->setName( name );
	btn->setCaption( caption );
	vector.push_back( btn );
	return btn;
}

VirtualButton* VirtualButtons::getButtonByName( const char* name, const char* group ){
	String n = group != 0 ? String( group )+"-"+String( name ) : name;
	
		for( auto& btn : vector ){
			
			if( n == btn->getName() ){
				return btn;
			}
		}
	
	return NULL;
}

void VirtualButtons::createGroup( const char* group, const char* label ){
	uint8_t i = 0;
	this->btn_state_update();
		
		if( group != 0 ) this->btnstr += "<fieldset>";
		else this->btnstr += "<div>";
	
		for( auto btn : vector ){
			String _group = btn->getGroup();
			String font_size = btn->getFontSize() ? "font-size:"+String(btn->getFontSize())+"px;" : "";
			String name = btn->getName();
			String css = btn->getCSS();
			
				if( _group == group && group != 0 ){
						
						if( i == 0 && label != 0 ){
							this->btnstr += "<legend>";
							this->btnstr += label;
							this->btnstr += "</legend>";
						}
						

					this->btnstr += "<button style='"+css+"' name='";
					this->btnstr += name;
					this->btnstr += "'>";
					this->btnstr += btn->getCaption();
					this->btnstr += "</button>";
					i++;
				} else if( _group == "" && group == 0 ){
					this->btnstr += "<button style='"+css+"' name='";
					this->btnstr += name;
					this->btnstr += "'>";
					this->btnstr += btn->getCaption();
					this->btnstr += "</button>";					
				}
				
				if( btn->callback_res ) this->res_callback = true;
		}
		
		if( group != 0 ) this->btnstr += "</fieldset>";
		else this->btnstr += "</div>"; 
}

String VirtualButtons::getButtonsForm( void ){
String h = this->header != "" ? "<h2>"+this->header+"</h2>" : "";
return h+this->Display()+"<form id='btn_form' method='GET' action='click'>"+this->get_btn()+"</form>";	
}

void VirtualButtons::setDisplay( String(*callback)( void ), uint16_t refresh_interval ){
	this->display_callback = callback;
	this->display_refresh_interval = refresh_interval;
}

void VirtualButtons::setHeaderText( const char* str ){
	this->header = str;
}

void VirtualButtons::setTitleText( const char* str ){
	this->title = str;
}

String VirtualButtons::getTitleText( void ){
	return this->title;
}