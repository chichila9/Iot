#include<WiFi.h>
const char *ssid="Arduino";
const char *password="123456789";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while(WiFi.status() !=WL_CONNECTED){
    delay(1000);
    Serial.println("Conectando a la red Wifi..." );
  }
  Serial.println("Conexión  esitosa");
  Serial.print("RSSI ");
  //Serial.println("DISTANCIA");
}

void loop() {
  int rssi = WiFi.RSSI();
  float pred=predict(rssi);
  Serial.print("RSSI ");
  Serial.print(rssi);
  Serial.print("   DISTANCIA: ");
  Serial.println(pred);
  delay(1000);
}

float predict(float x) {
    float coefficients[] = {0.00000000e+00, 1.29301789e+01, 1.21904834e+00, 4.56564712e-02, 7.78878780e-04, 4.68732162e-06};
    float intercept =35.42782857 ;

    float y_pred = intercept;
    for (int i = 0; i < 6; i++) {
        y_pred += coefficients[i] * pow(x, i);
    }

    return y_pred;
}

