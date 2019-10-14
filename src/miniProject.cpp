#include "miniProject.h"
#include "currentTime.h"

#include <BlynkApiWiringPi.h>
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

static const char *auth, *serv;
static uint16_t port;

#include <BlynkWidgets.h>

using namespace std;

void *dataThread(void *threadargs);
void toggleSampling(int param);
void updateSystemTime(void);
void resetSystemTime(void);

long lastInterruptTime = 0;

unsigned char startByte = 1;
unsigned char sendByte;
char configByte0 = 0b10000000;
char configByte1 = 0b10010000;
char configByte2 = 0b10100000;
char configByte3 = 0b10110000;

unsigned short humidityVal, lightVal, DACVal, TempVal;
double Vout, Vhum, Vlig, Vdac, Vtemp, Temp;
short outVal;

int sampleInterval = 1;
int counter;

bool running = false;

unsigned char ADCbuffer[3];
unsigned char DACbuffer[2];

char timeVal[20] = "";

int hours,mins,secs, sysHours, sysMins, sysSecs, secondsTimer;

BlynkTimer tmr;


BLYNK_READ(V0)
{
  Blynk.virtualWrite(V0, Vdac);
}


BLYNK_READ(V2)
{
  Blynk.virtualWrite(V2, Vhum);
}


BLYNK_READ(V3)
{
  Blynk.virtualWrite(V3, lightVal);
}

BLYNK_READ(V4)
{
  Blynk.virtualWrite(V5, Temp);
}

BLYNK_WRITE(V5)
{
    sampleInterval = param[0];
}

BLYNK_READ(V5)
{
    Blynk.virtualWrite(V5, sampleInterval);
}

BLYNK_WRITE(V7)
{
    toggleSampling(param[0]);
}

BLYNK_READ(V7)
{
    if (running){
        Blynk.virtualWrite(V7, 1);
    } else {
        Blynk.virtualWrite(V7, 0);
    }

}

BLYNK_WRITE(V6){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>200)
    {
        resetSystemTime();
        lastInterruptTime = currentTime;
    }
}

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
        printf ("Unable to open SPI device 1:\n") ;
        exit (1) ;
    }
    return 0;
}

void toggleSampling(int param){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>500)
    {
        if(param == 0)
        {
            running = false;
        }
        else{
            running = true;
        }
        lastInterruptTime = currentTime;
    }
}

void timeThread(void){
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        secondsTimer = secondsTimer + 1;
        updateSystemTime();

        sprintf(timeVal, "%d:%d:%d / %d:%d:%d", hours, mins, secs, sysHours, sysMins, sysSecs);
        //sprintf(timeVal, "%d:%d:%d", hours, mins, secs);

        Blynk.virtualWrite(V1, timeVal);
}


void *dataThread(void *threadargs){
    counter = 1;
    while(true){
        if (running){
            if (counter >= sampleInterval){
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

                Blynk.virtualWrite(V0, Vout);
                Blynk.virtualWrite(V2, Vhum);
                Blynk.virtualWrite(V3, lightVal);
                Blynk.virtualWrite(V4, Temp);

                printf("humidity: %d light: %d  Vout: %.2f Vdac: %.2f Temp: %.1f \n", humidityVal, lightVal, Vout, Vdac, Temp);
                counter = 0;
            } 
        } else {
            Blynk.virtualWrite(V0, "-");
            Blynk.virtualWrite(V2, "-");
            Blynk.virtualWrite(V3, "-");
            Blynk.virtualWrite(V4, "-");
        }
        counter++;
        timeThread();
        sleep(1);
    }
}

void updateSystemTime(void){
    sysMins = secondsTimer / 60;
    sysSecs = secondsTimer % 60;
    sysHours = sysMins / 60;
    sysMins = sysMins % 60;
}

void resetSystemTime(void){
    secondsTimer = 0;
}

void loop()
{
    Blynk.run();
    tmr.run();
    Blynk.syncVirtual(V7);
    Blynk.syncVirtual(V5);
    printf("%s \n", "Waiting..");
    //timeThread();
    delay(1000);
}

int main(int argc, char* argv[]){
    resetSystemTime();
    //Blynk.virtualWrite(V7, 0);

    parse_options(argc, argv, auth, serv, port);
    
    pthread_attr_t tattr1;
    pthread_t thread_id1;
    int newprio = 99;
    sched_param param;

    pthread_attr_init (&tattr1);
    pthread_attr_getschedparam (&tattr1, &param); /* safe to get existing scheduling param */
    param.sched_priority = newprio; /* set the priority; others are unchanged */
    pthread_attr_setschedparam (&tattr1, &param); /* setting the new scheduling param */
    pthread_create(&thread_id1, &tattr1, dataThread, (void *)1); /* with new priority specified */

    if (setup_gpio() == 1){
        return 0;
    }
    while (true){
        loop();
    }
    //pthread_join(thread_id0, NULL);
    //pthread_exit(NULL);
    return 0;
}
