#include "shell.h"
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "filesystem.h"

#include "host.h"

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Library includes. */
#include "stm32f4xx_it.h"
#include "stm32f4xx_tim.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32_eval_legacy.h"

#include "gfx.h"

static void prvLCDTask( void *pvParameters );
/*
static void prvLCDTask2( void *pvParameters );
*/
QueueHandle_t xLCDQueue;
TaskHandle_t xHandle;

static void prvLCDTask( void *pvParameters )
{
	unsigned char *pucMessage;
	unsigned long ulLine = Line3;
	const unsigned long ulLineHeight = 12;
	static char cMsgBuf[ 30 ];
	extern unsigned short usMaxJitter;

    ( void ) pvParameters;

    /* The LCD gatekeeper task as described in the comments at the top of this
    file. */

    /* Init LCD and LTCD. Enable layer1 only. */
    LCD_Init();
    LCD_LayerInit();
    LTDC_LayerCmd(LTDC_Layer1, ENABLE);
	LTDC_LayerCmd(LTDC_Layer2, DISABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
    LTDC_Cmd(ENABLE);
    LCD_SetLayer(LCD_BACKGROUND_LAYER);

    /* Display startup messages. */
    LCD_SetFont(&Font8x12);
    LCD_Clear(White);
    LCD_SetTextColor(Green);
    LCD_DisplayStringLine( Line0, (uint8_t *)"       www.freertos.org" );
    LCD_SetTextColor(Blue);
    LCD_DisplayStringLine( Line1, (uint8_t *)"     STM32F429i Discovery" );
    LCD_SetTextColor(Black);
	for( ;; )
    {
        /* Wait for a message to arrive to be displayed. */
        xQueueReceive( xLCDQueue, &pucMessage, portMAX_DELAY );

        /* Clear the current line of text. */
        LCD_ClearLine( ulLine );

        /* Move on to the next line. */
        ulLine += ulLineHeight;
        if( ulLine > LCD_LINE_18 )
        {
            ulLine = Line3;
        }

        /* Display the received text, and the max jitter value. */
        sprintf( cMsgBuf, "%s [%luns]", pucMessage, usMaxJitter * mainNS_PER_CLOCK );
        LCD_DisplayStringLine( ulLine, ( unsigned char * ) cMsgBuf );
    }
}
 
void mandelbrot(float x1, float y1, float x2, float y2, int parameter) {
    unsigned int i,j, width, height;
    uint16_t iter;
    color_t color;
    float fwidth, fheight;
   
    float sy = y2 - y1;
    float sx = x2 - x1;
    const int MAX = 512;

    width = (unsigned int)gdispGetWidth()/2;
    height = (unsigned int)gdispGetHeight()/2;
    fwidth = width;
    fheight = height;

    for(i = 0; i < width; i++) {
        for(j = 0; j < height; j++) {
            float cy = j * sy / fheight + y1;
            float cx = i * sx / fwidth + x1;
            float x=0.0f, y=0.0f, xx=0.0f, yy=0.0f;
            for(iter=0; iter <= MAX && xx+yy<4.0f; iter++) {
                xx = x*x;
                yy = y*y;
                y = 2.0f*x*y + cy;
                x = xx - yy + cx;
            }
            //color = ((iter << 8) | (iter&0xFF));
            color = RGB2COLOR(iter<<7, iter<<4, iter);
            //gdispDrawPixel(i, j, color);
			gdispDrawPixel(i+120*(parameter-1), j+160*(parameter-1), color);
        }
    }
}

void main_mandelbrot(void * parameter)
{
	float cx, cy;
    float zoom = 1.0f;

    gfxInit();

    /* where to zoom in */
    cx = -0.086f;
    cy = 0.85f;

	fio_printf(1, "\r\n%d\r\n", (int)parameter);
    fio_printf(1, "\r\n");

    while(TRUE) {
//        mandelbrot(-2.0f*zoom+cx, -1.5f*zoom+cy, 2.0f*zoom+cx, 1.5f*zoom+cy);
		mandelbrot(-2.0f*zoom+cx, -1.5f*zoom+cy, 2.0f*zoom+cx, 1.5f*zoom+cy, (int)parameter);
     
	   zoom *= 0.7f;
        if(zoom <= 0.00001f)
            zoom = 1.0f;
    }
}

GHandle GW1, GW2;
int gwintest() 
{
    coord_t     i, j;
    gfxInit();
    gdispClear(White);
	{
        GWindowInit wi;
        gwinClearInit(&wi);
        wi.show = TRUE; wi.x = 20; wi.y = 10; wi.width = 200; wi.height = 150;
        GW1 = gwinWindowCreate(0, &wi);
        wi.show = TRUE; wi.x = 50; wi.y = 190; wi.width = 150; wi.height = 100;
        GW2 = gwinWindowCreate(0, &wi);
    }
	gwinSetColor(GW1, Black);
    gwinSetBgColor(GW1, White);
    gwinSetColor(GW2, White);
    gwinSetBgColor(GW2, Blue);
    gwinClear(GW1);
    gwinClear(GW2);
    gwinDrawLine(GW1, 5, 30, 150, 110);
    for(i = 5, j = 0; i < 200 && j < 150; i += 3, j += i/20)
            gwinDrawPixel(GW1, i, j);
    gwinFillCircle(GW1, 20, 20, 15);
    gwinFillCircle(GW2, 20, 20, 15);
	while(TRUE) {
        gfxSleepMilliseconds(500);
    }
}
GEventMouse     ev;
#define COLOR_SIZE  20
#define PEN_SIZE    20
#define POFFSET     3
#define COLOR_BOX(a)        (ev.x >= a && ev.x <= a + COLOR_SIZE)
#define PEN_BOX(a)          (ev.y >= a && ev.y <= a + COLOR_SIZE)
#define GET_COLOR(a)        (COLOR_BOX(a * COLOR_SIZE + POFFSET))
#define GET_PEN(a)          (PEN_BOX(a * 2 * PEN_SIZE + POFFSET))
#define DRAW_COLOR(a)       (a * COLOR_SIZE + POFFSET)
#define DRAW_PEN(a)         (a * 2 * PEN_SIZE + POFFSET)
#define DRAW_AREA(x, y)     (x >= PEN_SIZE + POFFSET + 3 && x <= gdispGetWidth() && \
                             y >= COLOR_SIZE + POFFSET + 3 && y <= gdispGetHeight())
