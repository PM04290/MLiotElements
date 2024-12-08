//
typedef struct {
  int16_t minValue;
  int16_t maxValue;
  int16_t selValue;
  int16_t divider;
} inputRecord;

class Input : public Element
{
  public:
    Input(uint8_t childID, int16_t EEPadr, const __FlashStringHelper* name, const __FlashStringHelper* unit)
      : Element(childID, EEPadr, 0, name, unit, 1) {
#ifdef EEPROM_h
      inputRecord ir;
      EEPROM.get(EEPadr, ir);
      _curValue = ir.selValue;
      _mini = ir.minValue;
      _maxi = ir.maxValue;
      _divider = ir.divider;
#else
      _curValue = 255;
      _mini = 0;
      _maxi = 255;
      _divider = 1;
#endif
      // mandatory
      _elementType = rl_element_t::E_INPUTNUMBER;
      if (_divider == 1) {
        _dataType = rl_data_t::D_NUM;
      } else {
        _dataType = rl_data_t::D_FLOAT;
      }
    }
    virtual void setFloat(float newValue) {
      setValue((int32_t)(newValue * _divider));
#ifdef EEPROM_h
      inputRecord ir = {_mini, _maxi, (int16_t)_curValue, _divider};
      EEPROM.put(_eepadr, ir);
#endif
    }
    void Process() override {
      // no process for input
    }
    void PublishConfigDedicated() override {
      if (_elementType == E_INPUTNUMBER) {
        rl_configNums_t cnfn;
        memset(&cnfn, 0, sizeof(cnfn));
        cnfn.childID = _id;
        cnfn.divider = _divider;
        cnfn.mini = _mini;
        cnfn.maxi = _maxi;
        cnfn.step = 1;
        RLcomm.publishConfig(hubid, uid, (rl_configs_t*)&cnfn, C_NUMS);
	  }
	}

  private:
    int16_t _mini;
    int16_t _maxi;

};
