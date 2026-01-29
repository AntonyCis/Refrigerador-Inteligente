#include <TroykaDHT.h>        // Librería para sensor DHT11 (temperatura y humedad)
#include <SoftwareSerial.h>  // Permite comunicación serial por pines digitales

// ==================== DEFINICIÓN DE PINES ====================
// Sensor magnético (reed switch) para detectar apertura/cierre de puerta
#define PIN_REED 13

// Buzzer para alarmas sonoras
#define PIN_BUZZER 8

// Entrada analógica del sensor de gas (MQ)
#define PIN_GAS A0

// LED rojo: indica detección de gas
#define LED_ROJO 10

// LED azul: indica puerta abierta
#define LED_AZUL 11

// ==================== UMBRALES Y TIEMPOS ====================
// Valor analógico a partir del cual se considera presencia peligrosa de gas
#define UMBRAL_GAS 210

// Tiempo máximo permitido con la puerta abierta (15 segundos)
#define TIEMPO_ALERTA_PUERTA 15000

// ==================== COMUNICACIÓN ====================
// Comunicación serial por software con el módulo ESP8266
// RX = pin 2, TX = pin 3
SoftwareSerial esp8266(2, 3);

// Objeto para el sensor DHT11 conectado al pin 7
DHT dht(7, DHT11);

// ==================== CREDENCIALES ====================
// Credenciales de la red WiFi
String ssid = "CNT_GONZALEZ";
String password = "1716398019sgz";

// API Key y servidor de ThingSpeak
String apiKey = "FFP955F094GEX5DT";
String host = "api.thingspeak.com";

// ==================== VARIABLES DE TIEMPO ====================
// Controlan temporización sin usar delay()
unsigned long ultimaActualizacion = 0;    // Último envío a ThingSpeak
unsigned long inicioPuertaAbierta = 0;    // Momento en que se abrió la puerta
unsigned long ultimaAlarmaGas = 0;         // Última activación de alarma de gas
unsigned long ultimaAlarmaPuerta = 0;      // Última alarma por puerta abierta
unsigned long ultimoTickTiempo = 0;        // Control del contador por segundo

// ==================== CONTADORES ====================
// Tiempo acumulado (en segundos)
unsigned long tiempoPuertaAbierta = 0;
unsigned long tiempoPuertaCerrada = 0;

// ==================== ESTADOS ====================
// Indica si la puerta está actualmente abierta
bool puertaAbierta = false;

// Guarda el estado previo del gas para detectar cambios
bool gasPrevio = false;

// Contador de eventos de alarma por gas
int alarmasGas = 0;

// =============================================================
// FUNCIÓN SETUP: Se ejecuta una sola vez al iniciar el sistema
void setup() {
  // Comunicación serial para depuración
  Serial.begin(9600);

  // Comunicación inicial con ESP8266
  esp8266.begin(115200);

  // Configuración de pines
  pinMode(PIN_REED, INPUT_PULLUP); // Pull-up interno para el reed switch
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  // Inicialización del sensor DHT
  dht.begin();

  delay(2000); // Espera de estabilidad inicial

  // Mensaje de inicio del sistema
  Serial.println(F("\n╔════════════════════════════════════════╗"));
  Serial.println(F("║   REFRIGERADOR INTELIGENTE IOT         ║"));
  Serial.println(F("╚════════════════════════════════════════╝\n"));

  // Configuración del baudrate del ESP8266
  enviarComando("AT+UART_DEF=9600,8,1,0,0", 2000, false);
  esp8266.begin(9600);

  // Conexión a la red WiFi
  conectarWiFi();

  // Precalentamiento del sensor MQ (recomendado por el fabricante)
  Serial.println(F("[ ] Precalentando sensor MQ-4..."));
  for (int i = 0; i < 12; i++) {
    delay(5000);
    Serial.print(F("."));
  }
  Serial.println(F(" ✓\n"));
}

