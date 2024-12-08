/*
 * Module LoRa base elements
 * 
 */

#ifdef RL_CURRENT_VERSION
rl_packets _packet;
RadioLinkClass RLcomm;
uint8_t uid;
uint8_t hubid;
#endif

uint16_t mVCC = 5000; // 5.0 V

class Element
{
  public:
    Element(uint8_t id, int16_t EEPadr, float delta, const __FlashStringHelper* name, const __FlashStringHelper* unit, int16_t divider) {
      _id = id;
      _eepadr = EEPadr;
      _elementType = rl_element_t::E_CUSTOM;
      _dataType = rl_data_t::D_NUM;
      _delta = delta * divider;
      _divider = divider;
      _name = name;
      _unit = unit;
      if (_eepadr != EEP_NONE) {
        // default value is BYTE
#ifdef EEPROM_h
        _curValue = EEPROM.read(_eepadr);
#else
        _curValue = -999999;
#endif
      } else {
        _curValue = -999999;
      }
      _sentValue = -_curValue; // to force send at start
    }
    uint8_t getID() {
      return _id;
    }
    virtual int32_t getValue() {
      return _curValue;
    }
    virtual bool getBool() {
      return _curValue != 0;
    }
    virtual float getFloat() {
      return (float)_curValue / _divider;
    }
    virtual const char* getText() {
      // must be overrided
      return "?";
    }
    virtual void setValue(int32_t newValue) {
      _curValue = newValue;
    }
    virtual void setBool(uint8_t newValue) {
      setValue((int32_t)newValue);
    }
    virtual void setFloat(float newValue) {
      setValue((int32_t)(newValue * _divider));
    }
    virtual void setText(const char* newValue) {
      // must be overrided
    }
    virtual void Process() {
      // must be overrided
      DEBUGln(F("process not overrided"));
    }
#ifdef RL_CURRENT_VERSION
    virtual void PublishConfigDedicated() {
    }
    void publishConfig() {
      if (_id == 0) return;
      rl_configBase_t cnfb;
      memset(&cnfb, 0, sizeof(cnfb));
      cnfb.childID = _id;
      cnfb.deviceType = (uint8_t)_elementType;
      cnfb.dataType = (uint8_t)_dataType;
      // config Name
      uint8_t len = strlen_P(reinterpret_cast<const char*>(_name));
      strncpy_P(cnfb.name, reinterpret_cast<const char*>(_name), min(len, sizeof(cnfb.name)));
      RLcomm.publishConfig(hubid, uid, (rl_configs_t*)&cnfb, C_BASE);
      // config Unit
      if (_unit && strlen_P(reinterpret_cast<const char*>(_unit))) {
        rl_configText_t cnft;
        cnft.childID = _id;
        len = strlen_P(reinterpret_cast<const char*>(_unit));
        strncpy_P(cnft.text, reinterpret_cast<const char*>(_unit), min(len, sizeof(cnft.text)));
        if (len < sizeof(cnft.text)) {
          cnft.text[len] = 0;
        }
        RLcomm.publishConfig(hubid, uid, (rl_configs_t*)&cnft, C_UNIT);
      }
	  //
	  PublishConfigDedicated();
      //
      memset(&cnfb, 0, sizeof(cnfb));
      cnfb.childID = _id;
      cnfb.deviceType = (uint8_t)_elementType;
      cnfb.dataType = (uint8_t)_dataType;
      RLcomm.publishConfig(hubid, uid, (rl_configs_t*)&cnfb, C_END);
    }
    uint32_t Send() {
	  uint8_t l;
	  const char* text;
      if ((abs(_sentValue - _curValue) > _delta) && _id)
      {
        _sentValue = _curValue;
        switch ((int)_elementType) {
          case rl_element_t::E_BINARYSENSOR:
          case rl_element_t::E_NUMERICSENSOR:
          case rl_element_t::E_SWITCH:
          case rl_element_t::E_INPUTNUMBER:
          case rl_element_t::E_TRIGGER:
            DEBUG(_name); DEBUG(" > "); DEBUGln(_curValue);
            _packet.current.data.num.value = _curValue;
            _packet.current.data.num.divider = _divider;
            publishPacket();
            break;
          case rl_element_t::E_COVER:
          case rl_element_t::E_SELECT:
          case rl_element_t::E_TEXTSENSOR:
		    text = getText();
            DEBUG(_name); DEBUG(" > "); DEBUGln(text);
            l = strlen(text);
            if (l > sizeof(_packet.current.data.text)) l = sizeof(_packet.current.data.text);
            memset(_packet.current.data.text, 0, sizeof(_packet.current.data.text));
            strncpy(_packet.current.data.text, text, l);
            publishPacket();
            break;
        }
        return _curValue;
      }
      return false;
    }
#endif
  protected:
#ifdef RL_CURRENT_VERSION
    void publishPacket() {
      _packet.current.destinationID = hubid;
      _packet.current.senderID = uid;
      _packet.current.childID = _id;
      _packet.current.sensordataType = (_elementType << 3) + _dataType;
      RLcomm.publishPaquet(&_packet);
    }
#endif
    uint8_t _id;
    int16_t _eepadr;
    rl_element_t _elementType;
    rl_data_t _dataType;
    int32_t _curValue;
    int32_t _sentValue;
    int16_t _divider;
    int16_t _delta;
    const __FlashStringHelper* _name;
    const __FlashStringHelper* _unit;
};

