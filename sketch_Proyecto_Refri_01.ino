#include <TroykaDHT.h>
#include <SoftwareSerial.h>

// ===== PINES =====
#define PIN_REED 13
#define PIN_BUZZER 8
#define PIN_GAS A0
#define LED_ROJO 10
#define LED_AZUL 11

// ===== UMBRALES =====
#define UMBRAL_GAS 210
#define TIEMPO_ALERTA_PUERTA 15000  // 15 s

// ===== COMUNICACIÓN =====
SoftwareSerial esp8266(2, 3);
DHT dht(7, DHT11);

// ===== CREDENCIALES =====
String ssid = "CNT_GONZALEZ";
String password = "1716398019sgz";
String apiKey = "FFP955F094GEX5DT";
String host = "api.thingspeak.com";

// ===== TIEMPOS =====
unsigned long ultimaActualizacion = 0;
unsigned long inicioPuertaAbierta = 0;
unsigned long ultimaAlarmaGas = 0;
unsigned long ultimaAlarmaPuerta = 0;
unsigned long ultimoTickTiempo = 0;

// ===== CONTADORES =====
unsigned long tiempoPuertaAbierta = 0;
unsigned long tiempoPuertaCerrada = 0;

// ===== ESTADOS =====
bool puertaAbierta = false;
bool gasPrevio = false;
int alarmasGas = 0;

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);

  pinMode(PIN_REED, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  dht.begin();

  delay(2000);

  Serial.println(F("\n╔════════════════════════════════════════╗"));
  Serial.println(F("║   REFRIGERADOR INTELIGENTE IOT         ║"));
  Serial.println(F("╚════════════════════════════════════════╝\n"));

  enviarComando("AT+UART_DEF=9600,8,1,0,0", 2000, false);
  esp8266.begin(9600);
  conectarWiFi();

  Serial.println(F("[ ] Precalentando sensor MQ-4..."));
  for (int i = 0; i < 12; i++) {
    delay(5000);
    Serial.print(F("."));
  }
  Serial.println(F(" ✓\n"));
}

