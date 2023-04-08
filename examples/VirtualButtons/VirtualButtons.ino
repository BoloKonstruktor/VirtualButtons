#include <WiFiConnector.h>
#include <VirtualButtons.h>


WebServer server( 80 );
VirtualButtons vb(&Serial);

uint8_t VOL, CHAN;
bool PLAYING, MUTE;


void WCEvents(uint8_t event, WIFIParam* wifi);

void vol_incr_callback( void );
void vol_decr_callback( void );
void chan_incr_callback( void );
void chan_decr_callback( void );
void webradio_callback( void );
bool mute_callback( bool read_only );

void setup() {
  Serial.begin( 115200 );
  unsigned addr = 0;
  
  /*Podłączenie i logowanie do sieci Wi-Fi*/
  WiFiConnector* wc = new WiFiConnector;
  wc->registerCallback(WCEvents);
  wc->setSTAName("Virtual Buttons TEST");
  wc->begin(addr, &server);
  delete wc;
  wc = NULL;

  /*Dodawanie wirtualnych przycisków*/
  //Kolejność wywołania poniższych metod, decyduje o kolejności wyświetlania przycisków w grupie 
  auto btn1 = vb.addButton( "decr", "-", vol_decr_callback ); //Metoda addButton zwraca wskaźnik do klasy VirtualButton
  btn1->setGroup("volume");                                   //w celu dodatkowej konfiguracji pojedynczego przycisku
  btn1->setFontSize( 32 );                                     //(przypisanie do grupy, zmiana wielkości czcionki, zmiana koloru tła i.t.p.)
  
  auto btn2 = vb.addButton( "incr", "+", vol_incr_callback );
  btn2->setGroup("volume");
  btn2->setFontSize( 32 );
  
  auto btn3 = vb.addButton( "decr", "-", chan_decr_callback );
  btn3->setGroup("channel");
  btn3->setFontSize( 32 );
  
  auto btn4 = vb.addButton( "incr", "+", chan_incr_callback );
  btn4->setGroup("channel");
  btn4->setFontSize( 32 );

  vb.addButton( "play", "PLAY", play_callback );
  vb.addButton( "stop", "STOP", stop_callback );
  vb.addButton( "web_radio", "WEB RADIO", webradio_callback );
  vb.addButton( "mute", "MUTE", mute_callback );

  //Kolejność wywołania poniższych metod, decyduje o kolejności wyświetlania grup przycisków
  vb.createGroup( "channel", "Channel / Track" ); //Tworzenie grupy 'channel'
  vb.createGroup( "volume", "Volume" ); //Tworzenie grupy 'volume'
  vb.createGroup(); //Tworzenie grupy niezdefiniowanej

  vb.setDisplay( display_callback ); //Przypisanie funkcji wyświetlacza
  vb.setHeaderText( "VirtualButtons" ); //Ustawienie tekstu nagłówka
  vb.setTitleText( "Virtual Buttons - TEST" ); //Ustawienie tekstu w tytule strony 
  vb.begin( "/", &server );

  server.begin();  
}

void loop() {
  server.handleClient();
  vb.loop();
}

void WCEvents(uint8_t event, WIFIParam* wifi) {

  switch (event) {
    case WC_WIFI_SCAN:
      {
        Serial.println(F("Wyszukiwanie sieci ..."));
      }
      break;
    case WC_STA_START:
      {
        Serial.print(F("Łącze z siecią: "));
        Serial.print(wifi->ssid);
        Serial.print(F(" "));
      }
      break;
    case WC_STA_CONNECTING:
      {
        Serial.print(F("."));
      }
      break;
    case WC_STA_CONNECTED:
      {
        Serial.println();
        Serial.println(F("Połączony."));
        Serial.print(F("IP: "));
        Serial.println(wifi->ip);
      }
      break;
    case WC_STA_TIMEOUT:
      {
        Serial.println(F("Czas na połączenie z siecią: '"));
        Serial.print(wifi->ssid);
        Serial.println(F("' minął."));
      }
      break;
    case WC_AP_START:
      {
        Serial.println(F("Uruchomiono punkt dostępowy."));
        Serial.print(F("SSID: "));
        Serial.println(wifi->ssid);
        Serial.print(F("IP: "));
        Serial.println(wifi->ip);
      }
      break;
  }
}

//Callback dla przycisku monostabilnego (chwilowego)
void vol_incr_callback( void ){

    if( VOL < 21 ) VOL++;
  
  Serial.print( F("VOL_INCR ") );
  Serial.println( VOL );
}

void vol_decr_callback( void ){
    
    if( VOL ) VOL--;
  
  Serial.print( F("VOL_DECR ") );
  Serial.println( VOL );
}

void chan_incr_callback( void ){
    
    if( CHAN < 11 ) CHAN++;
  
  Serial.print( F("CHAN_INCR ") );
  Serial.println( CHAN );
}

void chan_decr_callback( void ){

    if( CHAN ) CHAN--;

  Serial.print( F("CHAN_DECR ") );
  Serial.println( CHAN );
}

void play_callback( void ){
  PLAYING = 1;
  Serial.println( F("PLAY") );
}

void stop_callback( void ){
   PLAYING = 0;
  Serial.println( F("STOP") );
}

void webradio_callback( void ){
  Serial.println( F("WEB_RADIO") );
}

//Callback zwracający stan dla przycisku bistabilnego.
bool mute_callback( bool read_only /*wartość 'true' blokuje zmianę stanu przy wywołaniu*/ ){
    if( !read_only ) MUTE ^= 1;    
  return MUTE; 
}

//Callback zwracający String z kodem HTML dla wyświetlacza
String display_callback( void ){
  String str = "<p>";
  str += "CHANNEL:";
  str += "<b>"+lead_chars( String( CHAN+1 ).c_str(), "00" )+"</b>";
  str += "/";
  str += String( 12 ); 
  str += "<span style='float:right;'>";
  str += "<b>";

    if( PLAYING ) str += "PLAYING";
    else str += "STOPPED";

  str += "</b>";
  str += "</span>";
  str += "</p>";
  str += "<p><b style='font-size:16px;'>";
  str += "Stacja nr "+String( CHAN+1 );
  str += "</b></p>";
  return str;
}

String lead_chars( const char* str, const char* mask ){
  uint8_t slen = strlen( str ), mlen = strlen( mask );
  String lead = "";
  
    for( uint8_t i=0; i<mlen-slen; i++ ) lead += mask[i];
  
  return lead+String( str );
}