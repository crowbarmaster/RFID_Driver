#include <ReceiveOnlySoftwareSerial.h>

ReceiveOnlySoftwareSerial RFID(3);
const long WAIT_TIME = 3000;
const long UNLOCK_TIME = 10000;
const char *MasterKeys[1] = {"0E000000000"};
const int LockOutput = A5;
const int SpkrOutput = A4;
const int AtHomeInd = 4;
const int AtHomeEntry = 7;
const int LEDG = 5;
const int LEDB = 6;
const int ExitBtn = 8;
const int BUFFER_SIZE = 14;
int buffer_index = 0;
bool AtHome = false;
char buffer[BUFFER_SIZE];
long LockTimer = 0;
long IdleTime = 0;

void setup()
{
  pinMode(LockOutput, OUTPUT);
  pinMode(SpkrOutput, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(AtHomeInd, OUTPUT);
  pinMode(ExitBtn, INPUT_PULLUP);
  pinMode(AtHomeEntry, INPUT_PULLUP);
  RFID.begin(9600);
  Serial.begin(9600);
  Serial.println("Serial init okay!");
}

void loop()
{
  long startTime = millis();
  if (digitalRead(ExitBtn) == LOW)
  {
    while (digitalRead(ExitBtn) == LOW)
    {
      delay(50);
      if (millis() >= startTime + 2000)
      {
        AtHome = !AtHome;
        digitalWrite(AtHomeInd, LOW);
        bool flash = false;
        for (int f = 0; f < 10; f++)
        {
          flash = !flash;
          if (f >= 7)
          {
            if (flash == true && AtHome == false)
            {
              flash = false;
            }
          }
          digitalWrite(AtHomeInd, flash);
          delay(500);
          digitalWrite(AtHomeInd, LOW);
        }

      }
    }
    if (millis() < startTime + 2000)
    {
      LockTimer = millis() + UNLOCK_TIME;
      Unlock();
    }
  }

  if (digitalRead(AtHomeEntry) == false && AtHome == true)
  {
    LockTimer = millis() + UNLOCK_TIME;
    Unlock();
  }

  digitalWrite(AtHomeInd, AtHome);

  if (RFID.available() > 0)
  {
    bool call_extract_tag = false;
    char startChar = 2;
    char endChar = 3;
    char ssvalue = RFID.read();
    if (ssvalue == -1)
    {
      return;
    }
    if (ssvalue == startChar)
    {
      buffer_index = 0;
    } else if (ssvalue == endChar)
    {
      call_extract_tag = true;
    }
    if (buffer_index >= BUFFER_SIZE)
    {
      return;
    }

    buffer[buffer_index++] = ssvalue;
    if (call_extract_tag == true)
    {
      char sendme[12];
      for (int i = 1; i < 13; i++)
      {
        sendme[i - 1] = buffer[i];
        Serial.write(buffer[i]);
      }
      bool ChkFlag = false;
      for (int x = 0; x < 8; x++) {
        char Key[13];
        char compKey[13];
        strcpy(Key, MasterKeys[x]);
        strcpy(compKey, sendme);
        if (strcmp(Key, compKey) == 0 && LockTimer == 0)
        {
          ChkFlag = true;
        }
      }
      if (ChkFlag == true)
      {
        LockTimer = millis() + UNLOCK_TIME;
        Unlock();
      }
      else
      {
        bool flash = false;
        for (int i = 0; i < 6; i++)
        {
          flash = !flash;
          digitalWrite(LEDB, flash);
          delay(25);
        }
        digitalWrite(LEDB, LOW);
      }
    }
  }
  if (millis() > IdleTime + 1000)
  {
    digitalWrite(LEDG, HIGH);
    delay(5);
    digitalWrite(LEDG, LOW);
    IdleTime = millis();
  }
}

void Unlock ()
{
  digitalWrite(LockOutput, HIGH);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, LOW);
  while (millis() <= LockTimer)
  {
    digitalWrite(LEDB, HIGH);
    delay(50);
    digitalWrite(LEDB, LOW);
    delay(100);
  }
  digitalWrite(LockOutput, LOW);
  digitalWrite(LEDB, LOW);
  while (RFID.available() > 0)
  {
    RFID.read();
  }
  LockTimer = 0;
}
