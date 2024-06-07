#include <LiquidCrystal_I2C.h> // Biblioteca para LCD I2C
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <EEPROM.h> // Biblioteca para EEPROM

#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 0  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3

LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço de acesso: 0x3F ou 0x27
RTC_DS1307 RTC;

// Configurações da EEPROM
const int maxRecords = 100;
const int recordSize = 8; // Tamanho de cada registro em bytes
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;

int lastLoggedMinute = -1;

// Triggers de condutividade, turbidez e ph
float trigger_c_max = 1500.0; // Exemplo: valor máximo de condutividade
float trigger_t_max = 50.0; // Exemplo: valor máximo de turbidez
float trigger_ph_min = 5.5; // Exemplo: valor mínimo de ph
float trigger_ph_max = 9.5; // Exemplo: valor máximo de ph

// Define os pinos para os LEDs
int ledVermelho = 5; 
int ledAmarelo = 4; 
int ledVerde = 3;
int buzzer = 13; // Pino para o buzzer
int potCondutividade = A0; // Pino analógico para o potenciômetro de condutividade
int potTurbidez = A1; // Pino analógico para o potenciômetro de turbidez
int potPh = A2; // Pino analógico para o potenciômetro de pH
int valorCE = 0; // Variável para armazenar o valor de condutividade
int valorTB = 0; // Variável para armazenar o valor de turbidez
int valorPH = 0; // Variável para armazenar o valor de pH
float condutividade; // Variável para armazenar a condutividade calculada
float turbidez; // Variável para armazenar a turbidez calculada
float ph; // Variável para armazenar o pH calculado

const unsigned long intervaloEvento = 2000;  // Intervalo de tempo para mudar as informações no display.
unsigned long tempoPrevio = 0; // Armazena a última vez que o display foi atualizado.

int estadoDisplay = 0; // Controla o estado da exibição de informações no LCD.

void setup() {
  Serial.begin(9600); // Inicia a comunicação serial a 115200 ou 96000 bits por segundo

  lcd.init();   // Inicialização do LCD
  lcd.backlight();  // Ligando o backlight do LCD
  RTC.begin();    // Inicialização do Relógio em Tempo Real
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // RTC.adjust(DateTime(2024, 5, 6, 08, 15, 00));  // Ajustar a data e hora apropriadas uma vez inicialmente, depois comentar
  EEPROM.begin();

  // Configura os pinos dos LEDs como saída
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  // Configura os pinos dos potenciômetros como saída
  pinMode(potCondutividade, INPUT);
  pinMode(potTurbidez, INPUT);
  pinMode(potPh, INPUT);
}

