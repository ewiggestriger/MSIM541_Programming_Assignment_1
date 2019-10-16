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
int curveType = 0; // normal distro default
int numCurvePoints = 100;
float* curveX = new float[numCurvePoints];
float* curveY1 = new float[numCurvePoints]; // for normal function
float* curveY2 = new float[numCurvePoints]; // for exp function

//Parameters
float mu = 0, sigma = 1;  // Normal distribution
float lambda = 1;	      // Exponential distribution
float parameterStep = 0.05; // Step size for changing parameter values

//Drawing parameters
int width = 800, height = 600;
float world_x_min, world_x_max, world_y_min, world_y_max;
float axis_x_min, axis_x_max, axis_y_min, axis_y_max;

//Compute all the points for normal distribution
void computeNormalFunc(float mu, float sigma)
{
	// This function computes the normal distribution and outputs to arrays
	// Normal distribution formula is y = (1 / sqrt(2*PI)*sigma)*exp(-(x - (mu)^2)/(2*(sigma)^2))
	// Determine the step size and compute the arrays curveX and curveY
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveX[i] += (maximum - minimum) / numCurvePoints;
		curveY1[i] = (1 / sqrt(2 * PI)*sigma)*exp(-0.5*pow(((curveX[i] - mu) / sigma), 2));
	}
}

//Compute all the points for exponential distribution
void computeExponentialFunc(float lambda)
{
	// Exponential distribution formula is y = (1/lambda)*exp(-x/lambda)
	// Determine the step size and compute the arrays curveX and curveY
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveX[i] += (maximum - minimum) / numCurvePoints;
		curveY2[i] = (1/lambda)*exp(-curveX[i]/lambda);
	}
}

void display(void) 
{
	/* clear all pixels */
	glClear(GL_COLOR_BUFFER_BIT);

	//Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(1);
	glColor3f(1.0, 1.0, 1.0); // axes will be white

	// Draw x and y axes
	glBegin(GL_LINES); 
		glVertex2f(axis_x_min/width, axis_y_min/height);
		glVertex2f(axis_x_max/width, axis_y_min/height);
		glVertex2f(axis_x_min/width, axis_y_min/height);
		glVertex2f(axis_x_min/width, axis_y_max/height);
	glEnd();

	// Display the maximum probability value
	// Produce string to be displayed
	stringstream probdens;
	string prbDnsValue;
	probdens << "Probability Density" << endl << maximum;
	probdens.str(prbDnsValue);
	// place text in proper position relative to graph
	glRasterPos2f((axis_x_min / width) + 10, axis_y_max / height);
	printString(prbDnsValue);
	
	// Draw probability histogram
	// set color
	glColor3f(0.0, 1.0, 0.0); // histogram intervals will be green
	// create the intervals, order LR-UR-UL-LL
	for (int i = 0; i < numIntervals; i++)
	{
		glBegin(GL_LINE_LOOP);
			glVertex2f(endPoints[i], axis_y_min);
			glVertex2f(endPoints[i], prob[i]);
			glVertex2f(endPoints[i] - width, prob[i]);
			glVertex2f(endPoints[i] - width, axis_y_min);
		glEnd();
	}

	// Draw the theoretical distribution using thicker lines
	glLineWidth(3);
	glColor3f(1.0, 0.0, 0.0); // theoretical distribution will be red
	glBegin(GL_LINE_STRIP);
	if (curveType == 0) // normal distribution
	{
		for (int j = 0; j < numCurvePoints; j++)
		{
			glVertex2f(curveX[j], curveY1[j]);
		}
	}
	else // exponential distribution
	{
		for (int j = 0; j < numCurvePoints; j++)
		{
			glVertex2f(curveX[j], curveY2[j]);
		}
	}
	glEnd();

	// Compute the top-left position of the annotation
	float topLeftX = axis_x_max * 0.7;
	float topLeftY = axis_y_max * 0.9;

	// File Name UPPER RIGHT
	glRasterPos2f(topLeftX, topLeftY);
	printString("File: " + currentFile);

	// Minimum UPPER RIGHT
	stringstream minval;
	string minUpperRight;
	minval << "Min: " << minimum;
	minval.str(minUpperRight);
	glRasterPos2f(topLeftX, topLeftY - 20.0);
	printString(minUpperRight);

	// Maximum UPPER RIGHT
	stringstream maxval;
	string maxUpperRight;
	maxval << "Max: " << maximum;
	maxval.str(maxUpperRight);
	glRasterPos2f(topLeftX, topLeftY - 40.0);
	printString(maxUpperRight);

	// Number of Intervals UPPER RIGHT
	stringstream numIntv;
	string intervalUpperRight;
	numIntv << "Num. of Intervals: " << numIntervals;
	numIntv.str(intervalUpperRight);
	glRasterPos2f(topLeftX, topLeftY - 60.0);
	printString(intervalUpperRight);

	// Draw theoretical distributions
	stringstream theorD;
	string whichDistro;
	if (curveType == 0)
	{
		theorD << "Distribution: Normal" << endl << "Mu: " << mu << "Sigma: " << sigma;
		theorD.str(whichDistro);
	}
	else
	{
		theorD << "Distribution: Exponential" << endl << "Beta: " << lambda;
		theorD.str(whichDistro);
	}
	glRasterPos2f(topLeftX, topLeftY - 80.0);
	printString(whichDistro);

	glFlush();
	glutSwapBuffers();
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
	float intWidth = range / numIntervals;
	for (int i = 0; i < numIntervals; i++)
	{
		endPoints[i] = minimum + (intWidth * i);
	}
	// Re-initialize the maximum probability after the number of intervals has been changed
	// TODO not sure what this is

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

void readFile(string fileName) 
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
	axis_x_min = minimum - 100.0;
	axis_x_max = maximum + 100.0;
	axis_y_min = 0.0; 
	axis_y_max = 15.0;

	world_x_min = axis_x_min - 25.0;
	world_x_max = axis_x_max + 25.0;
	world_y_min = axis_y_min - 25.0;
	world_y_max = axis_y_max + 25.0;

	// Compute the histogram
	computeProbability(numIntervals);

	// reset theoretical distro parameters to defaults
	float mu = 0, sigma = 1;  // Normal distribution
	float lambda = 1;	      // Exponential distribution

	// Compute the theoretical distribution
	computeNormalFunc(mu, sigma);
	computeExponentialFunc(lambda);
}

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gluOrtho2D(-100.0, 100.0, -100.0, 100.0);
	readFile("Normal.dat"); // initialize with the normal distribution file
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
	if (key == GLUT_KEY_LEFT) // decrease mu
	{
		mu -= parameterStep;
	}
	if (key == GLUT_KEY_RIGHT) // increase mu
	{
		mu += parameterStep;
	}
	if (key == GLUT_KEY_UP) // increase sigma or lambda
	{
		if (curveType == 0)
		{
			sigma += parameterStep;
		}
		else
			lambda +=parameterStep;
	}
	if (key == GLUT_KEY_DOWN) // decrease sigma or lambda
	{
		if (curveType == 0)
		{
			sigma -= parameterStep;
		}
		else
			lambda -= parameterStep;
	}
	glutPostRedisplay();
}

