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
#define BUTTON_INPUT_B 9

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

#define LOGO_WIDTH 32
#define LOGO_HEIGHT 24
const unsigned char LOGO_BMP[] =
    {
        B11111111, B11111111, B11111111, B11111111,
        B10000011, B11000000, B00000011, B11000001,
        B10000111, B11100000, B00000111, B11100001,
        B10000111, B11100000, B00000111, B11100001,
        B10000110, B01100000, B00000110, B01100001,
        B10000110, B01100000, B00000110, B01100001,
        B10001110, B01110000, B00001110, B01110001,
        B10001110, B01110000, B00001110, B01110001,
        B10001100, B00110000, B00001100, B00110001,
        B10011100, B00111000, B00011100, B00111001,
        B10011100, B00111000, B00011100, B00111001,
        B10011111, B11111000, B00011111, B11111001,
        B10011111, B11111000, B00011111, B11111001,
        B10111000, B00011100, B00111000, B00011101,
        B10111000, B00011100, B00111000, B00011101,
        B10110000, B00001100, B00110000, B00001101,
        B11110000, B00001110, B01110000, B00001111,
        B11110000, B00001110, B01110000, B00001111,
        B11100000, B00000110, B01100000, B00000111,
        B11100000, B00000110, B01100000, B00000111,
        B11100000, B00000111, B11100000, B00000111,
        B11100000, B00000111, B11100000, B00000111,
        B11000000, B00000011, B11000000, B00000011,
        B11111111, B11111111, B11111111, B11111111};

#define FOREGROUND_WIDTH 128
#define FOREGROUND_HEIGHT 64
const unsigned char FOREGROUND_BMP[] = {
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00011000, B00000000, B00000000, B00000011, B11111111, B11111110, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00011110, B00110000, B00000000, B00011110, B00000000, B00000010, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00001100, B00000000, B00000000, B00010011, B01111110, B00000001, B11110000, B00000000, B00000010, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000011, B11100000, B00111111, B11100000, B00000000, B00010001, B11000011, B00000011, B00000000, B00000000, B00000010, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00001110, B00111111, B11100000, B01010000, B00000000, B00010000, B00000001, B11001110, B00000000, B00000000, B00000010, B00000000, B00000000, B00000000,
    B01111000, B00000000, B01111010, B00000000, B00000000, B01001000, B00000000, B00010000, B00000000, B01111000, B00000000, B00000000, B00000010, B00000000, B00000000, B00000000,
    B11001111, B00001111, B10000010, B00000000, B00000000, B01001000, B00000000, B00010001, B11100111, B10000111, B10011110, B01111111, B11100010, B00000000, B00000000, B00000000,
    B10000001, B11110000, B00000010, B11110011, B11001111, B01000100, B00000000, B00010001, B00100100, B10000100, B10010010, B01000000, B00100010, B00000000, B00000000, B00000000,
    B10000000, B00000000, B00000010, B10010010, B01001001, B01000010, B00000000, B00010001, B00100100, B10000100, B10010010, B01000000, B00100010, B11110000, B00000000, B00000000,
    B10011110, B11111110, B11110010, B10010010, B01001001, B01000011, B00000000, B00010001, B11100111, B10000111, B10011110, B01111111, B11100011, B10011000, B00000000, B00000000,
    B10010010, B10000010, B10010010, B11110011, B11001111, B01000001, B11000000, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B00001111, B00000000, B00000000,
    B10010010, B10000010, B10010010, B00000000, B00000000, B01000000, B01111111, B11110000, B00000000, B00000000, B00000000, B00000000, B00000010, B00000001, B10000000, B00000000,
    B10011110, B11111110, B11110010, B00000000, B00000000, B01000000, B00000000, B00110000, B00000000, B00000000, B00000000, B00000000, B00000010, B11110000, B11100000, B00001111,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01011110, B00001111, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B10010000, B00100000, B00011001,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01010010, B00001001, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B10010000, B00100000, B00110001,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01010010, B00001001, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B11110000, B00110011, B11100001,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01011110, B00001111, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B00000000, B00011110, B00000001,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01000000, B00000000, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B00000000, B00000000, B00000001,
    B10000000, B00000000, B00000010, B00000000, B00000000, B01000000, B00000000, B00010000, B00000000, B00000000, B00000000, B00000000, B00000010, B00000000, B00000000, B00000001,
    B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B11111111,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000};

