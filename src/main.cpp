/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers
 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98
 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Input pin declarations
#define ANALOG_INPUT_PIN 17
#define BUTTON_INPUT_A 12

//Global variables, constants, and state flags
uint16_t DEBOUNCE_DELAY_MS = 200;

#define NUMFLAKES 10                        // Number of snowflakes in the animation example
static float turret_endpoint_x = 0;   //Turret endpoint X coordinate
static float turret_endpoint_y = 0;   //Turret endpoint Y coordinate

static float TURRET_ANGLE_DEGREES = 0;    //Turret angle in degrees
uint8_t TURRET_LENGTH_PIXELS = 10;       //Turret length in Pixels
uint32_t ARENA_WIDTH_PIXELS = 1000;
uint32_t ARENA_HEIGHT_PIXELS = 1000;
float GRAVITY_CONSTANT = 9.81;

bool player_fired_flag = false;

//Bitmap Sprites
#define TANK_HEIGHT 8
#define TANK_WIDTH 16

static const unsigned char PROGMEM logo_bmp[] =
    {

        B00000000, B11000000,
        B00000001, B11000000,
        B00000001, B11000000,
        B00000011, B11100000,
        B11110011, B11100000,
        B11111110, B11111000,
        B01111110, B11111111,
        B00110011, B10011111,
        B00011111, B11111100,
        B00001101, B01110000,
        B00011011, B10100000,
        B00111111, B11100000,
        B00111111, B11110000,
        B01111100, B11110000,
        B01110000, B01110000,
        B00000000, B00110000

};

const unsigned char test_bmp[] =
    {

        B00001111, B11110000,
        B00001111, B11110000,
        B00111111, B11111100,
        B01111111, B11111110,
        B11111111, B11111111,
        B11111111, B11111111,
        B10101010, B01010101,
        B11101110, B01110111};

//Function prototypes
void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h);
void draw_tank();
void draw_turret();
void simulate_projectile();
void ISR_button_press_a();                        //Function prototype for button press, to facilitate

void setup()
{
  Serial.begin(9600);

  //Set pin modes and interrupts
  pinMode(ANALOG_INPUT_PIN, INPUT);
  pinMode(BUTTON_INPUT_A, INPUT_PULLDOWN);                      
  attachInterrupt(BUTTON_INPUT_A, ISR_button_press_a, RISING);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  //display.display();
  //delay(2000); // Pause for 2 seconds

  // Clear the buffer
  //display.clearDisplay();

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  //display.display();
  //delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...
}

void loop()
{

  display.clearDisplay();

  draw_tank();
  
  if (player_fired_flag == false){
    TURRET_ANGLE_DEGREES = map(analogRead(ANALOG_INPUT_PIN), 0, 1023, 0, 89); //Map turret angle to analog input pin

    //Calculate turret endpoint using turret angle in degrees
    turret_endpoint_x = TURRET_LENGTH_PIXELS * cos(TURRET_ANGLE_DEGREES * (PI / 180)) + (TANK_WIDTH / 2);
    turret_endpoint_y = (SCREEN_HEIGHT - TANK_HEIGHT) - (TURRET_LENGTH_PIXELS * sin(TURRET_ANGLE_DEGREES * (PI / 180)));
    
  }

  draw_turret();
  
  if (player_fired_flag == true){
    simulate_projectile();
  }
  

  display.display();
  //testanimate(test_bmp, TANK_WIDTH, TANK_HEIGHT); // Animate bitmaps
}

void draw_tank()
{

  display.drawBitmap(0, (SCREEN_HEIGHT - TANK_HEIGHT), test_bmp, TANK_WIDTH, TANK_HEIGHT, 1);
}

//Set tank turret orientation
void draw_turret()
{
  
  //Display angle value in degrees
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(TURRET_ANGLE_DEGREES);
  display.print(" degrees");

  display.drawLine((TANK_WIDTH / 2), (SCREEN_HEIGHT - TANK_HEIGHT), turret_endpoint_x, turret_endpoint_y, SSD1306_WHITE);
}

//Calculate and animate projectile trajectory
void simulate_projectile()
{

  static float projectile_position_x_pixel = (TANK_WIDTH / 2);
  static float projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
  float projectile_delta_xy = 5;
  float projectile_delta_x = projectile_delta_xy * cos(TURRET_ANGLE_DEGREES * (PI / 180));
  float projectile_delta_y = projectile_delta_xy * sin(TURRET_ANGLE_DEGREES * (PI / 180));
  int32_t MAX_X_RANGE_PIXELS = ARENA_WIDTH_PIXELS;
  int32_t MAX_Y_RANGE_PIXELS = ARENA_HEIGHT_PIXELS;

  display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 1, SSD1306_WHITE);
  
  projectile_position_x_pixel = projectile_position_x_pixel + projectile_delta_x;
  projectile_position_y_pixel = projectile_position_y_pixel - projectile_delta_y;
  
  
  Serial.print(projectile_position_x_pixel);
  Serial.print("\t");
  Serial.println(-1 * projectile_position_y_pixel);

  //Limit projectile travel in the X and Y axes to prevent a non-terminating loop
  if (projectile_position_x_pixel >= MAX_X_RANGE_PIXELS || - projectile_position_y_pixel >= MAX_Y_RANGE_PIXELS){

    player_fired_flag = false;
    projectile_position_x_pixel = (TANK_WIDTH / 2);
    projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);

  }

}


void ISR_button_press_a()//Interrupt service routine that handles print frequency request one the push button is pressed
{

  noInterrupts(); //Prevent other interrupts

  //Local variable declarations
  static uint32_t previous_press_time_a_ms = 0;   //Previous time noted for debouncing
  uint32_t current_press_time_a_ms = millis();  //Current time noted for debouncing

  if ((current_press_time_a_ms - previous_press_time_a_ms) > DEBOUNCE_DELAY_MS) //Code segment that handles noting of frequency print request, with button debounce
  {
    player_fired_flag = true;
    previous_press_time_a_ms = current_press_time_a_ms; //Updates previous time to facilitate correct debounce timing
  }
  interrupts(); //Re-enable other interrupts
}


#define XPOS 0 // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2

void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h)
{
  int8_t f, icons[NUMFLAKES][3];
  static float current_time_ms = 0;
  static float previous_time_ms = 0;
  static float fps_value = 0;

  // Initialize 'snowflake' positions
  for (f = 0; f < NUMFLAKES; f++)
  {
    icons[f][XPOS] = random(1 - TANK_WIDTH, display.width());
    icons[f][YPOS] = -TANK_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  for (;;)
  { // Loop forever...
    previous_time_ms = current_time_ms;
    display.clearDisplay(); // Clear the display buffer
    // Draw each snowflake:
    for (f = 0; f < NUMFLAKES; f++)
    {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    //Calculate and display frame rate
    display.setTextSize(1); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(95, 0);
    current_time_ms = millis();
    fps_value = 1 / ((current_time_ms - previous_time_ms) / 1000);
    display.println(fps_value);
    display.display(); // Show the display buffer on the screen

    // Then update coordinates of each flake...
    for (f = 0; f < NUMFLAKES; f++)
    {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height())
      {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS] = random(1 - TANK_WIDTH, display.width());
        icons[f][YPOS] = -TANK_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}