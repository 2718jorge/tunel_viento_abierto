#include <HX711.h>
#include <LiquidCrystal_I2C.h>

const int ESPERA_ARRANQUE_TURBINA = 3000;
const int ESPERA_APAGADO_TURBINA = 7000;
const int TIEMPO_MEDIDAS = 10000;

const double CORR_X = 50558.5/68;
const double CORR_Y = 49526.0/68;

const int SENSOR_X_DATO_PIN = 4;
const int SENSOR_X_SINC_PIN = 5;
const int SENSOR_Y_DATO_PIN = 2;
const int SENSOR_Y_SINC_PIN = 3;
const int BOTON_PIN = 6;
const int RELE_PIN = 7;

const int COLS = 16;
const int LECTURAS = 1;
const int ITERS = 3;

                       // 0123456789012345 
const char BLANCOS[17] = "                ";

HX711 SENSOR_X;
HX711 SENSOR_Y;

LiquidCrystal_I2C LCD1(0x23,COLS,2);  
LiquidCrystal_I2C LCD2(0x27,COLS,2);

void setup() {
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);
  pinMode(BOTON_PIN, INPUT);

  Serial.begin(57600);
  SENSOR_X.begin(SENSOR_X_DATO_PIN, SENSOR_X_SINC_PIN);
  SENSOR_Y.begin(SENSOR_Y_DATO_PIN, SENSOR_Y_SINC_PIN);

  LCD1.init();  LCD2.init();
  LCD1.backlight();  LCD2.backlight();
  LCD1.clear();  LCD2.clear();
  escribeAccion("PULSE BOTON");
}

bool activo = false;

void loop() {
  int estadoBoton = digitalRead(BOTON_PIN);
  if (estadoBoton == LOW) return;
  LCD1.clear();  LCD2.clear();
  escribeAccion("EMPEZANDO ANALISIS");
  delay(2000);
  
  int ns = 0;
  long ts = 0;
  double xss = 0, yss = 0, xss2 = 0, yss2 = 0;

  for (int i = 0; i < ITERS; i++) {
    Serial.print("Intento "); Serial.println(i);

    escribeAccion("TARANDO SENSORES");
    SENSOR_X.tare();  SENSOR_Y.tare();
    SENSOR_X.set_scale(CORR_X);  SENSOR_Y.set_scale(CORR_Y);

    escribeAccion("ARRANCANDO TURBINA");
    digitalWrite(RELE_PIN, HIGH);
    int n = 0;
    long t0 = millis();
    while (millis()-t0 < ESPERA_ARRANQUE_TURBINA) {
      n++;
      float x = SENSOR_X.get_units(LECTURAS);
      float y = -SENSOR_Y.get_units(LECTURAS);
      escribeIteracion(n, millis()-t0);
      escribeMedida("X", 0, n, x, 0);
      escribeMedida("Y", 1, n, y, 0);
    }

    escribeAccion("TOMANDO MEDIDAS");
    double xs = 0, ys = 0, xs2 = 0, ys2 = 0;
    n = 0;
    t0 = millis();
    while (millis()-t0 < TIEMPO_MEDIDAS) {
      n++;
      float x = SENSOR_X.get_units(LECTURAS);
      float y = -SENSOR_Y.get_units(LECTURAS);
      xs += x;  ys += y;  xs2 += x*x;  ys2 += y*y;
      escribeIteracion(n, millis()-t0);
      escribeMedida("X", 0, n, x, xs);
      escribeMedida("Y", 1, n, y, ys);
    }

    ns += n; ts += millis() - t0;
    xss += xs; yss += ys; xss2 += xs2; yss2 += ys2;
    escribeIteracion(ns, ts);
    escribeTotal("X", 0, ns, xss, xss2);
    escribeTotal("Y", 1, ns, yss, yss2);

    escribeAccion("APAGANDO TURBINA");
    digitalWrite(RELE_PIN, LOW);
    delay(ESPERA_APAGADO_TURBINA);

    escribeAccion("PULSE BOTON");
  }
}

void escribeAccion(const char* accion) {
  Serial.println(accion);
  limpiaLinea(LCD1, 0);
  LCD1.print(accion);
}

void escribeIteracion(int n, long t) {
  char buf[COLS+1];
  saturaLinea(buf, sprintf(buf, "I:%3i T:%2.3f", n, t/1000.0));
  Serial.println(buf);
  LCD1.setCursor(0,1);
  //print_clear(LCD1, buf);
  LCD1.print(buf);
}

void escribeMedida(const char* eje, int line, int n, float v, float s) {
  char buf[COLS+1];
  saturaLinea(buf, sprintf(buf, "%s: %3.2f %3.2f", eje, v, s/n));
  Serial.println(buf);
  LCD2.setCursor(0,line);
  LCD2.print(buf);
}

void escribeTotal(const char* eje, int line, int n, float s, float s2) {
  char buf[COLS+1];
  float m = s/n;
  float dt = sqrt(s2/n - m*m);
  saturaLinea(buf, sprintf(buf, "%s: %3.2f %3.2f", eje, s/n, dt));
  Serial.println(buf);
  LCD2.setCursor(0,line);
  //print_clear(LCD2, buf);
  LCD2.print(buf);
}

void saturaLinea(char* buf, int n) {
  for (int i = n; i < COLS; i++) buf[i] = ' ';
  buf[COLS] = '\0';
}

void limpiaLinea(LiquidCrystal_I2C& lcd, int linea) {
  lcd.setCursor(0,linea);
  lcd.print(BLANCOS);
  lcd.setCursor(0,linea);
}
