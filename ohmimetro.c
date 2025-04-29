#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "matriz_LED.pio.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o ohmímetro
#define Botao_A 5  // GPIO para botão A
#define LED_PIN 7  // GPIO para o LED da matriz
#define NUM_PIXELS 25 // Número de LEDs na matriz

int R_conhecido = 2200;   // Resistor de 2.2k0 ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
uint ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

ssd1306_t ssd;             // Variável para o display OLED

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define Botao_B 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
}

// Matriz de LEDs

double led_buffer[25][3] = {0}; // Buffer para armazenar o estado dos LEDs

uint matrix_rgb(float r, float g, float b) // Função para converter RGB em um valor de 32 bits
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

// Função para desenhar na matriz

void desenho_pio(double desenho[25][3], uint32_t valor_led, PIO pio, uint sm)
{

  for (int16_t i = 0; i < NUM_PIXELS; i++)
  {
    valor_led = matrix_rgb(desenho[i][0], desenho[i][1], desenho[i][2]);
    pio_sm_put_blocking(pio, sm, valor_led);
  };
}

void atualizar_linha1(double cor[3]) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i == 4 || i == 5 || i == 14 || i == 15 || i == 24){
      led_buffer[i][0] = cor[0];
      led_buffer[i][1] = cor[1];
      led_buffer[i][2] = cor[2];
    }
  }
}

void atualizar_linha2(double cor[3]) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i == 3 || i == 6 || i == 13 || i == 16 || i == 23){
      led_buffer[i][0] = cor[0];
      led_buffer[i][1] = cor[1];
      led_buffer[i][2] = cor[2];
    }
  }
}
void atualizar_linha3(double cor[3]) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i == 2 || i == 7 || i == 12 || i == 17 || i == 22){
      led_buffer[i][0] = cor[0];
      led_buffer[i][1] = cor[1];
      led_buffer[i][2] = cor[2];
    }
  }
}
void atualizar_linha5(double cor[3]) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i == 0 || i == 9 || i == 10 || i == 19 || i == 20){
      led_buffer[i][0] = cor[0];
      led_buffer[i][1] = cor[1];
      led_buffer[i][2] = cor[2];
    }
  }
}

// Cores

double preto[3] = {0.0, 0.0, 0.0}; // Preto
double marrom[3] = {0.3, 0.1, 0.0}; // Marrom
double vermelho[3] = {1.0, 0.0, 0.0}; // Vermelho
double laranja[3] = {1.0, 0.5, 0.0}; // Laranja
double amarelo[3] = {1.0, 1.0, 0.0}; // Amarelo
double verde[3] = {0.0, 1.0, 0.0};  // Verde
double azul[3] = {0.0, 0.0, 1.0};  // Azul
double violeta[3] = {0.5, 0.0, 1.0}; // Violeta
double cinza[3] = {0.1, 0.1, 0.1}; // Cinza
double branco[3] = {1.0, 1.0, 1.0}; // Branco
double ouro[3] = {1.0, 0.84, 0.0}; // Ouro

