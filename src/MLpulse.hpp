/* if save counter in EEPROM
   #########################
   to prevent max EEPROM cycle writing (100K), each 50K writing
   change address to write in circular buffer (63 ulong)

  EEPROM mapping

           EEPROM_SIZE = 256 for ATtiny3216
  +------------------------+---+---------------------+
  | reserved data 63 bytes |idx| 48 long (192 bytes) |
  +------------------------+---+---------------------+
  capacity = 48 * 50K write = 2.4M write
*/
#ifdef COUNTER_ON_EEPROM
#define EEPROM_BUF_SIZE     48
#define EEPROM_BUF_START    EEPROM_SIZE-(EEPROM_BUF_SIZE*sizeof(uint32_t))
#define EEPROM_BUF_INDEX    (EEPROM_BUF_START-1)
#define EEPROM_MAX_WRITE    50000     // max is 100K, secure is 50K
const uint16_t CountByWrite = 10;                                   // write each "n" pulse (eg 10 liters)
const uint32_t CountByRotation = (EEPROM_MAX_WRITE * CountByWrite); // rotate each 50K write (eg 500000 liters)
#endif

// MANDATORY
#define INITIAL_COUNTER     0

class Pulse : public Element
{
  public:
    explicit Pulse(uint8_t pin, uint8_t childID, const __FlashStringHelper* name, const __FlashStringHelper* unit, float converter, int16_t divider)
      : Element(childID, EEP_NONE, 0, name, unit, divider) {
      // mandatory
      _elementType = rl_element_t::E_NUMERICSENSOR;
      _dataType = rl_data_t::D_NUM;

      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
      _converter = converter;
      _curValue = _sentValue = 0; // not send value at start
	  _oldState = digitalRead(_pin);
    }
	void setConverter(float converter) {
      _converter = converter;
	}
    void Process() override {
      uint8_t newState = digitalRead(_pin);
	  if (newState != _oldState) {
		_oldState = newState;
		if (!newState) {
          _curValue += _converter;
		}
	  }
    }
  protected:
    uint8_t _pin;
    float _converter;
	uint8_t _oldState;
#ifdef COUNTER_ON_EEPROM
    uint8_t _EEPidx;
#endif
};
