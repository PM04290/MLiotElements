//
class Relay : public Element
{
  public:
    Relay(uint8_t pin, uint8_t childID, const __FlashStringHelper* name)
      : Element(childID, EEP_NONE, 0, name, nullptr, 1) {
      _pin = pin;
      pinMode(_pin, OUTPUT);
      _curValue = digitalRead(_pin);
      // mandatory
      _elementType = rl_element_t::E_SWITCH;
      _dataType = rl_data_t::D_BOOL;
	  }
    int32_t getValue() override {
      return digitalRead(_pin);
    }
    bool getBool() override {
      return digitalRead(_pin);
    }
    void setValue(int32_t newValue) override {
      if (newValue != _curValue) {
        digitalWrite(_pin, newValue > 0);
        _curValue = newValue;
        DEBUG(_name); DEBUG(" = "); DEBUGln(newValue > 0 ? "ON" : "OFF");
      }
    }
    void setBool(uint8_t newValue) override {
      setValue((uint32_t)newValue);
    }
    void Process() override {
      // nothing to do for relay
    }
  private:
    uint8_t _pin;
};