Element** _elementList;
uint8_t _elementCount = 0;

Element* ML_addElement(Element* newElement) {
  _elementList = (Element**)realloc(_elementList, (_elementCount + 1) * sizeof(Element*));
  if (_elementList == nullptr) {
    return nullptr;
  }
  _elementList[_elementCount] = newElement;
  _elementCount++;
  return _elementList[_elementCount - 1];
}
void ML_ProcessElements() {
  for (uint8_t i = 0; i < _elementCount; i++)
  {
    ((Element*)_elementList[i])->Process();
  }
}
Element* ML_getElementByID(uint8_t id)
{
  for (uint8_t i = 0; i < _elementCount; i++)
  {
    if (((Element*)_elementList[i])->getID() == id)
      return ((Element*)_elementList[i]);
  }
  return nullptr;
}
#ifdef RL_CURRENT_VERSION
void ML_SendElements() {
  for (uint8_t i = 0; i < _elementCount; i++)
  {
    ((Element*)_elementList[i])->Send();
  }
}
void ML_PublishConfigElements(const __FlashStringHelper* deviceName, const __FlashStringHelper* deviceModel) {
  rl_configs_t cnf;
  uint8_t len;
  memset(&cnf, 0, sizeof(cnf));
  cnf.base.childID = RL_ID_CONFIG; // for device config
  cnf.base.deviceType = E_CUSTOM;
  cnf.base.dataType = D_TEXT;
  // config text contain Name
  len = strlen_P(reinterpret_cast<const char*>(deviceName));
  strncpy_P(cnf.base.name, reinterpret_cast<const char*>(deviceName), min(len, sizeof(cnf.base.name)));
  RLcomm.publishConfig(hubid, uid, &cnf, C_BASE);
  // config text contain Model
  memset(&cnf.text.text, 0, sizeof(cnf.text.text));
  len = strlen_P(reinterpret_cast<const char*>(deviceModel));//strlen(deviceModel);
  strncpy_P(cnf.text.text, reinterpret_cast<const char*>(deviceModel), min(len, sizeof(cnf.text.text)));
  RLcomm.publishConfig(hubid, uid, &cnf, C_OPTS);
  // end of conf
  cnf.text.text[0] = 0;
  RLcomm.publishConfig(hubid, uid, &cnf, C_END);
  //
  for (uint8_t i = 0; i < _elementCount; i++)
  {
    ((Element*)_elementList[i])->publishConfig();
  }
}
#endif
