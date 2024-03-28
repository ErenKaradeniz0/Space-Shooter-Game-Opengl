#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define BORDER_SIZE 10
#define MAX_X 1024
#define MAX_Y 768
#define SIDE_BAR_WIDTH 192 // 24 * 8

#define ROCKET_WIDTH 32 // 6 * 8
#define ROCKET_HEIGHT 65

#define SPACE_SHIP_WIDTH 80  // 6 * 8
#define SPACE_SHIP_HEIGHT 64 // 8 * 8

#define BULLET_SPEED 8
#define MAX_BULLETS 30

#define ROCKET_SPEED 8
#define MAX_ROCKETS 6
#define ROCKET_MOVE_DELAY 20
#define BULLET_MOVE_DELAY 2

#define RAND_MAX 112 // 944 / 8 test
typedef struct
{
    int x;
    int y;
    int active;  // Flag to bullet is active or not
    int avaible; // Flag to bullet is avaible to shot
} Bullet;

typedef struct
{
    int x;
    int y;
    int active; // Flag to rocket is active or not
} Rocket;

Bullet bullets[MAX_BULLETS];
Rocket rockets[MAX_ROCKETS];

int rocketMoveCounter = 0; // Counter to control rocket movement speed
int bulletMoveCounter = 0; // Counter to control bullet movement speed

int flag = 0; // flag 1 when key pressed
int quit_flag = 0;
int pause_flag = 0;
char current_key = '1';
int bullet_count = MAX_BULLETS;
int ship_x;
int ship_y;
int score = 0;
char score_str[3];   // maximum number of digits is 3
char bullets_str[2]; // maximum number of digits is 2

void clear_screen()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color to black (RGBA)
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer
    // Other drawing operations go here
    glFlush(); // Flush OpenGL pipeline to display the result
}

// void glClearRect(int x, int y, int x1, int x2)
// {
//     // SDL_SetRenderDrawColor( 0, 0, 0, 255); // Black color
//     // SDL_Rect clearRect = {x, y, x1, x2};
//     // SDL_RenderFillRect();
// }

void glClearRect(int x, int y, int width, int height)
{
    glScissor(x, y, width, height); // Enable scissor test to restrict clearing to the specified rectangle
    glEnable(GL_SCISSOR_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set the clear color (black in this case)
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

    glDisable(GL_SCISSOR_TEST); // Disable scissor test after clearing
}

void drawBoundaries()
{
    // Draw top border
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    glBegin(GL_QUADS);
    glVertex2f(0, MAX_Y);
    glVertex2f(MAX_X, MAX_Y);
    glVertex2f(MAX_X, MAX_Y - BORDER_SIZE);
    glVertex2f(0, MAX_Y - BORDER_SIZE);
    glEnd();

    // Draw bottom border
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(MAX_X, 0);
    glVertex2f(MAX_X, BORDER_SIZE);
    glVertex2f(0, BORDER_SIZE);
    glEnd();

    // Draw left border
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(BORDER_SIZE, 0);
    glVertex2f(BORDER_SIZE, MAX_Y);
    glVertex2f(0, MAX_Y);
    glEnd();

    // Draw right border
    glBegin(GL_QUADS);
    glVertex2f(MAX_X - BORDER_SIZE, 0);
    glVertex2f(MAX_X, 0);
    glVertex2f(MAX_X, MAX_Y);
    glVertex2f(MAX_X - BORDER_SIZE, MAX_Y);
    glEnd();

    // Draw SideBar
    glBegin(GL_QUADS);
    glVertex2f(SIDE_BAR_WIDTH, 0);
    glVertex2f(SIDE_BAR_WIDTH + BORDER_SIZE, 0);
    glVertex2f(SIDE_BAR_WIDTH + BORDER_SIZE, MAX_Y);
    glVertex2f(SIDE_BAR_WIDTH, MAX_Y);
    glEnd();

    glFlush();
}

// Function to convert an integer to its string representation
int int_to_string(int num, char *buffer)
{
    int i = 0;
    int digits = 0; // Variable to store the number of digits

    if (num == 0)
    {
        buffer[i++] = '0';
        digits = 1; // If the number is zero, it has one digit
    }
    else
    {
        // Calculate the number of digits
        int temp = num;
        while (temp != 0)
        {
            digits++;
            temp /= 10;
        }

        // Convert each digit to character and store in the buffer
        while (num != 0)
        {
            int digit = num % 10;
            buffer[i++] = '0' + digit;
            num /= 10;
        }
    }
    buffer[i] = '\0';

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end)
    {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }

    return digits; // Return the number of digits
}

