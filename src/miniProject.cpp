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

WidgetLED alarmLED(V8);

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
    long currentTime = millis();
    if (currentTime-lastInterruptTime>500)
    {
        if(param[0] == 0)
        {
            running = false;
        }
        else{
            running = true;
        }
        lastInterruptTime = currentTime;
    }
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

BLYNK_WRITE(V9){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>200)
    {
        resetAlarm();
        lastInterruptTime = currentTime;
    }
}

void triggerAlarm(void){
    if (lastAlarm == 0 || (secondsTimer-lastAlarm >= 180)){
        alarmed = true;
    }
}

void resetAlarm(void){
    if (alarmed){
        alarmed = false;
        lastAlarm = secondsTimer;
        printf("%s \n", "Alarm cleared");
    }
}

void syncAlarmLED(void){
    if (alarmed){
        alarmLED.on();
    } else {
        alarmLED.off();
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

void timeThread(void){
        hours = getHours();
        mins = getMins();
        secs = getSecs();
        secondsTimer = secondsTimer + 1;
        updateSystemTime();

        sprintf(timeVal, "%d:%d:%d / %d:%d:%d", hours, mins, secs, sysHours, sysMins, sysSecs);

        Blynk.virtualWrite(V1, timeVal);
}


void dataThread(void){
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

                if(Vout < 0.65 || Vout > 2.65){
                    triggerAlarm();
                }

                Blynk.virtualWrite(V0, Vout);
                Blynk.virtualWrite(V2, Vhum);
                Blynk.virtualWrite(V3, lightVal);
                Blynk.virtualWrite(V4, Temp);

                printf("humidity: %.2f light: %d  Vout: %.2f Vdac: %.2f Temp: %.1f \n", Vhum, lightVal, Vout, Vdac, Temp);
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
}

void updateSystemTime(void){
    sysMins = secondsTimer / 60;
    sysSecs = secondsTimer % 60;
    sysHours = sysMins / 60;
    sysMins = sysMins % 60;
}

void resetSystemTime(void){
    secondsTimer = 0;
    lastAlarm = 0;
    alarmed = false;
}

void loop()
{
    Blynk.run();
    tmr.run();
}

int main(int argc, char* argv[]){
    resetSystemTime();

    parse_options(argc, argv, auth, serv, port);

    if (setup_gpio() == 1){
        return 0;
    }
    counter = 1;
    tmr.setInterval(1000, [](){
        Blynk.syncVirtual(V7);
        Blynk.syncVirtual(V5);
        syncAlarmLED();
        printf("%s \n", timeVal);
    });
    tmr.setInterval(1000, [](){
        dataThread();
    });
    while (true){
        loop();
    }
    return 0;
}
