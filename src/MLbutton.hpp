//
class Button : public Element
{
	public:
	Button(uint8_t childID, const __FlashStringHelper* name)
	: Element(childID, EEP_NONE, 0, name, nullptr, 1) {
		// mandatory
		_elementType = rl_element_t::E_BUTTON;
		_dataType = rl_data_t::D_TEXT;
	}
	const char* getText() override {
		return nullptr;
	}
	void setText(const char* newValue) override {
	}
	void Process() override
	{
		// no process for option
	}
	private:
};