// void graphics_write_string( int x, int y, const char *text)
// {
//     SDL_Color textColor = {255, 255, 255}; // White color
//     SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
//     if (surface == NULL)
//     {
//         printf("Failed to render text! SDL_ttf Error: %s\n", TTF_GetError());
//         return;
//     }

//     SDL_Texture *texture = SDL_CreateTextureFromSurface( surface);
//     if (texture == NULL)
//     {
//         printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
//         SDL_FreeSurface(surface);
//         return;
//     }

//     SDL_Rect dstRect = {x, y, surface->w, surface->h}; // Position and size
//     SDL_RenderCopy( texture, NULL, &dstRect);

//     SDL_DestroyTexture(texture);
//     SDL_FreeSurface(surface);
// }

void graphics_write_string(float x, float y, const char *string)
{
    glRasterPos2f(x, y); // Set the position for the text
    for (const char *c = string; *c != '\0'; ++c)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c); // Render each character
    }
    glFlush();
}

void printScore(int x, int y)
{
    glClearRect(x - 2, y, 27, 15);
    int num_digits = int_to_string(score, score_str);
    graphics_write_string(x, y, score_str);
}

void bullet_counter()
{
    bullet_count = 0;
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].avaible)
        {
            bullet_count += 1;
        }
    }
}

void printBulletCount(int x, int y)
{

    glClearRect(x - 2, y, 27, 15);
    // SDL_SetRenderDrawColor( 255, 255, 255, 255); // White color
    int num_digits = int_to_string(bullet_count, bullets_str);
    graphics_write_string(x, y, bullets_str);
    if (bullet_count < 10)
        graphics_write_string(x + 8, y, " ");
}

void info()
{
    // SDL_SetRenderDrawColor( 255, 255, 255, 255); // Set color to draw (white)
    graphics_write_string(16, 740, "Welcome!");
    graphics_write_string(16, 720, "Save the World!");
    graphics_write_string(16, 700, "by Eren Karadeniz");
    graphics_write_string(16, 680, "200101070");

    graphics_write_string(16, 640, "Keys");
    graphics_write_string(16, 620, "A to move left");
    graphics_write_string(16, 600, "D to move right");
    graphics_write_string(16, 580, "Space to Shot");
    graphics_write_string(16, 560, "Q to quit game");
    graphics_write_string(16, 540, "R to restart game");
    graphics_write_string(16, 520, "P to pause game");
    graphics_write_string(16, 500, "Win after reach");
    graphics_write_string(16, 480, "25 Score");
}

void intro()
{
    drawBoundaries();
    info();
    graphics_write_string(16, 450, "Bullets:");
    printBulletCount(88, 450);

    graphics_write_string(16, 430, "Score:");
    printScore(80, 430);
}

// Draw A
void draw_a(int x, int y, int w, int h)
{
    glBegin(GL_LINES);
    // Draw the two diagonal lines of the 'A'
    glVertex2i(x + w + 3, y + 22);
    glVertex2i(x, y + h + 25);

    glVertex2i(x - w - 3, y + 22);
    glVertex2i(x, y + h + 25);

    // Draw the horizontal bar of the 'A'
    glVertex2i(x - 10, y + 10);
    glVertex2i(x - 10, y + 10 + h + 5);

    glVertex2i(x + 10, y + 10);
    glVertex2i(x + 10, y + 10 + h + 5);

    // Draw the connection between the two bars
    glVertex2i(x - 10, y + 15);
    glVertex2i(x + 10, y + 15);
    glEnd();
}

void drawCircle(int x, int y, int r)
{
    glBegin(GL_LINE_LOOP); // Use GL_LINE_LOOP instead of GL_TRIANGLE_FAN
    for (int i = 0; i < 360; ++i)
    {
        float rad = i * 3.14159 / 180;
        glVertex2f(x + cos(rad) * r, y + sin(rad) * r);
    }
    glEnd();
}

