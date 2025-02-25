#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "matriz_leds.h"

// Arquivo .pio para controle da matriz
#include "pio_matrix.pio.h"

// Pino que realizará a comunicação do microcontrolador com a matriz
#define OUT_PIN 7

// Gera o binário que controla a cor de cada célula do LED
// Rotina para definição da intensidade de cores do LE
uint32_t gerar_binario_cor(double red, double green, double blue)
{
    return ((uint8_t)(green * 255) << 24) | ((uint8_t)(red * 255) << 16) | ((uint8_t)(blue * 255) << 8);
}

uint configurar_matriz(PIO pio) {
    bool ok;

    // Define o clock para 128 MHz, facilitando a divisão pelo clock
    ok = set_sys_clock_khz(128000, false);

    // Inicializa todos os códigos stdio padrão que estão ligados ao binário.
    stdio_init_all();

    printf("iniciando a transmissão PIO");
    if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    // Configurações da PIO
    uint offset = pio_add_program(pio, &pio_matrix_program);
    uint sm = pio_claim_unused_sm(pio, true);
    pio_matrix_program_init(pio, sm, offset, OUT_PIN);

    return sm;
}

void imprimir_desenho(Matriz_leds_config configuracao, PIO pio, uint sm) {
    for (int contadorLinha = 4; contadorLinha >= 0; contadorLinha--) {
        int incremento = (contadorLinha % 2 == 0) ? -1 : 1;  // Decide o sentido de iteração para a linha
        int colunaInicial = (contadorLinha % 2 == 0) ? 4 : 0;  // Define a coluna inicial com base no par/ímpar

        for (int contadorColuna = colunaInicial; (contadorLinha % 2 == 0) ? (contadorColuna >= 0) : (contadorColuna < 5); contadorColuna += incremento) {
            uint32_t valor_cor_binario = gerar_binario_cor(
                configuracao[contadorLinha][contadorColuna].red,
                configuracao[contadorLinha][contadorColuna].green,
                configuracao[contadorLinha][contadorColuna].blue
            );
            pio_sm_put_blocking(pio, sm, valor_cor_binario);
        }
    }
}

RGB_cod obter_cor_por_parametro_RGB(int red, int green, int blue) {
    // Garantir que os valores de cor estejam dentro do intervalo [0, 255]
    red = red < 0 ? 0 : (red > 255 ? 255 : red);
    green = green < 0 ? 0 : (green > 255 ? 255 : green);
    blue = blue < 0 ? 0 : (blue > 255 ? 255 : blue);

    RGB_cod cor_customizada = {red / 255.0, green / 255.0, blue / 255.0};
    return cor_customizada;
}
