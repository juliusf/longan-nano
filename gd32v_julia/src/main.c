#include "lcd/lcd.h"
#include "gd32v_pjt_include.h"
#include "color.h"
#include "fast_hsv2rgb.h"
#include <string.h>
#include <fastmath.h>

#define RESX 160
#define RESY 40 // Render half-pictures

u16 buff[RESX][RESY];

#define PUTPIXEL(x, y, col)({ \
    buff[x][y] = (col.R << 11) + (col.G << 5) + col.B; \
    })

void showHalfPicture(int upper)
{

    if (upper)
    {
        LCD_Address_Set(0,0,159,39);
    }else
    {
        LCD_Address_Set(0,40,159,79);
    }
    
    for(int y= 0; y < RESY; y++)
    for(int x= 0; x < RESX; x++)
    {
        LCD_WR_DATA(buff[x][y]);
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


// julia set code taken from https://lodev.org/cgtutor/juliamandelbrot.html
void julia(int offsetX, int offsetY, double zoom, int numit)
{
    int h = RESY/2;            // render half-pictures
    int w = RESY;
    double cRe, cIm;           //real and imaginary part of the constant c, determinate shape of the Julia Set
    double newRe, newIm, oldRe, oldIm;   //real and imaginary parts of new and old
    double  moveX = 0, moveY = 0; //you can change these to zoom and change position
    RGB color; //the RGB color value for the pixel
    int maxIterations = numit; //after how much iterations the function should stop

    //pick some values for the constant c, this determines the shape of the Julia Set
    cRe = -0.7;
    cIm = 0.27015;

    //loop through every pixel
    for(int y = 0 + offsetY; y < h + offsetY; y++)
    for(int x = 0 + offsetX; x < w + offsetX; x++)
    {
        //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
        newRe = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
        newIm = (y - h / 2) / (0.5 * zoom * h) + moveY;
        //i will represent the number of iterations
        int i;
        //start the iteration process
        for(i = 0; i < maxIterations; i++)
        {
            //remember value of previous iteration
            oldRe = newRe;
            oldIm = newIm;
            //the actual iteration, the real and imaginary part are calculated
            newRe = oldRe * oldRe - oldIm * oldIm + cRe;
            newIm = 2 * oldRe * oldIm + cIm;
            //if the point is outside the circle with radius 2: stop
            if((newRe * newRe + newIm * newIm) > 4) break;
        }
        
        fast_hsv2rgb_32bit(i%256,255,255*(i<maxIterations),&(color.R), &(color.G), &(color.B));
        PUTPIXEL(x - offsetX, y - offsetY , color);
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

    double zoom = 3.0;
    int numit = 64;
    for(;;)
    {
        
            LEDR_TOG;
            julia(0,0, zoom, numit);
            showHalfPicture(1);
            julia(0,0 + 40, zoom, numit);
            LEDG_TOG;
            showHalfPicture(0);
            numit += 10;
            if (numit > 256)
            {
                numit = 256;
                zoom += 0.1;
            }
        
    }
}
