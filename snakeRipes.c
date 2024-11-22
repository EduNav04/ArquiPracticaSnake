#include "ripes_system.h"
#include <stdio.h>
#include <stdlib.h>

#define SW0 (0x01)


volatile unsigned int * initialPosition = (int*) LED_MATRIX_0_BASE;
volatile unsigned int * d_pad_up = (int*) D_PAD_0_UP;
volatile unsigned int * d_pad_do = (int*) D_PAD_0_DOWN;
volatile unsigned int * d_pad_le = (int*) D_PAD_0_LEFT;
volatile unsigned int * d_pad_ri = (int*) D_PAD_0_RIGHT;
volatile unsigned int * switch_base = (int*) SWITCHES_0_BASE;
const volatile unsigned int * TOP = (int *) LED_MATRIX_0_BASE;
const volatile unsigned int * BOTTOM = (int *) LED_MATRIX_0_BASE + (LED_MATRIX_0_SIZE / 4);
volatile unsigned int * snake[100]; 

volatile unsigned int* applePosition = NULL;

// Limpia la matriz de LEDs apagando todas las posiciones
void clean()
{
    volatile unsigned int * led = (int*) LED_MATRIX_0_BASE; // Apunta al primer LED
    for(int i = 0; i < (LED_MATRIX_0_SIZE)/4; i++)
    {
        *led = 0x00; 
        led++; 
    }
}

// Dibuja un cuadrado en la posición indicada con el color dado
void printSquare(volatile unsigned int* led, int color)
{
    *led = color;           
    led--;                  // Mueve a la izquierda
    *led = color;           
    led -= LED_MATRIX_0_WIDTH; // Mueve hacia arriba
    *led = color;           
    led++;                  // Mueve a la derecha
    *led = color;           
    led -= LED_MATRIX_0_WIDTH; // Mueve hacia arriba
}

// Verifica si una posición está fuera de los límites válidos
int isInside(volatile unsigned int* snake)
{
    int posIndex = snake - (volatile unsigned int * ) LED_MATRIX_0_BASE;
    int x = posIndex % LED_MATRIX_0_WIDTH;
    int y = posIndex / LED_MATRIX_0_WIDTH;

    return x == 0 || x == LED_MATRIX_0_WIDTH || y == 0 || y == LED_MATRIX_0_HEIGHT;
}

// Verifica si la posición siguiente colisiona con el cuerpo de la serpiente
int isSelfCollision(volatile unsigned int* nextPos, int size) {
    for (int i = 1; i < size; i++) {
        if (snake[i] == nextPos) {
            return 1;  // Colisión detectada
        }
    }
    return 0;  // No hay colisión
}

// Dibuja la serpiente en la matriz y actualiza su posición
void drawSnake(int size, int speed)
{
    for(int i = size; i > 0; i--)
    {
        snake[i] = snake[i - 1];            // Mueve las partes de la serpiente
        printSquare(snake[i], 0x0FF00);   
    }
    snake[0] += speed;                     // Calcula la nueva posición de la cabeza
    printSquare(snake[0], 0x00FF00);       
}

// Genera un número aleatorio 
int getRandom(int min, int max) {
    return min + rand() % (max - min);
}

// Verifica si una posición está la serpiente
int isPositionOccupied(volatile unsigned int* position, int size) {
    for(int i = 0; i < size; i++) {
        if(snake[i] == position) {
            return 1;  // Posición ocupada
        }
    }
    return 0;  // Posición libre
}

// Verifica si una posición está dentro de los límites de la matriz
int isValidPosition(volatile unsigned int* position) {
    int posIndex = position - (volatile unsigned int*)LED_MATRIX_0_BASE;
    int x = posIndex % LED_MATRIX_0_WIDTH;
    int y = posIndex / LED_MATRIX_0_WIDTH;
    
    return x > 1 && x < (LED_MATRIX_0_WIDTH - 2) && 
           y > 1 && y < (LED_MATRIX_0_HEIGHT - 2);
}

// Genera una nueva posición para la manzana y la dibuja
void generateAndDrawApple(int size) {
    volatile unsigned int* newPosition;
    int validPosition = 0;
    
    static int initialized = 0;
    if (!initialized) {
        srand(1234);  // Inicializa la semilla del generador aleatorio
        initialized = 1;
    }
    
    while(!validPosition) {
        int x = getRandom(2, LED_MATRIX_0_WIDTH - 2);
        int y = getRandom(2, LED_MATRIX_0_HEIGHT - 2);
        newPosition = (volatile unsigned int*)LED_MATRIX_0_BASE + 
                     (y * LED_MATRIX_0_WIDTH + x);
        
        if(isValidPosition(newPosition) && !isPositionOccupied(newPosition, size)) {
            validPosition = 1;
        }
    }
    
    applePosition = newPosition;               // posición de la manzana
    printSquare(applePosition, 0xFF0000);      
}

// Función principal que gestiona el juego
void main()
{
    clean(); // Limpia la matriz de LEDs al iniciar

    int size = 2; // Tamaño inicial de la serpiente
    int counter = 0;

    snake[size] = initialPosition;

    int speed = 1;

    snake[size] += 50;
    snake[size] += 200;

    while(1)
    {
        // Cambia la dirección
        if(*d_pad_up == 1)
            speed = -LED_MATRIX_0_WIDTH;
        if(*d_pad_do == 1)
            speed = LED_MATRIX_0_WIDTH;
        if(*d_pad_le == 1)
            speed = -1;
        if(*d_pad_ri == 1)
            speed = 1;

        volatile unsigned int* nextPos = snake[0] + speed;

        // colisión con bordes o el cuerpo de la serpiente
        if(isInside(nextPos) || isSelfCollision(nextPos, size)){
            for(int i = 0; i < size; i++)
            {
                snake[i] = 0;
            }
            clean();
            size = 2;
            snake[0] = initialPosition;
            snake[1] = initialPosition + 50;
            snake[2] = initialPosition + 200;
            speed = 1;
            counter = 0;
            generateAndDrawApple(size);
        }

        // ver si come la manzana
        if (snake[0] + speed == applePosition) {
            size += 2;
            snake[size] = snake[size - 1];
            generateAndDrawApple(size);
        }

        printSquare(snake[size], 0x00); // Limpia la posición anterior
        drawSnake(size, speed);        // Dibuja la serpiente avanzando

        // reinicia si pega con pared
        if(0x01 & *switch_base || isInside(snake[0]))
        {
            for(int i = 0; i < size; i++)
            {
                snake[i] = 0;
            }
            clean();
            size = 2;
            snake[0] = initialPosition;
            snake[0] += 50;
            snake[0] += 200;
            speed = 1;
            counter = 0;
            generateAndDrawApple(size);  
        }

        for(int i=0; i < 5000; i++); // delay
    }
}