void drawSpaceship(int x, int y, int w, int h)
{
    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white

    // // Draw 'A' four times
    draw_a(x, y + 5, w, h);
    draw_a(x + 70, y + 5, w, h);
    draw_a(x, y + 25, w, h);
    draw_a(x + 70, y + 25, w, h);

    // Draw '-'
    glBegin(GL_LINES);
    glVertex2i(x + 30, y + 50);
    glVertex2i(x + 39, y + 50);
    glEnd();

    // Draw 'I'
    glBegin(GL_LINES);
    glVertex2i(x + 35, y + 40);
    glVertex2i(x + 35, y + 50);
    glEnd();

    // Draw '-'
    glBegin(GL_LINES);
    glVertex2i(x + 30, y + 40);
    glVertex2i(x + 39, y + 40);
    glEnd();

    // // Draw '/'
    glBegin(GL_LINES);
    glVertex2i(x + 30, y + 35);
    glVertex2i(x + 24, y + 25);
    glEnd();

    // // Draw '\'
    glBegin(GL_LINES);
    glVertex2i(x + 40, y + 35);
    glVertex2i(x + 46, y + 25);
    glEnd();

    // Draw '-'
    glBegin(GL_LINES);
    glVertex2i(x + 30, y + 30);
    glVertex2i(x + 40, y + 30);
    glEnd();

    // Draw '/'
    glBegin(GL_LINES);
    glVertex2i(x + 10, y + 10);
    glVertex2i(x, y);
    glEnd();

    // Draw circles
    drawCircle(x + 20, y + 25 - w - 2, 5);
    drawCircle(x + 35, y + 25 - w - 2, 5);
    drawCircle(x + 50, y + 25 - w - 2, 5);

    // Draw '\'
    glBegin(GL_LINES);
    glVertex2i(x + 60, y + 10);
    glVertex2i(x + 70, y);
    glEnd();

    glFlush();
}

void clearSpaceship(int x, int y)
{
    // Calculate bottom-right corner coordinates
    int x2 = 92;
    int y2 = 54; // Maximum y-coordinate for the spaceship
    glClearRect(x - 12, y, x2, y2);
}

void drawCharacter(int x, int y)
{
    // Set color to draw (white)
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw the points to form the '^' character
    glBegin(GL_POINTS);
    glVertex2i(x, y);         // Top center point
    glVertex2i(x + 1, y - 1); // Upper left point
    glVertex2i(x + 2, y - 2); // Upper left point
    glVertex2i(x + 3, y - 3); // Upper left point
    glVertex2i(x - 1, y - 1); // Upper right point
    glVertex2i(x - 2, y - 2); // Upper right point
    glVertex2i(x - 3, y - 3); // Upper right point
    glEnd();
}

void moveBullet(int index)
{
    if (bulletMoveCounter % BULLET_MOVE_DELAY == 0)
    {

        // Clear previous bullet position
        glClearRect(bullets[index].x - 4, bullets[index].y - 4, 8, 5);
        if (bullets[index].y < MAX_Y - BORDER_SIZE - 4)
        {

            bullets[index].y += BULLET_SPEED; // Move the bullet upwards
            // Draw '^' character at the new position
            // SDL_SetRenderDrawColor( 255, 255, 255, 255);        // Set color to draw (white)
            drawCharacter(bullets[index].x, bullets[index].y); // Draw the bullet
        }
        else
            bullets[index].active = 0;
    }
}

void move_bullets()
{
    // Move all active bullets
    for (int index = 0; index < MAX_BULLETS; index++)
    {
        if (!pause_flag)
        {
            if (bullets[index].active && !bullets[index].avaible)
            {
                drawCharacter(bullets[index].x, bullets[index].y); // Draw the bullet
                moveBullet(index);
            }
        }
    }

    // Increment the bullet move counter
    bulletMoveCounter++;

    // Reset the counter to prevent overflow
    if (bulletMoveCounter >= BULLET_MOVE_DELAY)
        bulletMoveCounter = 0;
}

