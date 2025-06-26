//

const char* MotStateStop = "stop";
const char* MotStateOpen = "open";
const char* MotStateOpening = "opening";
const char* MotStateClosed = "closed";
const char* MotStateClosing = "closing";

const char* MotCommandOpen = "OPEN";
const char* MotCommandClose = "CLOSE";
const char* MotCommandStop = "STOP";

class Motor : public Element
{
  public:
    Motor(uint8_t pinFW, uint8_t pinBW, uint8_t childID, const __FlashStringHelper* name)
      : Element(childID, EEP_NONE, 0, name, nullptr, 1) {
      _pinFW = pinFW;
      _pinBW = pinBW;
      pinMode(_pinFW, OUTPUT);
      pinMode(_pinBW, OUTPUT);
	  _limitFW = nullptr;
	  _limitBW = nullptr;
	  _timeFW = _timeBW = 10; // 10s
	  _timeoutCommand = 0;
	  _curValue = stateMotor_t::stop; // stop
      // mandatory
      _elementType = rl_element_t::E_COVER;
      _dataType = rl_data_t::D_TEXT;
	}
	typedef enum {
	  stop = 0,
	  open = 2,
	  opening = 3,
	  closed = 4,
	  closing = 5,
	} stateMotor_t;
    const char* getText() override {
      switch ((stateMotor_t)_curValue) {
        case stateMotor_t::stop:
          return MotStateStop;
        case stateMotor_t::open:
          return MotStateOpen;
        case stateMotor_t::opening:
          return MotStateOpening;
        case stateMotor_t::closed:
          return MotStateClosed;
        case stateMotor_t::closing:
          return MotStateClosing;
      }
      return "?";
    }
    void setText(const char* newValue) override {
      if (strcmp(newValue, MotCommandOpen) == 0) {
        setValue((int32_t)stateMotor_t::opening);
      }
      if (strcmp(newValue, MotCommandClose) == 0) {
        setValue((int32_t)stateMotor_t::closing);
      }
      if (strcmp(newValue, MotCommandStop) == 0) {
        setValue((int32_t)stateMotor_t::stop);
      }
    }
    void setValue(int32_t newValue) override {
      if (newValue == (int32_t)stateMotor_t::opening && _curValue != (int32_t)stateMotor_t::open) {
		  if (_limitFW && !_limitFW->getBool()) {
			digitalWrite(_pinBW, 0);
			digitalWrite(_pinFW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _timeFW*1000) | 1;
		  }
		  if (_limitFW == nullptr) {
			digitalWrite(_pinBW, 0);
			digitalWrite(_pinFW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _timeFW*1000) | 1;
		  }
	  }
      if (newValue == (int32_t)stateMotor_t::closing && _curValue != (int32_t)stateMotor_t::closed) {
		  if (_limitBW && !_limitBW->getBool()) {
			digitalWrite(_pinFW, 0);
			digitalWrite(_pinBW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _timeBW*1000) | 1;
		  }
		  if (_limitBW == nullptr) {
			digitalWrite(_pinFW, 0);
			digitalWrite(_pinBW, 1);
			_curValue = newValue;
			_timeoutCommand = (millis() + _timeBW*1000) | 1;
		  }
	  }
	  if (newValue == (int32_t)stateMotor_t::stop) {
        digitalWrite(_pinFW, 0);
        digitalWrite(_pinBW, 0);
		_curValue = newValue;
	    _timeoutCommand = 0;
	  }
    }
    void setLimits(Binary* limitFW, Binary* limitBW)
    {
	  _limitFW = limitFW;
	  _limitBW = limitBW;
    }
    void setTimer(uint16_t seconds)
    {
	  _timeFW = _timeBW = seconds;
    }
	void actionLimit()
	{
		digitalWrite(_pinFW, 0);
		digitalWrite(_pinBW, 0);
		if (_curValue == (int32_t)stateMotor_t::closing) {
			_curValue = (int32_t)stateMotor_t::closed;
		} else
		if (_curValue == (int32_t)stateMotor_t::opening) {
			_curValue = (int32_t)stateMotor_t::open;
		} else
		{
			_curValue = (int32_t)stateMotor_t::stop;
		}
		_timeoutCommand = 0;
	}
    void Process() override {
      // detect contact limit
      if (_limitFW && _limitFW->getBool() && digitalRead(_pinFW) ) {
        digitalWrite(_pinFW, 0);
		_curValue = (int32_t)stateMotor_t::open;
		_timeoutCommand = 0;
      }
      if (_limitBW && _limitBW->getBool() && digitalRead(_pinBW) ) {
        digitalWrite(_pinBW, 0);
		_curValue = (int32_t)stateMotor_t::closed;
		_timeoutCommand = 0;
      }
      // protect time limit
	  if (digitalRead(_pinFW) && millis() > _timeoutCommand) {
        digitalWrite(_pinFW, 0);
		_curValue = (int32_t)stateMotor_t::open;
		_timeoutCommand = 0;
      }
	  if (digitalRead(_pinBW) && millis() > _timeoutCommand) {
        digitalWrite(_pinBW, 0);
		_curValue = (int32_t)stateMotor_t::closed;
		_timeoutCommand = 0;
      }
    }
  private:
    uint8_t _pinFW; // Forward
    uint8_t _pinBW; // Backward
	Binary* _limitFW;
	Binary* _limitBW;
	uint16_t _timeFW;
	uint16_t _timeBW;
	uint32_t _timeoutCommand;
};
