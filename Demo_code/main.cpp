#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <math.h>
#include <algorithm>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace std;
// Game variables
float planeX = -0.5f, planeY = 0.0f;    // Plane position
vector<pair<float, float>> obstacles;  // Obstacle positions (x, gapY)
int score = 0;

// Rain state flag
bool isRainy = false;

bool gameOver = false;                 // Game state
bool gameStarted = false;              // To start/restart the game
bool isMorning = true;                 // Flag for day/night mode
bool inMenu = true;                    // Flag to track if we're in the theme selection menu
float animationTime = 0.0f;            // Time for smooth animation
int bulletsRemaining = 50;             // Limit on the number of bullets
float timeRemaining = 100.0f;

// Rain particle struct
struct Rain {
    float x, y;  // Position of the raindrop
    float speed; // Speed of the raindrop
    bool active = true; // Whether the raindrop is active
};

// Vector to store rain particles
vector<Rain> rainParticles;

// Bullet struct to store bullet data
struct Bullet {
    float x, y; // Bullet position
    bool active = true; // Whether the bullet is active or not
};

// Bullet vector to store bullets
vector<Bullet> bullets;

// Function to play a sound
void playSound(const char* soundFile, bool loop = false) {
    string command = "open \"" + string(soundFile) + "\" type mpegvideo alias sound";
    mciSendString(command.c_str(), NULL, 0, NULL);
    mciSendString(loop ? "play sound repeat" : "play sound", NULL, 0, NULL);
}

// Function to stop the sound
void stopSound() {
    mciSendString("close sound", NULL, 0, NULL);
}

// Function to draw Sun or Moon
void drawSunOrMoon(bool isMorning, float time) {
    float centerX = 0.0f, centerY = 0.8f, radius = 0.5f; // Circular path
    float x = centerX + radius * cos(2 * M_PI * time); // X position
    float y = centerY + radius * sin(2 * M_PI * time); // Y position

    glColor3f(isMorning ? 1.0f : 0.8f, isMorning ? 1.0f : 0.8f, isMorning ? 0.0f : 0.9f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x + 0.05f * cos(angle), y + 0.05f * sin(angle));
    }
    glEnd();
}

void drawRain() {
    if (!isRainy) return; // Do not render rain if it's off

    glColor3f(0.0f, 0.0f, 1.0f);  // Blue color for raindrops
    for (int i = 0; i < 10; ++i) {  // Simulate 100 raindrops
        float x = (rand() % 200) / 100.0f - 1.0f;  // Random X position between -1 and 1
        float y = (rand() % 200) / 100.0f - 1.0f;  // Random Y position between -1 and 1
         float size = 0.05f + (rand() % 100) / 1000.0f;
        glLineWidth(1.0f);  // Ensure the line is thin
        glBegin(GL_LINES);
        glVertex2f(x, y);           // Starting point of the raindrop
        glVertex2f(x, y -size);    // Ending point of the raindrop (downward)
        glEnd();
    }
}
//draw cloud
void drawClouds() {
    glColor3f(1.0, 1.0, 1.0); // White clouds
    for (float i = -1.0; i < 1.0; i += 0.3f) {
        float x = i + animationTime * 0.05;  // Move clouds
        float y = 0.8f;
        glBegin(GL_POLYGON);
        for (int j = 0; j < 360; j++) {
            float angle = j * M_PI / 180.0f;
            glVertex2f(x + 0.2f * cos(angle), y + 0.1f * sin(angle));
        }
        glEnd();
    }
}

void updateSunOrMoon() {
    animationTime += 0.0005f; // Slow transition for day-night cycle
    if (animationTime > 1.0f) animationTime = 0.0f; // Reset after full cycle
}

// Function to check if an obstacle overlaps
bool isOverlapping(float xPos) {
    for (const auto& obs : obstacles) {
        if (fabs(obs.first - xPos) < 0.5f) { // Ensures at least 0.5f distance
            return true;
        }
    }
    return false;
}

// Initialize obstacles with no overlap
void init() {
    glClearColor(isMorning ? 0.53f : 0.1f, isMorning ? 0.81f : 0.1f, isMorning ? 0.98f : 0.05f, 1.0f);
    srand(time(0));
    obstacles.clear();

    float startX = 1.0f;
    float minDistance = 0.6f; // Ensure at least 0.6 units of spacing

    for (int i = 0; i < 5; ++i) {
        float gapY = (rand() % 10 - 5) / 10.0f;

        // Ensure each new obstacle is placed at least 'minDistance' away from the previous one
        if (!obstacles.empty()) {
            startX = obstacles.back().first + minDistance;
        }

        obstacles.push_back({startX, gapY});
        startX += 1.0f; // Maintain proper distance
    }

    score = 0;
    bulletsRemaining = 50;
    gameOver = false;
    gameStarted = true;
    playSound("C:\\Users\\Sneha Bansal\\Downloads\\Demo_code\\background.mp3", true);
}


