/*
 * Version 1.0
 * 
 */
#define EEP_NONE -1
#define PIN_NONE 0xFF

// ###########################################
// ## If Serial debug wanted                ##
// ##                                       ##
#ifdef DEBUG_SERIAL
#define DEBUGinit() Serial.begin(115200)
#define DEBUG(x) Serial.print(x)
#define DEBUGln(x) Serial.println(x)
#else
#define DEBUGinit()
#define DEBUG(x)
#define DEBUGln(x)
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

#include <MLelement.hpp>
#include "tools/kalman.hpp"
#include "MLbinary.hpp"
#include "MLanalog.hpp"
#include "MLbutton.hpp"
#include "MLrelay.hpp"
#include "MLinput.hpp"
#include "MLselect.hpp"
#include "MLregul.hpp"
#include "MLschedule.hpp"
#include "MLmotor.hpp"

