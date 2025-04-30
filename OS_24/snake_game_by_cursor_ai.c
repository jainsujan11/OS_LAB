// i want you to build something amazing which leeaves me speechless 

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <stdbool.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE_LENGTH 100

// Game states
typedef enum {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
} GameState;

// Direction enum
typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// Snake structure
typedef struct {
    int x[MAX_SNAKE_LENGTH];
    int y[MAX_SNAKE_LENGTH];
    int length;
    Direction dir;
} Snake;

// Food structure
typedef struct {
    int x;
    int y;
    int type;  // 0: normal, 1: bonus, 2: speed boost
} Food;

// Game structure
typedef struct {
    Snake snake;
    Food food;
    int score;
    int highScore;
    GameState state;
    bool soundEnabled;
    int gameSpeed;
} Game;

// Function prototypes
void initGame(Game* game);
void drawBorder();
void drawSnake(Game* game);
void drawFood(Game* game);
void updateGame(Game* game);
void handleInput(Game* game);
void generateFood(Game* game);
void playSound(int frequency, int duration);
void showMenu(Game* game);
void showGameOver(Game* game);
void setCursorPosition(int x, int y);
void hideCursor();
void clearScreen();
bool setupConsole();

int main() {
    // Setup console for the game
    if (!setupConsole()) {
        printf("Failed to setup console. Press any key to exit...");
        getch();
        return 1;
    }

    Game game;
    srand(time(NULL));
    hideCursor();
    initGame(&game);
    
    // Draw initial border
    drawBorder();
    
    while (1) {
        switch (game.state) {
            case MENU:
                showMenu(&game);
                break;
            case PLAYING:
                // Only clear the game area, not the entire screen
                for (int y = 1; y <= HEIGHT; y++) {
                    for (int x = 1; x <= WIDTH; x++) {
                        setCursorPosition(x, y);
                        printf(" ");
                    }
                }
                
                drawSnake(&game);
                drawFood(&game);
                handleInput(&game);
                updateGame(&game);
                
                // Update score display
                setCursorPosition(2, HEIGHT + 2);
                printf("Score: %d | High Score: %d | Speed: %d", 
                       game.score, game.highScore, game.gameSpeed);
                Sleep(game.gameSpeed);
                break;
            case GAME_OVER:
                showGameOver(&game);
                break;
        }
    }
    return 0;
}

bool setupConsole() {
    // Get console handle
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        printf("Error: Could not get console handle\n");
        return false;
    }

    // Set console window size
    SMALL_RECT windowSize = {0, 0, WIDTH + 3, HEIGHT + 4};
    if (!SetConsoleWindowInfo(hConsole, TRUE, &windowSize)) {
        printf("Error: Could not set window size\n");
        return false;
    }

    // Set console buffer size
    COORD bufferSize = {WIDTH + 4, HEIGHT + 5};
    if (!SetConsoleScreenBufferSize(hConsole, bufferSize)) {
        printf("Error: Could not set buffer size\n");
        return false;
    }

    // Set console title
    SetConsoleTitle("Snake Game");

    // Clear screen
    system("cls");
    
    return true;
}

void initGame(Game* game) {
    game->snake.length = 3;
    game->snake.dir = RIGHT;
    game->score = 0;
    game->highScore = 0;
    game->state = MENU;
    game->soundEnabled = true;
    game->gameSpeed = 100;
    
    // Initialize snake position
    for (int i = 0; i < game->snake.length; i++) {
        game->snake.x[i] = WIDTH/2 - i;
        game->snake.y[i] = HEIGHT/2;
    }
    
    generateFood(game);
}

void drawBorder() {
    for (int i = 0; i < WIDTH + 2; i++) {
        setCursorPosition(i, 0);
        printf("#");
        setCursorPosition(i, HEIGHT + 1);
        printf("#");
    }
    for (int i = 0; i < HEIGHT + 2; i++) {
        setCursorPosition(0, i);
        printf("#");
        setCursorPosition(WIDTH + 1, i);
        printf("#");
    }
}

void drawSnake(Game* game) {
    for (int i = 0; i < game->snake.length; i++) {
        setCursorPosition(game->snake.x[i] + 1, game->snake.y[i] + 1);
        if (i == 0) {
            printf("O");  // Head
        } else {
            printf("o");  // Body
        }
    }
}

void drawFood(Game* game) {
    setCursorPosition(game->food.x + 1, game->food.y + 1);
    switch (game->food.type) {
        case 0:
            printf("*");  // Normal food
            break;
        case 1:
            printf("@");  // Bonus food
            break;
        case 2:
            printf("+");  // Speed boost
            break;
    }
}

