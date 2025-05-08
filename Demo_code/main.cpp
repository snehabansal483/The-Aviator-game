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
float bgColorR = 0.0f; // Red component
float bgColorG = 0.5f; // Green component
float bgColorB = 1.0f; // Blue component
bool isDay = true;      // Track if it's day or night

float treeX = 0.0;  // X position of the tree
float treeY = 1.2;  // Start position (above the screen)
float treeSpeed = 0.01; // Speed of falling tree
bool treeVisible = true; // Show tree after restart

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
void drawSunOrMoon(bool isMorning, float animationTime) {
    if (isMorning) {
        glColor3f(1.0, 1.0, 0.0); // Yellow Sun
    } else {
        glColor3f(0.8, 0.8, 0.9); // Light Grey Moon
    }

    // Draw Circle (Sun or Moon)
    float x = -0.7f, y = 0.8f, radius = 0.1f;
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * 3.14159 / 180;
        glVertex2f(x + cos(angle) * radius, y + sin(angle) * radius);
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
void drawRealisticCloud(float x, float y, float scale) {
    glColor3f(1.0, 1.0, 1.0); // White clouds

    // Main large cloud body
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x + scale * 0.15f * cos(angle), y + scale * 0.08f * sin(angle));
    }
    glEnd();

    // Overlapping fluffy parts
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x - scale * 0.1f + scale * 0.1f * cos(angle), y + scale * 0.06f * sin(angle));
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x + scale * 0.1f + scale * 0.09f * cos(angle), y + scale * 0.07f * sin(angle));
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * M_PI / 180.0f;
        glVertex2f(x + scale * 0.03f + scale * 0.07f * cos(angle), y - scale * 0.02f * sin(angle));
    }
    glEnd();
}

void drawClouds() {
    float cloudPositions[][3] = {
        {-0.8f, 0.7f, 1.0f}, {-0.5f, 0.6f, 0.8f}, {-0.2f, 0.75f, 1.2f},
        {0.2f, 0.65f, 1.0f}, {0.6f, 0.72f, 0.9f}, {0.9f, 0.68f, 1.1f}
    };

    for (int i = 0; i < 6; i++) {
        drawRealisticCloud(cloudPositions[i][0], cloudPositions[i][1], cloudPositions[i][2]);
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
void drawTree(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0);

    // Trunk (Brown)
    glColor3f(0.5, 0.25, 0.0);
    glBegin(GL_POLYGON);
    glVertex2f(-0.02, -0.1);
    glVertex2f(0.02, -0.1);
    glVertex2f(0.02, 0.1);
    glVertex2f(-0.02, 0.1);
    glEnd();

    // Leaves (Green)
    glColor3f(0.0, 0.8, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.1, 0.1);
    glVertex2f(0.1, 0.1);
    glVertex2f(0.0, 0.3);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-0.08, 0.2);
    glVertex2f(0.08, 0.2);
    glVertex2f(0.0, 0.4);
    glEnd();

    glPopMatrix();
}
void updateTree() {
    if (treeVisible) {
        treeY -= treeSpeed; // Move tree down

        // **Check for collision with the plane**
        float treeLeft = treeX - 0.1;
        float treeRight = treeX + 0.1;
        float treeBottom = treeY - 0.2;
        float treeTop = treeY + 0.4;

        float planeLeft = planeX - 0.05;
        float planeRight = planeX + 0.05;
        float planeBottom = planeY - 0.05;
        float planeTop = planeY + 0.05;

        if (planeRight > treeLeft && planeLeft < treeRight &&
            planeTop > treeBottom && planeBottom < treeTop) {
            gameOver = true; // **Game Over if collision happens**
        }

        // **Reset tree if it moves off-screen**
        if (treeY < -1.2) {
            treeY = 1.2;
            treeX = (rand() % 200 - 100) / 100.0; // Random X position
        }
    }
}
void restartGame() {
    score = 0;
    gameOver = false;
    planeX = 0.0;
    planeY = -0.5;

    // **Reset tree position**
    treeX = (rand() % 200 - 100) / 100.0;
    treeY = 1.2;
    treeVisible = true;
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
    glPushMatrix();
    glTranslatef(planeX, planeY, 0); // Move plane to its position
    glScalef(0.2, 0.2, 1); // Scale the plane

    // Body of the airplane
    glColor3f(1.0, 1.0, 0.0); // Yellow body
    glBegin(GL_POLYGON);
    glVertex2f(-0.3, 0.05);
    glVertex2f(0.3, 0.05);
    glVertex2f(0.35, 0.02);
    glVertex2f(0.35, -0.02);
    glVertex2f(0.3, -0.05);
    glVertex2f(-0.3, -0.05);
    glEnd();

    // Cockpit (Front glass area)
    glColor3f(0.0, 0.0, 1.0); // Blue cockpit
    glBegin(GL_POLYGON);
    glVertex2f(0.2, 0.05);
    glVertex2f(0.28, 0.05);
    glVertex2f(0.3, 0.02);
    glVertex2f(0.2, 0.02);
    glEnd();

    // Wings
    glColor3f(1.0, 0.0, 0.0); // Red wings
    glBegin(GL_POLYGON);
    glVertex2f(-0.1, 0.05);
    glVertex2f(-0.3, 0.15);
    glVertex2f(-0.2, 0.15);
    glVertex2f(0.0, 0.05);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.1, -0.05);
    glVertex2f(-0.3, -0.15);
    glVertex2f(-0.2, -0.15);
    glVertex2f(0.0, -0.05);
    glEnd();

    // Tail
    glColor3f(1.0, 0.0, 0.0); // Red tail
    glBegin(GL_POLYGON);
    glVertex2f(-0.3, 0.05);
    glVertex2f(-0.35, 0.1);
    glVertex2f(-0.3, 0.1);
    glVertex2f(-0.2, 0.05);
    glEnd();

    glPopMatrix();
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
void updateLighting() {
    if (isMorning) {
        // Morning to Night transition
        bgColorR -= 0.001f;  // Reduce red
        bgColorG -= 0.001f;  // Reduce green
        bgColorB += 0.002f;  // Increase blue (toward dark blue)

        if (bgColorR <= 0.1f && bgColorG <= 0.1f && bgColorB >= 0.2f) {
            isMorning = false;  // Switch to night mode
        }
    } else {
        // Night to Morning transition
        bgColorR += 0.001f;
        bgColorG += 0.001f;
        bgColorB -= 0.002f;  // Reduce blue (toward bright sky)

        if (bgColorR >= 0.5f && bgColorG >= 0.7f && bgColorB <= 0.9f) {
            isMorning = true;  // Switch to morning mode
        }
    }

    glutPostRedisplay();  // Redraw the scene
}

