#pragma once

#define WIFI_SSID "IZZI-32A4"
#define WIFI_PASSWORD "yrCMGcqKkpfMh7EhNK"
#define SERVER_URL "https://monitor.molart.com.mx/api/device/heartbeat"
#define DEVICE_API_KEY "5a8e9f7aa90160cad1ffa008d370dbd44489116fe955f95c289ae26a4a09fc86"

// Solo para la primera prueba. En producción use false y complete ROOT_CA.
#define TLS_INSECURE true
static const char ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
PEGAR_AQUI_LA_CA_RAIZ
-----END CERTIFICATE-----
)EOF";
