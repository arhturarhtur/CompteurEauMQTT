//Détection de compteur d'eau
//Documentation pubsubclient : https://pubsubclient.knolleary.net/api.html

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SSID-pwd.h>
#include <Ticker.h>

Ticker boucle_comptage;
Ticker ping_mosquitto;

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
int nb_demi_lune_on = 0;
int nb_demi_lune_off = 0;

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

void reconnexion() {
  while (!client.connected()) {
    //Généré un ID aléatoire
    //Si 2 ID sont identiques le deuxième sera déconnecté
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    // Connexion du client avec lastwill (testament)
    // Le lastwill sera automatiquement mis en cas de déconnexion
    // https://pubsubclient.knolleary.net/api.html#connect2
    if (client.connect(clientId.c_str(), "outTopic", 0, true, "Déconnecté")) {
      client.publish("outTopic", "Connecté à Mosquitto");
      Serial.println("Connecté à Mosquitto");
    }
  }
}

void envoi_demi_lune_on() {
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
}

void envoi_demi_lune_off() {
  // Quand la demi lune n'est plus détectée envoie un message MQTT à Domoticz pour basculer l'interrupteur virtuel sur Off
  demi_lune_off = "{\"command\" : \"switchlight\",\"idx\" : " + String(idx_demi_lune) + ",\"switchcmd\" : \"Off\"}";
  demi_lune_off.toCharArray(mqttmsg_demi_lune_off, demi_lune_off.length() + 1);
  client.publish( mqttTopic, mqttmsg_demi_lune_off);
  Serial.print(mqttTopic);
  Serial.println(mqttmsg_demi_lune_off);
}


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  client.setServer(mqtt_server, 1883);
  Serial.begin(115200);
  boucle_comptage.attach_ms(5, comptage);
  setup_wifi();
}

void loop() {
  //Tant que le client est déconnecté
  if (!client.connected()) {
    reconnexion();
  } else {
    client.loop();
    if (nb_demi_lune_on != 0) {
      // A la detection de la demi lune envoie un message MQTT
      nb_demi_lune_on--;
      envoi_demi_lune_on();

    }
    if (nb_demi_lune_off != 0) {
      // Quand la demi lune n'est plus détectée envoie un message MQTT
      nb_demi_lune_off--;
      envoi_demi_lune_off();
    }
  }
}

void comptage() {
  if (analogRead(CAPTEUR) < 200) {  //Quand le CNY70 detecte quelque chose
    /*Le systeme de bascule permet de simuler un trigger de smith
      et de n'avoir une détection qu'au changement d'état.
    */
    if (bascule == true) {
      Serial.println("Demi lune actif");
      digitalWrite (BUILTIN_LED, HIGH);
      nb_demi_lune_on++;
      bascule = false;
    }
  }
  if (analogRead(CAPTEUR) > 600) {
    if (bascule == false) {
      Serial.println("Demi lune OFF");
      nb_demi_lune_off++;
      digitalWrite (BUILTIN_LED, LOW);
      bascule = true;
    }
  }
}