// Draw the plane
void drawPlane() {
    glColor3f(1.0, 1.0, 0.0); // Yellow plane
    glBegin(GL_POLYGON);
    glVertex2f(planeX - 0.05, planeY + 0.05);
    glVertex2f(planeX - 0.05, planeY - 0.05);
    glVertex2f(planeX + 0.05, planeY);
    glEnd();
}

// Draw the obstacles
void drawObstacles() {
    glColor3f(1.0, 0.0, 0.0); // Red obstacles
    for (auto& obs : obstacles) {
        float x = obs.first, gapY = obs.second;
        glBegin(GL_POLYGON); // Upper obstacle
        glVertex2f(x, 1.0);
        glVertex2f(x + 0.1, 1.0);
        glVertex2f(x + 0.1, gapY + 0.2);
        glVertex2f(x, gapY + 0.2);
        glEnd();
        glBegin(GL_POLYGON); // Lower obstacle
        glVertex2f(x, -1.0);
        glVertex2f(x + 0.1, -1.0);
        glVertex2f(x + 0.1, gapY - 0.2);
        glVertex2f(x, gapY - 0.2);
        glEnd();
    }
}

// Draw the ground
void drawGround() {
    glColor3f(0.0, 0.5, 0.0); // Green ground
    glBegin(GL_POLYGON);
    glVertex2f(-1.0, -1.0);
    glVertex2f(1.0, -1.0);
    glVertex2f(1.0, -0.8);
    glVertex2f(-1.0, -0.8);
    glEnd();
}

// Draw the bullets
void drawBullets() {
    glColor3f(0.0, 1.0, 0.0); // Green bullets
    for (auto& bullet : bullets) {
        glBegin(GL_POLYGON);
        glVertex2f(bullet.x - 0.02, bullet.y + 0.02);
        glVertex2f(bullet.x + 0.02, bullet.y + 0.02);
        glVertex2f(bullet.x + 0.02, bullet.y - 0.02);
        glVertex2f(bullet.x - 0.02, bullet.y - 0.02);
        glEnd();
    }
}

void checkCollision() {
    for (auto& obs : obstacles) {
        float obsX = obs.first;
        float gapY = obs.second;

        // Plane collision
        if (planeX + 0.05 > obsX && planeX - 0.05 < obsX + 0.1) {
            if (planeY + 0.05 > gapY + 0.2 || planeY - 0.05 < gapY - 0.2) {
                gameOver = true; // Plane hit an obstacle
                 stopSound();
                  playSound("C:\\Users\\Sneha Bansal\\Downloads\\Demo_code\\hit.mp3", true);
            }
        }


    }
}

void renderText(float x, float y, const char* text) {
    glColor3f(0.0f, 0.0f, 1.0f); // Set text color to white
    glRasterPos2f(x, y);      // Set the position of the text

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}
void drawStars() {
    if (!isMorning) {
        glColor3f(1.0f, 1.0f, 1.0f);
        for (int i = 0; i < 100; i++) {
            float x = (rand() % 200) / 100.0f - 1.0f;
            float y = (rand() % 200) / 100.0f;
            glBegin(GL_POINTS);
            glVertex2f(x, y);
            glEnd();
        }
    }
}




// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (inMenu) {
        // Menu screen
        glColor3f(1.0, 0.0, 0.0);

        // Display welcome message
        glRasterPos2f(-0.5, 0.5);
        string welcomeText = "Welcome to the Aviator Game!";
        for (char c : welcomeText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        // Display game instructions
        glRasterPos2f(-0.3, 0.3);
        string instTitle = "Instructions:";
        for (char c : instTitle) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.3, 0.15);
        string instLine1 = "1. Use the arrow keys to move the plane.";
        for (char c : instLine1) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.3, 0.05);
        string instLine2 = "2. Press 'Space' to shoot bullets.";
        for (char c : instLine2) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.3, -0.05);
        string instLine3 = "3. Avoid obstacles and shoot them to score points.";
        for (char c : instLine3) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.3, -0.15);
        string instLine4 = "4. Press 'M' for Morning theme or 'N' for Night theme.";
        for (char c : instLine4) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        glRasterPos2f(-0.3, -0.25);
        string instLine5 = "5. Press 'T' for Star and Stop the Rain.";
        for (char c : instLine5) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

        renderText(-0.8f, -0.3f, "Team Members:");
        renderText(-0.8f, -0.4f, "1. Sneha Bansal");
        renderText(-0.8f, -0.5f, "2. Laxita Alizad");
        renderText(-0.8f, -0.6f, "3. Amisha Kumari");
        renderText(-0.8f, -0.7f, "4. Sanjana Ghagde");

        // Creative decoration: Glowing border effect
        glColor3f(1.0f, 0.7f, 0.0f);  // Yellow color for the border effect
        glLineWidth(5.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.9f, 0.9f);
        glVertex2f(0.9f, 0.9f);
        glVertex2f(0.9f, -0.9f);
        glVertex2f(-0.9f, -0.9f);
        glEnd();

        glRasterPos2f(-0.3, -0.8);
        string menuText = "Press 'Enter' to Start the Game!";
        for (char c : menuText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    } else {
        drawGround();
        drawSunOrMoon(isMorning, animationTime);
        drawClouds();
        updateSunOrMoon();
        drawStars();

        if (gameStarted && !gameOver) {
            drawPlane();
            drawObstacles();
            drawBullets();
            checkCollision();
            drawRain();

            // Display score
            glColor3f(1.0, 0.1, 0.1);
            glRasterPos2f(-0.95, 0.9);
            string scoreText = "Score: " + to_string(score);
            for (char c : scoreText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

            glRasterPos2f(-0.15, 0.9);
            string start = "Menu : S";
            for (char c : start) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);


            // Display time remaining
            char timeText[20];
            sprintf(timeText, "Time: %.2f", timeRemaining);
            renderText(0.7, 0.9, timeText);
        }

        if (gameOver) {
            char scoreText[50];
            sprintf(scoreText, "Game Over! Press 'R' to Restart. Score: %d", score);
            renderText(-0.5, 0.0, scoreText);
        }
    }

    glutSwapBuffers();
}


