#include <GLFW\glfw3.h>
//#include "linmath.h"    Took this out because it wasn't needed
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

int brickcollisionnum = 0;
int one_ball = 0;

class Brick
{
public:

	float red, green, blue;
	float x, y, width;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
	};

	// Draw bricks
	void drawBrick()
	{
		if (onoff == ON) 
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			// Create more rectangular shape for squares by dividing the halfside by 2 on the y coords

			glVertex2d(x + halfside, y + (halfside / 2));
			glVertex2d(x + halfside, y - (halfside / 2));
			glVertex2d(x - halfside, y - (halfside / 2));
			glVertex2d(x - halfside, y + (halfside / 2));

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01; // Slower speed
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void CheckCollision(Brick* brk)
	{
		

		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.03;

			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
			// First the brick is green, then it gets hit and turns red, if it is hit again it disappears.
				if (brk->onoff == ON) { // If brick is visible, then bounce off and go in a different direction.

						direction = GetRandomDirection();
						x = x - 0.01;
						y = y - 0.01;

					if (brk->red == 1) { // If brick is red and is hit again, then make invisible.
						// If the ball hits the red brick then reset it at x and y
						x = 0.00;
						y = 0.00;
						brk->onoff = OFF;
					}
					if (brickcollisionnum == 0) { //  if brickcollisionnum is zero, then change the color of the brick to red
						brk->red = 1;
						brk->green = 0;
						brk->blue = 0;
					}
				}
				else if (brk->onoff == OFF) { 
					// If brick is not visible, then do nothing.
				}


			}
		}
	}

	int GetRandomDirection()
	{
		

		return (rand() % 8) + 1;




	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;

int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Module 8 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Brick #, Brick type, x, y, size, Colors: Start all at green (0, 1, 0)
	Brick brick(DESTRUCTABLE, 0, 0.9, 0.2, 0, 1, 0);
	Brick brick2(DESTRUCTABLE, -0.4, 0.9, 0.2, 0, 1, 0);
	Brick brick3(DESTRUCTABLE, -0.8, 0.9, 0.2, 0, 1, 0);
	Brick brick4(DESTRUCTABLE, 0.4, 0.9, 0.2, 0, 1, 0);
	Brick brick5(DESTRUCTABLE, 0.8, 0.9, 0.2, 0, 1, 0);
	Brick brick6(DESTRUCTABLE, 0, 0.7, 0.2, 0, 1, 0);
	Brick brick7(DESTRUCTABLE, -0.4, 0.7, 0.2, 0, 1, 0);
	Brick brick8(DESTRUCTABLE, -0.8, 0.7, 0.2, 0, 1, 0);
	Brick brick9(DESTRUCTABLE, 0.4, 0.7, 0.2, 0, 1, 0);
	Brick brick10(DESTRUCTABLE, 0.8, 0.7, 0.2, 0, 1, 0);
	Brick brick11(DESTRUCTABLE, 0, 0.5, 0.2, 0, 1, 0);
	Brick brick12(DESTRUCTABLE, -0.4, 0.5, 0.2, 0, 1, 0);
	Brick brick13(DESTRUCTABLE, -0.8, 0.5, 0.2, 0, 1, 0);
	Brick brick14(DESTRUCTABLE, 0.4, 0.5, 0.2, 0, 1, 0);
	Brick brick15(DESTRUCTABLE, 0.8, 0.5, 0.2, 0, 1, 0);

	// User control bar for game - colored blue
	Brick player(REFLECTIVE, 0.0, -0.85, 0.3, 0, 0, 1);




	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			// Check to see if ball(s) collide with brick(s)
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&brick6);
			world[i].CheckCollision(&brick7);
			world[i].CheckCollision(&brick8);
			world[i].CheckCollision(&brick9);
			world[i].CheckCollision(&brick10);
			world[i].CheckCollision(&brick11);
			world[i].CheckCollision(&brick12);
			world[i].CheckCollision(&brick13);
			world[i].CheckCollision(&brick14);
			world[i].CheckCollision(&brick15);

			// User control
			world[i].CheckCollision(&player);

			world[i].MoveOneStep();
			world[i].DrawCircle();


		}

		
		// Draw bricks
		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		brick6.drawBrick();
		brick7.drawBrick();
		brick8.drawBrick();
		brick9.drawBrick();
		brick10.drawBrick();
		brick11.drawBrick();
		brick12.drawBrick();
		brick13.drawBrick();
		brick14.drawBrick();
		brick15.drawBrick();

		// draw User control
		player.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{


	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) //  Cloe window if user presses escape
		glfwSetWindowShouldClose(window, true);
	
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{

		// Allows the user to only put one ball on the screen
		if (one_ball == 0) {
			one_ball =+ 1; 
			//Circle B(0, 0, 02, 2, 0.05, r, g, b);
			double r, g, b;
			r = rand() / 10000;
			g = rand() / 10000;
			b = rand() / 10000;
			Circle B(0, 0, 02, 2, 0.05, r, g, b);
			world.push_back(B);
		}
		else {
			// Do nothing if space bar is pressed again
		}

		
		
	}
	else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {

		// If user presses the 1 key, they can send as many balls as they want into the animation.

		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 2, 0.05, r, g, b);
		world.push_back(B);

	}
	// If player pushes left key, then the user control bar moves left.
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {

		//Brick player(REFLECTIVE, 0.0, -0.85, 0.3, 0, 0, 1);
		
		Brick player(REFLECTIVE, -0.1, -0.85, 0.3, 0, 0, 1);
		player.drawBrick();

	}
	// If player pushes right key, then the user control bar moves right.
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {

		Brick player(REFLECTIVE, 0.1, -0.85, 0.3, 0, 0, 1);
		player.drawBrick();

	}

}