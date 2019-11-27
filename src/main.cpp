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
#include <BITMAPS.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Input pin declarations
#define ANALOG_INPUT_PIN 17
#define BUTTON_INPUT_A 12
#define BUTTON_INPUT_B 9

//Global variables, constants, and state flags
uint16_t DEBOUNCE_DELAY_MS = 200;
float turret_endpoint_x = 0;                                       //Turret endpoint X coordinate
float turret_endpoint_y = 0;                                       //Turret endpoint Y coordinate
float projectile_position_x_pixel = (TANK_WIDTH / 2);              //X coordinate of projectile
float projectile_position_y_pixel = (SCREEN_HEIGHT - TANK_HEIGHT); //Y coordinate of projectile
static float turret_angle_degrees = 0;                             //Turret angle in degrees
uint8_t TURRET_LENGTH_PIXELS = 10;                                 //Turret length in Pixels
int16_t PLANE_X_POSITION = SCREEN_WIDTH;                           //X coordinate of plane position
int16_t PLANE_Y_POSITION = 0;                                      //Y coordinate of plane position
uint32_t player_score = 0;                                         //Accumulate player score
uint8_t player_life = 3;                                           //Number of player lives
bool button_a_pressed = false;                                     //Flag for 'a' button state
bool button_b_pressed = false;                                     //Flag for 'b' button state
bool player_fired_flag = false;                                    //Flag to indicate that player has fired
bool plane_hit_flag = false;                                       //Flag to signify plane is hit
bool projectile_launched = false;                                  //Flag to indicate that projectile is in-flight

//Function prototypes
void reset_data();
void draw_start_screen();
void draw_pause_screen();
void draw_game_over_screen();
void draw_background();
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

//Initial setup
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

  display.clearDisplay(); //Clear OLED display

  switch (current_game_state) //Overall state machine governing the game
  {

  case START_SCREEN:
    draw_start_screen();
    reset_data(); //Reset variables for a new game

    //Set game state to PLAYING
    if (button_a_pressed == true)
    {
      button_a_pressed = false;
      current_game_state = PLAYING;
    }

    break;

  case PLAYING:

    //Set game state to PAUSED
    if (button_b_pressed == true)
    {
      button_b_pressed = false;
      current_game_state = PAUSED;
    }

    draw_background();
    draw_tank();
    draw_plane();
    display_player_score_and_life();
    calculate_turret_orientation();
    draw_turret();

    //Launch a projectile
    if (button_a_pressed == true)
    {
      player_fired_flag = true;
      button_a_pressed = false;
    }

    //Simulate projectile flight
    if (player_fired_flag == true)
    {
      simulate_projectile();
    }

    //Set game state to GAME_OVER when player life reaches 0
    if (player_life == 0)
    {
      current_game_state = GAME_OVER;
    }

    break;

  case PAUSED:
    draw_pause_screen();

    //Reset game
    if (button_a_pressed == true)
    {

      button_a_pressed = false;
      current_game_state = START_SCREEN;
    }

    //Resume game
    if (button_b_pressed == true)
    {

      button_b_pressed = false;
      current_game_state = PLAYING;
    }
    break;

  case GAME_OVER:
    //Signify that the game has ended
    draw_game_over_screen();
    //Restart the game
    if (button_a_pressed == true || button_b_pressed == true)
    {

      button_a_pressed = false;
      button_b_pressed = false;
      current_game_state = START_SCREEN;
    }

    break;
  }

  //Update displayed pixels on OLED
  display.display();
}

//Reset variables
void reset_data()
{
  player_score = 0;
  player_life = 3;
  player_fired_flag = false;
  projectile_launched = false;
  PLANE_X_POSITION = SCREEN_WIDTH;
  PLANE_Y_POSITION = (random(0, 3) * PLANE_HEIGHT);
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
void draw_background()
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

//Draw pause screen
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

//Draw game over screen
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

//Draw and animate planes, simulate plane damage and destruction when hit
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

  //Prepare to spawn new plane once plane exits screen, subtract 1 life if plane that exits is damaged or fresh
  if ((PLANE_X_POSITION <= -PLANE_WIDTH) || (PLANE_Y_POSITION >= SCREEN_HEIGHT))
  {

    //Subtract one life from player if fresh or damaged plane exits the screen
    if (current_plane_state != DESTROYED)
    {
      player_life = player_life - 1;
    }

    PLANE_X_POSITION = SCREEN_WIDTH;
    PLANE_DELTA_X = random(1, 3);
    PLANE_Y_POSITION = (random(0, 3) * PLANE_HEIGHT);
    current_plane_state = FRESH;
    player_score_added = false;
  }
}

//Display player score
void display_player_score_and_life()
{
  display.setTextSize(1);
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

    display.drawCircle(projectile_position_x_pixel, projectile_position_y_pixel, 1, SSD1306_WHITE); //Draw projectile

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

void ISR_button_press_a() //Interrupt service routine that handles inputs from button 'a'
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

void ISR_button_press_b() //Interrupt service routine that handles inputs from button 'b'
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