void loop() {
  DateTime now = RTC.now();

  // Calculando o deslocamento do fuso horário
  int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
  now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual

  // Convertendo o novo tempo para DateTime
  DateTime adjustedTime = DateTime(now);

  unsigned long tempoAtual = millis(); // Armazena o tempo atual em milissegundos desde que o Arduino foi reiniciado

  int numeroLeituras = 10; // Número de leituras
  int condutividadeSoma = 0; // Soma das leituras de condutividade
  int turbidezSoma = 0; // Soma das leituras de turbidez
  int phSoma = 0; // Soma das leituras de ph
  
  for (int i = 0; i < numeroLeituras; i++) { // Lê 10 vezes
    valorCE = analogRead(potCondutividade); // Lê o valor do potenciômetro de condutividade
    valorTB = analogRead(potTurbidez); // Lê o valor do potenciômetro de turbidez
    valorPH = analogRead(potPh); // Lê o valor do potenciômetro de pH

    condutividade = map(valorCE, 0, 1023, 0, 3000); // µS/cm
    turbidez = map(valorTB, 0, 1023, 0, 250); // NTU
    ph = map(valorPH, 0, 1023, 55, 100) / 10.0; // pH de 6.5 a 8.5

    condutividadeSoma += condutividade; // Acumula o valor mapeado na variável 'condutividadeSoma'
    turbidezSoma += turbidez; // Acumula o valor mapeado na variável 'turbidezSoma'
    phSoma += ph; // Acumula o valor mapeado na variável 'phSoma'
    delay(1); // Pausa a execução por 1 milissegundo antes da próxima leitura
  }
  
  int condutividadeMedia = condutividadeSoma / numeroLeituras; // Média das leituras da condutividade
  int turbidezMedia = turbidezSoma / numeroLeituras; // Média das leituras da turbidez
  int phMedia = phSoma / numeroLeituras; // Média das leituras do ph

  if (tempoAtual - tempoPrevio >= intervaloEvento) { // Verifica se o intervalo de tempo especificado passou para atualizar o display
    switch (estadoDisplay) { // Estrutura condicional que controla qual informação será exibida no display.
      case 0: // No estado 0:
        mostrarCondutividade(condutividadeMedia); // Mostra a média da condutividade.
        estadoDisplay = 1; // Altera o estado do display para 1.
        break; // Sai da estrutura condicional
      case 1: // No estado 1:
        mostrarTurbidez(turbidezMedia); // Mostra a média da turbidez.
        estadoDisplay = 2; // Altera o estado do display para 2.
        break; // Sai da estrutura condicional
      case 2: // No estado 2:
        mostrarPh(phMedia); // Mostra a média do ph.
        estadoDisplay = 3; // Altera o estado do display para 0.
        break; // Sai da estrutura condicional
      case 3:
        lcd.setCursor(0, 0);
        lcd.print("DATA: ");
        lcd.print(adjustedTime.day() < 10 ? "0" : ""); // Adiciona zero à esquerda se dia for menor que 10
        lcd.print(adjustedTime.day());
        lcd.print("/");
        lcd.print(adjustedTime.month() < 10 ? "0" : ""); // Adiciona zero à esquerda se mês for menor que 10
        lcd.print(adjustedTime.month());
        lcd.print("/");
        lcd.print(adjustedTime.year());
        lcd.setCursor(0, 1);
        lcd.print("HORA: ");
        lcd.print(adjustedTime.hour() < 10 ? "0" : ""); // Adiciona zero à esquerda se hora for menor que 10
        lcd.print(adjustedTime.hour());
        lcd.print(":");
        lcd.print(adjustedTime.minute() < 10 ? "0" : ""); // Adiciona zero à esquerda se minuto for menor que 10
        lcd.print(adjustedTime.minute());
        lcd.print(":");
        lcd.print(adjustedTime.second() < 10 ? "0" : ""); // Adiciona zero à esquerda se segundo for menor que 10
        lcd.print(adjustedTime.second());
        estadoDisplay = 0;
        break;
    }

    // Condições Ideais
    if (condutividadeMedia <= 500 && turbidezMedia <= 5 && phMedia >= 6.5 && phMedia <= 8.5) {
      acendeLedVerde();
    }
      
    // Condições de Alerta
    if ((condutividadeMedia > 500 && condutividadeMedia <= 1500) || (turbidezMedia > 5 && turbidezMedia <= 50) || (phMedia > 5.5 && phMedia < 6.5 || phMedia > 8.5 && phMedia < 9.5) && !(condutividadeMedia > 1500) && !(turbidezMedia > 50) && !(phMedia < 5.5 || phMedia > 9.5)) {
      acendeLedAmarelo();
    }
      
    // Condições Críticas
    if (condutividadeMedia > 1500 || turbidezMedia > 50 || phMedia < 5.5 || phMedia > 9.5) {
      acendeLedVermelho();
    }

    // Imprime os valores de condutividade, turbidez e ph
    Serial.print("Condutividade 0 - 3000 = ");
    Serial.println(condutividadeMedia);
    Serial.print("Turbidez 0 - 250 = ");
    Serial.println(turbidezMedia);
    Serial.print("PH 5.5 - 9.5 = ");
    Serial.println(phMedia);
    if (LOG_OPTION) get_log();

    tempoPrevio = tempoAtual; // Atualiza o tempo anterior com o tempo atual para marcar o momento da última atualização.
  }

  // Verifica se o minuto atual é diferente do minuto do último registro
  if (adjustedTime.minute() != lastLoggedMinute) {
      lastLoggedMinute = adjustedTime.minute();

      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);                       // wait for a second

      // Verificar se os valores estão fora dos triggers
      if (condutividadeMedia > trigger_c_max && turbidezMedia > trigger_t_max && (phMedia < trigger_ph_min || phMedia > trigger_ph_max)) {
          // Escrever dados na EEPROM
          EEPROM.put(currentAddress, now.unixtime());
          EEPROM.put(currentAddress + 4, condutividadeMedia);
          EEPROM.put(currentAddress + 6, turbidezMedia);

          delay(1000);

          // Atualiza o endereço para o próximo registro
          getNextAddress();
      }
  }

  if (SERIAL_OPTION) {
    Serial.print(adjustedTime.day());
    Serial.print("/");
    Serial.print(adjustedTime.month());
    Serial.print("/");
    Serial.print(adjustedTime.year());
    Serial.print(" ");
    Serial.print(adjustedTime.hour() < 10 ? "0" : ""); // Adiciona zero à esquerda se hora for menor que 10
    Serial.print(adjustedTime.hour());
    Serial.print(":");
    Serial.print(adjustedTime.minute() < 10 ? "0" : ""); // Adiciona zero à esquerda se minuto for menor que 10
    Serial.print(adjustedTime.minute());
    Serial.print(":");
    Serial.print(adjustedTime.second() < 10 ? "0" : ""); // Adiciona zero à esquerda se segundo for menor que 10
    Serial.print(adjustedTime.second());
    Serial.print("\n");
  }
}

