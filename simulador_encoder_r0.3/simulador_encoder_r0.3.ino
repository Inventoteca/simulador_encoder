/*****************************************************************
  ROBOT-ENCODER
  Para hacer pruebas con el encoder.
  Tiene 3 botones y una entrada para la señal de la electroválvula.

  1. Giro contínuo. Termina hasta que se presiona el botón de paro.
  2. Diez vueltas. Se para cuando completa 10 vueltas o se presiona el botón de paro.
  3. Botón de paro.
  4. Señal de electroválvula. Se detiene cuando baja el voltaje o se presiona el botón de paro.

  Materiales:
  Arduino Nano
  Motor NEMA 17
  Módulo A4988 https://www.pololu.com/file/0j450/a4988_dmos_microstepping_driver_with_translator.pdf
  Regulador Mini560
*****************************************************************/

const unsigned int duracion_inercia = 1000; //milisegundos
unsigned long t_inercia; //momento en el que comienza el giro por inercia

// Definir pines
// salidas al controlador del motor
#define DIR 3 //direction
#define STP 4 //step
#define SLP 5 //sleep
#define RST 6 //reset
#define MS3 7 //
#define MS2 8 // } micro-step
#define MS1 9 //
#define EN 10 //enable
// entradas de botones y señal de la válvula
#define VAL  2  //señal de la válvula
#define BINI  A0 //botón inicio de giro
#define BDIEZ A3 //botón diez vueltas
#define BPARO A5 //botón paro
#define POT  A7 //pot para controlar la velocidad

// Se usa un motor NEMA17 con 200 pasos por vuelta,
// 1.8 grados por paso. Si se realizan medios pasos
// se requieren 400 pulsos por vuelta.
#define PULSOS_VUELTA 400
// el retardo mínimo entre pasos es 400 micro segundos
// la duración de un pulso para dar un paso puede ser de 1 micro segundo
// delayMicroseconds() works very accurately in the range 3 microseconds and up

// Modos de funcionamiento
#define MPARO 0 //modo paro
#define MGIRO 1 //modo giro contínuo
#define MDIEZ 2 //modo diez vueltas
#define MVAL  3 //modo valvula
#define MINRC 4 //modo inercia
byte modo = MPARO; //inicia detenido

// Variables
const float DEL_MAX = 9000.0; //retardo máximo entre pulsos
const unsigned int DEL_MIN = 500; //retardo mínimo entre pulsos
unsigned int del; //delay o retardo (microsegundos)
//del = DEL_MIN + (DEL_MAX * POT / 1023)
float pot; //(float)analogRead(POT);
//estado de los botones
bool ini,  ini_prev;  //digitalRead(BINI);
bool diez, diez_prev; //digitalRead(BDIEZ);
bool val,  val_prev;  //digitalRead(VAL);
bool paro, paro_prev; //digitalRead(BPARO);
unsigned int cont; //contador de pulsos

void setup() {
  // Pines como salida
  for (int i = DIR; i <= EN; i++) {
    pinMode(i, OUTPUT);
  }
  // Entradas digitales
  pinMode(VAL, INPUT_PULLUP);
  pinMode(BINI, INPUT_PULLUP);
  pinMode(BDIEZ, INPUT_PULLUP);
  pinMode(BPARO, INPUT_PULLUP);
  // Entrada analogica
  analogRead(POT);

  digitalWrite(EN,  1); //inicia deshabilitado
  digitalWrite(DIR, 0);
  digitalWrite(STP, 0);
  digitalWrite(SLP, 1); //no sleep
  digitalWrite(RST, 1); //no reset
  digitalWrite(MS3, 0); //
  digitalWrite(MS2, 1); // } micro-step
  digitalWrite(MS1, 0); //
  /*
    MS1 MS2 MS3 Microstep Resolution Excitation Mode
    L   L   L   Full Step 2 Phase
    H   L   L   Half Step 1-2 Phase
    L   H   L   Quarter Step W1-2 Phase
    H   H   L   Eighth Step 2W1-2 Phase
    H   H   H   Sixteenth Step 4W1-2 Phase
  */

  Serial.begin(115200);
  Serial.println("ROBOT-ENCODER");
  Serial.println("400 pulsos por vuelta");
  Serial.println("(micro-step: medio paso)");
  //delay(200);
}

void loop() {
  // leer entradas
  float pot  = (float)analogRead(POT); //leer pot
  del = DEL_MIN + int(DEL_MAX * pot / 1023.0); //calcular delay entre pasos
  ini_prev  = ini;   ini  = digitalRead(BINI); //guardar estado previo de los botones
  diez_prev = diez;  diez = digitalRead(BDIEZ);
  val_prev  = val;   val  = digitalRead(VAL);
  paro_prev = paro;  paro = digitalRead(BPARO);

  // Si se está presionando el botón de paro
  if (paro == 0) { //parar y mostrar el número de pulsos
    modo = MPARO; //modo paro
    if (paro_prev == 1) {
      Serial.println(cont);
    }
  }
  //Serial.print(val); Serial.print("  ");
  //Serial.print(ini); Serial.print("  ");
  //Serial.print(diez); Serial.print("  ");
  //Serial.print(paro); Serial.print("  ");
  //Serial.println(pot);

  switch (modo) { //actuar dependiendo del modo de funcionamiento

    case MPARO: //cuando está detenido revisa el estado de las entradas -------------
      digitalWrite(EN, 1); //motor deshabilitado

      if (ini == 0 and ini_prev == 1) {//se acaba de presionar el botón de giro contínuo
        Serial.println("giro");
        modo = MGIRO; //modo giro continuo
        cont = 0;
      }
      if (diez == 0 and diez_prev == 1) {//se acaba de presionar el botón de diez vueltas
        Serial.println("diez");
        modo = MDIEZ; //modo diez vueltas
        cont = 0;
      }
      if (val == 0 and val_prev == 1) {//se acaba de activar la válvula
        Serial.println("valvula");
        modo = MVAL; //modo valvula
        cont = 0;
      }

      delayMicroseconds(DEL_MIN); //retardo entre loops
      break;

    case MGIRO: //modo giro continuo -----------------------------------------------
      pulso();
      //Serial.println(cont);
      break;

    case MDIEZ: //modo diez vueltas -------------------------------------------------
      pulso(); //dar un paso
      if (cont >= PULSOS_VUELTA * 10) {//si se completaron las 10 vueltas
        Serial.println(cont); //
        modo = MPARO; //se detiene
      }
      break;

    case MVAL: //modo valvula ----------------------------------------------------
      pulso(); //dar un paso
      if (val == 1) {//si se desactivó la válvula
        Serial.println("inercia"); //
        modo = MINRC; //MPARO; //cambia a modo inercia
        t_inercia = millis();
      }
      break;

    case MINRC: //modo inercia ------------------------------------------
      pulso();
      // si pasa el tiempo de duración de la inercia
      if (millis() - t_inercia > duracion_inercia) {
        Serial.println(cont); //
        modo = MPARO; //se detiene
      }
      break;
  }
}

void pulso() {
  digitalWrite(EN, 0); //habilitado
  digitalWrite(STP, 1);
  delayMicroseconds(10);
  digitalWrite(STP, 0);
  delayMicroseconds(del);
  cont++;
}

/*void dar_diez_vueltas() {
  digitalWrite(EN, 0); //habilitado
  for (int i = 0; i < 4000; i++) {
    digitalWrite(STP, 1);
    delayMicroseconds(10);
    digitalWrite(STP, 0);
    delayMicroseconds(400);
  }
  digitalWrite(EN, 1);
  }*/
