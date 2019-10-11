#include "miniProject.h"
#include "currentTime.h"

using namespace std;

unsigned char startByte = 1;
unsigned char sendByte;
char configByte0 = 0b10000000;
char configByte1 = 0b10010000;

unsigned short humidityVal, lightVal;
double Vout, Vhum, Vlig;

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
        buffer[1] = configByte0;
        buffer[2] = 0;

        wiringPiSPIDataRW (SPI_CHAN, buffer, 3);
        buffer[1] = buffer[1] & 0b00000011;
        humidityVal = (buffer[1] << 8) + buffer[2];
        Vhum = ((double)humidityVal/(double)1023)*3.3;
        
        buffer[0] = startByte;
        buffer[1] = configByte1;
        buffer[2] = 0;
        
        wiringPiSPIDataRW (SPI_CHAN, buffer, 3);
        buffer[1] = buffer[1] & 0b00000011;
        lightVal = (buffer[1] << 8) + buffer[2];
        Vlig = (double)lightVal/(double)1023;
        
        Vout = Vlig * Vhum;
        
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        
        printf("time: %d:%d:%d    humidity: %d light: %d  Vout: %.2f \n", hours,mins,secs, humidityVal, lightVal, Vout);
        
        sleep(1);
    }
}