void shot_bullet(Bullet *bullet)
{
    bullet->active = 1;
    bullet->avaible = 0;
    bullet->x = ship_x + 35; // Adjust bullet position to appear from spaceship's center
    bullet->y = ship_y + 60;
}

void drawRocket(int x, int y)
{
    // Draw \||/
    glBegin(GL_LINES);
    glVertex2f(x + 20, y + 15);
    glVertex2f(x + 30, y + 23);

    glVertex2f(x + 10, y + 10);
    glVertex2f(x + 10, y + 35);

    glVertex2f(x + 20, y + 10);
    glVertex2f(x + 20, y + 35);

    glVertex2f(x, y + 25);
    glVertex2f(x + 10, y + 15);
    glEnd();

    // Draw ___
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x + 30, y);
    glEnd();

    // Draw |o|
    glBegin(GL_LINES);
    glVertex2f(x, y + 25);
    glVertex2f(x, y - 20);
    glVertex2f(x + 30, y + 25);
    glVertex2f(x + 30, y - 20);
    glEnd();

    drawCircle(x + 15, y - 10, 5);

    // Draw '\'
    glBegin(GL_LINES);
    glVertex2f(x, y - 15);
    glVertex2f(x + 15, y - 30);
    glEnd();

    // Draw /
    glBegin(GL_LINES);
    glVertex2f(x + 30, y - 15);
    glVertex2f(x + 15, y - 30);
    glEnd();
}
void clearRocket(int x, int y)
{
    // Calculate bottom-right corner coordinates
    int x2 = ROCKET_WIDTH;
    int y2 = ROCKET_HEIGHT; // Maximum y-coordinate for the spaceship
    glClearRect(x - 1, y - 30, x2, y2);
}

unsigned int get_system_timer_value()
{
    unsigned int val;
    // Read the value of the system timer (assuming x86 architecture)
    asm volatile("rdtsc" : "=a"(val));
    return val;
}

// Define some global variables for the random number generator
static unsigned long next;

// A function to generate a pseudo-random integer
int rand(void)
{
    next = get_system_timer_value();
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % RAND_MAX;
}

int randRocketAxis()
{
    int min_x = SIDE_BAR_WIDTH / 8 + 1;
    int max_x = MAX_X / 8 - ROCKET_WIDTH / 8;
    int x = rand(); // 1 - 88
    while (min_x > x || x > max_x)
    {
        x = rand();
    }
    return x + 8;
}

// Function to generate a single rocket from passive rocket
void generateRocket(Rocket *rocket)
{
    int newRocketX, newRocketY;
    int collisionDetected;

    do
    {
        // Generate random position for the new rocket
        newRocketX = 8 * randRocketAxis(); // Adjust range to prevent overflow
        newRocketY = MAX_Y - 52;           // Adjust range as needed

        // Check for collision with existing rockets based on X position only
        collisionDetected = 0;
        for (int j = 0; j < MAX_ROCKETS; j++)
        {
            if (rockets[j].active &&
                (newRocketX >= rockets[j].x - 20 - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
            {
                collisionDetected = 1;
                break;
            }
        }
    } while (collisionDetected);

    // Set the position of the new rocket
    rocket->x = newRocketX;
    rocket->y = newRocketY;
    rocket->active = 1;
}

void generate_rockets()
{
    // Generate new rockets if there are inactive rockets
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!rockets[i].active)
        {
            generateRocket(&rockets[i]);
        }
    }
}

void moveRocket(int index)
{
    if (rocketMoveCounter % ROCKET_MOVE_DELAY == 0)
    {                                                    // Move the rocket every ROCKET_MOVE_DELAY frames
        clearRocket(rockets[index].x, rockets[index].y); // Clear previous rocket position
        rockets[index].y -= ROCKET_SPEED;                // Move the rocket downwards
        drawRocket(rockets[index].x, rockets[index].y);
    }
}

void move_rockets()
{
    // Draw and move the rocket
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (!pause_flag)
        {
            if (rockets[i].active)
            {
                drawRocket(rockets[i].x, rockets[i].y);
                moveRocket(i);
            }
        }
    }

    // Increment the rocket move counter
    rocketMoveCounter++;
    // Reset the counter to prevent overflow
    if (rocketMoveCounter >= ROCKET_MOVE_DELAY)
        rocketMoveCounter = 0;
    if (current_key != 'p')
    {
        generate_rockets();
    }
}

