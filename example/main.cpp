#include <wiringPi.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>


#include "../src/Config/DEV_Config.h"
#include "../src/GUI/GUI_Paint.h"
#include "../src/GUI/GUI_BMPfile.h"
#include "../src/GUI/image.h"

static const double PI = 3.14159265358979;

unsigned long long currentTimeMillis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return
        (unsigned long long)(currentTime.tv_sec) * 1000 +
        (unsigned long long)(currentTime.tv_usec) / 1000;
}

long long lastTime = currentTimeMillis();

void drawSine(float offset, float speed,  int maxX, int maxY, float waveHeight, int color, DOT_PIXEL width) {
    bool first = true;;
    int lx = -1, ly = -1;
    double vx = 0;
    float vmax = 2;
    for (int x = 0; x < maxX; ++x) {
        vx = (speed * offset) + (6 * PI) * x / maxX;
        int y = (maxY/2) - sin(vx) * (maxY*waveHeight/2);
        if (first) {
            first = false;
            continue;
        }
        else {
            Paint_DrawLine(lx, ly, x, y, color, LINE_STYLE_SOLID, width);
        }
        lx = x;
        ly = y;
    }
}
long long lastSecond;
int fps, lastFPS=0;

void frame(int frameCount) {
    long long now = currentTimeMillis();
    
    float refVoltage = 5;

    long long second = now / 1000;
    if (lastSecond < 0) lastSecond = second;

    if (lastSecond != second) {
        lastFPS = fps;
        fps = 0;
        lastSecond = second;
    }
    ++fps;
    
    UBYTE* BlackImage=0;
    int Imagesize = LCD_WIDTH * LCD_HEIGHT * 2;
    if ((BlackImage = (UBYTE*)malloc(Imagesize)) == NULL) {
        printf("Failed to allocate memory for black image...\r\n");
        exit(0);
    }

    Paint_NewImage(BlackImage, LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
    Paint_Clear(BLACK);
    Paint_SetRotate(270);



    /* ywRobot */
    /*
    int minX = 1;
    int minY = 3;
    int maxX = LCD_HEIGHT-1;
    int maxY = LCD_WIDTH-1;
    */

    int minX = 1;
    int minY = 1;
    int maxX = LCD_HEIGHT ;
    int maxY = LCD_WIDTH ;

    int midY = minY + (maxY - minY) / 2;


    Paint_DrawLine(minX, minY,    maxX, minY, WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);  //top
    Paint_DrawLine(minX, maxY,    maxX, maxY, WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);
    Paint_DrawLine(maxX, minY,    maxX, maxY, WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);  //right
    Paint_DrawLine(minX, minY,    minX, maxY, WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);  //left


// x-axis
//    Paint_DrawLine(1, maxY, maxX, maxY, ~WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);   

    for (int v = 1; v < refVoltage; ++v) {
        int y = maxY - ((v / refVoltage) * maxY);
        Paint_DrawLine(1, y, maxX, y, BROWN,    LINE_STYLE_SOLID, DOT_PIXEL_1X1);
    }//   Paint_DrawLine(1, 1, maxX, 1, ~BLUE,     LINE_STYLE_SOLID, DOT_PIXEL_1X1);


    // y-axis
//    Paint_DrawLine(1, maxY, 1, 1, WHITE, LINE_STYLE_SOLID, DOT_PIXEL_1X1);

    int lineColor[9] = {
        //1      2     3        4      5         6        7      8    
        //                            orange
        YELLOW, GREEN, MAGENTA, CYAN,  BRRED,   RED, LIGHTBLUE, LGRAY, -1
    };

    DOT_PIXEL width = DOT_PIXEL_2X2;
    for (int line=0; line < 4; ++line) {
        drawSine(frameCount+line*(PI), .25, maxX, maxY, .65, lineColor[line], width);
        width = DOT_PIXEL_1X1;
    }


    char message[32];
    sprintf(message, "%4.2f", refVoltage);
    Paint_DrawString_EN(2, minY+1, message, &Font24, LBBLUE, LGRAY);

    sprintf(message, "%d-fps", lastFPS);
    Paint_DrawString_EN(maxX - (17 * strlen(message))-2, minY+1, message, &Font24, BLACK, WHITE);

    LCD_Display(BlackImage);
    
    free(BlackImage);  
    
}



int main(void)
{
    int spiHandle;
    int spiChannel = 0;
    int spiSpeed = 65000000;  // 65 mHz for long cable, 90 mHz for short cable

    wiringPiSetup();
    spiHandle=wiringPiSPISetup(spiChannel, spiSpeed);

    GPIO_Config();

    pinMode(10, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(DEV_CS_PIN, OUTPUT);

    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(DEV_CS_PIN, LOW);

    LCD_Init();
    LCD_Clear(WHITE);


    int frameCount = 0;
	while (true)
	{
        frame(frameCount++);
	}


	return 0;
}