// =============================================================
// FUNCIÓN LOOP: Se ejecuta continuamente
void loop() {

  // Lectura del sensor DHT
  dht.read();

  // Obtención de temperatura y humedad solo si la lectura es válida
  float temperatura = (dht.getState() == DHT_OK) ? dht.getTemperatureC() : -1;
  float humedad     = (dht.getState() == DHT_OK) ? dht.getHumidity() : -1;

  // Lectura del sensor de gas
  int nivelGas = analogRead(PIN_GAS);

  // Lectura del estado de la puerta
  // LOW = cerrada, HIGH = abierta
  int estadoPuerta = (digitalRead(PIN_REED) == LOW) ? 0 : 1;

  // ==================== CONTROL DE PUERTA ====================
  // Detecta el momento en que la puerta se abre
  if (estadoPuerta == 1 && !puertaAbierta) {
    puertaAbierta = true;
    inicioPuertaAbierta = millis();
    ultimaAlarmaPuerta = 0;

    Serial.println(F("PUERTA ABIERTA"));
  }

  // Detecta el cierre de la puerta
  if (estadoPuerta == 0 && puertaAbierta) {
    puertaAbierta = false;
    Serial.println(F("PUERTA CERRADA"));
  }

  // LED azul indica visualmente si la puerta está abierta
  digitalWrite(LED_AZUL, puertaAbierta);

  // ==================== CONTADOR DE TIEMPO ====================
  // Incrementa contadores cada segundo sin bloquear el programa
  while (millis() - ultimoTickTiempo >= 1000) {
    ultimoTickTiempo += 1000;
    if (puertaAbierta) tiempoPuertaAbierta++;
    else tiempoPuertaCerrada++;
  }

  // ==================== ALARMA DE PUERTA ====================
  // Se activa si la puerta permanece abierta más de 15 segundos
  if (puertaAbierta && millis() - inicioPuertaAbierta >= TIEMPO_ALERTA_PUERTA) {
    if (ultimaAlarmaPuerta == 0 || millis() - ultimaAlarmaPuerta >= TIEMPO_ALERTA_PUERTA) {
      tone(PIN_BUZZER, 1000, 500);
      Serial.println(F("ALERTA: Puerta abierta > 15s"));
      ultimaAlarmaPuerta = millis();
    }
  }

  // ==================== DETECCIÓN DE GAS ====================
  bool gasDetectado = (nivelGas > UMBRAL_GAS);

  if (gasDetectado) {
    // Cuenta la alarma solo cuando cambia de NO gas a gas
    if (!gasPrevio) {
      alarmasGas++;
      Serial.println(F("ALARMA DE GAS"));
    }

    // Parpadeo del LED rojo
    digitalWrite(LED_ROJO, !digitalRead(LED_ROJO));

    // Activación periódica del buzzer
    if (millis() - ultimaAlarmaGas >= 2000) {
      tone(PIN_BUZZER, 2000, 500);
      ultimaAlarmaGas = millis();
    }
  } else {
    digitalWrite(LED_ROJO, LOW);
  }

  gasPrevio = gasDetectado;

  // ==================== ENVÍO A THINGSPEAK ====================
  // Envío de datos cada 15 segundos
  if (millis() - ultimaActualizacion >= 15000) {
    enviarDatos(temperatura, humedad, nivelGas, puertaAbierta,
                alarmasGas, tiempoPuertaAbierta, tiempoPuertaCerrada);
    ultimaActualizacion = millis();
  }

  delay(100); // Pequeña pausa para estabilidad
}

// =============================================================
// FUNCIÓN: CONEXIÓN A WiFi
void conectarWiFi() {
  enviarComando("AT+RST", 2000, false);      // Reinicio del ESP8266
  enviarComando("AT+CWMODE=1", 1000, false); // Modo estación
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  enviarComando(cmd, 10000, false);          // Conexión a la red
}

// =============================================================
// FUNCIÓN: ENVÍO DE DATOS A THINGSPEAK
void enviarDatos(float temp, float hum, int gas, bool puerta,
                 int alarmas, unsigned long tAbierta, unsigned long tCerrada) {

  // Apertura de conexión TCP
  enviarComando("AT+CIPSTART=\"TCP\",\"" + host + "\",80", 3000, false);

  // Construcción de la petición HTTP GET
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

  // Envío de datos
  enviarComando("AT+CIPSEND=" + String(datos.length()), 1000, false);
  esp8266.print(datos);
  delay(2000);

  // Cierre de conexión
  enviarComando("AT+CIPCLOSE", 1000, false);
}

// =============================================================
// FUNCIÓN: ENVÍO DE COMANDOS AT AL ESP8266
String enviarComando(String cmd, int timeout, bool debug) {
  esp8266.println(cmd);
  String respuesta = "";
  unsigned long t = millis();

  // Lee la respuesta del ESP8266 durante el tiempo indicado
  while (millis() - t < timeout) {
    while (esp8266.available()) {
      respuesta += char(esp8266.read());
    }
  }

  // Muestra la respuesta si se habilita el modo debug
  if (debug) Serial.println(respuesta);
  return respuesta;
}
