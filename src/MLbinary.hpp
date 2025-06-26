//
typedef enum {
  stateNormal = 0,
  stateInverted = 1,
  trigRise = 2,
  trigFall = 3,
  trigAny = 4,
} sendMode_t;

class Binary : public Element
{
  public:
    Binary(uint8_t pin, uint8_t childID, const __FlashStringHelper* name, sendMode_t sendmode, uint8_t (*callback)())
      : Element(childID, EEP_NONE, 0, name, nullptr, 1) {
      _pin = pin;
      _sendmode = sendmode;
      _callbackProcess = callback;

      pinMode(_pin, INPUT_PULLUP);
	  
      // mandatory
	  if (_sendmode == stateNormal || _sendmode == stateInverted)
      {
        _elementType = rl_element_t::E_BINARYSENSOR;
        _dataType = rl_data_t::D_BOOL;
        setValue(_sendmode == stateInverted ? !digitalRead(_pin) : digitalRead(_pin));
	  } else {
        _elementType = rl_element_t::E_TRIGGER;
        _dataType = rl_data_t::D_NUM;
        _sentValue = _curValue;
      }
	}
    void Process() override {
      if (_callbackProcess)
      {
	    _curValue = _callbackProcess();
      } else
      {
        _curValue = (_sendmode == stateInverted ? !digitalRead(_pin) : digitalRead(_pin));
      }
	  if (_sendmode == trigRise && _sentValue != _curValue && _curValue == false)
      { // send Rising edge only, mask send
        _sentValue = _curValue;
      }
	  if (_sendmode == trigFall && _sentValue != _curValue && _curValue == true)
      { // send Falling edge only, mask send
        _sentValue = _curValue;
      }
    }
  private:
    uint8_t _pin;
    sendMode_t _sendmode;
    uint8_t (*_callbackProcess)();
};
