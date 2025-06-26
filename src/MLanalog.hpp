//
class Analog : public Element
{
  public:
    Analog(uint8_t pin, uint8_t childID, const __FlashStringHelper* name, const __FlashStringHelper* unit, float delta, int16_t divider)
      : Element(childID, EEP_NONE, delta, name, unit, divider) {
      _pin = pin;
      _callbackProcess = nullptr;
      _callbackText = nullptr;
      _p1 = _p2 = _p3 = 0;
      // mandatory
      if (_callbackText) {
        _elementType = rl_element_t::E_TEXTSENSOR;
        _dataType = rl_data_t::D_TEXT;
      } else {
        _elementType = rl_element_t::E_NUMERICSENSOR;
        if (_divider == 1) {
          _dataType = rl_data_t::D_NUM;
        } else {
          _dataType = rl_data_t::D_FLOAT;
        }
      }
	  if (_pin != PIN_NONE) {
        pinMode(_pin, INPUT);
        _Kfilter = new KalmanFilter(analogRead(_pin));
	  }
    }
    const char* getText() override {
      if (_callbackText) {
        return ( _callbackText(_curValue) );
      }
      return Element::getText();
    }
    void Process() override {
      uint32_t ADCraw = 0;
	  if (_pin != PIN_NONE)
      {
        for (uint8_t i = 0; i < 10; i++)
        {
          ADCraw += analogRead(_pin);
        }
        ADCraw = _Kfilter->update(ADCraw / 10);
	  }
	  if (_callbackProcess) {
        setFloat(_callbackProcess(ADCraw, _p1, _p2, _p3));
      } else {
        uint32_t mvADC = (ADCraw * (int32_t)mVCC) / 1023L;
        setValue(mvADC);
      }
    }
    void setParams(float (*callback)(uint16_t, uint16_t, uint16_t, uint16_t), uint16_t p1, uint16_t p2, uint16_t p3) {
      _callbackProcess = callback;
      _p1 = p1;
      _p2 = p2;
      _p3 = p3;
    }
    void setTextConvert(const char* (*callbackText)(int16_t)) {
      _callbackText = callbackText;
    }
  private:
    uint8_t _pin;
    uint16_t _p1;
    uint16_t _p2;
    uint16_t _p3;
    float (*_callbackProcess)(uint16_t, uint16_t, uint16_t, uint16_t);
    const char* (*_callbackText)(int16_t);
    KalmanFilter* _Kfilter;
};
