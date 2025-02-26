# SnakeBit: Jogo da Cobrinha para BitDogLab

## Resumo do Projeto

Este projeto é uma implementação do clássico jogo Snake (Jogo da Cobrinha) desenvolvido especificamente para a plataforma BitDogLab, que utiliza o microcontrolador RP2040. O jogo aproveita os recursos nativos da placa para criar uma experiência interativa e divertida.

### Características Principais

- **Controle por Joystick Analógico**: Movimente a cobra usando o joystick integrado da placa BitDogLab
- **Display OLED SSD1306**: Interface gráfica com resolução de 128x64 pixels via comunicação I2C
- **Feedback Visual com LED RGB**: Indicação de eventos do jogo (verde ao comer, vermelho em game over)
- **Mecânica Clássica**: A cobra cresce ao comer comida e morre ao colidir com as bordas ou consigo mesma
- **Interface Amigável**: Telas de início, controles e game over para melhor experiência do usuário

### Tecnologias Utilizadas

- **Microcontrolador**: RP2040 (Raspberry Pi Pico)
- **Periféricos**: ADC (joystick), I2C (display), PWM (LEDs), GPIO (botão)
- **Linguagem**: C
- **Ambiente de Desenvolvimento**: SDK do Raspberry Pi Pico

### Funcionalidades

- Sistema de pontuação
- Geração aleatória de comida
- Prevenção de movimento reverso (180°)
- Feedback visual através de LEDs
- Ajuste de velocidade e sensibilidade do controle

## Imagens e Demonstração

[Link para o vídeo de demonstração](https://youtu.be/FhoXMTbeI5w)

## Como Compilar e Executar

1. Clone este repositório
2. Configure o ambiente do SDK do Raspberry Pi Pico
3. Execute `mkdir build && cd build && cmake .. && make`
4. Copie o arquivo `Snake.uf2` gerado para a placa BitDogLab em modo bootloader

## Estrutura do Projeto

- `Snake.c`: Código principal do jogo
- `inc/ssd1306.c` e `inc/ssd1306.h`: Driver para o display OLED
- `inc/font.h`: Definições de fonte para o display

## Licença

Este projeto é disponibilizado sob a licença MIT.

## Autoria

Desenvolvido como projeto final para o programa EmbarcaTech 2025.