#define mainFLASH_TASK_PRIORITY             ( tskIDLE_PRIORITY + 1 )
#define mainLCD_TASK_PRIORITY               ( tskIDLE_PRIORITY + 2 )
void drawScreen(void)
{
    char *msg = "uGFX";
    font_t      font1, font2;
    font1 = gdispOpenFont("DejaVuSans24*");
    font2 = gdispOpenFont("DejaVuSans12*");
    gdispClear(White);
    gdispDrawString(gdispGetWidth()-gdispGetStringWidth(msg, font1)-3, 3, msg, font1, Black);
    gdispFillArea(0 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);    /* Black */
    gdispFillArea(1 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Red);      /* Red */
    gdispFillArea(2 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Yellow);   /* Yellow */
    gdispFillArea(3 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Green);    /* Green */
    gdispFillArea(4 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Blue);     /* Blue */
    gdispDrawBox (5 * COLOR_SIZE + 3, 3, COLOR_SIZE, COLOR_SIZE, Black);    /* White */
    gdispFillStringBox(POFFSET * 2, DRAW_PEN(1), PEN_SIZE, PEN_SIZE, "1", font2, White, Black, justifyCenter);
    gdispFillStringBox(POFFSET * 2, DRAW_PEN(2), PEN_SIZE, PEN_SIZE, "2", font2, White, Black, justifyCenter);
    gdispFillStringBox(POFFSET * 2, DRAW_PEN(3), PEN_SIZE, PEN_SIZE, "3", font2, White, Black, justifyCenter);
    gdispFillStringBox(POFFSET * 2, DRAW_PEN(4), PEN_SIZE, PEN_SIZE, "4", font2, White, Black, justifyCenter);
    gdispFillStringBox(POFFSET * 2, DRAW_PEN(5), PEN_SIZE, PEN_SIZE, "5", font2, White, Black, justifyCenter);
    gdispCloseFont(font1);
    gdispCloseFont(font2);
}
static void prvLCDTask2(void *pvParameters)
{
    color_t color = Black;
    uint16_t pen = 0;
    ( void ) pvParameters;
    gfxInit();
    ginputGetMouse(9999);

    drawScreen();
    while (TRUE) {
        ginputGetMouseStatus(0, &ev);
        if (!(ev.current_buttons & GINPUT_MOUSE_BTN_LEFT))
            continue;
        if(ev.y >= POFFSET && ev.y <= COLOR_SIZE) {
                 if(GET_COLOR(0))   color = Black;
            else if(GET_COLOR(1))   color = Red;
            else if(GET_COLOR(2))   color = Yellow;
            else if(GET_COLOR(3))   color = Green;
            else if(GET_COLOR(4))   color = Blue;
            else if(GET_COLOR(5))   color = White;
        } else if(ev.x >= POFFSET && ev.x <= PEN_SIZE) {
                 if(GET_PEN(1))     pen = 0;
            else if(GET_PEN(2))     pen = 1;
            else if(GET_PEN(3))     pen = 2;
            else if(GET_PEN(4))     pen = 3;
            else if(GET_PEN(5))     pen = 4;
		} else if(DRAW_AREA(ev.x, ev.y)) {
            if(pen == 0)
                gdispDrawPixel(ev.x, ev.y, color);
            else
                gdispFillCircle(ev.x, ev.y, pen, color);
        }
    }
}