//Global variables, constants, and state flags
uint16_t DEBOUNCE_DELAY_MS = 200;

float turret_endpoint_x = 0; //Turret endpoint X coordinate
float turret_endpoint_y = 0; //Turret endpoint Y coordinate

float projectile_position_x_pixel = (TANK_WIDTH / 2);
float projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);

static float turret_angle_degrees = 0; //Turret angle in degrees
uint8_t TURRET_LENGTH_PIXELS = 10;     //Turret length in Pixels

int16_t PLANE_X_POSITION = SCREEN_WIDTH;
int16_t PLANE_Y_POSITION = 0;

uint32_t player_score = 0;
uint8_t player_life = 3;

bool button_a_pressed = false;
bool button_b_pressed = false;
bool player_fired_flag = false;
bool plane_hit_flag = false;

//Function prototypes
void reset_data();
void draw_start_screen();
void draw_pause_screen();
void draw_game_over_screen();
void draw_foreground();
void draw_tank();
void draw_plane();
void display_player_score_and_life();
void calculate_turret_orientation();
void draw_turret();
void simulate_projectile();
void ISR_button_press_a();
void ISR_button_press_b();

//Declaration of possible states
enum game_states //Define possible game states
{
  START_SCREEN,
  PLAYING,
  PAUSED,
  GAME_OVER,
};
static enum game_states current_game_state = START_SCREEN;

enum plane_states //Define possible plane states
{
  FRESH,
  DAMAGED,
  DESTROYED,
};
static enum plane_states current_plane_state = FRESH;

void setup()
{
  Serial.begin(9600);

  //Set pin modes and interrupts
  pinMode(ANALOG_INPUT_PIN, INPUT);
  pinMode(BUTTON_INPUT_A, INPUT_PULLDOWN);
  pinMode(BUTTON_INPUT_B, INPUT_PULLDOWN);
  attachInterrupt(BUTTON_INPUT_A, ISR_button_press_a, RISING);
  attachInterrupt(BUTTON_INPUT_B, ISR_button_press_b, RISING);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  display.display();
  delay(2000);
}

void loop()
{

  display.clearDisplay();

  switch (current_game_state)
  {

  case START_SCREEN:
    draw_start_screen();
    reset_data(); //Reset variables

    if (button_a_pressed == true)
    {
      button_a_pressed = false;
      current_game_state = PLAYING;
    }

    break;

  case PLAYING:

    if (button_b_pressed == true)
    {
      button_b_pressed = false;
      current_game_state = PAUSED;
    }

    draw_foreground();
    draw_tank();
    draw_plane();
    display_player_score_and_life();
    calculate_turret_orientation();
    draw_turret();

    if (button_a_pressed == true)
    {
      player_fired_flag = true;
      button_a_pressed = false;
    }

    if (player_fired_flag == true)
    {
      simulate_projectile();
    }

    if (player_life == 0)
    {
      current_game_state = GAME_OVER;
    }

    break;

  case PAUSED:
    draw_pause_screen();
    if (button_a_pressed == true)
    {

      button_a_pressed = false;
      current_game_state = START_SCREEN;
    }
    if (button_b_pressed == true)
    {

      button_b_pressed = false;
      current_game_state = PLAYING;
    }
    break;

  case GAME_OVER:
    draw_game_over_screen();
    if (button_a_pressed == true || button_b_pressed == true)
    {

      button_a_pressed = false;
      button_b_pressed = false;
      current_game_state = START_SCREEN;
    }

    break;
  }

  display.display();
}

//Reset variables
void reset_data()
{
  player_score = 0;
  player_life = 3;
  player_fired_flag = false;
  PLANE_X_POSITION = SCREEN_WIDTH;
  PLANE_Y_POSITION = (random(0, 4) * PLANE_HEIGHT);
  projectile_position_x_pixel = (TANK_WIDTH / 2);
  projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
  current_plane_state = FRESH;
}

