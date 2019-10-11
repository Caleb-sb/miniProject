/** MiniProject.cpp
  * Written by Caleb Bredekamp
  * and Sama Naraghi for EEE3096S
  * NRGSAM001, BRDCAL003
  */
#include "miniProject.h"

using namespace std;

long lastInterruptTime = 0;
bool stopped = true;
bool sound_alarm = false;

int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();

    //setting up the buttons
    pinMode(BTNS[0], INPUT);
    pinMode(BTNS[1], INPUT);
    pinMode(BTNS[2], INPUT);
    pinMode(BTNS[3], INPUT);

    pullUpDnControl(BTNS[0], PUD_UP);
    pullUpDnControl(BTNS[1], PUD_UP);
    pullUpDnControl(BTNS[2], PUD_UP);
    pullUpDnControl(BTNS[3], PUD_UP);

    wiringPiISR(BTNS[0], INT_EDGE_FALLING, &reset_isr);
    wiringPiISR(BTNS[1], INT_EDGE_FALLING, &freq_switch_isr);
    wiringPiISR(BTNS[2], INT_EDGE_FALLING, &start_stop_isr);  //no affect System timer
    wiringPiISR(BTNS[3], INT_EDGE_FALLING, &alarm_dismiss_isr);

    //Stetting up Alarm LED
    softPwmCreate(ALARM_LED, 0, 20);

    return 0;
}

void cleanup(int a){
    printf("Cleaning up ...\n")

    exit(0);
}


void start_stop_isr(void){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>INTERRUPT)
    {
        if(stopped)
        {
            stopped=false;
        }
        else{
            stopped = true;
        }
        lastInterruptTime = currentTime;
    }
}
void reset_isr(void){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>INTERRUPT)
    {
        //TODO
    }
}
void freq_switch_isr(void){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>INTERRUPT)
    {
        //TODO
    }
}

void alarm_dismiss_isr(void){
    long currentTime = millis();
    if (currentTime-lastInterruptTime>INTERRUPT)
    {
        sound_alarm = false;
    }
}

void alarm_sounding(){
    currentAlarmTime = millis();
    if (currentAlarmTime - lastAlarmTime < ALARM_WAIT)
    {
        return;
    }
    while (sound_alarm)
    {
        for(i=0;i<=20;i++) 
        {
			pwmWrite(ALARM_LED, i);
			delay(1);
		}
		delay(300);

		for(i=20;i>=0;i--) 
        {
			pwmWrite(ALARM_LED, i);
			delay(1);
		}
    }
}


int main()
{
    if(setup_gpio()==-1){
        return 0;
    }

}