typedef struct {
	const char *name;
	cmdfunc *fptr;
	const char *desc;
} cmdlist;

void ls_command(int, char **);
void man_command(int, char **);
void cat_command(int, char **);
void ps_command(int, char **);
void host_command(int, char **);
void help_command(int, char **);
void host_command(int, char **);
void mmtest_command(int, char **);
void test_command(int, char **);
void mandel_command(int, char **);
//void ugfx_command(int, char **);
void delete_command(int, char **);
void mandelbrot_command(int, char **);
void gwin_command(int, char **);
void notepad_command(int, char **);

#define MKCL(n, d) {.name=#n, .fptr=n ## _command, .desc=d}

cmdlist cl[]={
	MKCL(ls, "List directory"),
	MKCL(man, "Show the manual of the command"),
//	MKCL(cat, "Concatenate files and print on the stdout"),
	MKCL(ps, "Report a snapshot of the current processes"),
//	MKCL(host, "Run command on host"),
//	MKCL(mmtest, "heap memory allocation test"),
	MKCL(help, "help"),
	MKCL(test, "test LCD"),
	MKCL(mandel, "test mandelbrot"),
//	MKCL(ugfx, "test ugfx"),
	MKCL(delete, "delete task"),
	MKCL(mandelbrot, "run mandelbrot task"),
	MKCL(notepad,"notepad"),
};

int parse_command(char *str, char *argv[]){
	int b_quote=0, b_dbquote=0;
	int i;
	int count=0, p=0;
	for(i=0; str[i]; ++i){
		if(str[i]=='\'')
			++b_quote;
		if(str[i]=='"')
			++b_dbquote;
		if(str[i]==' '&&b_quote%2==0&&b_dbquote%2==0){
			str[i]='\0';
			argv[count++]=&str[p];
			p=i+1;
		}
	}
	/* last one */
	argv[count++]=&str[p];

	return count;
}

void ls_command(int n, char *argv[]){
    fio_printf(1, "\r\n");
}

