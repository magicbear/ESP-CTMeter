#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "tcs34725.h"
#include "driver/adc.h"

tcs34725 rgb_sensor;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define Threshold 70 /* Greater the value, more the sensitivity */

typedef struct {
  uint32_t meas_start;
  uint32_t meas_end;
  uint32_t fix_start;
  uint32_t fix_end;
} fix_map_t;

fix_map_t fix_maps[] = {
  {3500, 4680, 3500, 5000},
//  {4400, 4950, 5000, 5300},
  {4680, 4950, 5000, 5300},
  {4950, 5550, 5300, 6450},
  {5550, 5850, 6450, 6850},
  {5850, 6100, 6850, 7250},
  {6100, 7100, 7250, 13000},
  {7100, 9500, 13000, 15400},
  {9500, 11600, 15400, 16800},
  {11600, 14500, 16800, 24000}
};
          
fix_map_t fix_lux_maps[] = {
  {700, 8000, 700, 8000}
};
// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    5
#define OLED_CS    17
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void callback(){
   //placeholder callback function
}

void setup(void) {
  Serial.begin(115200);
  Wire.begin(19, 22);

}

float Min(float a, float b) {
  return a <= b ? a : b;
}

float Max(float a, float b) {
  return a >= b ? a : b;
}

void loop(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.ssd1306_command(SSD1306_DISPLAYON);
//  display.ssd1306_command(SSD1306_SETCONTRAST);
//  display.ssd1306_command(128); // Where c is a value from 0 to 255 (sets contrast e.g. brightness)
  display.display();

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); // @gremlins Bright light, bright light!  
  
  rgb_sensor.begin();
  delay(10);