void getNextAddress() {
    currentAddress += recordSize;
    if (currentAddress >= endAddress) {
        currentAddress = 0; // Volta para o começo se atingir o limite
    }
}

void get_log() {
  Serial.println("Data stored in EEPROM:");
  Serial.println("Timestamp\t\tCondutividade\tTurbidez");

  for (int address = startAddress; address < endAddress; address += recordSize) {
    long timeStamp;
    int condutividadeMedia, turbidezMedia;

    // Ler dados da EEPROM
    EEPROM.get(address, timeStamp);
    EEPROM.get(address + 4, condutividadeMedia);
    EEPROM.get(address + 6, turbidezMedia);

    // Verificar se os dados são válidos antes de imprimir
    if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
        //Serial.print(timeStamp);
        DateTime dt = DateTime(timeStamp);
        Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
        Serial.print("\t");
        Serial.print(condutividadeMedia);
        Serial.print(" uS/cm\t");
        Serial.print(turbidezMedia);
        Serial.println(" NTU");
    }
  }
}

// Função que acende o led verde
void acendeLedVerde() {
  apagaLeds(); // Ativa a função para apagar os Leds
  digitalWrite(ledVerde, HIGH); // Acende o LED verde
  digitalWrite(buzzer, LOW); // Desativa o buzzer
}

// Função que acende o led amarelo
void acendeLedAmarelo() {
  apagaLeds(); // Ativa a função para apagar os Leds
  digitalWrite(ledAmarelo, HIGH); // Acende o LED amarelo
  digitalWrite(buzzer, HIGH); // Ativa o buzzer
  tone(buzzer, 220, 100); // Define o tom do buzzer
}

// Função que acende o led vermelho
void acendeLedVermelho() {
  apagaLeds(); // Ativa a função para apagar os Leds
  digitalWrite (ledVermelho, HIGH); // Acende o LED vermelho
  digitalWrite(buzzer, HIGH); // Ativa o buzzer
  tone(buzzer, 220, 900); // Define o tom do buzzer
  Serial.println("POLUIÇÃO DETECTADA");
}

// Função para apagar todos os LEDs
void apagaLeds () {
  digitalWrite(ledVerde, LOW); 
  digitalWrite(ledAmarelo, LOW); 
  digitalWrite(ledVermelho, LOW); 
}

