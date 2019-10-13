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

//Define buttons
static const int BTNS[17, 27, 22, 23];

//Interrupt Times
static const int INTERRUPT = 200;
static const int ALARM_WAIT = 180000;

//ALARM LED
static const int ALARM_LED = 19;

//RTC Constants
static const char RTCAddr = 0x6f;
static const char SEC = 0x00; 
static const char MIN = 0x01;
static const char HOUR = 0x02;
static const char TIMEZONE = 2;

//SPI Settings
static const int SPI_CHAN = 0;        
static const int SPI_SPEED = 256000;  


#endif
