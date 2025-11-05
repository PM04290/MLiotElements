//
typedef enum {
  Manu = 0,
  ON_inner,
  ON_outer,
  HIGH_hysteresis,
  LOW_hysteresis
} regulMode_t;

class Regul : public Select
{
  public:
    Regul(uint8_t childID, int16_t EEPadr, Element* measure, Element* action, Input* setPointL, Input* setPointH, regulMode_t regulMode, const __FlashStringHelper* name)
      : Select(childID, EEPadr, name) {
      _measure = measure;
      _action = action;
      _setPointL = setPointL;
      _setPointH = setPointH;
      _regulMode = regulMode;
      _conditionElt = nullptr;
      _conditionEltValON = false;
      _conditionCallback = nullptr;
	  addOption(F("manu"));
	  addOption(F("auto"));
      // mandatory
      _elementType = rl_element_t::E_SELECT;
      _dataType = rl_data_t::D_TEXT;
	}
    void setCondition(Element* condition, bool valON)
    {
      _conditionElt = condition;
      _conditionEltValON = valON;
    }
    void setCondition(bool (*conditionCallback)())
    {
      _conditionCallback = conditionCallback;
    }
    void Process() override
    {
      if (_curValue == 0)
      { // mode MANU
        //if (_action->getValue()) {
        //  _action->setValue(0);
        //}
        return;
      }
      if (_curValue == 1)
      { // mode AUTO
        if (_regulMode == Manu) return;
        bool relayOut = _action->getValue() > 0;
        if (_regulMode == ON_inner) {
          if (_setPointL && _setPointH) {
            // relay ON if measure between L and H
            relayOut = _setPointL->getFloat() < _measure->getFloat() && _measure->getFloat() < _setPointH->getFloat();
          } else
          if (_setPointL) {
            // relay ON if measure upper L
            relayOut = _setPointL->getFloat() < _measure->getFloat();
          } else
          if (_setPointH) {
            // relay ON if measure under H
            relayOut = _measure->getFloat() < _setPointH->getFloat();
          }
        }
        if (_regulMode == ON_outer) {
          // relay ON if measure below L and above H
          relayOut = _measure->getFloat() < _setPointL->getFloat()  || _setPointH->getFloat() < _measure->getFloat();
        }
        if (_regulMode == HIGH_hysteresis) {
          // Set relay to ON when measure goes over H
          if (!relayOut && _setPointH->getFloat() < _measure->getFloat()) {
            relayOut = true;
          }
          // Set relay to OFF when measure go below L
          if (relayOut && _measure->getFloat() < _setPointL->getFloat()) {
            relayOut = false;
          }
        }
        if (_regulMode == LOW_hysteresis) {
          // Set relay to ON when measure goes below L
          if (!relayOut && _measure->getFloat() < _setPointL->getFloat()) {
            relayOut = true;
          }
          // Set relay to OFF when measure go over H
          if (relayOut && _setPointH->getFloat() < _measure->getFloat() ) {
            relayOut = false;
          }
        }
        if (_conditionElt && relayOut) {
          if (_conditionElt->getBool() != _conditionEltValON) {
            relayOut = false;
          }
        }
        if (_conditionCallback && relayOut) {
          if (!_conditionCallback()) {
            relayOut = false;
          }
        }
        _action->setValue(relayOut);
      }
    }
  private:
    regulMode_t _regulMode;
    Element* _setPointH;
    Element* _setPointL;
    Element* _measure;
    Element* _action;
    Element* _conditionElt;
    bool _conditionEltValON;
    bool (*_conditionCallback)();
};