// Sprites
double apagar_leds[25][3] =      // Apagar LEDs da matriz
 {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
  {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
  {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
  {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
  {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

void setup(){
  // Matriz de LEDs
  PIO pio = pio0;   // Seleciona o PIO0
  uint sm = 0;      // Seleciona o estado da máquina 0
  uint offset = pio_add_program(pio, &pio_matrix_program); // Adiciona o programa ao PIO
  pio_matrix_program_init(pio, sm, offset, 0);             // Inicializa o programa no PIO
  pio_sm_set_enabled(pio, sm, true);                       // Habilita a máquina de estado

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
  gpio_init(Botao_B);
  gpio_set_dir(Botao_B, GPIO_IN);
  gpio_pull_up(Botao_B);
  gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display
}

int main()
{
  stdio_init_all(); // Inicializa a biblioteca de entrada e saída padrão
  setup();          // Inicializa os pinos/periféricos

  // Configurações da PIO/Matriz de LEDs
  PIO pio = pio0;
  bool frequenciaClock;
  uint16_t i;
  uint valor_led;
  float r = 0.0, b = 0.0, g = 0.0;

  frequenciaClock = set_sys_clock_khz(128000, false); // frequência de clock de 128MHz

  uint offset = pio_add_program(pio, &pio_matrix_program);
  uint sm = pio_claim_unused_sm(pio, true);
  pio_matrix_program_init(pio, sm, offset, LED_PIN);
  desenho_pio(apagar_leds, valor_led, pio, sm); // Apaga os LEDs atuais da matriz

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string

  bool cor = true;

  while (true)
  {
    adc_select_input(2); // Seleciona o ADC no pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

    // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

    sprintf(str_x, "%1.0f", R_x);   // Converte o float em string

    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    ssd1306_line(&ssd, 3, 15, 123, 15, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 48, 123, 48, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "Ohmimetro", 30, 6);     // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.:", 6, 53);    // Desenha uma string
    ssd1306_draw_string(&ssd, str_x, 90, 53);          // Desenha uma string
    ssd1306_send_data(&ssd);                           // Atualiza o display

    float e24 = 0.0; // Variável de valor da resistência na escala E24
    double mult[3]; // Armazena a cor do multiplicador

    // Converte a resistência para o valor na escala E24 e define o multiplicador
    if (R_x > 400 && R_x < 950) {         // Intervalo 400Ω a 1000Ω
      e24 = R_x / 100;
      mult[0] = marrom[0];
      mult[1] = marrom[1];
      mult[2] = marrom[2];
    }
    else if (R_x >= 950 && R_x < 10000) { // Intervalo 1000Ω a 10kΩ
      e24 = R_x / 1000;
      mult[0] = vermelho[0];
      mult[1] = vermelho[1];
      mult[2] = vermelho[2];
    }
    else if (R_x >= 10000 && R_x < 100000) { // Intervalo 10kΩ a 100kΩ
      e24 = R_x / 10000;
      mult[0] = laranja[0];
      mult[1] = laranja[1];
      mult[2] = laranja[2];
    } 
    else if (R_x >= 100000 && R_x < 1000000) { // Intervalo 100kΩ a 1MΩ
      e24 = R_x / 100000;
      mult[0] = amarelo[0];
      mult[1] = amarelo[1];
      mult[2] = amarelo[2];
    } else {
      printf("Valor fora do intervalo\n"); // Valor fora do intervalo
      ssd1306_draw_string(&ssd, "Valor fora do", 20, 20);
      ssd1306_draw_string(&ssd, "intervalo", 20, 29);
      ssd1306_send_data(&ssd);
    }
    //Considerando valores de resistência de 510 a 100k Ohm

    // Resistor 1.0Ω (Marrom, Preto, Multiplicador, Ouro)
    if (0.95 < e24 && e24 < 1.05) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 PRETO", 20, 29);
      atualizar_linha2(preto);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.1Ω (Marrom, Marrom, Multiplicador, Ouro)
    else if (1.05 < e24 && e24 < 1.15) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 MARROM", 20, 29);
      atualizar_linha2(marrom);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.2Ω (Marrom, Vermelho, Multiplicador, Ouro)
    else if (1.15 < e24 && e24 < 1.25) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 VERMELHO", 20, 29);
      atualizar_linha2(vermelho);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.3Ω (Marrom, Laranja, Multiplicador, Ouro)
    else if (1.25 < e24 && e24 < 1.35) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 LARANJA", 20, 29);
      atualizar_linha2(laranja);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.5Ω (Marrom, Verde, Multiplicador, Ouro)
    else if (1.45 < e24 && e24 < 1.55) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 VERDE", 20, 29);
      atualizar_linha2(verde);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.6Ω (Marrom, Azul, Multiplicador, Ouro)
    else if (1.55 < e24 && e24 < 1.65) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 AZUL", 20, 29);
      atualizar_linha2(azul);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 1.8Ω (Marrom, Cinza, Multiplicador, Ouro)
    else if (1.75 < e24 && e24 < 1.85) {
      ssd1306_draw_string(&ssd, "1 MARROM", 20, 20);
      atualizar_linha1(marrom);
      ssd1306_draw_string(&ssd, "2 CINZA", 20, 29);
      atualizar_linha2(cinza);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 2.0Ω (Vermelho, Preto, Multiplicador, Ouro)
    else if (1.9 < e24 && e24 < 2.1) {
      ssd1306_draw_string(&ssd, "1 VERMELHO", 20, 20);
      atualizar_linha1(vermelho);
      ssd1306_draw_string(&ssd, "2 PRETO", 20, 29);
      atualizar_linha2(preto);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 2.2Ω (Vermelho, Vermelho, Multiplicador, Ouro)
    else if (2.1 < e24 && e24 < 2.3) {
      ssd1306_draw_string(&ssd, "1 VERMELHO", 20, 20);
      atualizar_linha1(vermelho);
      ssd1306_draw_string(&ssd, "2 VERMELHO", 20, 29);
      atualizar_linha2(vermelho);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 2.4Ω (Vermelho, Amarelo, Multiplicador, Ouro)
    else if (2.3 < e24 && e24 < 2.5) {
      ssd1306_draw_string(&ssd, "1 VERMELHO", 20, 20);
      atualizar_linha1(vermelho);
      ssd1306_draw_string(&ssd, "2 AMARELO", 20, 29);
      atualizar_linha2(amarelo);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 2.7Ω (Vermelho, Violeta, Multiplicador, Ouro)
    else if (2.6 < e24 && e24 < 2.8) {
      ssd1306_draw_string(&ssd, "1 VERMELHO", 20, 20);
      atualizar_linha1(vermelho);
      ssd1306_draw_string(&ssd, "2 VIOLETA", 20, 29);
      atualizar_linha2(violeta);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 3.0Ω (Laranja, Preto, Multiplicador, Ouro)
    else if (2.85 < e24 && e24 < 3.15) {
      ssd1306_draw_string(&ssd, "1 LARANJA", 20, 20);
      atualizar_linha1(laranja);
      ssd1306_draw_string(&ssd, "2 PRETO", 20, 29);
      atualizar_linha2(preto);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 3.3Ω (Laranja, Laranja, Multiplicador, Ouro)
    else if (3.15 < e24 && e24 < 3.45) {
      ssd1306_draw_string(&ssd, "1 LARANJA", 20, 20);
      atualizar_linha1(laranja);
      ssd1306_draw_string(&ssd, "2 LARANJA", 20, 29);
      atualizar_linha2(laranja);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 3.6Ω (Laranja, Azul, Multiplicador, Ouro)
    else if (3.45 < e24 && e24 < 3.75) {
      ssd1306_draw_string(&ssd, "1 LARANJA", 20, 20);
      atualizar_linha1(laranja);
      ssd1306_draw_string(&ssd, "2 AZUL", 20, 29);
      atualizar_linha2(azul);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 3.9Ω (Laranja, Branco, Multiplicador, Ouro)
    else if (3.75 < e24 && e24 < 4.05) {
      ssd1306_draw_string(&ssd, "1 LARANJA", 20, 20);
      atualizar_linha1(laranja);
      ssd1306_draw_string(&ssd, "2 BRANCO", 20, 29);
      atualizar_linha2(branco);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 4.3Ω (Amarelo, Laranja, Multiplicador, Ouro)
    else if (4.05 < e24 && e24 < 4.55) {
      ssd1306_draw_string(&ssd, "1 AMARELO", 20, 20);
      atualizar_linha1(amarelo);
      ssd1306_draw_string(&ssd, "2 LARANJA", 20, 29);
      atualizar_linha2(laranja);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 4.7Ω (Amarelo, Violeta, Multiplicador, Ouro)
    else if (4.55 < e24 && e24 < 4.95) {
      ssd1306_draw_string(&ssd, "1 AMARELO", 20, 20);
      atualizar_linha1(amarelo);
      ssd1306_draw_string(&ssd, "2 VIOLETA", 20, 29);
      atualizar_linha2(violeta);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 5.1Ω (Verde, Marrom, Multiplicador, Ouro)
    else if (4.85 < e24 && e24 < 5.35) {
      ssd1306_draw_string(&ssd, "1 VERDE", 20, 20);
      atualizar_linha1(verde);
      ssd1306_draw_string(&ssd, "2 MARROM", 20, 29);
      atualizar_linha2(marrom);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 5.6Ω (Verde, Azul, Ouro)
    else if (5.32 < e24 && e24 < 5.88) {
      ssd1306_draw_string(&ssd, "1 VERDE", 20, 20);
      atualizar_linha1(verde);
      ssd1306_draw_string(&ssd, "2 AZUL", 20, 29);
      atualizar_linha2(azul);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 6.2Ω (Azul, Vermelho, Ouro)
    else if (5.89 < e24 && e24 < 6.51) {
      ssd1306_draw_string(&ssd, "1 AZUL", 20, 20);
      atualizar_linha1(azul);
      ssd1306_draw_string(&ssd, "2 VERMELHO", 20, 29);
      atualizar_linha2(vermelho);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 6.8Ω (Azul, Cinza, Ouro)
    else if (6.46 < e24 && e24 < 7.14) {
      ssd1306_draw_string(&ssd, "1 AZUL", 20, 20);
      atualizar_linha1(azul);
      ssd1306_draw_string(&ssd, "2 CINZA", 20, 29);
      atualizar_linha2(cinza);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 7.5Ω (Violeta, Verde, Ouro)
    else if (7.13 < e24 && e24 < 7.87) {
      ssd1306_draw_string(&ssd, "1 VIOLETA", 20, 20);
      atualizar_linha1(violeta);
      ssd1306_draw_string(&ssd, "2 VERDE", 20, 29);
      atualizar_linha2(verde);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 8.2Ω (Cinza, Vermelho, Ouro)
    else if (7.79 < e24 && e24 < 8.61) {
      ssd1306_draw_string(&ssd, "1 CINZA", 20, 20);
      atualizar_linha1(cinza);
      ssd1306_draw_string(&ssd, "2 VERMELHO", 20, 29);
      atualizar_linha2(vermelho);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }

    // Resistor 9.1Ω (Branco, Marrom, Ouro)
    else if (8.65 < e24 && e24 < 9.55) {
      ssd1306_draw_string(&ssd, "1 BRANCO", 20, 20);
      atualizar_linha1(branco);
      ssd1306_draw_string(&ssd, "2 MARROM", 20, 29);
      atualizar_linha2(marrom);
      if (mult[0] == marrom[0]){
        ssd1306_draw_string(&ssd, "3 MARROM", 20, 38);
      }
      else if (mult[0] == vermelho[0]){
        ssd1306_draw_string(&ssd, "3 VERMELHO", 20, 38);
      }
      else if (mult[0] == laranja[0]){
        ssd1306_draw_string(&ssd, "3 LARANJA", 20, 38);
      }
      else if (mult[0] == amarelo[0]){
        ssd1306_draw_string(&ssd, "3 AMARELO", 20, 38);
      }
      ssd1306_send_data(&ssd);
    }
    else {
        desenho_pio(apagar_leds, valor_led, pio, sm); // Apaga os LEDs
    }
    atualizar_linha3(mult); // Desenha a faixa do multiplicador correspondente
    atualizar_linha5(ouro); // Desenha a faixa dourada de 5% de tolerância
    desenho_pio(led_buffer, valor_led, pio, sm); // Desenha os LEDs do buffer na matriz
    ssd1306_send_data(&ssd); // Envia os dados para o display
    printf("e24: %.2f\n", e24); // Imprime o valor da resistência no terminal
    sleep_ms(700);
  }
}
