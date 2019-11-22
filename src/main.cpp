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

#define NUMFLAKES 10                // Number of snowflakes in the animation example
static float turret_endpoint_x = 0; //Turret endpoint X coordinate
static float turret_endpoint_y = 0; //Turret endpoint Y coordinate

static float TURRET_ANGLE_DEGREES = 0; //Turret angle in degrees
uint8_t TURRET_LENGTH_PIXELS = 10;     //Turret length in Pixels

static int16_t PLANE_X_POSITION = SCREEN_WIDTH;
static int16_t PLANE_Y_POSITION = 0;

static int32_t player_score = 0;

bool player_fired_flag = false;
bool plane_hit_flag = false;

//Function prototypes
void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h);
void draw_tank();
void draw_plane();
void display_player_score();
void calculate_turret_orientation();
void draw_turret();
void simulate_projectile();
void ISR_button_press_a();

//Bitmap Sprites
#define TANK_HEIGHT 8
#define TANK_WIDTH 16
const unsigned char TANK_BMP[] =
    {

        B00001111, B11110000,
        B00001111, B11110000,
        B00111111, B11111100,
        B01111111, B11111110,
        B11111111, B11111111,
        B11111111, B11111111,
        B10101010, B01010101,
        B11101110, B01110111};

#define PLANE_WIDTH 16
#define PLANE_HEIGHT 8
const unsigned char PLANE_BMP[] =
    {
        B00001100, B00000011,
        B10010010, B00000111,
        B10111111, B11111111,
        B11111111, B11111111,
        B11111000, B00111000,
        B10111111, B11111111,
        B10111111, B00000000,
        B00011110, B00000000};

const unsigned char PLANE_DESTROYED_BMP[] =
    {
        B00001100, B00000011,
        B10010010, B00000011,
        B10111111, B00001111,
        B11111111, B00100111,
        B11110000, B00111000,
        B10100000, B01111111,
        B10000001, B00000000,
        B00000110, B00000000};

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
  //display.display();
  //delay(2000);
}

void loop()
{

  display.clearDisplay();

  draw_tank();
  draw_plane();
  display_player_score();

  if (player_fired_flag == false)
  {
    calculate_turret_orientation();
  }

  draw_turret();

  if (player_fired_flag == true)
  {
    simulate_projectile();
  }

  display.display();
}

void draw_tank()
{

  display.drawBitmap(0, (SCREEN_HEIGHT - TANK_HEIGHT), TANK_BMP, TANK_WIDTH, TANK_HEIGHT, 1);
}

//Draw and animate planes, simulate plane destruction when hit
void draw_plane()
{

  static uint8_t PLANE_DELTA_X = random(1, 3);
  const uint8_t CRASH_SPEED_Y = 4;
  static bool player_score_added = false;
  enum plane_states //Define possible plane states
  {
    FRESH,
    DAMAGED,
    DESTROYED,
  };
  static enum plane_states current_plane_state = FRESH;

  switch (current_plane_state)
  {

  case FRESH:
    //Draw and animate plane
    display.drawBitmap(PLANE_X_POSITION, PLANE_Y_POSITION, PLANE_BMP, PLANE_WIDTH, PLANE_HEIGHT, 1);
    PLANE_X_POSITION = PLANE_X_POSITION - PLANE_DELTA_X;
    //Damage plane when hit
    if (plane_hit_flag == true)
    {
      current_plane_state = DAMAGED;
      plane_hit_flag = false;
    }
    break;

  case DAMAGED:
    //Draw and animate plane
    display.drawBitmap(PLANE_X_POSITION, PLANE_Y_POSITION, PLANE_BMP, PLANE_WIDTH, PLANE_HEIGHT, 1);
    //Draw and animate smoke on plane
    display.drawCircle((PLANE_X_POSITION + PLANE_WIDTH), (PLANE_Y_POSITION + PLANE_HEIGHT / 2), 1, SSD1306_WHITE);
    PLANE_X_POSITION = PLANE_X_POSITION - PLANE_DELTA_X;
    //Destroy plane when it is hit in its damaged state
    if (plane_hit_flag == true)
    {
      current_plane_state = DESTROYED;
      plane_hit_flag = false;
    }
    break;

  case DESTROYED:
    //Draw and animate destroyed plane
    display.drawBitmap(PLANE_X_POSITION, PLANE_Y_POSITION, PLANE_DESTROYED_BMP, PLANE_WIDTH, PLANE_HEIGHT, 1);
    PLANE_Y_POSITION = PLANE_Y_POSITION + CRASH_SPEED_Y;
    //Add 1 to player score if it hasn't been added to already
    if (player_score_added == false)
    {
      player_score = player_score + 1;
      player_score_added = true;
    }

    break;
  }

  //Prepare to spawn new plane once plane exits screen
  if ((PLANE_X_POSITION <= -PLANE_WIDTH) || (PLANE_Y_POSITION >= SCREEN_HEIGHT))
  {
    PLANE_X_POSITION = SCREEN_WIDTH;
    PLANE_DELTA_X = random(1, 3);
    PLANE_Y_POSITION = (random(0, 3) * PLANE_HEIGHT);
    current_plane_state = FRESH;
    player_score_added = false;
  }
}

