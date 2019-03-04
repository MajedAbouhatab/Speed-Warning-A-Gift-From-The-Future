#include <SDHCI.h>
#include <Audio.h>
SDClass ThisSD;
AudioClass *ThisAudio;
File ThisFile;
// Singal from truck speed sensor
int InPin = 15;
// Signal to disconnect truck radio
int OutPin = 12;
// The calculated truck speed based on sensor's pulses
int Speed = 0;
// digits 0 to 9 in binary (inverted because using common anode)
int ThisDigit[10][7] = {{0, 0, 0, 0, 0, 0, 1},//0
  {1, 0, 0, 1, 1, 1, 1},//1                          ____A____
  {0, 0, 1, 0, 0, 1, 0},//2                         |         |
  {0, 0, 0, 0, 1, 1, 0},//3                         F         B
  {1, 0, 0, 1, 1, 0, 0},//4                         |____G____|
  {0, 1, 0, 0, 1, 0, 0},//5                         |         |
  {0, 1, 0, 0, 0, 0, 0},//6                         E         C
  {0, 0, 0, 1, 1, 1, 1},//7                         |____D____|
  {0, 0, 0, 0, 0, 0, 0},//8               Pins = > 0 1 2 3 4 5 6
  {0, 0, 0, 0, 1, 0, 0}//9 <<<-------------<<< =  {A,B,C,D,E,F,G}
};

void setup()
{
  // Signal from truck speed sensor
  pinMode(InPin, INPUT);
  // Signal to disconnect truck radio
  pinMode(OutPin, OUTPUT);
  // Setup seven segment displays' pins
  digitalWrite(OutPin, HIGH);
  for (int i = 0; i <= 8; i++) {
    pinMode(i, OUTPUT);
  }
  // Initializing audio for the alarm
  ThisAudio = AudioClass::getInstance();
  ThisAudio->begin();
  ThisAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  ThisAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, AS_SP_DRV_MODE_LINEOUT);
  ThisAudio->setVolume(0);
  ThisAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);
  // Sound check
  for (int i = 0; i <= 2; i++) {
    ThisAudio->setBeep(1, -20, 3000);
    delay(50);
    ThisAudio->setBeep(0, 0, 0);
    delay(50);
  }
}
void loop()
{
  //27mph => 30hz
  //32mph => 35hz
  //36mph => 40hz
  // Calculate speed based on the high half of the pulse coming from the truck speed sensor
  Speed = 450000 / pulseIn(InPin, HIGH);
  // Getting the loop to go faster
  if (Speed < 100) {
    // Show the two digits current speed
    for (int i = 0; i <= 15; i++) {
      DisplayDigits(Speed);
    }
    // Check if speed above the threshold
    if (Speed > 30) {
      // Cut off the truck's radio sound
      digitalWrite(OutPin, LOW);
      // Open alarm sound file
      ThisFile = ThisSD.open("S.mp3");
      // Prime sound player
      ThisAudio->writeFrames(AudioClass::Player0, ThisFile);
      // Start sound player
      ThisAudio->startPlayer(AudioClass::Player0);
      // Getting the next chunk(s) of sound until the end
      while (!ThisAudio->writeFrames(AudioClass::Player0, ThisFile));
      // Stop audio player
      ThisAudio->stopPlayer(AudioClass::Player0);
      // Close alarm sound file
      ThisFile.close();
      // If speed below the threshold and no sound from truck's radio
    } else if (digitalRead(OutPin) == LOW) {
      // Turn radio sound back on
      digitalWrite(OutPin, HIGH);
    }
  }
}

void DisplayDigits(int j) {
  // Right digit
  OneDigit(7, j - (int(j / 10) * 10));
  // Left digit
  OneDigit(8, int(j / 10) );
}

void OneDigit(int D, int j) {
  // Enable this digit
  digitalWrite(D, 1);
  // Assign values to pins
  for (int i = 0; i <= 6; i++)
  {
    digitalWrite(i, ThisDigit[j][i]);
  }
  delay(10);
  // Disable this digit
  digitalWrite(D, 0);
}