void initBullets()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        bullets[i].x = 1;
        bullets[i].y = 1;
        bullets[i].active = 0;
        bullets[i].avaible = 1;
    }
}

void initRockets()
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        int newRocketX, newRocketY;
        int collisionDetected;

        do
        {
            // Generate random position for the new rocket
            newRocketX = 8 * randRocketAxis();
            newRocketY = MAX_Y - 52;

            // Check for collision with existing rockets based on X position only
            collisionDetected = 0;
            for (int j = 0; j < i; j++)
            {
                if (rockets[j].active &&
                    (newRocketX >= rockets[j].x - ROCKET_WIDTH && newRocketX <= rockets[j].x + ROCKET_WIDTH)) // Check only X position
                {
                    collisionDetected = 1;
                    i = 0;
                    break;
                }
            }
        } while (collisionDetected);

        // Set the position of the new rocket
        rockets[i].x = newRocketX;
        rockets[i].y = newRocketY;
        rockets[i].active = 1;
    }
}

int collisionBullet()
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (bullets[i].active)
        {
            for (int j = 0; j < MAX_ROCKETS; j++)
            {
                if (rockets[j].active &&
                    bullets[i].x >= rockets[j].x - 8 && bullets[i].x < rockets[j].x + ROCKET_WIDTH &&
                    bullets[i].y <= rockets[j].y && bullets[i].y > rockets[j].y - ROCKET_HEIGHT)
                {
                    score += 1;

                    printScore(80, 430);
                    bullets[i].active = 0; // Deactivate bullet
                    rockets[j].active = 0; // Deactivate rocket
                     glClearRect(bullets[i].x - 4, bullets[i].y - 4, 8, 5);
                    clearRocket(rockets[j].x, rockets[j].y);
                    break;
                }
            }
        }
    }
}

