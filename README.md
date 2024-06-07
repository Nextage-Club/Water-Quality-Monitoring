# Monitoramento de Qualidade da Água

Este projeto visa monitorar a qualidade da água utilizando sensores de condutividade, turbidez e pH, e exibir os dados em um display LCD. Além disso, o sistema alerta o usuário através de LEDs e um buzzer sobre a qualidade da água.

## Funcionalidades

- Leitura de valores de condutividade, turbidez e pH.
- Exibição das leituras em um display LCD.
- Armazenamento de registros na EEPROM.
- Alertas visuais e sonoros para condições ideais, de alerta e críticas.
- Ajuste automático do tempo para o fuso horário UTC-3.

## Requisitos

- Arduino (qualquer modelo compatível)
- Sensores (Simulados por potenciômetros):
  - Sensor de Condutividade (EZO-EC)
  - Sensor de Turbidez (SEN0189)
  - Sensor de pH (SEN0161)
- Display LCD I2C (16x2)
- RTC DS1307
- LEDs (Verde, Amarelo, Vermelho)
- Buzzer
- Potenciômetros
- Bibliotecas:
  - `LiquidCrystal_I2C`
  - `RTClib`
  - `Wire`
  - `EEPROM`

## Instalação

1. Clone o repositório:

    ```sh
    git clone https://github.com/seu-usuario/seu-repositorio.git
    ```

2. Abra o arquivo `.ino` no Arduino IDE.

3. Instale as bibliotecas necessárias:
   - Vá para **Sketch** > **Include Library** > **Manage Libraries**.
   - Procure e instale `LiquidCrystal_I2C`, `RTClib`, `Wire` e `EEPROM`.

4. Faça o upload do código para o seu Arduino.

## Configuração

1. Conecte os sensores aos pinos analógicos do Arduino conforme especificado no código:
   - Potenciômetro de Condutividade no pino A0
   - Potenciômetro de Turbidez no pino A1
   - Potenciômetro de pH no pino A2

2. Conecte os LEDs aos pinos digitais:
   - LED Verde no pino 3
   - LED Amarelo no pino 4
   - LED Vermelho no pino 5

3. Conecte o buzzer ao pino 13.

4. Conecte o Display LCD I2C e o RTC DS1307 conforme a pinagem I2C.

## Uso

1. Ao iniciar o Arduino, o display LCD irá mostrar a média das leituras dos sensores em ciclos de 2 segundos.
2. Os LEDs e o buzzer irão indicar a qualidade da água:
   - Verde: Condições ideais
   - Amarelo: Condições de alerta
   - Vermelho: Condições críticas

3. Os valores de condutividade, turbidez e pH serão armazenados na EEPROM se estiverem fora dos limites pré-definidos.

## Simulação

Você pode simular este projeto online usando o [Wokwi Simulator](https://wokwi.com/projects/400073345184683009).

