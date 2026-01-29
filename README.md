# 🧊 Refrigerador Inteligente IoT

Proyecto académico de **Internet de las Cosas (IoT)** desarrollado con **Arduino**, orientado al **monitoreo, seguridad y control remoto** de un refrigerador mediante sensores y comunicación WiFi.

---

## 📌 Descripción del Proyecto

Este sistema permite supervisar en tiempo real:

* 🌡️ Temperatura interna
* 💧 Humedad relativa
* 🔥 Presencia de gas inflamable
* 🚪 Estado de la puerta (abierta / cerrada)
* ⏱️ Tiempo acumulado de puerta abierta y cerrada
* 🚨 Número de alarmas por gas

Además, genera **alarmas locales** (sonoras y visuales) ante condiciones de riesgo y envía los datos periódicamente a la plataforma **ThingSpeak**, permitiendo su visualización remota y análisis histórico.

---

## 🏗️ Arquitectura del Sistema

* **Microcontrolador:** Arduino
* **Sensores:**

  * DHT11 (temperatura y humedad)
  * MQ (detección de gas)
  * Reed Switch (estado de la puerta)
* **Actuadores:**

  * Buzzer (alarma sonora)
  * LEDs (indicadores visuales)
* **Comunicación:** ESP8266 (WiFi)
* **Plataforma IoT:** ThingSpeak

El sistema sigue una arquitectura **Edge + Cloud**, donde Arduino actúa como nodo de borde y ThingSpeak como servicio en la nube.

---

## ⚙️ Funcionalidades Principales

* Lectura continua de sensores ambientales
* Detección de fugas de gas con alarma automática
* Alerta por puerta abierta más de 15 segundos
* Conteo de eventos críticos
* Medición de tiempos en segundos (puerta abierta/cerrada)
* Envío de datos cada 15 segundos a la nube
* Monitoreo remoto en tiempo real

---

## 🔌 Asignación de Pines

| Componente         | Pin |
| ------------------ | --- |
| Sensor DHT11       | 7   |
| Sensor de Gas (MQ) | A0  |
| Reed Switch        | 13  |
| Buzzer             | 8   |
| LED Rojo (Gas)     | 10  |
| LED Azul (Puerta)  | 11  |
| ESP8266 RX         | 2   |
| ESP8266 TX         | 3   |

---

## 🌐 Envío de Datos a ThingSpeak

Los datos se envían mediante una petición **HTTP GET** utilizando comandos AT del ESP8266.

| Campo ThingSpeak | Variable                  |
| ---------------- | ------------------------- |
| field1           | Temperatura (°C)          |
| field2           | Humedad (%)               |
| field3           | Nivel de gas              |
| field4           | Estado de la puerta       |
| field5           | Alarmas de gas            |
| field6           | Tiempo puerta abierta (s) |
| field7           | Tiempo puerta cerrada (s) |

---

## 🧠 Conceptos Aplicados

* Programación no bloqueante con `millis()`
* Manejo de estados
* Sensado y adquisición de datos
* Sistemas embebidos
* Comunicación TCP/IP
* Internet de las Cosas (IoT)

---

## 📂 Estructura del Proyecto

```
Refrigerador-IoT/
│── refrigerador_iot.ino
│── README.md
```

---

## 🚀 Requisitos

* Arduino IDE
* Módulo ESP8266
* Librerías:

  * TroykaDHT
  * SoftwareSerial
* Cuenta en ThingSpeak

---

## 📖 Uso

1. Clonar el repositorio
2. Configurar credenciales WiFi y API Key de ThingSpeak
3. Cargar el código en Arduino
4. Energizar el sistema
5. Visualizar datos en ThingSpeak

---

## 🎓 Contexto Académico

Este proyecto fue desarrollado con fines **educativos**, aplicando conceptos de:

* Internet de las Cosas
* Electrónica básica
* Programación de microcontroladores
* Sistemas de monitoreo y seguridad

---

## 👨‍💻 Autores

**Antony Cisneros**
**David Cajamarca**
**Alan Logroño**
**Felipe Zapata**
Estudiantes de Desarrollo de Software

---

✨ *Proyecto diseñado para demostrar monitoreo inteligente, seguridad preventiva y conectividad IoT en sistemas embebidos.*
