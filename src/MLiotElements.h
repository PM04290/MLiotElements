/*
 * Version 1.3
 * 
 */
#pragma once

#define EEP_NONE -1
#define PIN_NONE 0xFF

// ###########################################
// ## If Serial debug wanted                ##
// ##                                       ##
#ifdef DEBUG_SERIAL
#define DEBUGinit() DSerial.begin(115200)
#define DEBUG(x) DSerial.print(x)
#define DEBUGln(x) DSerial.println(x)
#define DEBUGf(...) DSerial.printf(__VA_ARGS__)
#else
#define DEBUGinit()
#define DEBUG(x)
#define DEBUGln(x)
#define DEBUGf(x,y)
#endif
// ##                                       ##
// ###########################################


// ###########################################
// ## If RTC DS3231 is installed            ##
// ##                                       ##
#include "MLdatetime.hpp"
DateTime rtc;
#ifdef HARDWARE_RTC
#define RTC_READ rtc.readDS3231()
#define RTC_WRITE rtc.writeDS3231()
#define RTC_READY rtc.isValidDS3231()
#else
#define RTC_READ
#define RTC_WRITE
#define RTC_READY 1
#endif
// ##                                       ##
// ###########################################

#include "tools/kalman.hpp"
#include "MLelement.hpp"
#include "MLbinary.hpp"
#include "MLpulse.hpp"
#include "MLanalog.hpp"
#include "MLbutton.hpp"
#include "MLrelay.hpp"
#include "MLinput.hpp"
#include "MLselect.hpp"
#include "MLregul.hpp"
#include "MLschedule.hpp"
#include "MLmotor.hpp"
#include "MLtext.hpp"

