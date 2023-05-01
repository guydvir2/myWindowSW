#include "myWindowSW.h"

WinSW::WinSW()
{
  _id = _next_id++;
}
bool WinSW::loop()
{
  _readSW();
  _timeout_looper();
  return newMSGflag;
}
void WinSW::set_id(uint8_t i)
{
  _id = i;
}
void WinSW::set_name(const char *_name)
{
  strlcpy(name, _name, MAX_TOPIC_SIZE);
}
void WinSW::set_input(uint8_t upin, uint8_t dpin)
{
  _mainSW.set_input(upin, dpin);
  _mainSW.get_SWstate(); // <--- To read init state at boot, and ignore switch state // maybe it is not needed ?
}
void WinSW::set_output(uint8_t outup_pin, uint8_t outdown_pin)
{
  if (outup_pin != UNDEF_INPUT && outdown_pin != UNDEF_INPUT)
  {
    outpins[0] = outup_pin;
    outpins[1] = outdown_pin;
    pinMode(outpins[0], OUTPUT);
    pinMode(outpins[1], OUTPUT);
    virtCMD = false;
  }
  else
  {
    virtCMD = true;
  }
  _allOff();
}
void WinSW::set_ext_input(uint8_t upin, uint8_t dpin)
{
  if (upin != UNDEF_INPUT && dpin != UNDEF_INPUT)
  {
    useExtSW = true;
    _extSW.set_input(upin, dpin);
    _extSW.get_SWstate(); // <--- To read init state at boot, and ignore switch state //
  }
}
void WinSW::set_WINstate(uint8_t state, uint8_t reason) /* External Callback */
{
  _switch_cb(state, reason);
}
void WinSW::set_extras(bool useLockdown, int timeout_clk)
{
  _uselockdown = useLockdown;
  _timeout_clk = timeout_clk;
}

void WinSW::init_lockdown()
{
  if (_uselockdown && _lockdownState == false)
  {
    _switch_cb(DOWN, LCKDOWN);
    _lockdownState = true;
  }
}
void WinSW::release_lockdown()
{
  if (_uselockdown && _lockdownState == true)
  {
    _lockdownState = false;
  }
}
void WinSW::print_preferences()
{
  Serial.print(F("\n >>>>>> Window #"));
  Serial.print(_id);
  Serial.println(F(" <<<<<< "));

  Serial.print(F("Output :\t"));
  Serial.println(virtCMD ? "Virutal" : "Relay");

  Serial.print(F("MQTT:\t"));
  Serial.println(name);

  Serial.print(F("in_pins #:\t"));
  Serial.print(_mainSW.get_pins(0));
  Serial.print(F("; "));
  Serial.println(_mainSW.get_pins(1));

  Serial.print(F("out_pins #:\t"));
  Serial.print(outpins[0]);
  Serial.print(F("; "));
  Serial.println(outpins[1]);

  Serial.print(F("ext_pins #:\t"));
  Serial.print(_extSW.get_pins(0));

  Serial.print(F("; "));
  Serial.println(_extSW.get_pins(1));

  Serial.print(F("use timeout:\t"));
  Serial.println(_timeout_clk);

  Serial.print(F("use lockdown:\t"));
  Serial.println(_uselockdown ? "YES" : "NO");

  Serial.println(F(" >>>>>>>> END <<<<<<<< \n"));
}
void WinSW::clear_newMSG()
{
  newMSGflag = false;
}
uint8_t WinSW::get_id()
{
  return _id;
}
uint8_t WinSW::get_winState()
{
  bool uprel = digitalRead(outpins[0]);
  bool downrel = digitalRead(outpins[1]);

  if (downrel == !RELAY_ON && uprel == !RELAY_ON)
  {
    return STOP;
  }
  else if (downrel == RELAY_ON && uprel == !RELAY_ON)
  {
    return DOWN;
  }
  else if (downrel == !RELAY_ON && uprel == RELAY_ON)
  {
    return UP;
  }
  else
  {
    return ERR;
  }
}
void WinSW::get_Win_props(Win_props &win_props)
{
  win_props.id = _id;
  win_props.name = name;
  win_props.extSW = useExtSW;
  win_props.virtCMD = virtCMD;
  win_props.lockdown = _uselockdown;

  win_props.outpins[0] = outpins[0];
  win_props.outpins[1] = outpins[1];
  win_props.inpins[0] = _mainSW.get_pins(0);
  win_props.inpins[1] = _mainSW.get_pins(1);
  win_props.inpins2[0] = useExtSW ? _extSW.get_pins(0) : 255;
  win_props.inpins2[1] = useExtSW ? _extSW.get_pins(1) : 255;
}
uint8_t WinSW::_next_id = 0;