void gameOver()
{
    clear_screen();
    info();
    drawBoundaries();
    graphics_write_string((MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2 + 15, "You lost, Press R for Play Again");
    graphics_write_string((MAX_X - SIDE_BAR_WIDTH) / 2 + 140, MAX_Y / 2 + 30, "Score: ");
    graphics_write_string((MAX_X - SIDE_BAR_WIDTH) / 2 + 200, MAX_Y / 2 + 30, score_str);
}

// Function to check for collision between rocket and spaceship
void collisionSpaceShip()
{
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        // Check if any of the edges of the rocket box lie outside the spaceship box

        if (ship_x <= rockets[i].x + ROCKET_WIDTH - 1 && ship_x + SPACE_SHIP_WIDTH - 1 >= rockets[i].x && rockets[i].y + ROCKET_HEIGHT - 40 <= ship_y)
        {
            quit_flag = 1;
            gameOver();
            graphics_write_string((MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2, "Spaceship destroyed by rocket");
        }
    }
}

void init()
{
    // Set background color
    clear_screen();
    initBullets();
    initRockets();
    intro();

    ship_x = (MAX_X + SIDE_BAR_WIDTH) / 2 - SPACE_SHIP_WIDTH / 4; // base x of spaceship 49th pixel
    ship_y = 15;                                                  // base y of spaceship 87th pixel
}

void quitGame()
{
    clear_screen();
    drawBoundaries();
    info();
    graphics_write_string(MAX_X / 2, MAX_Y / 2, "Press R for Play Again");
}

void restartGame()
{
    init(); // Initialize the game
}

void handleUserInput(char current_key, Bullet bullets[MAX_BULLETS])
{
    if (!pause_flag)
    {
        switch (current_key)
        {
        case 'a':
            if (ship_x - 1 > SIDE_BAR_WIDTH + 20)
            {
                clearSpaceship(ship_x, ship_y);
                ship_x -= 8;
            }
            break;
        case 'd':
            if (ship_x + SPACE_SHIP_WIDTH < MAX_X - 16)
            {
                clearSpaceship(ship_x, ship_y);
                ship_x += 8;
            }
            break;
        case ' ':
            for (int i = 0; i < MAX_BULLETS; i++)
            {
                if (!bullets[i].active && bullets[i].avaible)
                {
                    shot_bullet(&bullets[i]);
                    bullet_counter();
                    printBulletCount(88, 450);
                    break;
                }
            }
            break;
        case 'q':
            quitGame();
            quit_flag = 1;
            break;
        case 'r':
            score = 0;
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(); // Restart the game
            break;
        case 'p':
            pause_flag = !pause_flag; // Toggle pause_flag
            if (pause_flag)
            {
                graphics_write_string(MAX_X / 2, MAX_Y / 2, "Paused, Press p to continue");
            }
            break;
        }
        flag = 0;
    }
    else
    {
        if (current_key == 'p')
        {
            flag = 0;
            pause_flag = 0;
            glClearRect(MAX_X / 2, MAX_Y / 2 -10, 245, 30);
        }
    }
}

void winGame()
{
    clear_screen();
    info();
    drawBoundaries();
    graphics_write_string(MAX_X / 2, MAX_Y / 2, "You Win!");
    graphics_write_string(MAX_X / 2, MAX_Y / 2 + 15, "Press R for Play Again");
}

int continueGame()
{
    // Check if all rockets have reached the bottom of the screen

    int rocketReachedBottom = 0;
    for (int i = 0; i < MAX_ROCKETS; i++)
    {
        if (rockets[i].y + ROCKET_HEIGHT <= 0)
        {
            rocketReachedBottom = 1;
            if (rocketReachedBottom)
            {
                quit_flag = 1;
                gameOver();
                graphics_write_string((MAX_X - SIDE_BAR_WIDTH) / 2 + 30, MAX_Y / 2, "Rockets Reached Bottom.");
                return 0;
            }
        }
    }

    if (score == 25)
    {
        quit_flag = 1;
        winGame();
        score = 0;
        return 0;
    }

    return 1;
}

void busy_wait(unsigned int milliseconds)
{
    // Calculate the number of iterations needed for the desired milliseconds
    unsigned int iterations = milliseconds * 10000; // Adjust this value as needed based on your system's clock speed

    // Execute an empty loop for the specified number of iterations
    for (unsigned int i = 0; i < iterations; ++i)
    {
        // Do nothing, just wait
    }
}

void display()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set clear color to black
    glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

    glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
    init();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height); // Set the viewport to cover the entire window
    glMatrixMode(GL_PROJECTION);     // Select the projection matrix
    glLoadIdentity();                // Reset the projection matrix
    gluOrtho2D(0, width, 0, height); // Set up an orthographic projection
    glMatrixMode(GL_MODELVIEW);      // Select the modelview matrix
    glLoadIdentity();                // Reset the modelview matrix
}

char keyboard(unsigned char key, int x, int y)
{
    current_key = key;
    flag = 1;
}

void gameLoop(int value)
{
    // Check if the game should continue and the quit_flag is not set
    if (quit_flag == 0 && continueGame())
    {
        // Draw the spaceship
        drawSpaceship(ship_x, ship_y, 4, 4);
        if (flag)
        {
            // Handle user input
            handleUserInput(current_key, bullets);
        }

        // Move bullets and rockets
        move_bullets();
        move_rockets();

        // Check for collisions
        collisionBullet();
        collisionSpaceShip();

        // Wait for 50 milliseconds
        // busy_wait(800);
    }

    // Check if 'r' key is pressed to restart the game
    if (flag && !pause_flag)
    {
        if (current_key == 'r')
        {
            quit_flag = 0;
            bullet_count = MAX_BULLETS;
            restartGame(); // Restart the game
        }
    }

    // Call the game loop function again
    glutTimerFunc(25, gameLoop, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);                       // Initialize GLUT
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // Set up the display mode
    glutInitWindowSize(MAX_X, MAX_Y);            // Set the window size
    glutCreateWindow("Save The World");          // Create a window with the given title
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display); // Set the display function
    glutReshapeFunc(reshape); // Set the reshape function

    // Start the game loop
    glutTimerFunc(0, gameLoop, 0);

    glutMainLoop(); // Enter the GLUT event processing loop
    return 0;
}