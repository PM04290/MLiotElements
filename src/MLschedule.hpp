//
typedef enum {
  off = 0,
  periodic, // 0 to 6 skip day (0 = every days, 1 = every other day, 2 = one day out of three, etc)
} schedMode_t;

typedef enum {
  makePause = 0,
  abortRun,
} conditionAction_t;

class Schedule : public Select
{
  public:
    Schedule(uint8_t childID, int16_t EEPadr, Element* action, Element* confDay, Element* startHour, Element* duration, const __FlashStringHelper* name)
      : Select(childID, EEPadr, name)
    {
      _EltAction = action;
      _EltConfDay = confDay;
      _EltStartHour = startHour;
      _EltDuration = duration;
      _EltConditionBinary = nullptr;
      _EltConditionAnalog = nullptr;
      _EltConditionMeasure = nullptr;
      _conditionValue = false;
      _conditionMode = conditionAction_t::makePause;
      _durationRemain = 0;
      _oldDay = 99; // to force running today
      _dtStart = SECONDS_FROM_1970_TO_2000;
      _dtStop = SECONDS_FROM_1970_TO_2000;
	  addOption(F("off"));
	  addOption(F("periodic"));
      // mandatory
      _elementType = rl_element_t::E_SELECT;
      _dataType = rl_data_t::D_TEXT;
    }
    bool getBool() override {
      return isActive();
    }
    void setDuration(int16_t newDuration)
    {
      _durationRemain = max((int16_t)(24 * 60 - 5), newDuration); // 24h - 5min to protect covering
      DEBUG("Duration: "); DEBUGln(_durationRemain);
    }
    void setBinaryCondition(Binary* binary, bool valueRun)
    {
      _EltConditionBinary = binary;
      _conditionValue = valueRun;
    }
    void setInputCondition(Input* analog, Analog* measure, bool above)
    {
      _EltConditionAnalog = analog;
      _EltConditionMeasure = measure;
      _conditionValue = above;
    }
    void setConditionMode(conditionAction_t mode)
    {
      _conditionMode = mode;
    }
    void Process() override
    {
      if ((schedMode_t)_curValue == schedMode_t::off)
      {
        // offline
        if (isActive())
        {
          _EltAction->setBool(0);
          _dtStart = SECONDS_FROM_1970_TO_2000;
          _dtStop = SECONDS_FROM_1970_TO_2000;
        }
      } else {
        if (isActive())
        {
          bool RelayState = true;
          bool needAbort = rtc > _dtStop;
          if (_EltConditionBinary)
          {
            if (_EltConditionBinary->getBool() != _conditionValue)
            {
              RelayState = false;
              needAbort = _conditionMode == conditionAction_t::abortRun;
            }
          }
          if (_EltConditionAnalog)
          {
            if (_conditionValue)
            { // above
              if (_EltConditionMeasure->getFloat() < _EltConditionAnalog->getFloat())
              {
                RelayState = false;
                needAbort = _conditionMode == conditionAction_t::abortRun;
              }
            } else
            { // below
              if (_EltConditionMeasure->getFloat() > _EltConditionAnalog->getFloat())
              {
                RelayState = false;
                needAbort = _conditionMode == conditionAction_t::abortRun;
              }
            }
          }
          if (needAbort)
          {
            RelayState = false;
            _oldDay = rtc.dayOfTheWeek();
            _dtStart = SECONDS_FROM_1970_TO_2000;
            _dtStop = SECONDS_FROM_1970_TO_2000;
          }
          _EltAction->setBool(RelayState);
        } else {
          if (isDayToRun() && isTimeToRun())
          {
            _dtStart = DateTime(rtc.year(), rtc.month(), rtc.day(), _EltStartHour->getValue(), 0, 0);
            DEBUG("Start at "); DEBUGln(_dtStart.timestamp());
            uint16_t d = _durationRemain;
            if (_EltDuration)
            {
              d = _EltDuration->getValue();
            }
            _dtStop = _dtStart.unixtime() + ((uint32_t)d * 60);
            DEBUG("Stop at "); DEBUGln(_dtStop.timestamp());
          }
        }
      }
    }
    bool isActive()
    {
      return _dtStart != SECONDS_FROM_1970_TO_2000;
    }
  protected:
    bool isDayToRun()
    {
      if ((schedMode_t)_curValue == periodic) {
        uint8_t skipday = 0;
        if (_EltConfDay)
        {
          skipday = _EltConfDay->getValue();
        }
        return (_oldDay == 99) || (rtc.dayOfTheWeek() != ((_oldDay + skipday) % 7));
      }
      return false;
    }
    bool isTimeToRun()
    {
      DateTime timeToStart = DateTime(rtc.year(), rtc.month(), rtc.day(), _EltStartHour->getValue(), 0, 0);
      uint32_t d = _durationRemain;
      if (_EltDuration)
      {
        d = _EltDuration->getValue();
      }
      DateTime timeToStop = timeToStart.unixtime() +  (d * 60);
      return (d > 0) && (timeToStart <= rtc) && (rtc < timeToStop);
    }
  private:
    Element* _EltAction;
    Element* _EltConfDay;
    Element* _EltStartHour;
    Element* _EltDuration;
    Element* _EltConditionBinary;
    Element* _EltConditionAnalog;
    Element* _EltConditionMeasure;
    bool _conditionValue;
    conditionAction_t _conditionMode;
    int8_t _oldDay;
    DateTime _dtStart;
    DateTime _dtStop;
    int16_t _durationRemain; // in minutes
};