void WinSW::_allOff()
{
  if (!virtCMD)
  {
    digitalWrite(outpins[0], !RELAY_ON);
    digitalWrite(outpins[1], !RELAY_ON);
    delay(10);
  }
}
void WinSW::_winUP()
{
  _allOff();
  if (!virtCMD)
  {
    digitalWrite(outpins[0], RELAY_ON);
  }
}
void WinSW::_winDOWN()
{
  _allOff();
  if (!virtCMD)
  {
    digitalWrite(outpins[1], RELAY_ON);
  }
}
void WinSW::_switch_cb(uint8_t state, uint8_t i)
{
  if (((_uselockdown && _lockdownState == false) || _uselockdown == false) && (state != get_winState() || virtCMD == true))
  {
    if (state == STOP)
    {
      _allOff();
      newMSGflag = true;
      _timeoutcounter = 0;
    }
    else if (state == UP)
    {
      _winUP();
      newMSGflag = true;
      _timeoutcounter = millis();
    }
    else if (state == DOWN)
    {
      _winDOWN();
      newMSGflag = true;
      _timeoutcounter = millis();
    }
    else
    {
      MSG.state = ERR;
      MSG.reason = i;
      return;
    }
    MSG.state = state;
    MSG.reason = i;
  }
}
void WinSW::_readSW()
{
  /*  0: stop; 1: up; 2: down; 4:err ; 3: nochange*/
  uint8_t switchRead = _mainSW.get_SWstate();
  if (switchRead < 3)
  {
    _switch_cb(switchRead, BUTTON);
    return;
  }
  if (useExtSW)
  {
    switchRead = _extSW.get_SWstate();
    if (switchRead < 3)
    {
      _switch_cb(switchRead, BUTTON2);
      return;
    }
  }
}
void WinSW::_timeout_looper()
{
  if (_timeout_clk > 0 && _timeoutcounter > 0)
  {
    if (millis() - _timeoutcounter > _timeout_clk * 1000)
    {
      _switch_cb(STOP, TIMEOUT);
    }
  }
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

RockerSW::RockerSW()
{
}
uint8_t RockerSW::get_pins(uint8_t i)
{
  return _pins[i];
}
void RockerSW::set_input(uint8_t upPin, uint8_t downPin, uint8_t active_dir)
{
  _pins[0] = upPin;
  _pins[1] = downPin;

  for (uint8_t i = 0; i < 2; i++)
  {
    pinMode(_pins[i], active_dir);
    _last_state[i] = digitalRead(_pins[i]);
  }
}
uint8_t RockerSW::get_SWstate()
{
  uint8_t _up = _readPin(0);
  uint8_t _down = _readPin(1);

  if (_up == STATE_NOCHG && _down == STATE_NOCHG)
  {
    return STATE_NOCHG;
  }
  else if (_up == PRESSED && _down == PRESSED)
  {
    return STATE_ERR;
  }
  else if (_up != PRESSED && _down != PRESSED)
  {
    return STATE_OFF;
  }
  else if (_up == PRESSED && _down != PRESSED)
  {
    return STATE_1;
  }
  else if (_up != PRESSED && _down == PRESSED)
  {
    return STATE_2;
  }
  else
  {
    return STATE_ERR;
  }

  /*
  Return codes:
  0 - Both are off
  1 - Pin0 is Pressed
  2 - Pin1 is Pressed
  3 - no change
  4 - err
  */
}
uint8_t RockerSW::_readPin(uint8_t i)
{
  _last_state[i] = _state[i];
  _state[i] = digitalRead(_pins[i]);
  unsigned long now = millis();

  if (_state[i] == PRESSED)
  {
    if (_last_state[i] != PRESSED)
    {
      _down_ms[i] = now;
      _is_triggered[i] = false;
    }
    else if (_is_triggered[i] == false && (now - _down_ms[i] > DEBOUNCE_MS))
    {
      _is_triggered[i] = true;
      return PRESSED;
    }
  }
  else if (_state[i] != PRESSED)
  {
    if (_last_state[i] == PRESSED)
    {
      _down_rel_ms[i] = now - _down_ms[i];
      if (_down_rel_ms[i] >= DEBOUNCE_MS)
      {
        return !PRESSED;
      }
    }
  }
  return STATE_NOCHG;
}