int filedump(const char *filename){
	char buf[128];
/*
	int fd=fs_open(filename, 0, O_RDONLY);

	if(fd==OPENFAIL)
		return 0;

	fio_printf(1, "\r\n");

	int count;
	while((count=fio_read(fd, buf, sizeof(buf)))>0){
		fio_write(1, buf, count);
	}

	fio_close(fd);
	return 1;
*/
}

void ps_command(int n, char *argv[]){
	signed char buf[1024];
	vTaskList(buf);
	fio_printf(1, "\r\n%s\r\n", buf);	
}
/*
void cat_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: cat <filename>\r\n");
		return;
	}

	if(!filedump(argv[1]))
		fio_printf(2, "\r\n%s no such file or directory.\r\n", argv[1]);
}
*/
void man_command(int n, char *argv[]){
	if(n==1){
		fio_printf(2, "\r\nUsage: man <command>\r\n");
		return;
	}

	char buf[128]="/romfs/manual/";
	strcat(buf, argv[1]);

	if(!filedump(buf))
		fio_printf(2, "\r\nManual not available.\r\n");
}
/*
void host_command(int n, char *argv[]){
	if(n>1){
		int len=strlen(argv[1]), rnt;
		if(argv[1][0]=='\''){
			argv[1][len-1]='\0';
			rnt=host_system(argv[1]+1);
		}else
			rnt=host_system(argv[1]);
		fio_printf(1, "\r\nfinish with exit code %d.\r\n", rnt);
	}else
		fio_printf(2, "\r\nUsage: host 'command'\r\n");
}
*/
void help_command(int n,char *argv[]){
	int i;
	fio_printf(1, "\r\n");
	for(i=0;i<sizeof(cl)/sizeof(cl[0]); ++i){
		fio_printf(1, "%s - %s\r\n", cl[i].name, cl[i].desc);
	}
}

void test_command(int n,char *argv[]){
    fio_printf(1, "\r\n");
	/* Create the queue used by the LCD task.  Messages for display on the LCD
	are received via this queue. */
	xLCDQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );
	/* Start the LCD gatekeeper task - as described in the comments at the top
	of this file. */	
	xTaskCreate( prvLCDTask, "LCD", configMINIMAL_STACK_SIZE * 2, NULL, mainLCD_TASK_PRIORITY, &xHandle );
}

int abc=0;
void mandelbrot_command(int n,char *argv[]){
	abc++;
    fio_printf(1, "\r\n");
	xLCDQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );
	xTaskCreate(main_mandelbrot,
                    (signed portCHAR *) "Mandelbrot",
                    512 /* stack size */, (void *)abc, tskIDLE_PRIORITY , NULL);
}

void mandel_command(int n,char *argv[]){
	fio_printf(1, "\r\n");
	xLCDQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );
    xTaskCreate( main_mandelbrot, "Mandelbrot", configMINIMAL_STACK_SIZE * 2, NULL, mainLCD_TASK_PRIORITY - 2, &xHandle );
}
void notepad_command(int n,char *argv[])
{
    fio_printf(1, "\r\n");
    xTaskCreate( prvLCDTask2, "LCD2", configMINIMAL_STACK_SIZE * 2, NULL, mainLCD_TASK_PRIORITY-2, NULL );
}

#if 0
void ugfx_command(int n,char *argv[]){
	//xLCDQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );
	/* Start the LCD task */
	xTaskCreate( prvLCDTask2, "LCD", configMINIMAL_STACK_SIZE * 2, NULL, mainLCD_TASK_PRIORITY, NULL );
}
#endif
void delete_command(int n,char *argv[]){
	fio_printf(1, "\r\n");
	vTaskDelay( ( TickType_t ) 0 );
	vTaskDelete( xHandle );
}

cmdfunc *do_command(const char *cmd){

	int i;

	for(i=0; i<sizeof(cl)/sizeof(cl[0]); ++i){
		if(strcmp(cl[i].name, cmd)==0)
			return cl[i].fptr;
	}
	return NULL;	
}
