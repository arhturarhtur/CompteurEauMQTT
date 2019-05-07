//Détection d'objet

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SSID-pwd.h>


/************************* Serveur MQTT *********************************/

const char* mqtt_server = "192.168.0.143";
const char* mqttTopic = "domoticz/in";


const int CAPTEUR = A0;
int idx_compteur_eau = 78;
int idx_demi_lune = 79;
bool bascule = true;
String compteur = "";
String demi_lune_on = "";
String demi_lune_off = "";

char mqttmsg_compteur[128];
char mqttmsg_demi_lune_on[128];
char mqttmsg_demi_lune_off[128];


WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("espClient")) {
    // Once connected, publish an announcement...
    client.publish("outTopic","Connecté à Mosquitto");
    // ... and resubscribe
    Serial.println("Connecté à Mosquitto");
    client.subscribe(mqttTopic);
  }
  return client.connected();
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  client.setServer(mqtt_server, 1883);
  Serial.begin(115200);
  setup_wifi();
  lastReconnectAttempt = 0;
}

void loop() {

  if (!client.connected()) {
    long now = millis();
    Serial.println("MQTT déconnecté");
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
    comptage();

    client.loop();
  }
}

void comptage() {
    if (analogRead(CAPTEUR) < 200) {  //Quand le CNY70 detecte quelque chose
      /*Le systeme de bascule permet de simuler un trigger de smith
       * et de n'avoir une détection qu'au changement d'état.
       */
/*      if (bascule == true) {
        digitalWrite (BUILTIN_LED, HIGH);
        //A la detection de la demi lune envoie un message MQTT à Domoticz pour incrémenter le compteur
        compteur = "{\"idx\" : " + String(idx_compteur_eau) + ",\"nvalue\" : 0,\"svalue\" : \"1\"}";
        compteur.toCharArray(mqttmsg_compteur, compteur.length() + 1);
        client.publish( mqttTopic, mqttmsg_compteur);
        Serial.print(mqttTopic);
        Serial.println(mqttmsg_compteur);
        //A la detection de la demi lune envoie un message MQTT à Domoticz pour basculer un interrupteur virtuel sur On
        demi_lune_on = "{\"command\" : \"switchlight\",\"idx\" : " + String(idx_demi_lune) + ",\"switchcmd\" : \"On\"}";
        demi_lune_on.toCharArray(mqttmsg_demi_lune_on, demi_lune_on.length() + 1);
        client.publish( mqttTopic, mqttmsg_demi_lune_on);
        Serial.print(mqttTopic);
        Serial.println(mqttmsg_demi_lune_on);
        bascule = false;
      }
*/    }
    if (analogRead(CAPTEUR) > 600) {
/*      if (bascule == false) {
        digitalWrite (BUILTIN_LED, LOW);
        bascule = true;
        //Quand la demi lune n'est plus détectée envoie un message MQTT à Domoticz pour basculer l'interrupteur virtuel sur Off
        demi_lune_off = "{\"command\" : \"switchlight\",\"idx\" : " + String(idx_demi_lune) + ",\"switchcmd\" : \"Off\"}";
        demi_lune_off.toCharArray(mqttmsg_demi_lune_off, demi_lune_off.length() + 1);
        client.publish( mqttTopic, mqttmsg_demi_lune_off);
        Serial.print(mqttTopic);
        Serial.println(mqttmsg_demi_lune_off);
      }
*/    } 
  
}
