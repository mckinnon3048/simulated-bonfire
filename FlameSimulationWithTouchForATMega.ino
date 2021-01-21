#include <CapacitiveSensor.h>
// CapacitiveSensor library, raises pin13 and counts the delay before pin 11 reaches the same state
// assembled with 10MOhm resistor between pins 13 & 11)
// syntax is (send_pin, receive_pin)
CapacitiveSensor  Sensor = CapacitiveSensor(13, 11);
// stores the value measured by the CapacitiveSensor
long touchVal;
// stores the current state of the device
bool toggleBool;
int touchLoopDelayCount = 0;
// skips touch sensitivity for this many seconds after it is toggled to prevent 'double click'
int touchLoopDelayTimer = 5;

// controlls the sensitivity increasing it makes it less sensitive
long triggerThreshold = 17500;

// pins 2-8 are PWM, each controlling a MOSFET with 3 led clusters
int clusterPins[]{2, 3, 4, 5, 6, 7, 8};
//uses a minimum resolution of 'delay' in order to skip adjusting flame values randomly
int clusterDelay[]{5, 5, 5, 5, 5, 5, 5};
int clusterBrightness[]{50, 50, 50, 50, 50, 50, 50};
int clusterBrightnessLeveling[]{50, 50, 50, 50, 50, 50, 50};
int clusterCount = 7;
  // Modify these constants to change the behavior of the flame
const int flameLoopDelay = 50; //ms, multiplied by the clusterDelay on each pin
  // 0-255 sets the minimum brightness
const int minBrightness = 30; //default is 50 
  // 0-255 sets the maximum brightness
const int maxBrightness = 255; 
  // milliseconds before a random value is pulled again controlls the fastest the light can flicker
const int minimumDelay = 30; // default is 80  
  //milliseconds before a random value is pulled again controlls the slowest the light can flicker
const int maximumDelay = 100; // default is 190
  // The loop forces all values greater than the maxBrightness back down the the minimum brightness,
  // increasing this value will cause the flame to rest at minBrightness for a higher percentage of the time,
  // but also increases the odds of having higher flame intensity
  // values >126 and < 255 will increasingly cause a gradual "build up" of the flame intensity (like a flickering candle)
  // values <126 will cause a slow, soft flicker of light, increasing the rarity of max brightness events (like coals/embers)
const int oddsOfDimModifier = 180; //180
  // These value causes the flame to decay slower, values close to 100 will vary more from cycle to cycle
  // values close to zero will cause the brightness to randomize based on the oddsOfDimModifier more
  // a manual value may be set if desired, I found 80 to be a 'happy medium'
const int minLeveling = 50;
const int maxLeveling = 70;

//uses a minimum resolution of 'delay' in order to skip adjusting flame values randomly

// Iterates over the pinIDs in clusterPins, setting them to output mode & enables serial communication for debugging
void setup() { 
  for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++){
    pinMode(clusterPins[clusterIndex], OUTPUT);
    Serial.begin(19200);
  }
  touchLoopDelayTimer = touchLoopDelayTimer * 1000 / flameLoopDelay;
}

// randomizes brightness of the indexed clusterPin, unless pin has been identified to delay, then it skips this function and decrements the delay counter
void cluster_flame_effect(int clusterIndex){
  //Serial.println(clusterIndex);
  if (clusterDelay[clusterIndex] > 0){
    clusterDelay[clusterIndex] = clusterDelay[clusterIndex] - 1;
  }
  else{
    clusterBrightnessLeveling[clusterIndex] = (random(minLeveling, maxLeveling));
    int randomBrightness = random(oddsOfDimModifier);
    clusterDelay[clusterIndex] = random(minimumDelay, maximumDelay)/flameLoopDelay;
    clusterBrightness[clusterIndex] = randomBrightness + ((clusterBrightness[clusterIndex] * clusterBrightnessLeveling[clusterIndex])/100);
    if (clusterBrightness[clusterIndex] > maxBrightness) {
      clusterBrightness[clusterIndex] = maxBrightness;
    }
    if (clusterBrightness[clusterIndex] < minBrightness){
      clusterBrightness[clusterIndex] = minBrightness;
    }
    analogWrite(clusterPins[clusterIndex], clusterBrightness[clusterIndex]);
  }
}

// uses CapacitiveSensor to toggle the flame effect lighting on or off.
void touch_activation(){
  if (touchLoopDelayCount >0){
    touchLoopDelayCount --;
  }
  else{
    touchVal = Sensor.capacitiveSensor(30);
    if (touchVal >= triggerThreshold){
      toggleBool = !toggleBool; 
      touchLoopDelayCount = touchLoopDelayTimer;
    }
  }
}


// calls touch sensor function and if touch has been triggered iterates over the clusterPins array, triggering the cluster_flame_effect over each index
void loop() {
  touch_activation();
  //Serial.println(touchVal);
  if (toggleBool == true){
    for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++){
      cluster_flame_effect(clusterIndex);
      Serial.println(clusterIndex);
      //Serial.println(clusterBrightness[clusterIndex]);    
    }
  }
  else{
    for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++){
      analogWrite(clusterPins[clusterIndex], 0); 
    }
  }
  delay(flameLoopDelay);
  
}
