#include "miniProject.h"
#include "currentTime.h"

#include <string>

#include <BlynkApiWiringPi.h>
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

static const char *auth, *serv;
static uint16_t port;

#include <BlynkWidgets.h>

using namespace std;

unsigned char startByte = 1;
unsigned char sendByte;
char configByte0 = 0b10000000;
char configByte1 = 0b10010000;
char configByte2 = 0b10100000;
char configByte3 = 0b10110000;
string timeVal;

unsigned short humidityVal, lightVal, DACVal, TempVal;
double Vout, Vhum, Vlig, Vdac, Vtemp, Temp;
short outVal;

unsigned char ADCbuffer[3];
unsigned char DACbuffer[2];

BlynkTimer tmr;

int hours,mins,secs;

/*
 * Setup Function. Called once
 */
int setup_gpio(void){
    // Set up Blynk
    Blynk.begin(auth, serv, port);
    
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

void *dataThread(void *threadargs){
    while(true){
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
        Vdac = ((double)DACVal/(double)1024)*3.3;
        
        ADCbuffer[0] = startByte;
        ADCbuffer[1] = configByte3;
        ADCbuffer[2] = 0;
        wiringPiSPIDataRW (SPI_CHAN0, ADCbuffer, 3);
        ADCbuffer[1] = ADCbuffer[1] & 0b00000011;
        TempVal = (ADCbuffer[1] << 8) + ADCbuffer[2];
        Vtemp = ((double)TempVal/(double)1024)*3.3;
        Temp = (Vtemp-0.5)/0.01;
        
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        
        char timeVal[20] = "";
        
        sprintf(timeVal, "%d:%d:%d", hours,mins,secs);
        
        Blynk.virtualWrite(V0, Vout);
        Blynk.virtualWrite(V1, timeVal);
        Blynk.virtualWrite(V2, humidityVal);
        Blynk.virtualWrite(V3, lightVal);
        Blynk.virtualWrite(V4, Temp);
        
        printf("time: %d:%d:%d    humidity: %d light: %d  Vout: %.2f Vdac: %.2f Temp: %.1f \n", hours,mins,secs, humidityVal, lightVal, Vout, Vdac, Temp);
        sleep(2);
    }
}

int main(int argc, char* argv[]){
    parse_options(argc, argv, auth, serv, port);
    
    pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;

    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); /* safe to get existing scheduling param */
    param.sched_priority = newprio; /* set the priority; others are unchanged */
    pthread_attr_setschedparam (&tattr, &param); /* setting the new scheduling param */
    pthread_create(&thread_id, &tattr, dataThread, (void *)1); /* with new priority specified */
    if (setup_gpio() == 1){
        return 0;
    }
    while (true){
        Blynk.run();
        printf("%s \n", "waiting...");
        sleep(1);
    }
    pthread_join(thread_id, NULL);
    pthread_exit(NULL);
    return 0;
}
