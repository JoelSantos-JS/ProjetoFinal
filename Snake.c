#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

#include "inc/ssd1306.h"
#include "inc/font.h"

// -------- DEFINA PINOS --------
// I2C do display
#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define OLED_ADDR 0x3C

// Joystick (ADC)
#define JOY_X 26 // ADC input 0
#define JOY_Y 27 // ADC input 1

// LED RGB
#define LED_R 11
#define LED_G 12
#define LED_B 13

// Botao (opcional)
#define BUTTON_RESET 5

// -------- SNAKE CONFIG --------
#define SCREEN_W 128
#define SCREEN_H 64
#define MAX_LENGTH 100

typedef struct {
    int x, y;
} Segment;

static ssd1306_t display;

// Variaveis snake
static Segment cobra[MAX_LENGTH];
static int tam = 3;
static int dx=1, dy=0;
static int food_x, food_y;
static int score=0;
static bool game_over=false;

// ---------- FUNCOES ----------

// Ajusta LED via PWM (0..255)
void set_led(uint8_t r, uint8_t g, uint8_t b){
    pwm_set_gpio_level(LED_R, r);
    pwm_set_gpio_level(LED_G, g);
    pwm_set_gpio_level(LED_B, b);
}

// Piscar LED verde
void led_green_feedback(){
    set_led(0,255,0); sleep_ms(100);
    set_led(0,0,0);
}

// Piscar LED vermelho
void led_red_feedback(){
    set_led(255,0,0); sleep_ms(100);
    set_led(0,0,0);
}

// Random simples
int random_int(int min, int max){
    uint32_t t = to_ms_since_boot(get_absolute_time());
    return min + (t % (max - min + 1));
}

void gerar_comida(){
    // Gera comida em posição que não esteja sobre a cobra
    int tentativas = 0;
    bool valido;
    
    do {
        food_x = random_int(0, SCREEN_W - 3);
        food_y = random_int(0, SCREEN_H - 3);
        
        valido = true;
        // Verifica se a comida não está sobrepondo a cobra
        for(int i=0; i<tam; i++) {
            if(food_x == cobra[i].x && food_y == cobra[i].y) {
                valido = false;
                break;
            }
        }
        
        tentativas++;
        if(tentativas > 100) break; // Evita loop infinito
    } while(!valido);
}

void init_game(){
    score = 0; 
    tam = 3; 
    game_over = false;
    
    // Posiciona cobra no meio (distância de 2 entre segmentos)
    int cx = SCREEN_W/2, cy = SCREEN_H/2;
    for(int i=0; i<tam; i++){
        cobra[i].x = cx - i*2;
        cobra[i].y = cy;
    }
    
    // Define direção inicial
    dx = 1; 
    dy = 0;
    
    gerar_comida();
}

// Move corpo + cabeça
void move_cobra(){
    for(int i=tam-1; i>0; i--){
        cobra[i] = cobra[i-1];
    }
    cobra[0].x += dx;
    cobra[0].y += dy;
}

void check_collision(){
    // Verifica colisão com paredes
    if(cobra[0].x < 0 || cobra[0].x >= SCREEN_W ||
       cobra[0].y < 0 || cobra[0].y >= SCREEN_H){
        game_over = true;
        return;
    }
    
    // Verifica colisão com próprio corpo (a partir do segundo segmento)
    for(int i=2; i<tam; i++){
        if(cobra[0].x == cobra[i].x && cobra[0].y == cobra[i].y){
            game_over = true;
            return;
        }
    }
}

void check_food(){
    // Verifica se a cabeça da cobra pegou a comida
    if(cobra[0].x == food_x && cobra[0].y == food_y){
        score++;
        if(tam < MAX_LENGTH) tam++;
        led_green_feedback();
        gerar_comida();
    }
}

void draw_snake_screen(){
    // Limpa o display
    ssd1306_fill(&display, false);
    
    // Desenha a cobra (cada segmento como 2x2 pixels)
    for(int i=0; i<tam; i++) {
        ssd1306_pixel(&display, cobra[i].x, cobra[i].y, true);
        ssd1306_pixel(&display, cobra[i].x+1, cobra[i].y, true);
        ssd1306_pixel(&display, cobra[i].x, cobra[i].y+1, true);
        ssd1306_pixel(&display, cobra[i].x+1, cobra[i].y+1, true);
    }
    
    // Desenha a comida
    ssd1306_pixel(&display, food_x, food_y, true);
    ssd1306_pixel(&display, food_x+1, food_y, true);
    ssd1306_pixel(&display, food_x, food_y+1, true);
    ssd1306_pixel(&display, food_x+1, food_y+1, true);

    // Exibe pontuação com destaque
    char txt[20];
    sprintf(txt, "Score:%d", score);
    ssd1306_draw_string(&display, txt, 0, 0);
    
    ssd1306_send_data(&display);
}