void generateFood(Game* game) {
    bool validPosition;
    do {
        validPosition = true;
        game->food.x = rand() % WIDTH;
        game->food.y = rand() % HEIGHT;
        
        // Check if food spawns on snake
        for (int i = 0; i < game->snake.length; i++) {
            if (game->food.x == game->snake.x[i] && 
                game->food.y == game->snake.y[i]) {
                validPosition = false;
                break;
            }
        }
    } while (!validPosition);
    
    // Randomly choose food type
    game->food.type = rand() % 3;
}

void updateGame(Game* game) {
    // Move snake body
    for (int i = game->snake.length - 1; i > 0; i--) {
        game->snake.x[i] = game->snake.x[i-1];
        game->snake.y[i] = game->snake.y[i-1];
    }
    
    // Move snake head
    switch (game->snake.dir) {
        case UP:
            game->snake.y[0]--;
            break;
        case DOWN:
            game->snake.y[0]++;
            break;
        case LEFT:
            game->snake.x[0]--;
            break;
        case RIGHT:
            game->snake.x[0]++;
            break;
    }
    
    // Check collision with walls
    if (game->snake.x[0] < 0 || game->snake.x[0] >= WIDTH ||
        game->snake.y[0] < 0 || game->snake.y[0] >= HEIGHT) {
        game->state = GAME_OVER;
        return;
    }
    
    // Check collision with self
    for (int i = 1; i < game->snake.length; i++) {
        if (game->snake.x[0] == game->snake.x[i] && 
            game->snake.y[0] == game->snake.y[i]) {
            game->state = GAME_OVER;
            return;
        }
    }
    
    // Check food collision
    if (game->snake.x[0] == game->food.x && 
        game->snake.y[0] == game->food.y) {
        if (game->soundEnabled) {
            playSound(800, 100);
        }
        
        // Increase score based on food type
        switch (game->food.type) {
            case 0:
                game->score += 10;
                break;
            case 1:
                game->score += 30;
                break;
            case 2:
                game->score += 20;
                game->gameSpeed = (game->gameSpeed > 50) ? game->gameSpeed - 10 : 50;
                break;
        }
        
        // Update high score
        if (game->score > game->highScore) {
            game->highScore = game->score;
        }
        
        // Increase snake length
        if (game->snake.length < MAX_SNAKE_LENGTH) {
            game->snake.length++;
        }
        
        generateFood(game);
    }
}

void handleInput(Game* game) {
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
            case 'w':
                if (game->snake.dir != DOWN) game->snake.dir = UP;
                break;
            case 's':
                if (game->snake.dir != UP) game->snake.dir = DOWN;
                break;
            case 'a':
                if (game->snake.dir != RIGHT) game->snake.dir = LEFT;
                break;
            case 'd':
                if (game->snake.dir != LEFT) game->snake.dir = RIGHT;
                break;
            case 'p':
                game->state = PAUSED;
                break;
            case 'm':
                game->soundEnabled = !game->soundEnabled;
                break;
        }
    }
}

void playSound(int frequency, int duration) {
    Beep(frequency, duration);
}

void showMenu(Game* game) {
    clearScreen();
    printf("\n\n");
    printf("    +----------------------------------+\n");
    printf("    |           SNAKE GAME            |\n");
    printf("    +----------------------------------+\n");
    printf("    |                                  |\n");
    printf("    |  Press 'P' to start the game    |\n");
    printf("    |  Press 'M' to toggle sound      |\n");
    printf("    |  Press 'Q' to quit              |\n");
    printf("    |                                  |\n");
    printf("    +----------------------------------+\n");
    
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
            case 'p':
                game->state = PLAYING;
                break;
            case 'm':
                game->soundEnabled = !game->soundEnabled;
                break;
            case 'q':
                exit(0);
                break;
        }
    }
}

void showGameOver(Game* game) {
    clearScreen();
    printf("\n\n");
    printf("    +----------------------------------+\n");
    printf("    |           GAME OVER             |\n");
    printf("    +----------------------------------+\n");
    printf("    |                                  |\n");
    printf("    |  Final Score: %-17d |\n", game->score);
    printf("    |  High Score: %-18d |\n", game->highScore);
    printf("    |                                  |\n");
    printf("    |  Press 'P' to play again        |\n");
    printf("    |  Press 'Q' to quit              |\n");
    printf("    |                                  |\n");
    printf("    +----------------------------------+\n");
    
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
            case 'p':
                initGame(game);
                game->state = PLAYING;
                break;
            case 'q':
                exit(0);
                break;
        }
    }
}

void setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

void clearScreen() {
    system("cls");
} 
