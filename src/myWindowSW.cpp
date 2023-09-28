#include "myWindowSW.h"

WinSW::WinSW(bool use_debug)
{
  _id = _next_id++;
  useDebug = use_debug;
  RockerSW_V[0] = new RockerSW;
}
bool WinSW::loop()
{
  _readSW();
  _calc_current_position();
  _stop_position_reached();
  return MSG.newMSG;
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
  RockerSW_V[0]->set_input(upin, dpin);
  RockerSW_V[0]->get_SWstate(); // <--- To read init state at boot, and ignore switch state // maybe it is not needed ?
}
void WinSW::set_output(uint8_t outup_pin, uint8_t outdown_pin)
{
  outpins[0] = outup_pin;
  outpins[1] = outdown_pin;
  if (outup_pin != UNDEF_INPUT && outdown_pin != UNDEF_INPUT)
  {
    pinMode(outpins[0], OUTPUT);
    pinMode(outpins[1], OUTPUT);
    _virtCMD = false;
  }
  else
  {
    _virtCMD = true;
  }
  _allOff();
}
void WinSW::set_ext_input(uint8_t upin, uint8_t dpin)
{
  if (upin != UNDEF_INPUT && dpin != UNDEF_INPUT)
  {
    _useExtSW = true;
    RockerSW_V[1] = new RockerSW;
    RockerSW_V[1]->set_input(upin, dpin);
    RockerSW_V[1]->get_SWstate(); // <--- To read init state at boot, and ignore switch state //
  }
}
void WinSW::set_WINstate(uint8_t state, uint8_t reason) /* set windows state: up/ down/ off */
{
  _switch_window_state(state, reason);
}
void WinSW::set_Win_position(float position, uint8_t i) /* set open value 0-100 */
{
  bool lockdown_OK = (_uselockdown == true && _lockdownState == false) || _uselockdown == false;
  if (lockdown_OK && _virtCMD == false)
  {
    _validate_position_value(position);
    win_position.request_position = position;

    // Serial.print("request Pos: ");
    // Serial.print(win_position.request_position);
    // Serial.print("; Current Pos: ");
    // Serial.println(win_position.current_position);

    if (position > win_position.current_position)
    {
      _winUP();
      _start_timing_movement();
      MSG.newMSG = true;
      MSG.reason = i;
      MSG.position = position; // get_Win_position();
    }
    else if (position < win_position.current_position)
    {
      _winDOWN();
      _start_timing_movement();
      MSG.newMSG = true;
      MSG.reason = i;
      MSG.position = position; // get_Win_position();
    }
    else
    {
      return;
    }
  }
  else if (_virtCMD == true)
  {
    if (abs(position - m_properties.MAX_OPEN_POSITION) < 1)
    {
      MSG.state = UP;
    }
    else
    {
      MSG.state = DOWN;
    }
    MSG.newMSG = true;
    MSG.reason = i;
  }
}
void WinSW::set_extras(bool useLockdown)
{
  _uselockdown = useLockdown;
}
void WinSW::set_motor_properties(float to_to_up, float time_to_down, float stick_time, float end_move_time)
{
  m_properties.UP_DURATION = to_to_up;
  m_properties.DOWN_DURATION = time_to_down;
  m_properties.STALL_SEC = stick_time;
  m_properties.EXTRA_TIME_END = end_move_time;
}

