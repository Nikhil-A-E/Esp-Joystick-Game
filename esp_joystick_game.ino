#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Joystick pins
#define JOY_X 34
#define JOY_SW 32

// Paddle
int paddleX = 54;
const int paddleWidth = 20;
const int paddleHeight = 3;

// Ball
float ballX = 64, ballY = 40;
float ballSpeedX = 1.5, ballSpeedY = -1.5;
const int ballSize = 3;

// Blocks
const int blockRows = 3;
const int blockCols = 8;
const int blockWidth = 16;
const int blockHeight = 6;

bool blocks[blockRows][blockCols];
bool specialBlock[blockRows][blockCols];  // Bonus blocks

// Game state
bool gameOver = false;
bool youWin = false;
int score = 0;

unsigned long lastUpdate = 0;
const int updateInterval = 30;

void resetGame() {
  paddleX = 54;
  ballX = 64;
  ballY = 40;
  ballSpeedX = 1.5;
  ballSpeedY = -1.5;
  gameOver = false;
  youWin = false;
  score = 0;

  for (int r = 0; r < blockRows; r++) {
    for (int c = 0; c < blockCols; c++) {
      blocks[r][c] = true;
      specialBlock[r][c] = (random(0, 10) < 2); // ~20% chance of bonus block
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(JOY_X, INPUT);
  pinMode(JOY_SW, INPUT_PULLUP);
  randomSeed(analogRead(0));

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }

  resetGame();
}

void loop() {
  if (!gameOver && !youWin) {
    if (millis() - lastUpdate > updateInterval) {
      updateJoystick();
      updateBall();
      drawGame();
      lastUpdate = millis();
    }
  } else {
    drawEndScreen();
    if (digitalRead(JOY_SW) == LOW) {
      delay(300); // debounce
      resetGame();
    }
  }
}

void updateJoystick() {
  int xVal = analogRead(JOY_X);

  if (xVal < 1800 && paddleX > 0) {
    paddleX -= 4;
  } else if (xVal > 2200 && paddleX < SCREEN_WIDTH - paddleWidth) {
    paddleX += 4;
  }
}

void updateBall() {
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  if (ballX <= 0 || ballX >= SCREEN_WIDTH - ballSize)
    ballSpeedX *= -1;

  if (ballY <= 0)
    ballSpeedY *= -1;

  // Paddle bounce
  if (ballY >= SCREEN_HEIGHT - paddleHeight - ballSize &&
      ballX + ballSize >= paddleX && ballX <= paddleX + paddleWidth) {
    ballSpeedY = -abs(ballSpeedY);
  }

  // Block collision
  for (int r = 0; r < blockRows; r++) {
    for (int c = 0; c < blockCols; c++) {
      if (blocks[r][c]) {
        int bx = c * blockWidth;
        int by = r * blockHeight + 10;
        if (ballX + ballSize > bx && ballX < bx + blockWidth &&
            ballY + ballSize > by && ballY < by + blockHeight) {
          blocks[r][c] = false;
          ballSpeedY *= -1;
          score += specialBlock[r][c] ? 5 : 1;
        }
      }
    }
  }

  if (score >= (blockRows * blockCols) + (countSpecials() * 4)) {
    youWin = true;
  }

  if (ballY > SCREEN_HEIGHT) {
    gameOver = true;
  }
}

int countSpecials() {
  int count = 0;
  for (int r = 0; r < blockRows; r++)
    for (int c = 0; c < blockCols; c++)
      if (specialBlock[r][c]) count++;
  return count;
}

void drawGame() {
  display.clearDisplay();

  // Draw paddle
  display.fillRect(paddleX, SCREEN_HEIGHT - paddleHeight, paddleWidth, paddleHeight, SSD1306_WHITE);

  // Draw ball
  display.fillRect((int)ballX, (int)ballY, ballSize, ballSize, SSD1306_WHITE);

  // Draw blocks
  for (int r = 0; r < blockRows; r++) {
    for (int c = 0; c < blockCols; c++) {
      if (blocks[r][c]) {
        int x = c * blockWidth;
        int y = r * blockHeight + 10;

        if (specialBlock[r][c]) {
          // Striped bonus block (simulate diff color)
          for (int i = 0; i < blockHeight; i += 2) {
            display.drawFastHLine(x, y + i, blockWidth - 1, SSD1306_WHITE);
          }
        } else {
          display.fillRect(x, y, blockWidth - 1, blockHeight - 1, SSD1306_WHITE);
        }
      }
    }
  }

  // Score
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Score: ");
  display.print(score);

  display.display();
}

void drawEndScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(25, 20);
  display.print(youWin ? "You Win! :D" : "Game Over ;C");
  display.setCursor(15, 40);
  display.print("Press to Restart");
  display.setCursor(35, 55);
  display.print("Score: ");
  display.print(score);
  display.display();
}