// Update game state
void update(int value) {
    if (!gameOver) {
             // Decrease time remaining every frame
        timeRemaining -= 0.01f; // Adjust this for frame rate control
        if (timeRemaining <= 0) {
            timeRemaining=0;  // Ensure it doesn't go negative
            gameOver = false;
        }
        animationTime += 0.005f;
        if (animationTime > 1.0f) animationTime = 0.0f;

       // Move obstacles to the left
        for (int i = 0; i < obstacles.size(); i++) {
            obstacles[i].first -= 0.005f; // Move left
            obstacles[i].second += 0.005f * sin(obstacles[i].first * 5.0f);

            // If an obstacle goes off-screen, reposition it at the right
            if (obstacles[i].first < -1.2f) {
                float newGapY = (rand() % 10 - 5) / 10.0f;

                // Ensure proper spacing from the last obstacle
                float lastObstacleX = (i == 0) ? obstacles.back().first : obstacles[i - 1].first;
                obstacles[i].first = lastObstacleX + 0.6f;  // Maintain spacing
                obstacles[i].second = newGapY;
                score++;
            }
        }


        for (auto& bullet : bullets) {
    bullet.x += 0.05; // Move the bullet forward
    if (bullet.x > 1.0) bullet.active = false; // Deactivate bullets that go off-screen
}

// Remove inactive bullets
bullets.erase(remove_if(bullets.begin(), bullets.end(),
                        [](Bullet& b) { return !b.active; }),
              bullets.end());
              //bullet and obstacle collision
              for(auto& bullet : bullets){
                for(auto& obs : obstacles){
                    if(bullet.active && bullet.x>obs.first && bullet.x<obs.first + 0.1 && (bullet.y>obs.second+0.2||bullet.y<obs.second-0.2)){
                        bullet.active=false;
                        obs.first = 1.0;
                        obs.second=(rand()%10-5)/10.0f;
                        score++;
                        break;
                    }
                }
              }
    }
    checkCollision();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Handle keyboard input
void keyboard(unsigned char key, int x, int y) {
    if (inMenu) {
        if (key == 'm' || key == 'M') {
            isMorning = true;
            inMenu = false;
            init();
        } else if (key == 'n' || key == 'N') {
            isMorning = false;
            inMenu = false;
            init();
        }
    }else if (!gameOver) {
        if (key == 's' || key == 'S') {
            // When the game is over, pressing 'm' or 'M' returns to the menu
            inMenu = true;
            init();  // Reset game and show the menu
        }
    }  else {
        if (key == 'r' || key == 'R') {
            init();
        } else if (key == ' ' && !gameOver && bulletsRemaining > 0) {
            bullets.push_back({planeX + 0.05, planeY});
            bulletsRemaining--;
             // Play MP3 file
    mciSendString("close mp3", NULL, 0, NULL); // Close any previously opened MP3
   mciSendString("open \"C:\\Users\\Sneha Bansal\\Downloads\\Demo_code\\bullet.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
    mciSendString("play mp3", NULL, 0, NULL);
        }
        else if (key == 't' || key == 'T') {
            isRainy = !isRainy;  // Toggle rain state (T for Toggle)
            if (isRainy) {
                cout << "Rain started!" << endl;
            } else {
                cout << "Rain stopped!" << endl;
            }
        }
    }
}

// Handle special keys (for plane movement)
void specialKeys(int key, int x, int y) {
    if (!gameOver) {
        if (key == GLUT_KEY_UP && planeY < 0.95) planeY += 0.05;
        else if (key == GLUT_KEY_DOWN && planeY > -0.95) planeY -= 0.05;
        else if (key == GLUT_KEY_LEFT && planeX > -0.95) planeX -= 0.05;
        else if (key == GLUT_KEY_RIGHT && planeX < 0.95) planeX += 0.05;
    }
}

// Reshape callback
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("2D Avaitor Game");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
