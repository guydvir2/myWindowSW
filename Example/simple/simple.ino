#include <myWindowSW.h>

#define numW 1

WinSW *winSW_V[] = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

void startWindow(uint8_t i)
{
  for (uint8_t x = 0; x < i; x++)
  {
    winSW_V[x] = new WinSW;
    winSW_V[x]->def(D5, D6, D3, D4);
    winSW_V[x]->def_extSW(D7, D2);
    winSW_V[x]->def_extras();
    winSW_V[x]->start();
  }
}
void loopWindow(uint8_t i)
{
  for (uint8_t x = 0; x < i; x++)
  {
    winSW_V[x]->loop();
    if (winSW_V[x]->newMSGflag)
    {
      output_func(winSW_V[x]->MSG.state, winSW_V[x]->MSG.reason, x);
      winSW_V[x]->newMSGflag = false;
    }
  }
}

void output_func(uint8_t state, uint8_t reason, uint8_t i)
{
  char msg[30];
  sprintf(msg, "Window [#%d] is [%s] by [%s]", i, STATES_TXT[state], REASONS_TXT[reason]);
  Serial.println(msg);
}
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("\n\nStart");
  startWindow(numW);
}

void loop()
{
  // put your main code here, to run repeatedly:
  loopWindow(numW);
}
