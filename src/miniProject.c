#include "miniProject.h"
#include "currentTime.h"

using namespace std;

unsigned char startByte = 1;
unsigned char sendByte;
char configByte = 0b10000000;

unsigned short adcVal;

unsigned char buffer[3];

int hours,mins,secs;

/*
 * Setup Function. Called once
 */
int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();

    //setting up the SPI interface
    if (wiringPiSPISetup (SPI_CHAN, SPI_SPEED) < 0)
    {
        printf ("Unable to open SPI device 0:\n") ;
        exit (1) ;
    }
    return 0;
}

int main(void){
    if (setup_gpio() == 1){
        return 0;
    }
    while (true){
        buffer[0] = startByte;
        buffer[1] = configByte;
        buffer[2] = 0;

        wiringPiSPIDataRW (SPI_CHAN, buffer, 3);
        buffer[1] = buffer[1] & 0b00000011;
        adcVal = (buffer[1] << 8) + buffer[2];
        
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        
        printf("time: %d:%d:%d      val: %d \n", hours,mins,secs, adcVal);
        
        sleep(1);
    }
}
