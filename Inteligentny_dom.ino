#include <EtherCard.h> //dodanie biblioteki do modulu sieciowego
#include "DHT.h" //dodanie biblioteki do czujniuka temp i wilg
#define DHTPIN 9 //definiowanie czujnika dht11
#define DHTTYPE DHT11



//do modulu sieciowego
#define STATIC 0
#if STATIC
static byte myip[] = { 192,168,1,200 };
static byte gwip[] = { 192,168,1,1 };
#endif

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[4096];
BufferFiller bfill;


int rh, rt, fot1, ziemia1, gaz1, deszcz1; //deklaracja zmiennych

//definiowanie portow
int ledr = 3; //RGB RED
int ledg = 2; //RGB GREEN
//int ledb = 4; //RGB BLUE
int fotor = A0; // fotorezystor
int ziemia = A1; //czujnik gleby
int pompa = 7; //pompa do nawadniania kwiatka
int gaz = A2; //czujnik gazu
int led_gaz = 6;
int deszcz = A3;
bool zamykanie = false;
//uruchamianie czujnika dht
DHT dht(DHTPIN, DHTTYPE);




void setup(){

  pinMode(ledr, OUTPUT);
  pinMode(ledg, OUTPUT);
  pinMode(pompa, OUTPUT);
  pinMode(led_gaz, OUTPUT);



  digitalWrite(ledr, LOW);
  digitalWrite(ledg, LOW);
  digitalWrite(pompa, LOW);
  digitalWrite(led_gaz, LOW);

  Serial.begin(9600);
  Serial.println("Start programu");

  dht.begin();


  if (ether.begin(sizeof Ethernet::buffer, mymac, 53) == 0)
    Serial.println( "Failed to access Ethernet controller");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");
#endif
  Serial.println("Polaczono");
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  Serial.println("");
  Serial.println("");
}

//strona html
static word strona()
{
bfill = ether.tcpOffset();
  bfill.emit_p(PSTR(
"HTTP/1.0 503 Service Unavailable\r\n"
"Content-Type: text/html\r\n"
"Retry-After: 600\r\n"
"\r\n"
"<html lang='pl'>"
  "<head><meta charset='utf-8'><title>"
    "Arduino"
  "</title></head>"
  "<body style='font-size:30; text-align:center; background-color:#FFAE42;'>"
    "<h3>Strona z arduino</h3>"
	  "Temperatura: $D <br> "
	  "Wilgorność: $D<br>"
    "Jasność: $D<br>"
    "Wilgotność gleby: $D<br>"
    "Poziom gazu: $D<br>"
    "Deszcz: $D<br>"

    "<p style='color:green; font-size:40'>"
    "<script>"
      "if ($D > 800) {document.write('Brak opadów');}"
      "else {document.write('Pada deszcz');}"
    "</script></p>"

    "<p style='color:red; font-size:40'>"
    "<script>"
      "if ($D > 800) {document.write('Wysoki poziom gazu!');}"
    "</script></p>"

    
  "</body>"
"</html>"

),
rt, rh, fot1, ziemia1, gaz1, deszcz1, deszcz1, gaz1); //zmienne ktore sa na stronie
  return bfill.position();
}




void loop(){

//definiowanie zmiennych do odczytu czujnikow
  rt = dht.readTemperature();
  rh = dht.readHumidity();


//definiowanie zmiennych do odczytu analogowych czujnika wilg gleby i fotorezystora
  fot1 = analogRead(fotor);
  ziemia1 = analogRead(ziemia);
  gaz1 = analogRead(gaz);
  deszcz1 = analogRead(deszcz);

  //delay(100);

//wyswietlanie w serial monitorze odczytow z czujnikow
  Serial.print("temperatura:");
  Serial.print(rt);
  Serial.print("  wilgotnosc:");
  Serial.print(rh);
  Serial.print("  jasnosc:");
  Serial.print(fot1);
  Serial.print("  gleba:");
  Serial.print(ziemia1);
  Serial.print("  deszcz:");
  Serial.print(deszcz1);
  Serial.print("  poziom_gazu:");
  //Serial.println(gaz1);

//gaz
  if (gaz1 > 800)
  {
    Serial.print(gaz1);
    Serial.println("  Wysoki poziom gazu!") ;
    digitalWrite(led_gaz, HIGH);
  }
  else
  {
    Serial.println(gaz1);
    digitalWrite(led_gaz, LOW);
  }

//kwiatek
  if(ziemia1 > 1000) //980 wartosc zeby nie zalaczala sie pompa
  {
    digitalWrite(pompa, HIGH);
    Serial.println("Trwa nawadnianie");
    delay(2000);
  }
  else
  {
    digitalWrite(pompa, LOW);
  }



//rolety
  if(!zamykanie){
    if(fot1 > 700 && !zamykanie)
    {
      //zamykanie
      digitalWrite(ledg, HIGH);
      Serial.println("Trwa zaslanianie rolet");
      delay(2000);
      digitalWrite(ledg, LOW);
      zamykanie = true;
    }
  }
    else if (fot1 < 300)
  {
    //otwieranie
    digitalWrite(ledr, HIGH);
    Serial.println("Trwa otwieranie rolet");
    delay(2000);
    digitalWrite(ledr, LOW);
    zamykanie = false;
  }


//wyswitlanie strony internetowej
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);





//zamkniecie wywolania strony
  if (pos){
    ether.httpServerReply(strona());
  }



}