//  rgb_sensor.enable();
//  delay(10);
  rgb_sensor.getData();
  Serial.print(F("Gain:"));
  Serial.print(rgb_sensor.againx);
  Serial.print(F("x "));
  Serial.print(F("Time:"));
  Serial.print(rgb_sensor.atime_ms);
  Serial.print(F("ms (0x"));
  Serial.print(rgb_sensor.atime, HEX);
  Serial.println(F(")"));

  Serial.print(F("Raw R:"));
  Serial.print(rgb_sensor.r);
  Serial.print(F(" G:"));
  Serial.print(rgb_sensor.g);
  Serial.print(F(" B:"));
  Serial.print(rgb_sensor.b);
  Serial.print(F(" C:"));
  Serial.println(rgb_sensor.c);

  Serial.print(F("IR:"));
  Serial.print(rgb_sensor.ir);
  Serial.print(F(" CRATIO:"));
  Serial.print(rgb_sensor.cratio);
  Serial.print(F(" Sat:"));
  Serial.print(rgb_sensor.saturation);
  Serial.print(F(" Sat75:"));
  Serial.print(rgb_sensor.saturation75);
  Serial.print(F(" "));
  Serial.println(rgb_sensor.isSaturated ? "*SATURATED*" : "");

  Serial.print(F("CPL:"));
  Serial.print(rgb_sensor.cpl);
  Serial.print(F(" Max lux:"));
  Serial.println(rgb_sensor.maxlux);

  Serial.print(F("Compensated R:"));
  Serial.print(rgb_sensor.r_comp);
  Serial.print(F(" G:"));
  Serial.print(rgb_sensor.g_comp);
  Serial.print(F(" B:"));
  Serial.print(rgb_sensor.b_comp);
  Serial.print(F(" C:"));
  Serial.println(rgb_sensor.c_comp);

  Serial.print(F("Lux:"));
  Serial.print(rgb_sensor.lux);
  Serial.print(F(" CT:"));
  Serial.print(rgb_sensor.ct);
  Serial.println(F("K"));

  // Clear the buffer
  display.clearDisplay();
  display.setTextColor(WHITE);        // Draw white text
  display.setTextSize(3);             // Normal 1:1 pixel scale
  display.setCursor(0,0);             // Start at top-left corner
  if (isnan(rgb_sensor.ct))
  {
      display.printf("ERROR");
  } else {
      uint32_t ct = (uint32_t)rgb_sensor.ct;
      uint32_t lux = (uint32_t)rgb_sensor.lux;
      for (int i = 0; i < sizeof(fix_maps) / sizeof(fix_map_t); i++)
      {
          if (rgb_sensor.ct >= fix_maps[i].meas_start && rgb_sensor.ct < fix_maps[i].meas_end)
          {
              ct = map((uint32_t)rgb_sensor.ct, fix_maps[i].meas_start, fix_maps[i].meas_end, fix_maps[i].fix_start, fix_maps[i].fix_end);
              Serial.printf("Fixed CT: %d\n", map((uint16_t)rgb_sensor.ct, fix_maps[i].meas_start, fix_maps[i].meas_end, fix_maps[i].fix_start, fix_maps[i].fix_end));
          }
      }
      display.printf("%dK", ct);
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setCursor(0,30);             // Start at top-left corner
      if (isinf(rgb_sensor.lux))
      {
          display.printf("LUX: <0");
      } else {
          for (int i = 0; i < sizeof(fix_lux_maps) / sizeof(fix_map_t); i++)
          {
              if (rgb_sensor.lux >= fix_lux_maps[i].meas_start && rgb_sensor.lux < fix_lux_maps[i].meas_end)
              {
                  lux = map((uint32_t)rgb_sensor.lux, fix_lux_maps[i].meas_start, fix_lux_maps[i].meas_end, fix_lux_maps[i].fix_start, fix_lux_maps[i].fix_end);
                  Serial.printf("Fixed LUX: %d\n", lux);
              }
          }
          display.printf("%ld lx", lux);
      }

      
      float r = (rgb_sensor.r / 65536.0f);
      float g = (rgb_sensor.g / 65536.0f);
      float b = (rgb_sensor.b / 65536.0f);
      float H,S,L;
    
      float min = Min(Min(r, g), b);
      float max = Max(Max(r, g), b);
      float delta = max - min;
      L = (max + min) / 2;
    
      if (delta == 0)
      {
        H = 0;
        S = 0.0f;
      }
      else
      {
        S = (L <= 0.5) ? (delta / (max + min)) : (delta / (2 - max - min));
    
        float hue;
    
        if (r == max)
        {
          hue = ((g - b) / 6) / delta;
        }
        else if (g == max)
        {
          hue = (1.0f / 3) + ((b - r) / 6) / delta;
        }
        else
        {
          hue = (2.0f / 3) + ((r - g) / 6) / delta;
        }
    
        if (hue < 0)
          hue += 1;
        if (hue > 1)
          hue -= 1;
    
         H = (int)(hue * 360);
      }
      display.setTextSize(1);             // Normal 1:1 pixel scale
      display.setCursor(0,55);             // Start at top-left corner
      display.printf("H:%d S:%d%%", (uint16_t)H, (uint16_t)(S * 100));
  }
  display.display();

  Serial.println();
  rgb_sensor.disable();
  digitalWrite(4, LOW); // @gremlins Bright light, bright light!  
  delay(5000);
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  display.ssd1306_command(SSD1306_CHARGEPUMP);            // disable charge pump
  
  pinMode(     OLED_RESET, OUTPUT);
  digitalWrite(OLED_RESET, HIGH);
  delay(1);                   // VDD goes high at start, pause for 1 ms
  digitalWrite(OLED_RESET, LOW);  // Bring reset low
  pinMode(     OLED_RESET, INPUT);
  pinMode(     OLED_DC, INPUT);
  pinMode(     OLED_CLK, INPUT);
  pinMode(     OLED_MOSI, INPUT);

//  Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(T3, callback, Threshold);

//  adc_power_off();
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();
  esp_deep_sleep_start();
}