void game_over_screen(){
    ssd1306_fill(&display, false);
    ssd1306_draw_string(&display, "GAME OVER", 30, 20);
    char txt[20];
    sprintf(txt,"Score:%d", score);
    ssd1306_draw_string(&display, txt, 40, 35);
    ssd1306_send_data(&display);
    led_red_feedback();
}

// Lê joystick e define dx,dy
void read_joystick(){
    // Eixo X = input 0
    adc_select_input(0);
    uint16_t vx = adc_read();
    // Eixo Y = input 1
    adc_select_input(1);
    uint16_t vy = adc_read();

    // Valores centrais
    const uint16_t central_x = 2048;
    const uint16_t central_y = 2048;
    
    // Zona morta
    const uint16_t dead_zone = 800;
    
    // Guarda direção anterior
    int old_dx = dx;
    int old_dy = dy;

    // Calcula diferenças
    int diff_x = abs((int)vx - central_x);
    int diff_y = abs((int)vy - central_y);
    
    if (diff_x > dead_zone || diff_y > dead_zone) {
        // Determina qual eixo teve o maior desvio
        if (diff_x > diff_y) {
            // Movimento horizontal
            if (vx > central_x) {
                dx = 1; dy = 0;  // Direita
            } else {
                dx = -1; dy = 0; // Esquerda
            }
        } else {
            // Movimento vertical
            if (vy > central_y) {
                dx = 0; dy = 1;  // Baixo
            } else {
                dx = 0; dy = -1; // Cima
            }
        }
    }
    
    // Previne reversão (180°)
    if(old_dx == -dx && old_dy == -dy) {
        dx = old_dx;
        dy = old_dy;
    }
}

void display_controls() {
    ssd1306_fill(&display, false);
    ssd1306_draw_string(&display, "CONTROLES:", 25, 10);
    ssd1306_draw_string(&display, "Cima: Joystick p/cima", 0, 25);
    ssd1306_draw_string(&display, "Baixo: Joystick p/baixo", 0, 35);
    ssd1306_draw_string(&display, "Esq: Joystick p/esq", 0, 45);
    ssd1306_draw_string(&display, "Dir: Joystick p/dir", 0, 55);
    ssd1306_send_data(&display);
    
    // Aguardar pressionar botão
    while(gpio_get(BUTTON_RESET)==1) {
        tight_loop_contents();
    }
    sleep_ms(300); // debounce
}

void game_loop(){
    while(!game_over){
        read_joystick();
        move_cobra();
        check_collision();
        if(game_over) break;
        check_food();
        draw_snake_screen();
        
        sleep_ms(60);
    }
    game_over_screen();
}

// ----- SETUP HARDWARE -----

void init_hardware(){
    stdio_init_all();

    // i2c p/ display
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // SSD1306
    ssd1306_init(&display, WIDTH, HEIGHT, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&display);

    // ADC p/ joystick
    adc_init();
    adc_gpio_init(JOY_X);
    adc_gpio_init(JOY_Y);

    // LED RGB como PWM
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    uint slice_r = pwm_gpio_to_slice_num(LED_R);
    uint slice_g = pwm_gpio_to_slice_num(LED_G);
    uint slice_b = pwm_gpio_to_slice_num(LED_B);

    pwm_set_wrap(slice_r, 255);
    pwm_set_wrap(slice_g, 255);
    pwm_set_wrap(slice_b, 255);

    pwm_set_enabled(slice_r, true);
    pwm_set_enabled(slice_g, true);
    pwm_set_enabled(slice_b, true);

    // Botao
    gpio_init(BUTTON_RESET);
    gpio_set_dir(BUTTON_RESET, GPIO_IN);
    gpio_pull_up(BUTTON_RESET);

    set_led(0,0,0); // apaga LED
}


void display_init_msg() {
    ssd1306_fill(&display, false); // Limpa o display
    ssd1306_rect(&display, 3, 3, 122, 58, true, false); // Desenha um retângulo
    ssd1306_draw_string(&display, "EMBARCATECH", 20, 10);
    ssd1306_draw_string(&display, "26/02", 20, 25);
    ssd1306_draw_string(&display, "Projeto Final", 20, 40);
    ssd1306_send_data(&display); // Envia os dados para o display
}

int main() {
    init_hardware();

    // Exibir tela inicial do EmbarcaTech
    display_init_msg();
    sleep_ms(4000); // Exibe a tela por 4 segundos

    while(true) {
        // Tela inicial do jogo
        ssd1306_fill(&display, false);
        ssd1306_draw_string(&display, "SNAKE GAME", 25, 20);
        ssd1306_draw_string(&display, "Press Button", 20, 35);
        ssd1306_send_data(&display);
        
        // Aguardar pressionar botão
        while(gpio_get(BUTTON_RESET)==1){
            tight_loop_contents();
        }
        sleep_ms(300); // debounce
        
        // Mostrar tela de controles
        display_controls();

        init_game();
        game_loop();

        // Espera para nova partida
        sleep_ms(2000); // Mostra game over por 2 segundos
    }

    return 0;
}