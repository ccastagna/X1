/*
   SerialOutput sketch
   Print numbers to the serial port
*/
#include "lib/Kalman.h" // para quitar el ruido de la medición del acelerómetro.
#include <SoftwareSerial.h>    // para la comunicación bluetooth

/*
   Utilizadas para marcar el intervalo de lectura de medición de los sensores.
*/
unsigned long interval = 4000; // the time we need to wait
unsigned long previousMillis = 0; // millis() returns an unsigned long.

/*
   Asignación de sensores a los distintos pines del arduino.
*/
#define FLEX_SENSOR_PULGAR  A0
#define FLEX_SENSOR_INDICE  A1
#define FLEX_SENSOR_MEDIO   A2
#define BLUETOOTH_RX        7
#define BLUETOOTH_TX        8
#define BLUE_LED            0
#define RGB_LED_RED         1
#define RGB_LED_BLUE        2
#define RGB_LED_GREEN       3

/*
   Voltaje que reciven los sensores
*/
#define VCC 4.98 // Measured voltage of Ardunio 5V line

/*
   Valor de las resistencias de cada sensor flex.
   @BEND_RESISTANCE es la medición con el flex cerrado.
   @STRAIGHT_RESISTANCE es la medicón con el flex completmente abierto.
   @DIVISOR_RESISTANCE es la medición de la resistencia fija del divisor de tensión.
*/
#define STRAIGHT_RESISTANCE_FLEX_MEDIO  550.0 // resistance when straight
#define BEND_RESISTANCE_FLEX_MEDIO      75.0 // resistance at 90 deg
#define DIVISOR_RESISTANCE_FLEX_MEDIO   98.0 // Measured resistance of 3.3k resistor

// falta sacar medicion de este
#define STRAIGHT_RESISTANCE_FLEX_INDICE  550.0 // resistance when straight
#define BEND_RESISTANCE_FLEX_INDICE      75.0 // resistance at 90 deg
#define DIVISOR_RESISTANCE_FLEX_INDICE   98.0 // Measured resistance of 3.3k resistor

// falta sacar medicion de este
#define STRAIGHT_RESISTANCE_FLEX_PULGAR  550.0 // resistance when straight
#define BEND_RESISTANCE_FLEX_PULGAR      75.0 // resistance at 90 deg
#define DIVISOR_RESISTANCE_FLEX_PULGAR   98.0 // Measured resistance of 3.3k resistor

/*
 * Valor de cada movimiento.
 * Es el valor que se enviará vía Bluetooth a la aplicación Android.
 */
#define PLAY_PAUSE          0
#define NEXT                1
#define PREVIOUS            2
#define SPEED_X1            3
#define SPEED_X2            4
#define SPEED_X05           5
#define NOT_VALID_GESTURE   8
#define NOT_MOVEMENT        9

/*
 * Valor de la posición de cada sensor flex.
 */
#define BEND_FLEX         0
#define MEDIUM_BEND_FLEX  1
#define STRAIGHT_FLEX     2

/*
 * Valor del sentido al que se mueve el acelerómetro.
 */
#define LEFT  0
#define RIGHT 1


/*
   Variables utilizadas para Kalman e instanciación de un objeto de Kalman.
*/
double measurement;
double filteredMeasurement;
Kalman myFilter(0.125, 32, 1023, 0); //suggested initial values for high noise filtering

/* 
 *  Variables globales utilizzadas para controlar los movimientos de la mano.
 */
int previousMovement = (int) NOT_MOVEMENT;
int newMovement = (int) NOT_MOVEMENT;
double angleFlexPulgar;
double angleFlexIndice;
double angleFlexMedio;
int flexPulgarPosition;
int flexIndicePosition;
int flexMedioPosition;

// Valor a enviar vía bluetooth.
int mensajeBluetooth;

// Declarar objeto de comunicación bluetooth.
SoftwareSerial bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