//Display player score
void display_player_score()
{
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0.5 * SCREEN_WIDTH, SCREEN_HEIGHT - 8);
  display.print("SCORE: ");
  display.println(player_score);
}

//Calculate tank turret orientation
void calculate_turret_orientation()
{
  TURRET_ANGLE_DEGREES = map(analogRead(ANALOG_INPUT_PIN), 0, 1023, 0, 89); //Map turret angle to analog input pin

  //Calculate turret endpoint using turret angle in degrees
  turret_endpoint_x = TURRET_LENGTH_PIXELS * cos(TURRET_ANGLE_DEGREES * (PI / 180)) + (TANK_WIDTH / 2);
  turret_endpoint_y = (SCREEN_HEIGHT - TANK_HEIGHT) - (TURRET_LENGTH_PIXELS * sin(TURRET_ANGLE_DEGREES * (PI / 180)));
}

//Draw tank turret in the correct orientation
void draw_turret()
{

  display.drawLine((TANK_WIDTH / 2), (SCREEN_HEIGHT - TANK_HEIGHT), turret_endpoint_x, turret_endpoint_y, SSD1306_WHITE);
}

//Animate projectile trajectory and check for hits
void simulate_projectile()
{

  static float projectile_position_x_pixel = (TANK_WIDTH / 2);
  static float projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
  float projectile_delta_xy = PLANE_HEIGHT / 2;                                            //Projectile speed, set to a value smaller than the plane height to avoid skipping the plane
  float projectile_delta_x = projectile_delta_xy * cos(TURRET_ANGLE_DEGREES * (PI / 180)); //X component of projectile speed
  float projectile_delta_y = projectile_delta_xy * sin(TURRET_ANGLE_DEGREES * (PI / 180)); //Y component of projectile speed
  int32_t MAX_X_RANGE_PIXELS = SCREEN_WIDTH;                                               //Set projectile max range to that of the screen size
  int32_t MAX_Y_RANGE_PIXELS = SCREEN_HEIGHT;                                              //Set projectile max range to that of the screen size

  //Draw projectile
  display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 1, SSD1306_WHITE);

  //Update projectile position
  projectile_position_x_pixel = projectile_position_x_pixel + projectile_delta_x;
  projectile_position_y_pixel = projectile_position_y_pixel - projectile_delta_y;

  //Limit projectile travel in the X and Y axes to prevent a non-terminating loop
  if (projectile_position_x_pixel >= MAX_X_RANGE_PIXELS || -projectile_position_y_pixel >= MAX_Y_RANGE_PIXELS)
  {

    player_fired_flag = false;
    projectile_position_x_pixel = (TANK_WIDTH / 2);
    projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
  }

  //Check if projectile hits
  if ((projectile_position_x_pixel >= PLANE_X_POSITION) && (projectile_position_x_pixel <= (PLANE_X_POSITION + PLANE_WIDTH)))
  {
    if ((projectile_position_y_pixel >= PLANE_Y_POSITION) && (projectile_position_y_pixel <= (PLANE_Y_POSITION + PLANE_HEIGHT)))
    {
      //Change projectile to its exploded version
      display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 8, SSD1306_WHITE);
      //Allow player to fire once again
      player_fired_flag = false;
      //Reset projectile position
      projectile_position_x_pixel = (TANK_WIDTH / 2);
      projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
      //Register plane hit
      plane_hit_flag = true;
    }
  }
}

void ISR_button_press_a() //Interrupt service routine that handles print frequency request one the push button is pressed
{

  noInterrupts(); //Prevent other interrupts

  //Local variable declarations
  static uint32_t previous_press_time_a_ms = 0; //Previous time noted for debouncing
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