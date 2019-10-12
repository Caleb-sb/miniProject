#include "miniProject.h"
#include "currentTime.h"

using namespace std;

unsigned char startByte = 1;
unsigned char sendByte;
char configByte0 = 0b10000000;
char configByte1 = 0b10010000;
char configByte2 = 0b10100000;
char configByte3 = 0b10110000;

unsigned short humidityVal, lightVal, DACVal, TempVal;
double Vout, Vhum, Vlig, Vdac, Vtemp, Temp;
short outVal;

unsigned char ADCbuffer[3];
unsigned char DACbuffer[2];

int hours,mins,secs;

/*
 * Setup Function. Called once
 */
int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();

    //setting up the SPI interface
    if (wiringPiSPISetup (SPI_CHAN0, SPI_SPEED) < 0)
    {
        printf ("Unable to open SPI device 0:\n") ;
        exit (1) ;
    }
    if (wiringPiSPISetup (SPI_CHAN1, SPI_SPEED) < 0)
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
        ADCbuffer[0] = startByte;
        ADCbuffer[1] = configByte0;
        ADCbuffer[2] = 0;

        wiringPiSPIDataRW (SPI_CHAN0, ADCbuffer, 3);
        ADCbuffer[1] = ADCbuffer[1] & 0b00000011;
        humidityVal = (ADCbuffer[1] << 8) + ADCbuffer[2];
        Vhum = ((double)humidityVal/(double)1023)*3.3;
        
        ADCbuffer[0] = startByte;
        ADCbuffer[1] = configByte1;
        ADCbuffer[2] = 0;
        
        wiringPiSPIDataRW (SPI_CHAN0, ADCbuffer, 3);
        ADCbuffer[1] = ADCbuffer[1] & 0b00000011;
        lightVal = (ADCbuffer[1] << 8) + ADCbuffer[2];
        Vlig = (double)lightVal/(double)1023;
        
        Vout = Vlig * Vhum;
        
        outVal = (short)((Vout/3.3)*1023);
        DACbuffer[1] = (unsigned char)(outVal << 2);
        DACbuffer[0] = (unsigned char)(0b00110000 | (outVal >> 6));
        wiringPiSPIDataRW (SPI_CHAN1, DACbuffer, 2);
        
        ADCbuffer[0] = startByte;
        ADCbuffer[1] = configByte2;
        ADCbuffer[2] = 0;
        wiringPiSPIDataRW (SPI_CHAN0, ADCbuffer, 3);
        ADCbuffer[1] = ADCbuffer[1] & 0b00000011;
        DACVal = (ADCbuffer[1] << 8) + ADCbuffer[2];
        Vdac = ((double)DACVal/(double)1023)*3.3;
        
        ADCbuffer[0] = startByte;
        ADCbuffer[1] = configByte3;
        ADCbuffer[2] = 0;
        wiringPiSPIDataRW (SPI_CHAN0, ADCbuffer, 3);
        ADCbuffer[1] = ADCbuffer[1] & 0b00000011;
        TempVal = (ADCbuffer[1] << 8) + ADCbuffer[2];
        Vtemp = ((double)TempVal/(double)1023)*3.3;
        Temp = (Vtemp-0.5)/0.01;
        
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        
        printf("time: %d:%d:%d    humidity: %d light: %d  Vout: %.2f Vdac: %.2f Temp: %.1f \n", hours,mins,secs, humidityVal, lightVal, Vout, Vdac, Temp);
        
        sleep(1);
    }
}
