/*
 * miniProject.cpp
 *
 * Written for EEE3096S 2019 by Caleb Bredekamp
 */

#ifndef MINIPROJECT_H
#define MINIPROJECT_H

//Includes
#include <wiringPi.h>
#include <vector> //lists was used for buffer?
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <iostream>

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
bool alarmed = false;

unsigned char ADCbuffer[3];
unsigned char DACbuffer[2];

char timeVal[20] = "";

int hours,mins,secs, sysHours, sysMins, sysSecs, secondsTimer;
int lastAlarm;

//RTC Constants
static const char RTCAddr = 0x6f;
static const char SEC = 0x00;
static const char MIN = 0x01;
static const char HOUR = 0x02;
static const char TIMEZONE = 2;

//SPI Settings
static const int SPI_CHAN0 = 0;
static const int SPI_CHAN1 = 1;
static const int SPI_SPEED = 256000;

int setup_gpio(void);
int main(int argc, char* argv[]);
void dataThread(void);
void toggleSampling(int param);
void updateSystemTime(void);
void resetSystemTime(void);
void triggerAlarm(void);
void resetAlarm(void);
void syncAlarmLED(void);
void timeThread(void);
void loop();



#endif