//Draw tank body
void draw_tank()
{

  display.drawBitmap(0, (SCREEN_HEIGHT - TANK_HEIGHT), TANK_BMP, TANK_WIDTH, TANK_HEIGHT, 1);
}

//Draw foreground
void draw_foreground()
{

  display.drawBitmap(0, -5, FOREGROUND_BMP, FOREGROUND_WIDTH, FOREGROUND_HEIGHT, 1);
}

//Draw title/start screen
void draw_start_screen()
{

  display.drawBitmap(48, 0, LOGO_BMP, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 30);
  display.println("AIR DEFENCE SIMULATOR");
  display.setCursor(20, 50);
  display.println("Push A to start");
}

void draw_pause_screen()
{

  display.drawBitmap(48, 0, LOGO_BMP, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 50);
  display.println("Push B to resume");
  display.setCursor(0, 30);
  display.println("Push A to return to");
  display.println("start screen");
}

void draw_game_over_screen()
{
  display.drawBitmap(48, 0, LOGO_BMP, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 30);
  display.println("GAME OVER");
  display.setCursor(0, 40);
  display.print("FINAL SCORE: ");
  display.println(player_score);
  display.println("Push any button to");
  display.println("restart the game");
}

//Draw and animate planes, simulate plane destruction when hit
void draw_plane()
{

  static uint8_t PLANE_DELTA_X = random(1, 3);
  const uint8_t CRASH_SPEED_Y = 4;
  static bool player_score_added = false;

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

    display.drawBitmap(PLANE_X_POSITION, PLANE_Y_POSITION, PLANE_BMP, PLANE_WIDTH, PLANE_HEIGHT, 1); //Draw and animate plane

    //Draw smoke on plane
    for (uint8_t smoke_i = 1; smoke_i <= 4; smoke_i++)
    {

      display.drawCircle((PLANE_X_POSITION + ((PLANE_WIDTH + 4) * smoke_i)), (PLANE_Y_POSITION + PLANE_HEIGHT / 2), (4 - smoke_i), SSD1306_WHITE);
    }

    PLANE_X_POSITION = PLANE_X_POSITION - PLANE_DELTA_X; //Move plane

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

  //Prepare to spawn new plane once plane exits screen, subtract 1 life
  if ((PLANE_X_POSITION <= -PLANE_WIDTH) || (PLANE_Y_POSITION >= SCREEN_HEIGHT))
  {

    //Subtract one life from player if fresh or damaged plane exits the screen
    if (current_plane_state != DESTROYED)
    {
      player_life = player_life - 1;
    }

    PLANE_X_POSITION = SCREEN_WIDTH;
    PLANE_DELTA_X = random(1, 3);
    PLANE_Y_POSITION = (random(0, 4) * PLANE_HEIGHT);
    current_plane_state = FRESH;
    player_score_added = false;
  }
}

//Display player score
void display_player_score_and_life()
{
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0.45 * SCREEN_WIDTH, SCREEN_HEIGHT - 17);
  display.print("LIVES: ");
  display.println(player_life);
  display.setCursor(0.45 * SCREEN_WIDTH, SCREEN_HEIGHT - 7);
  display.print("SCORE: ");
  display.println(player_score);
}

//Calculate tank turret orientation
void calculate_turret_orientation()
{
  turret_angle_degrees = map(analogRead(ANALOG_INPUT_PIN), 0, 1023, 0, 90); //Map turret angle to analog input pin

  //Calculate turret endpoint using turret angle in degrees
  turret_endpoint_x = TURRET_LENGTH_PIXELS * cos(turret_angle_degrees * (PI / 180)) + (TANK_WIDTH / 2);
  turret_endpoint_y = (SCREEN_HEIGHT - TANK_HEIGHT) - (TURRET_LENGTH_PIXELS * sin(turret_angle_degrees * (PI / 180)));
}

//Draw tank turret in the correct orientation
void draw_turret()
{

  display.drawLine((TANK_WIDTH / 2), (SCREEN_HEIGHT - TANK_HEIGHT), turret_endpoint_x, turret_endpoint_y, SSD1306_WHITE);
}

