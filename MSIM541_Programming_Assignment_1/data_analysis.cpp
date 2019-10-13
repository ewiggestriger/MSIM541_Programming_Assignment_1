// include header files
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cfloat>
#include <cmath>
#include <gl/glew.h>
#include <gl/glut.h>
#include "utility.h"

using namespace std;

// defines
constexpr auto PI = 3.1415926f;

//File names
char* fileName1, *fileName2;
string currentFile;

//Input data
float* data_ptr;
int numDataPoints;
float minimum, maximum;

//Histogram 
int numIntervals = 30;
float* endPoints;
float* prob;
float maxProb = -1;

//Theoretical distributions
int curveType = 0;
int numCurvePoints = 100;
float* curveX = new float[numCurvePoints];
float* curveY = new float[numCurvePoints];

//Parameters
float mu = 0, sigma = 1;  // Normal distribution
float lambda = 1;	      // Exponential distribution
float parameterStep = 0.05; // Step size for changing parameter values

//Drawing parameters
int width = 800, height = 600;
float world_x_min, world_x_max, world_y_min, world_y_max;
float axis_x_min, axis_x_max, axis_y_min, axis_y_max;

//Compute all the points for normal distribution
// TODO re-work the normal and exp functions i think they're wrong
void computeNormalFunc(float mu, float sigma)
{
	// This function computes the normal distribution and outputs to arrays
	// Normal distribution formula is y = (1 / sqrt(2*PI)*sigma)*exp(-(x - (mu)^2)/(2*(sigma)^2))
	// Determine the step size and compute the arrays curveX and curveY
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveX[i] += parameterStep;
		curveY[i] = (1 / sqrt(2 * PI)*sigma)*exp(-0.5*pow(((curveX[i] - mu) / sigma), 2));
	}
}

//Compute all the points for exponential distribution
void computeExponentialFunc(float lambda)
{
	// Exponential distribution formula is y = (1/lambda)*exp(-x/lambda)
	// Determine the step size and compute the arrays curveX and curveY
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveX[i] += parameterStep;
		curveY[i] = (1/lambda)*exp(-curveX[i]/lambda);
	}
}

void display(void) // TODO Finish display function
{
	/* clear all pixels */
	glClear(GL_COLOR_BUFFER_BIT);

	//Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(1);
	glColor3f(1.0, 1.0, 1.0);

	// Draw x and y axes

	// Display the maximum probability value

	// Draw probability histogram

	// Draw the theoretical distribution using thicker lines

	// Compute the top-left position of the annotation

	// File Name

	// Minimum

	// Maximum

	// Number of Intervals

	// Draw theoretical distributions

	glFlush();
	glutSwapBuffers();
}

void init(void) 
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

//Compute the probability for the histogram (vertical axis)
void computeProbability(int numIntervals)
{
	// Delete previously allocated memory
	if (endPoints != NULL)
	{
		delete endPoints;
	}

	if (prob != NULL)
	{
		delete prob;
	}

	// establish arrays for endpoints and probabilities 
	float* endPoints = new float[numIntervals];
	float* prob = new float[numIntervals];

	// Determine the end points for each interval (update the array endPoints)
	float range = maximum - minimum;
	//int numberOfEndpoints = range / numIntervals;
	float width = range / numIntervals;
	for (int i = 0; i < numIntervals; i++)
	{
		endPoints[i] = minimum + (width * i);
	}
	// TODO re-init historgram func 
	// Re-initialize the maximum probability after the number of intervals has been changed

	// Compute the probability for each interval (update the array prob)
	// loop thru endPoints array until value is less than endPoints val
	for (int i = 0; i < sizeof(data_ptr); i++)
	{
		for (int j = 0; i < sizeof(endPoints); j++)
		{
			if (data_ptr[i] <= endPoints[j])
			{
				prob[j] = prob[j]++;
				break;
			}
		}
	}

}

void readFile(string fileName) // TODO read file function
{
	ifstream inFile(fileName);

	if (!inFile.is_open())
	{
		cout << fileName << " couldn't be opened.\n";
		system("pause");
		exit(1);
	}

	inFile >> numDataPoints;

	// Memory management
	if (data_ptr != NULL)
	{
		delete data_ptr;
	}
	data_ptr = new float[numDataPoints];

	minimum = FLT_MAX;
	maximum = -FLT_MAX;

	// Read the data and compute minimum and maximum
	for (int i = 0; i < numDataPoints; i++)
	{
		inFile >> data_ptr[i];
		if (data_ptr[i] < minimum)
		{
			minimum = data_ptr[i];
		}
		if (data_ptr[i] > maximum)
		{
			maximum = data_ptr[i];
		}
	}

	// Compute the limits for the axes and world

	// Compute the histogram

	// Compute the theoretical distribution
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'q' || key == 'Q' || key == 27)
	{
		exit(0);
	}
}

void specialKey(int key, int x, int y) // for the arrow keys
{
	//Update the parameters and theoretical distributions
	glutPostRedisplay();
}

void topMenuFunc(int id) // TODO menu items
{
	exit(0);
}

void fileMenuFunction(int id)
{
	// Read file

	// Update projection since the data has changed

	glutPostRedisplay();
}

void histogramMenuFunction(int id)
{
	// Update the number of intervals and recompute the histogram

	// Update the projection since the histogram has changed due to the
	// change of the number of bars
	glutPostRedisplay();
}

void parameterStepMenuFunction(int id)
{
	// Update the parameter step size
}

void createMenu()
{
	// Create the menus
}

void reshape(int w, int h) // TODO reshape func
{
	// Set matrix mode and projection
}

int main(int argc, char** argv) // TODO main method
{
	//Program initialization
	//GLUT initialization
	//Set up callbacks
	//Enter the GLUT main loop
}