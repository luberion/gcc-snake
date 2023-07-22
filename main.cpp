#include <iostream>
#include <random>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <fstream>

using std::cin;
using std::cout;
using std::endl;

const int BOARD_WIDTH = 20;
const int BOARD_HEIGHT = 20;
const int MAX_TAIL_LENGTH = 100;

bool gameOver;
int x, y, fruitX, fruitY, score;
int tailX[MAX_TAIL_LENGTH], tailY[MAX_TAIL_LENGTH];
int nTail;
int lives;           // New variable for life system
int highScore = 0;   // Variable to store the high score
int level = 1;       // Variable to store the current level
int speed = 100;     // Variable to control the speed of the snake
bool paused = false; // Variable to track the pause state

enum class Direction
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

Direction dir;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> disX(0, BOARD_WIDTH - 1);
std::uniform_int_distribution<> disY(0, BOARD_HEIGHT - 1);

void Setup() {
    gameOver = false;
    dir = Direction::STOP;
    x = BOARD_WIDTH / 2;
    y = BOARD_HEIGHT / 2;
    fruitX = disX(gen);
    fruitY = disY(gen);
    score = 0;
    nTail = 0;
    lives = 3; // Initialize lives to 3

    std::ifstream highScoreFile("highscore.txt");
    if (highScoreFile.is_open()) {
        highScoreFile >> highScore;
        highScoreFile.close();
    }
}

void Draw() {
    system("clear");
    for (int i = 0; i < BOARD_WIDTH + 2; i++) {
        cout << "#";
    }
    cout << endl;

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (j == 0) {
                cout << "#";
            }
            if (i == y && j == x) {
                cout << "O";
            } else if (i == fruitY && j == fruitX) {
                cout << "F";
            } else {
                bool printTail = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        cout << "o";
                        printTail = true;
                    }
                }
                if (!printTail) {
                    cout << " ";
                }
            }

            if (j == BOARD_WIDTH - 1) {
                cout << "#";
            }
        }
        cout << endl;
    }

    for (int i = 0; i < BOARD_WIDTH + 2; i++) {
        cout << "#";
    }
    cout << endl;
    cout << "Score:" << score << endl;
    cout << "Lives:" << lives << endl;
    cout << "High Score:" << highScore << endl;
    cout << "Level:" << level << endl;
}

void Input() {
    int key;
    struct termios oldSettings, newSettings;
    tcgetattr(STDIN_FILENO, &oldSettings);
    newSettings = oldSettings;
    newSettings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);
    key = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);

    switch (key) {
    case 'a':
        if (dir != Direction::RIGHT) {
            dir = Direction::LEFT;
        }
        break;
    case 'd':
        if (dir != Direction::LEFT) {
            dir = Direction::RIGHT;
        }
        break;
    case 'w':
        if (dir != Direction::DOWN) {
            dir = Direction::UP;
        }
        break;
    case 's':
        if (dir != Direction::UP) {
            dir = Direction::DOWN;
        }
        break;
    case 'x':
        lives = 0;
        break;
    case 'p':
    case 'P':
        paused = !paused;
        break;
    }
}

void UpdateSnakePosition() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
    case Direction::LEFT:
        x--;
        break;
    case Direction::RIGHT:
        x++;
        break;
    case Direction::UP:
        y--;
        break;
    case Direction::DOWN:
        y++;
        break;
    default:
        break;
    }

    if (x >= BOARD_WIDTH) {
        x = 0;
    } else if (x < 0) {
        x = BOARD_WIDTH - 1;
    }

    if (y >= BOARD_HEIGHT) {
        y = 0;
    } else if (y < 0) {
        y = BOARD_HEIGHT - 1;
    }

    if (!paused) {
        std::this_thread::sleep_for(std::chrono::milliseconds(speed));
    }
}

bool CheckCollision() {
    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y) {
            lives--;
            if (lives == 0) {
                return true;
            }
        }
    }
    return false;
}

void CheckFruitCollision() {
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = disX(gen);
        fruitY = disY(gen);
        nTail++;

        if (score > highScore) {
            highScore = score;
            std::ofstream highScoreFile("highscore.txt");
            if (highScoreFile.is_open()) {
                highScoreFile << highScore;
                highScoreFile.close();
            }
        }

        if (score % 50 == 0) {
            level++;
            speed -= 10;
            if (speed < 50) {
                speed = 50;
            }
        }
    }
}

void GameOver() {
    cout << "Game Over!" << endl;
    cout << "Play again? (y/n): ";
    char choice;
    cin >> choice;

    if (choice != 'y' && choice != 'Y') {
        gameOver = true;
    }
}

int main() {
    bool playAgain = true;

    while (playAgain) {
        Setup();
        while (!gameOver) {
            Draw();
            Input();
            UpdateSnakePosition();
            if (CheckCollision()) {
                gameOver = true;
            } else {
                CheckFruitCollision();
            }
        }

        GameOver();
    }

    return 0;
}