/*
   Configuración inicial de pines y comunicación.
*/
void setup()
{
  pinMode(FLEX_SENSOR_PULGAR, INPUT);
  pinMode(FLEX_SENSOR_INDICE, INPUT);
  pinMode(FLEX_SENSOR_MEDIO, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RGB_LED_RED, OUTPUT);
  pinMode(RGB_LED_BLUE, OUTPUT);
  pinMode(RGB_LED_GREEN, OUTPUT);

  bluetooth.begin(9600); // Inicializa comunicación bluetooth.
  Serial.begin(9600); // Inicializa comunicación en serie.
}

/*
   Sentencias a ejecutar constantemente en el arduino.
*/
void loop()
{
  // Obtener tiempo actual
  unsigned long currentMillis = millis();

  // Si interval es menor a la diferencia de currentMillis y previousMillis tomo las mediciones de los sensores.
  if ((unsigned long)(currentMillis - previousMillis) >= interval)
  {
    if(bluetooth.available())
    {
      digitalWrite(BLUE_LED, HIGH);
      angleFlexPulgar = readFlex(FLEX_SENSOR_PULGAR, STRAIGHT_RESISTANCE_FLEX_PULGAR, BEND_RESISTANCE_FLEX_PULGAR, DIVISOR_RESISTANCE_FLEX_PULGAR);
      angleFlexIndice = readFlex(FLEX_SENSOR_INDICE, STRAIGHT_RESISTANCE_FLEX_INDICE, BEND_RESISTANCE_FLEX_INDICE, DIVISOR_RESISTANCE_FLEX_INDICE);
      angleFlexMedio = readFlex(FLEX_SENSOR_MEDIO, STRAIGHT_RESISTANCE_FLEX_MEDIO, BEND_RESISTANCE_FLEX_MEDIO, DIVISOR_RESISTANCE_FLEX_MEDIO);
  
      flexPulgarPosition = convertAngleIntoPosition(angleFlexPulgar);
      flexIndicePosition = convertAngleIntoPosition(angleFlexIndice);
      flexMedioPosition = convertAngleIntoPosition(angleFlexMedio);
  
      newMovement = convertInformationIntoMovement (flexPulgarPosition, flexIndicePosition, flexMedioPosition, 5);
  
      // si el movimiento es distinto al anterior envío la información a la aplicación Android.
      if (newMovement != previousMovement && newMovement != (int) NOT_VALID_GESTURE)
      {
        
        sendInformationViaBluetooth (newMovement);
        previousMovement = newMovement;
      }
      else if (newMovement == (int) NOT_VALID_GESTURE)
      {
        rgbColor(LOW, LOW, LOW);
      }
    }
    else
    {
      digitalWrite(BLUE_LED, LOW);
    }

    // Actualizo previousMillis para que vuelva a ingresar en próximas iteraciones.
    previousMillis = millis();
  }

  /*
    // Ejemplo de invocación a Kalman.. nos va a servir para el acelerómetro.
    filteredMeasurement = myFilter.getFilteredValue(measurement);
    Serial.println("KalmanFilter: " + String(filteredMeasurement) + " kalmanValue");
    Serial.println();
  */
}

/*
   Función utilizada para convertir la medición recibida en ohms de los sensores flex en el ángulo de inclinación correspondiente.

   Recibe:
   @sensorName: sensor flex que recibe.
   @straightResistance: medida de la resistencia con el flex completamente abierto.
   @bedResistance: medida de la resistencia con el flex completamente cerrado.
   @divisorResistance: medida de la resistencia fija del divisor de tensión.

   Retorna:
   @angleValue: valor del ángulo en el que se encuentra el flex.
*/
double readFlex (const int sensorName, const int straightResistance, const int bendResistance, const int divisorResistance)
{
  double sensorValue = (double) analogRead(sensorName); //read new value from sensor in analog pin A1, for example

  double flexVoltaje = sensorValue * VCC / 1023.0;
  double flexResistance = divisorResistance * (VCC / flexVoltaje - 1.0);

  // Uso la resistencia calculada para estimar el angulo de inclinación del sensor
  double angleValue = map(flexResistance, straightResistance, bendResistance, 0, 90.0);

  return angleValue;
}