void topMenuFunc(int id) 
{
	exit(0);
}

void fileMenuFunction(int id)
{
	// Read file
	switch (id)
	{
	case 1: readFile("normal.dat");
		break;
	case 2: readFile("expo.dat");
		break;
	case 3: readFile("4.dat");
		break;
	case 4: readFile("16.dat");
		break;
		}
	// Update projection since the data has changed
	glutPostRedisplay();
}

void distributionMenuFunction(int id)
{
	//choose which theoretical distribution to use
	switch (id)
	{
	case 1: curveType = 0;
		break;
	case 2: curveType = 1;
		break;
	}
	// redraw the theoretical distribution curve
	glutPostRedisplay();
}

void histogramMenuFunction(int id)
{
	// Update the number of intervals and recompute the histogram
	switch (id)
	{
	case 1: numIntervals = 30;
		break;
	case 2: numIntervals = 40;
		break;
	case 3: numIntervals = 50;
		break;
	}
	computeProbability(numIntervals);
	// Update the projection since the histogram has changed due to the
	// change of the number of bars
	glutPostRedisplay();
}

void parameterStepMenuFunction(int id)
{
	// Update the parameter step size switch case
	switch (id)
	{
	case 1: parameterStep = 0.05f;
		break;
	case 2: parameterStep = 0.02f;
		break;
	case 3: parameterStep = 0.01f;
		break;
	}
}

void MenuPicker(int id)
{
	switch (id)
	{
	case 5: topMenuFunc(1);
		break;
	}
}

void createMenu()
{
	// Create the menus
	int file_submenu = glutCreateMenu(fileMenuFunction);
	glutAddMenuEntry("normal.dat", 1);
	glutAddMenuEntry("expo.dat", 2);
	glutAddMenuEntry("4.dat", 3);
	glutAddMenuEntry("16.dat", 4);
	int distro_submenu = glutCreateMenu(distributionMenuFunction);
	glutAddMenuEntry("Normal Distribution", 1);
	glutAddMenuEntry("Exponential Distribution", 2);
	int histo_submenu = glutCreateMenu(histogramMenuFunction);
	glutAddMenuEntry("30 Intervals", 1);
	glutAddMenuEntry("40 Intervals", 2);
	glutAddMenuEntry("50 Intervals", 3);
	int param_submenu = glutCreateMenu(parameterStepMenuFunction);
	glutAddMenuEntry("0.05", 1);
	glutAddMenuEntry("0.02", 2);
	glutAddMenuEntry("0.01", 3);
	glutCreateMenu(MenuPicker);
	glutAddSubMenu("File", file_submenu);
	glutAddSubMenu("Distribution", distro_submenu);
	glutAddSubMenu("Histogram", histo_submenu);
	glutAddSubMenu("Parameter Step", param_submenu);
	glutAddMenuEntry("Exit", 5);
}

void reshape(int w, int h) 
{
	// Set matrix mode and projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	width = w;
	height = h;
}

int main(int argc, char** argv) 
{
	//Program initialization
	init();
	//GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("MSIM541 Programming Assignment 1");
	//Set up callbacks
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	createMenu();
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//Enter the GLUT main loop
	glutMainLoop();
}