// Função que exibe a condutividade no display
void mostrarCondutividade(float condutividadeMedia) {
  lcd.clear(); // Limpa o display LCD para a nova mensagem.

  if (condutividadeMedia <= 500) { // Exibe a mensagem indicando condutividade boa.
    lcd.setCursor(0,0);
    lcd.print("Cond Boa");
    lcd.setCursor(0,1);
    lcd.print("Cond: ");
    lcd.print(condutividadeMedia);
    lcd.print("uS/cm");
  } else if (condutividadeMedia > 500 && condutividadeMedia <= 1500) { // Exibe a mensagem indicando condutividade moderada.
    lcd.setCursor(0,0);
    lcd.print("Cond Moderada");
    lcd.setCursor(0,1);
    lcd.print("Cond: ");
    lcd.print(condutividadeMedia);
    lcd.print("uS/cm");
  } else { // Exibe a mensagem indicando condutividade ruim.
    lcd.setCursor(0,0);
    lcd.print("Cond Ruim");
    lcd.setCursor(0,1);
    lcd.print("Con: ");
    lcd.print(condutividadeMedia);
    lcd.print("uS/cm");
  }
}

// Função que exibe a turbidez no display
void mostrarTurbidez(float turbidezMedia) {
  lcd.clear(); // Limpa o display LCD para a nova mensagem.

  if (turbidezMedia <= 5) {  // Exibe a mensagem indicando turbidez boa.
    lcd.setCursor(0,0);
    lcd.print("Turbidez Boa");
    lcd.setCursor(0,1);
    lcd.print("Turb: ");
    lcd.print(turbidezMedia);
    lcd.print("NTU");
  } else if (turbidezMedia > 5 && turbidezMedia <= 50) { // Exibe a mensagem turbidez moderada.
    lcd.setCursor(0,0);
    lcd.print("Turb Moderada");
    lcd.setCursor(0,1);
    lcd.print("Turb: ");
    lcd.print(turbidezMedia);
    lcd.print("NTU");
  } else { // Exibe a mensagem indicando turbidez ruim.
    lcd.setCursor(0,0);
    lcd.print("Turbidez Ruim");
    lcd.setCursor(0,1);
    lcd.print("Turb: ");
    lcd.print(turbidezMedia);
    lcd.print("NTU");
  }
}

// Função que exibe o ph no display
void mostrarPh(float phMedia) {
  lcd.clear(); // Limpa o display LCD para a nova mensagem.

  if (phMedia >= 6.5 && phMedia <= 8.5) { // Exibe a mensagem indicando ph bom.
    lcd.setCursor(0,0);
    lcd.print("PH Bom");
    lcd.setCursor(0,1);
    lcd.print("PH: ");
    lcd.print(phMedia);
  } else if (phMedia > 5.5 && phMedia < 6.5 || phMedia > 8.5 && phMedia < 9.5) { // Exibe a mensagem indicando ph moderado.
    lcd.setCursor(0,0);
    lcd.print("PH Moderado");
    lcd.setCursor(0,1);
    lcd.print("PH: ");
    lcd.print(phMedia);
  } else { // Exibe a mensagem indicando ph ruim.
    lcd.setCursor(0,0);
    lcd.print("PH Ruim");
    lcd.setCursor(0,1);
    lcd.print("PH: ");
    lcd.print(phMedia);
  }
}

// Definição das Categorias:

// Condutividade Elétrica: EZO-EC (Atlas Scientific)
// Boa: 0 a 500 µS/cm
// Moderada: 500 a 1500 µS/cm
// Ruim: Acima de 1500 µS/cm

// Turbidez: SEN0189 (DFRobot)
// Boa: 0 a 5 NTU
// Moderada: 5 a 50 NTU
// Ruim: Acima de 50 NTU

// pH: SEN0161 (DFRobot)
// Bom: 6.5 a 8.5
// Moderado: 5.5 a 6.5 ou 8.5 a 9.5
// Ruim: Abaixo de 5.5 ou acima de 9.5