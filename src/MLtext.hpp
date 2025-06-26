//
class Text : public Element
{
	public:
	Text(uint8_t childID, const __FlashStringHelper* name)
	: Element(childID, EEP_NONE, 0, name, nullptr, 1) {
		// mandatory
		_elementType = rl_element_t::E_TEXTSENSOR;
		_dataType = rl_data_t::D_TEXT;
		_text = "";
	}
	const char* getText() override {
		return _text.c_str();
	}
	void setText(const char* newValue) override {
		if (_text != String(newValue)) {
			_curValue = -_sentValue; // to force Send
			_text = String(newValue);
		}
	}
	void Process() override
	{
		// no process for option
	}
	private:
      String _text;
};
