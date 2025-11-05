//
typedef struct __attribute__((packed)) {
  uint8_t protectMode; // 0 = time, 1 = switch, 2 = current, 3 : encoder
  uint8_t limitTime;   // secure limit time for all modes
  uint16_t limitFW;    // Foward : mode 0 in second, mode 1 none, mode 2 in mA, mode 3 position
  uint16_t limitBW;    // Backward : mode 0 in second, mode 1 none, mode 2 in mA, mode 3 position
} motorRecord;

const char* MotStateStop = "stop";
const char* MotStateOpen = "open";
const char* MotStateOpening = "opening";
const char* MotStateClosed = "closed";
const char* MotStateClosing = "closing";

const char* MotCommandOpen = "OPEN";
const char* MotCommandClose = "CLOSE";
const char* MotCommandStop = "STOP";

typedef enum {
  SMstop = 0,
  SMopen = 2,
  SMopening = 3,
  SMclosed = 4,
  SMclosing = 5,
} stateMotor_t;

class Motor : public Element
{
  public:
    Motor(uint8_t pinFW, uint8_t pinBW, uint8_t childID, int16_t EEPadr, const __FlashStringHelper* name)
      : Element(childID, EEP_NONE, 0, name, nullptr, 1) {
      _pinFW = pinFW;
      _pinBW = pinBW;
      pinMode(_pinFW, OUTPUT);
      pinMode(_pinBW, OUTPUT);
#ifdef EEPROM_h
      motorRecord mr;
      EEPROM.get(EEPadr, mr);
      _protectMode = mr.protectMode;
	  _limitTime = mr.limitTime;
	  _limitFW = mr.limitFW;
	  _limitBW = mr.limitBW;
#else
      _protectMode = 0;  // time mode
	  _limitTime = 10;   // in second
	  _limitFW = 0;      // TBD in main code)
	  _limitBW = 0;      // TBD in main code)
#endif
	  _curValue = stateMotor_t::SMstop; // stop

	  _switchFW = nullptr;
	  _switchBW = nullptr;
	  _callbackSensor = nullptr;

	  _timeoutCommand = 0;
      // mandatory
      _elementType = rl_element_t::E_COVER;
      _dataType = rl_data_t::D_TEXT;
	}
    const char* getText() override {
      switch ((stateMotor_t)_curValue) {
        case stateMotor_t::SMstop:
          return MotStateStop;
        case stateMotor_t::SMopen:
          return MotStateOpen;
        case stateMotor_t::SMopening:
          return MotStateOpening;
        case stateMotor_t::SMclosed:
          return MotStateClosed;
        case stateMotor_t::SMclosing:
          return MotStateClosing;
      }
      return "?";
    }
    void setText(const char* newValue) override {
      if (strcmp(newValue, MotCommandOpen) == 0) {
        setValue((int32_t)stateMotor_t::SMopening);
      }
      if (strcmp(newValue, MotCommandClose) == 0) {
        setValue((int32_t)stateMotor_t::SMclosing);
      }
      if (strcmp(newValue, MotCommandStop) == 0) {
        setValue((int32_t)stateMotor_t::SMstop);
      }
    }
    void setValue(int32_t newValue) override {
      if (newValue == (int32_t)stateMotor_t::SMopening && _curValue != (int32_t)stateMotor_t::SMopen) {
		  if (_switchFW && !_switchFW->getBool()) {
			digitalWrite(_pinBW, 0);
			digitalWrite(_pinFW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _limitTime*1000) | 1;
		  }
		  if (_switchFW == nullptr) {
			digitalWrite(_pinBW, 0);
			digitalWrite(_pinFW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _limitTime*1000) | 1;
		  }
	  }
      if (newValue == (int32_t)stateMotor_t::SMclosing && _curValue != (int32_t)stateMotor_t::SMclosed) {
		  if (_switchBW && !_switchBW->getBool()) {
			digitalWrite(_pinFW, 0);
			digitalWrite(_pinBW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _limitTime*1000) | 1;
		  }
		  if (_switchBW == nullptr) {
			digitalWrite(_pinFW, 0);
			digitalWrite(_pinBW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _limitTime*1000) | 1;
		  }
	  }
	  if (newValue == (int32_t)stateMotor_t::SMstop) {
        digitalWrite(_pinFW, 0);
        digitalWrite(_pinBW, 0);
		_curValue = newValue;
	    _timeoutCommand = 0;
	  }
    }
	void setProtectTime(uint8_t limitTime)
	{
      _protectMode = 0;
      _limitTime = limitTime;
#ifdef EEPROM_h
      motorRecord mr = {_protectMode, _limitTime, _limitFW, _limitBW};
      EEPROM.put(_eepadr, mr);
#ifdef ESP32
	  EEPROM.commit();
#endif
#endif
	}
    void setProtectSwitch(Binary* switchFW, Binary* switchBW, uint8_t limitTime = 10)
    {
      _protectMode = 1;
	  _switchFW = switchFW;
	  _switchBW = switchBW;
      _limitTime = limitTime;
	  _limitFW = 0;
	  _limitBW = 0;
#ifdef EEPROM_h
      motorRecord mr = {_protectMode, _limitTime, _limitFW, _limitBW};
      EEPROM.put(_eepadr, mr);
#ifdef ESP32
	  EEPROM.commit();
#endif
#endif
    }
	void setProtectSensor(float (*callbackSensor)(void*))
	{
	  _callbackSensor = callbackSensor;
	}
	void setProtectCurrent(uint8_t limitTime, uint16_t limitFW, uint16_t limitBW)
	{
      _protectMode = 2;
      _limitTime = limitTime;
	  _limitFW = limitFW;
	  _limitBW = limitBW;
#ifdef EEPROM_h
      motorRecord mr = {_protectMode, _limitTime, _limitFW, _limitBW};
      EEPROM.put(_eepadr, mr);
#ifdef ESP32
	  EEPROM.commit();
#endif
#endif
	}
	void setProtectCurrentTime(uint8_t limitTime)
	{
      if (_protectMode == 2) {
		  _limitTime = limitTime;
#ifdef EEPROM_h
		motorRecord mr = {_protectMode, _limitTime, _limitFW, _limitBW};
		EEPROM.put(_eepadr, mr);
#ifdef ESP32
		EEPROM.commit();
#endif
#endif
	  }
	}
	void setProtectEncoder()
	{
		// TODO
	}
	uint16_t getLimit() {
		return _limitFW; // TODO BW ?
	}
	void actionLimit()
	{
		digitalWrite(_pinFW, 0);
		digitalWrite(_pinBW, 0);
		if (_curValue == (int32_t)stateMotor_t::SMclosing) {
			_curValue = (int32_t)stateMotor_t::SMclosed;
		} else
		if (_curValue == (int32_t)stateMotor_t::SMopening) {
			_curValue = (int32_t)stateMotor_t::SMopen;
		} else
		{
			_curValue = (int32_t)stateMotor_t::SMstop;
		}
		_timeoutCommand = 0;
	}
    void Process() override {
		switch (_protectMode) {
			case 0 : // time
				// performed all the time, below
				break;
			case 1 : // switch
				// detect contact limit
				if (_switchFW && _switchFW->getBool() && digitalRead(_pinFW) ) {
					setValue((int32_t)stateMotor_t::SMopen);
				}
				if (_switchBW && _switchBW->getBool() && digitalRead(_pinBW) ) {
					setValue((int32_t)stateMotor_t::SMclosed);
				}
				break;
			case 2 : // current
				if (_callbackSensor) {
					if (_callbackSensor(this) >= _limitFW && digitalRead(_pinFW)) {
						setValue((int32_t)stateMotor_t::SMopen);
					}
					if (_callbackSensor(this) >= _limitBW && digitalRead(_pinBW)) {
						setValue((int32_t)stateMotor_t::SMclosed);
					}
				}
				break;
			case 3 : // encoder
				if (_callbackSensor) {
					if (_callbackSensor(this) >= _limitFW && digitalRead(_pinFW)) {
						setValue((int32_t)stateMotor_t::SMopen);
					}
					if (_callbackSensor(this) <= _limitBW && digitalRead(_pinBW)) {
						setValue((int32_t)stateMotor_t::SMclosed);
					}
				}
				break;
		}
		// protect time limit
		if (digitalRead(_pinFW) && millis() > _timeoutCommand) {
            DEBUGln("FW time limit");
			digitalWrite(_pinFW, 0);
			_curValue = (int32_t)stateMotor_t::SMopen;
			_timeoutCommand = 0;
		}
		if (digitalRead(_pinBW) && millis() > _timeoutCommand) {
            DEBUGln("BW time limit");
			digitalWrite(_pinBW, 0);
			_curValue = (int32_t)stateMotor_t::SMclosed;
			_timeoutCommand = 0;
		}
    }
  private:
    uint8_t _pinFW; // Forward
    uint8_t _pinBW; // Backward
	uint8_t _protectMode;
	uint8_t _limitTime;
	uint16_t _limitFW;
	uint16_t _limitBW;
	Binary* _switchFW;
	Binary* _switchBW;
	uint32_t _timeoutCommand;
	float (*_callbackSensor)(void*);  // callback
};
