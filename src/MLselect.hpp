//
class Select : public Element
{
	public:
	Select(uint8_t childID, int16_t EEPadr, const __FlashStringHelper* name)
	: Element(childID, EEPadr, 0, name, nullptr, 1) {
		// mandatory
		_elementType = rl_element_t::E_SELECT;
		_dataType = rl_data_t::D_TEXT;
		_optcount = 0;
		_optelts = nullptr;
	}
	const char* getText() override {
		if (_curValue < _optcount) {
			return _optelts[_curValue];
		}
		return Element::getText();
	}
	void setText(const char* newValue) override {
		for (uint8_t i = 0; i < _optcount; i++) {
			if (strcmp(newValue, _optelts[i]) == 0) {
				setValue(i);
				return;
			}
		}
		#ifdef EEPROM_h
		EEPROM.write(_eepadr, (uint8_t)_curValue);
		#endif
	}
	void addOption(const __FlashStringHelper* option) {
		_optelts = (char**)realloc(_optelts, (_optcount + 1) * sizeof(char*));
		if (_optelts == nullptr) {
			return;
		}
		_optelts[_optcount] = new char[strlen_P(reinterpret_cast<const char*>(option))+1];
		strcpy_P(_optelts[_optcount], reinterpret_cast<const char*>(option));
		_optcount++;
	}
	void Process() override
	{
		// no process for option
	}
    void PublishConfigDedicated() override {
      if (_elementType == E_SELECT && _optelts) {
        rl_configText_t cnft;
		unsigned int len;
		for (uint8_t i = 0; i < _optcount; i++) {
			memset(&cnft, 0, sizeof(cnft));
			cnft.childID = _id;
			cnft.index = i;
			len = strlen(_optelts[i]);
			strncpy(cnft.text, _optelts[i], min(len, sizeof(cnft.text)));
			MLiotComm.publishConfig(hubid, uid, (rl_configs_t*)&cnft, C_OPTS);
		}
      }
	}
	private:
	uint8_t _optcount;
	char** _optelts;
};