/*
   Función utilizada para identificar la posición de los sensores flex.

   Recibe:
   @flexAngle: valor del ángulo del sensor flex.
   
   Retorna:
   @flexPosition: posición en la que se encuentra el sensor.   

*/
int convertAngleIntoPosition (const float flexAngle)
{
  int flexPosition;

  if (flexAngle < 40.0)
  {
    flexPosition = 0;
  }
  else if (flexAngle > 84.0)
  {
    flexPosition = 2;
  }
  else
  {
    flexPosition = 1;
  }

  return flexPosition;
}

/*
 * Función utilizada para convertir la información de los sensores en un único valor 
 * a enviar vía Bluetooth a la aplicación Android.
 * 
 * Recibe:
 * @sensorFlexPulgar: posición de sensor flex del pulgar.
 * @sensorFlexIndice: posición de sensor flex del índice.
 * @sensorFlexMedio: posición de sensor flex del medio.
 * @sensorAcelerometer: sentido en el que se movió el acelerómetro.
 * 
 * Retorna:
 * @movement: movimiento realizado.
 */
int convertInformationIntoMovement (const int sensorFlexPulgar, const int sensorFlexIndice, const int sensorFlexMedio, const int sensorAcelerometer)
{
  int movement;
  if (sensorFlexPulgar == (int) BEND_FLEX)
  {
    if (sensorFlexIndice == (int) BEND_FLEX && sensorFlexMedio == (int) BEND_FLEX)
    {
      rgbColor(255, 0, 0); // Color rojo.
      return movement = (int) PLAY_PAUSE;  
    }
    else if (sensorFlexIndice == (int) STRAIGHT_FLEX)
    {
      if (sensorFlexMedio == (int) STRAIGHT_FLEX)
      {
        rgbColor(0, 255, 0); // Color verde.
        return movement = (int) SPEED_X2;
      }
      else if (sensorFlexMedio == (int) BEND_FLEX)
      {
        rgbColor(0, 0, 255); // Color azul.
        return movement = (int) SPEED_X1;
      }
    }
    else if (sensorFlexIndice == (int) MEDIUM_BEND_FLEX && sensorFlexMedio == (int) BEND_FLEX)
    {
        rgbColor(255, 0, 255); // Color magenta.
        return movement = (int) SPEED_X05;
    } 
  }
  else if (sensorFlexPulgar == (int) STRAIGHT_FLEX && sensorFlexIndice == (int) STRAIGHT_FLEX && sensorFlexMedio == (int) STRAIGHT_FLEX)
  {
    if (sensorAcelerometer == (int) LEFT)
    {
        rgbColor(255, 255, 0); // Color amarillo.
        return movement = (int) NEXT;
    }
    else if (sensorAcelerometer == (int) RIGHT)
    {
        rgbColor(255, 255, 125); // Color frambuesa.
        return movement = (int) PREVIOUS;
    }
  }
  
  return movement = (int) NOT_VALID_GESTURE;
}

/*
 * Función utilizada para enviar información vía Bluetooth
 * 
 * Recibe:
 * @movement: movimiento a enviar a la aplicación Android.
 */
void sendInformationViaBluetooth (const int movement) 
{
    bluetooth.write(movement);
}


/*
 * Función utilizada para cambiar el color de led RGB
 * Entrada:
 * @redLightValue: cantidad de color rojo.
 * @blueLightValue: cantidad de color azul.
 * @greenLightValue: cantidad de color verde.
 */
void rgbColor (int redLightValue, int greenLightValue, int blueLightValue)
{
  analogWrite(RGB_LED_RED, redLightValue);
  analogWrite(RGB_LED_BLUE, blueLightValue);
  analogWrite(RGB_LED_GREEN, greenLightValue);
}