void WinSW::init_lockdown()
{
  if (_uselockdown && _lockdownState == false)
  {
    _switch_window_state(DOWN, LCKDOWN);
    _lockdownState = true;
    MSG.lockdown_state = true;
  }
}
void WinSW::release_lockdown()
{
  if (_uselockdown && _lockdownState == true)
  {
    _lockdownState = false;
    MSG.lockdown_state = false;
  }
}
void WinSW::print_preferences()
{
  if (useDebug)
  {
    Serial.print(F("\n >>>>>> Window #"));
    Serial.print(_id);
    Serial.println(F(" <<<<<< "));

    Serial.print(F("Output :\t"));
    Serial.println(_virtCMD ? "Virutal" : "Relay");

    Serial.print(F("MQTT:\t\t"));
    Serial.println(name);

    Serial.print(F("in_pins #:\t"));
    Serial.print(RockerSW_V[0]->get_pins(0));
    Serial.print(F("; "));
    Serial.println(RockerSW_V[0]->get_pins(1));

    Serial.print(F("out_pins #:\t"));
    Serial.print(outpins[0]);
    Serial.print(F("; "));
    Serial.println(outpins[1]);

    Serial.print(F("extra_input :\t"));
    Serial.println(_useExtSW ? "Yes" : "No");
    if (_useExtSW)
    {
      Serial.print(F("ext_pins #:\t"));
      Serial.print(RockerSW_V[1]->get_pins(0));

      Serial.print(F("; "));
      Serial.println(RockerSW_V[1]->get_pins(1));
    }

    Serial.print(F("use lockdown:\t"));
    Serial.println(_uselockdown ? "YES" : "NO");

    Serial.println(F(" >>>>>>>> END <<<<<<<< \n"));
  }
}
void WinSW::clear_newMSG()
{
  MSG.newMSG = false;
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
float WinSW::get_Win_position()
{
  return win_position.current_position;
}
void WinSW::get_Win_props(Win_props &win_props)
{
  win_props.id = _id;
  win_props.name = name;
  win_props.extSW = _useExtSW;
  win_props.virtCMD = _virtCMD;
  win_props.lockdown = _uselockdown;

  win_props.outpins[0] = outpins[0];
  win_props.outpins[1] = outpins[1];
  win_props.inpins[0] = RockerSW_V[0]->get_pins(0);
  win_props.inpins[1] = RockerSW_V[0]->get_pins(1);
  win_props.inpins2[0] = _useExtSW ? RockerSW_V[1]->get_pins(0) : 255;
  win_props.inpins2[1] = _useExtSW ? RockerSW_V[1]->get_pins(1) : 255;
}

uint8_t WinSW::_next_id = 0;

void WinSW::_allOff()
{
  if (!_virtCMD)
  {
    digitalWrite(outpins[0], !RELAY_ON);
    digitalWrite(outpins[1], !RELAY_ON);
    win_position.is_rotating = false;
    delay(10); // for safety
  }
  MSG.state = STOP;
  MSG.position = get_Win_position();
  Serial.print("end_pos: ");
  Serial.println(win_position.current_position);
  // Serial.print("delta_time: ");
  // Serial.println(millis() - win_position.start_clk);
}
void WinSW::_winUP()
{
  if (!_virtCMD)
  {
    digitalWrite(outpins[1], !RELAY_ON);
    delay(10);
    digitalWrite(outpins[0], RELAY_ON);
    win_position.is_rotating = true;
    MSG.state = UP;
  }
}
void WinSW::_winDOWN()
{
  if (!_virtCMD)
  {
    digitalWrite(outpins[0], !RELAY_ON);
    delay(10);
    digitalWrite(outpins[1], RELAY_ON);
    win_position.is_rotating = true;
    MSG.state = DOWN;
  }
}
void WinSW::_switch_window_state(uint8_t state, uint8_t i)
{
  if ((state != get_winState() || _virtCMD == true))
  {
    if (state != ERR)
    {
      switch (state)
      {
      case UP:
        set_Win_position(m_properties.MAX_OPEN_POSITION, i);
        break;
      case DOWN:
        set_Win_position(m_properties.MIN_OPEN_POSITION, i);
        break;
      case STOP:
        _allOff();
        MSG.newMSG = true;
        break;
      default:
        break;
      }
    }
    else
    {
      MSG.state = ERR;
    }
    MSG.reason = i;
  }
}
void WinSW::_readSW()
{
  /*  0: stop; 1: up; 2: down; 4:err ; 3: nochange*/
  uint8_t switchRead = RockerSW_V[0]->get_SWstate();
  if (switchRead < 3)
  {
    _switch_window_state(switchRead, BUTTON);
    return;
  }
  if (_useExtSW)
  {
    switchRead = RockerSW_V[1]->get_SWstate();
    if (switchRead < 3)
    {
      _switch_window_state(switchRead, BUTTON2);
      return;
    }
  }
}
void WinSW::_calc_current_position()
{
  if (win_position.is_rotating)
  {
    unsigned long millis_delta = millis() - win_position.start_clk;
    uint8_t state = get_winState();

    if (state == DOWN && win_position.current_position > m_properties.MIN_OPEN_POSITION)
    {
      win_position.current_position = win_position.init_position - (m_properties.MAX_OPEN_POSITION * millis_delta * 0.001) / (m_properties.DOWN_DURATION);
      MSG.position = win_position.current_position;
    }
    else if (state == UP && win_position.current_position < m_properties.MAX_OPEN_POSITION)
    {
      win_position.current_position = win_position.init_position + (m_properties.MAX_OPEN_POSITION * millis_delta * 0.001) / (m_properties.UP_DURATION);
      MSG.position = win_position.current_position;
    }
    else
    {
      Serial.println("ERRR");
    }
  }
}
void WinSW::_validate_position_value(float &value)
{
  if (value != UNDEF_POSITION)
  {
    if (value >= (float)m_properties.MAX_OPEN_POSITION)
    {
      value = (float)m_properties.MAX_OPEN_POSITION;
    }
    else if (value < (float)m_properties.MIN_OPEN_POSITION)
    {
      value = (float)m_properties.MIN_OPEN_POSITION;
    }
    else
    {
      return;
    }
  }
}
void WinSW::_stop_position_reached()
{
  if (win_position.is_rotating)
  {
    if (abs(win_position.last_position - win_position.request_position) < abs(win_position.current_position - win_position.last_position) || abs(win_position.current_position - win_position.request_position) < 0.02)
    {
      _allOff();
      if (win_position.request_position == m_properties.MAX_OPEN_POSITION || win_position.request_position == m_properties.MIN_OPEN_POSITION)
      {
        delay(m_properties.EXTRA_TIME_END * 1000);
      }
      Serial.print("Reached requested position: ");
      Serial.println(win_position.current_position);
      if (abs(win_position.current_position - win_position.request_position) < 0.5 && abs(win_position.current_position - win_position.request_position) > 0.01)
      {
        Serial.print("Position err correction: ");
        Serial.println(abs(win_position.current_position - win_position.request_position));
        win_position.current_position = win_position.request_position;
      }
      else
      {
        Serial.println("Position err to big");
      }
      MSG.reason = 0; // TIMEOUT
      MSG.newMSG = true;
    }
    win_position.last_position = win_position.current_position;
  }
}
void WinSW::_start_timing_movement()
{
  delay(m_properties.STALL_SEC * 1000); // <--- motor stick extra time
  win_position.start_clk = millis();
  win_position.init_position = win_position.current_position;
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