void timer(int value) {
    updateLighting(); // **Update background lighting over time**
    glutTimerFunc(100, timer, 0); // Call this function every 100ms
}





// Display callback
void display() {
    glClearColor(bgColorR, bgColorG, bgColorB, 1.0f);
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
        string menuText = "Press 'M OR N' to Start the Game!";
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
            drawTree(treeX, treeY);
            updateTree();

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
void updateBullets() {
    for (auto& bullet : bullets) {
        if (bullet.active) {
            bullet.x += 0.02f; // Move bullet to the right

            // Deactivate bullet if it moves out of the screen
            if (bullet.x > 1.0f) {
                bullet.active = false;
            }
        }
    }

    // Remove inactive bullets from the vector
    bullets.erase(remove_if(bullets.begin(), bullets.end(),
        [](Bullet& b) { return !b.active; }),
        bullets.end());
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
    } else if (!gameOver) {
        if (key == 's' || key == 'S') {
            inMenu = true;
            init();  // Reset game and show the menu
        }
        else if (key == ' ' && bulletsRemaining > 0) {  // Fire bullet
            bullets.push_back({planeX + 0.05, planeY, true}); // Add bullet with 'active' flag
            bulletsRemaining--;

            // Play bullet sound
            mciSendString("close bullet_sound", NULL, 0, NULL);
            mciSendString("open \"C:\\Users\\Sneha Bansal\\Downloads\\Demo_code\\bullet.mp3\" type mpegvideo alias bullet_sound", NULL, 0, NULL);
            mciSendString("play bullet_sound", NULL, 0, NULL);
        }
        else if (key == 't' || key == 'T') {  // Toggle rain
            isRainy = !isRainy;
            cout << (isRainy ? "Rain started!" : "Rain stopped!") << endl;
        }
    } else {  // Game Over condition
        if (key == 'r' || key == 'R') {
            restartGame();
            init();
        }
    }

    glutPostRedisplay(); // Refresh screen after key press
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
    glutTimerFunc(100, timer, 0);
    glutMainLoop();
    return 0;
}


