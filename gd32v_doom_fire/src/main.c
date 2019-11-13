#include "lcd/lcd.h"
#include "gd32v_pjt_include.h"
#include <string.h>
#include <fastmath.h>
#include <stdlib.h>

#define RESX 160
#define RESY 80 
#define FIRE_WIDTH 160
#define FIRE_HEIGHT 80
#define BUFFER_SIZE FIRE_WIDTH * FIRE_HEIGHT

uint8_t buff[BUFFER_SIZE];

uint16_t colorPalette[] = 
{
    0x20,
    0x1820,
    0x2860,
    0x4060,
    0x50a0,
    0x60e0,
    0x70e0,
    0x8920,
    0x9960,
    0xa9e0,
    0xba20,
    0xc220,
    0xda20,
    0xdaa0,
    0xdaa0,
    0xd2e0,
    0xd2e0,
    0xd321,
    0xcb61,
    0xcba1,
    0xcbe1,
    0xcc22,
    0xc422,
    0xc462,
    0xc4a3,
    0xbce3,
    0xbce3,
    0xbd24,
    0xbd24,
    0xbd65,
    0xb565,
    0xb5a5,
    0xb5a6,
    0xce6d,
    0xdef3,
    0xef78,
    0xffff,
};

/*
#define PUTPIXEL(x, y, col_idx)({ \
    buff[x][y] = col_idx; \
    })
*/

void drawScreen()
{
    uint16_t col_idx;

    LCD_Address_Set(0,0,159,79);    
    
    for(int x= 0; x < BUFFER_SIZE; x++)
    {
        col_idx = buff[x];
        LCD_WR_DATA(colorPalette[col_idx]);
    }
}

void init_uart0(void)
{	
	/* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

	/* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    usart_interrupt_enable(USART0, USART_INT_RBNE);
}

void spreadFire(uint16_t src)
{
    if (buff[src] == 0)
        buff[src - FIRE_WIDTH] = buff[src];
    else
    {
        int rnd = rand() % 3;
        int dst = src - rnd + 1;
        buff[dst - FIRE_WIDTH] = buff[src] - (rnd &1);
    }
}

int main(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1|GPIO_PIN_2);

    init_uart0();

    Lcd_Init();			// init OLED
    LCD_Clear(BLACK);
    BACK_COLOR=BLACK;

    LEDR(1);
    LEDG(1);
    LEDB(1);    

    // init screen
    for (int x = 0; x < BUFFER_SIZE; x++)
    {
        if(x < BUFFER_SIZE - FIRE_WIDTH )
            buff[x] = 0; // most is black
        else
            buff[x] = 36; // last line initialized as pure white   
    }

    for(;;)
    {
        
        for (int x = 0; x < FIRE_WIDTH; x++)
        {
        for (int y = 1; y < FIRE_HEIGHT; y++)
        {
            spreadFire(y * FIRE_WIDTH + x);
        }
        }   
            drawScreen();
           // delay_1ms(1500);
            //LEDG_TOG; 
    
    }
}
  