//Animate projectile's linear trajectory based on turret angle upon launch and check for hits
void simulate_projectile()
{

  static float projectile_delta_x = 0;          //X component of projectile speed
  static float projectile_delta_y = 0;          //Y component of projectile speed
  static float projectile_angle_degrees = 0;    //Projectile angle decoupled from turret
  float projectile_delta_xy = PLANE_HEIGHT / 2; //Projectile speed, set to a value smaller than the plane height to avoid skipping the plane
  static bool projectile_launched = false;

  if (projectile_launched == false) //Set projectile angle to turret angle
  {
    projectile_angle_degrees = turret_angle_degrees;
    projectile_delta_x = projectile_delta_xy * cos(projectile_angle_degrees * (PI / 180)); //X component of projectile speed
    projectile_delta_y = projectile_delta_xy * sin(projectile_angle_degrees * (PI / 180)); //Y component of projectile speed
    projectile_launched = true;
  }

  else if (projectile_launched == true) //Decouple projectile angle from turret angle
  {

    projectile_delta_x = projectile_delta_xy * cos(projectile_angle_degrees * (PI / 180)); //X component of projectile speed
    projectile_delta_y = projectile_delta_xy * sin(projectile_angle_degrees * (PI / 180)); //Y component of projectile speed
  }

  int32_t MAX_X_RANGE_PIXELS = SCREEN_WIDTH;  //Set projectile max range to that of the screen size
  int32_t MAX_Y_RANGE_PIXELS = SCREEN_HEIGHT; //Set projectile max range to that of the screen size

  //Draw projectile
  display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 1, SSD1306_WHITE);

  //Update projectile position
  projectile_position_x_pixel = projectile_position_x_pixel + projectile_delta_x;
  projectile_position_y_pixel = projectile_position_y_pixel - projectile_delta_y;

  //Limit projectile travel in the X and Y axes to prevent a non-terminating loop, 'reload' cannon
  if (projectile_position_x_pixel >= MAX_X_RANGE_PIXELS || -projectile_position_y_pixel >= MAX_Y_RANGE_PIXELS)
  {

    projectile_launched = false; //'Despawn' projectile to allow the creation of a new one
    player_fired_flag = false;   //Allow player to fire once again
    projectile_position_x_pixel = (TANK_WIDTH / 2);
    projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);
  }

  //Check if projectile hits
  if ((projectile_position_x_pixel >= PLANE_X_POSITION) && (projectile_position_x_pixel <= (PLANE_X_POSITION + PLANE_WIDTH)))
  {
    if ((projectile_position_y_pixel >= PLANE_Y_POSITION) && (projectile_position_y_pixel <= (PLANE_Y_POSITION + PLANE_HEIGHT)))
    {

      display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 8, SSD1306_WHITE); //Change projectile to its exploded version

      projectile_launched = false; //'Despawn' projectile to allow the creation of a new one

      player_fired_flag = false; //Allow player to fire once again
      //Reset projectile position
      projectile_position_x_pixel = (TANK_WIDTH / 2);
      projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT);

      plane_hit_flag = true; //Register plane hit
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
    button_a_pressed = true;

    previous_press_time_a_ms = current_press_time_a_ms; //Updates previous time to facilitate correct debounce timing
  }
  interrupts(); //Re-enable other interrupts
}

void ISR_button_press_b() //Interrupt service routine that handles print frequency request one the push button is pressed
{

  noInterrupts(); //Prevent other interrupts

  //Local variable declarations
  static uint32_t previous_press_time_b_ms = 0; //Previous time noted for debouncing
  uint32_t current_press_time_b_ms = millis();  //Current time noted for debouncing

  if ((current_press_time_b_ms - previous_press_time_b_ms) > DEBOUNCE_DELAY_MS) //Code segment that handles noting of frequency print request, with button debounce
  {
    button_b_pressed = true;

    previous_press_time_b_ms = current_press_time_b_ms; //Updates previous time to facilitate correct debounce timing
  }
  interrupts(); //Re-enable other interrupts
}