void loop() {
  dht.read();

  float temperatura = (dht.getState() == DHT_OK) ? dht.getTemperatureC() : -1;
  float humedad = (dht.getState() == DHT_OK) ? dht.getHumidity() : -1;
  int nivelGas = analogRead(PIN_GAS);
  int estadoPuerta = (digitalRead(PIN_REED) == LOW) ? 0 : 1;  // 0 cerrada, 1 abierta

  // ===== CONTROL DE PUERTA =====
  if (estadoPuerta == 1 && !puertaAbierta) {
    puertaAbierta = true;
    inicioPuertaAbierta = millis();
    ultimaAlarmaPuerta = 0;

    Serial.println(F("\n┌─────────────────────────┐"));
    Serial.println(F("│     PUERTA ABIERTA      │"));
    Serial.println(F("└─────────────────────────┘"));
  }

  if (estadoPuerta == 0 && puertaAbierta) {
    puertaAbierta = false;

    Serial.println(F("\n┌─────────────────────────┐"));
    Serial.println(F("│     PUERTA CERRADA      │"));
    Serial.println(F("└─────────────────────────┘"));
  }

  digitalWrite(LED_AZUL, puertaAbierta);

  // ===== CONTADOR DE TIEMPO EN TIEMPO REAL =====
  while (millis() - ultimoTickTiempo >= 1000) {
    ultimoTickTiempo += 1000;
    if (puertaAbierta) tiempoPuertaAbierta++;
    else tiempoPuertaCerrada++;
  }

  // ===== ALARMA PUERTA ABIERTA =====
  if (puertaAbierta && millis() - inicioPuertaAbierta >= TIEMPO_ALERTA_PUERTA) {
    if (ultimaAlarmaPuerta == 0 || millis() - ultimaAlarmaPuerta >= TIEMPO_ALERTA_PUERTA) {
      tone(PIN_BUZZER, 1000, 500);
      Serial.println(F("ALERTA: Puerta abierta >15s"));
      ultimaAlarmaPuerta = millis();
    }
  }

  // ===== GAS =====
  bool gasDetectado = (nivelGas > UMBRAL_GAS);

  if (gasDetectado) {
    if (!gasPrevio) {
      alarmasGas++;
      Serial.println(F("\n╔═══════════════════════════╗"));
      Serial.println(F("║      ALARMA DE GAS        ║"));
      Serial.println(F("╚═══════════════════════════╝"));
    }

    digitalWrite(LED_ROJO, !digitalRead(LED_ROJO));

    if (millis() - ultimaAlarmaGas >= 2000) {
      tone(PIN_BUZZER, 2000, 500);
      ultimaAlarmaGas = millis();
    }
  } else {
    digitalWrite(LED_ROJO, LOW);
  }
  gasPrevio = gasDetectado;

  // ===== ENVÍO A THINGSPEAK =====
  if (millis() - ultimaActualizacion >= 15000) {
    Serial.println(F("\n════════════════════════════════════════"));
    Serial.println(F("     ENVIANDO DATOS A THINGSPEAK"));
    Serial.println(F("════════════════════════════════════════\n"));

    Serial.println(F("┌──────────────────────────────────────┐"));
    Serial.print(F("│   Temperatura:        "));
    Serial.print(temperatura >= 0 ? String(temperatura, 1) + " °C" : "ERROR");
    Serial.println(F("        │"));

    Serial.print(F("│   Humedad:            "));
    Serial.print(humedad >= 0 ? String(humedad, 0) + " %" : "ERROR");
    Serial.println(F("           │"));

    Serial.print(F("│   Nivel de Gas:       "));
    Serial.print(nivelGas);
    Serial.println(F(" ppm         │"));

    Serial.print(F("│   Estado de Puerta:   "));
    Serial.print(puertaAbierta ? F("ABIERTA") : F("CERRADA"));
    Serial.println(F("        │"));

    Serial.print(F("│   Alarmas de Gas:     "));
    Serial.print(alarmasGas);
    Serial.println(F("              │"));

    Serial.print(F("│   Tiempo Abierta:     "));
    Serial.print(tiempoPuertaAbierta);
    Serial.println(F(" seg         │"));

    Serial.print(F("│   Tiempo Cerrada:     "));
    Serial.print(tiempoPuertaCerrada);
    Serial.println(F(" seg         │"));

    Serial.println(F("└──────────────────────────────────────┘\n"));

    enviarDatos(temperatura, humedad, nivelGas, puertaAbierta,
                alarmasGas, tiempoPuertaAbierta, tiempoPuertaCerrada);

    ultimaActualizacion = millis();
  }

  delay(100);
}

// ===== FUNCIONES =====

void conectarWiFi() {
  enviarComando("AT+RST", 2000, false);
  enviarComando("AT+CWMODE=1", 1000, false);
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  enviarComando(cmd, 10000, false);
}

void enviarDatos(float temp, float hum, int gas, bool puerta,
                 int alarmas, unsigned long tAbierta, unsigned long tCerrada) {

  enviarComando("AT+CIPSTART=\"TCP\",\"" + host + "\",80", 3000, false);

  String datos =
    "GET /update?api_key=" + apiKey +
    "&field1=" + String(temp) +
    "&field2=" + String(hum) +
    "&field3=" + String(gas) +
    "&field4=" + String(puerta) +
    "&field5=" + String(alarmas) +
    "&field6=" + String(tAbierta) +
    "&field7=" + String(tCerrada) +
    " HTTP/1.1\r\nHost: " + host + "\r\n\r\n";

  enviarComando("AT+CIPSEND=" + String(datos.length()), 1000, false);
  esp8266.print(datos);
  delay(2000);
  enviarComando("AT+CIPCLOSE", 1000, false);
}

String enviarComando(String cmd, int timeout, bool debug) {
  esp8266.println(cmd);
  String respuesta = "";
  unsigned long t = millis();

  while (millis() - t < timeout) {
    while (esp8266.available()) {
      respuesta += char(esp8266.read());
    }
  }

  if (debug) Serial.println(respuesta);
  return respuesta;
}
