/// \file
/// This is the histogram analysis system from Programming Assignment 1.
/// The program reads in a data set from a file, builds a histogram from it,
/// displays it along with its metrics. It then superimposes a theoretical 
/// distribution curve, which the user can select between the normal distribution
/// and the exponential distribution. The user can manipulate the theoretical 
/// curve with the arrow keys in order to determine a best fit to the histogram.
/// The system will also conduct an automatic test for normality on the data set 
/// and display the results on screen.
/// \author Steve Kostoff
/// \version 1.0
/// \date 10/18/2019

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

/// Global variables and data structures
/// defines
constexpr auto PI = 3.1415926f;

/// File names
char* fileName1, *fileName2;
string currentFile;

/// Input data
float* data_ptr;
int numDataPoints;
float minimum, maximum;

/// Histogram 
int numIntervals = 30;
float* endPoints;
float* prob;
float maxProb = -1;

/// Theoretical distributions
int curveType = 0; /// normal distro default, 1 == exponential 
int numCurvePoints = 100;
float* curveXn = new float[numCurvePoints](); /// for normal distribution
float* curveYn = new float[numCurvePoints](); 
float* curveXx = new float[numCurvePoints](); /// for exponential distribution
float* curveYx = new float[numCurvePoints]();
bool isNormal; /// result of normality test

/// Parameters for theoretical distributions
float mu = 0.0, sigma = 1.0;  /// Normal distribution
float lambda = 1.0;	      /// Exponential distribution
float parameterStep = 0.05; /// Step size for changing parameter values

/// Drawing parameters
int width = 800, height = 600;
float world_x_min, world_x_max, world_y_min, world_y_max;
float axis_x_min, axis_x_max, axis_y_min, axis_y_max;

/// This function computes all the x and y coordinate points for the theoretical normal 
/// distribution curve.
/// \param mu Mu is the mean of the distribution
/// \param sigma Sigma is the standard deviation of the distribution
void computeNormalFunc(float mu, float sigma)
{
	float stepSizeN = (7.0) / numCurvePoints; /// 7.0 yields curve that extends from -3.5 to 3.5, good match for standard normal
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveXn[i] = -3.5 + stepSizeN * i; 
		curveYn[i] = (1 / (sqrt(2 * PI))) * exp(-((curveXn[i] - mu)*(curveXn[i] - mu) / (2 * sigma)));
	}
}

/// This function computes all the x and y coordinate points for the theoretical 
/// exponential distribution curve.
/// \param lambda Lambda is the rate parameter for this distribution, equal to the inverse of the mean
void computeExponentialFunc(float lambda)
{
	float stepSizeX = 7.0 / numCurvePoints; /// 7.0 a good fit once projected onto the histogram
	for (int i = 0; i < numCurvePoints; i++)
	{
		curveXx[i] = 0.0 + stepSizeX * i;
		curveYx[i] = (lambda)*exp(-(curveXx[i] * lambda)); /// note using lambda formula, not 1/beta as given in the assignment notes
	}
}

void testForDistro()
{
	/// This function automatically determines which theoretical distribution is the best
	/// fit for the input data. It uses a Jarque-Bera test for normality. Since there are only 
	/// normal and exponential distributions in this assignment, any curve failing this test 
	/// is assumed to be exponential fit.

	/// compute descriptive statistics for data file
	float datafileSum = 0.0;
	for (int i = 0; i < numDataPoints; i++)
	{
		datafileSum = datafileSum + data_ptr[i];
	}
	float fileMean = datafileSum / (float)numDataPoints;
	float fileVar = 0.0;
	float fileStdDev = 0.0;
	float fileSkewness = 0.0;
	float fileKurtosis = 0.0;

	for (int j = 0; j < numDataPoints; j++)
	{
		fileVar += pow((data_ptr[j] - fileMean), 2);
		fileSkewness += pow((data_ptr[j] - fileMean), 3);
		fileKurtosis += pow((data_ptr[j] - fileMean), 4);
	}
	fileVar = fileVar / numDataPoints;
	fileSkewness = (fileSkewness / numDataPoints) / pow(fileVar, 1.5);
	fileKurtosis = (fileKurtosis / numDataPoints) / pow(fileVar, 2);

	/// apply Jarque-Bera test
	float JB = (numDataPoints / 6) * (pow(fileSkewness, 2) + 0.25 * pow((fileKurtosis - 3.0), 2));
	if (JB < 3.0) /// less than 3.0 fits standard normal distribution
	{
		isNormal = true;
	}
	else
		isNormal = false;
}

void display(void) 
{
	/// This function is the OpenGL display callback. It executes all the necessary drawing commands,
	/// including drawing the graph axes, all the display text, the histogram, and the 
	/// theoretical distribution curve.

	/// clear all pixels 
	glClear(GL_COLOR_BUFFER_BIT);

	///Reset modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(1);
	glColor3f(1.0, 1.0, 1.0); /// axes will be white

	/// Draw x and y axes
	glBegin(GL_LINES); 
		glVertex2f(axis_x_min, axis_y_min);
		glVertex2f(axis_x_max, axis_y_min);
		glVertex2f(axis_x_min, axis_y_min);
		glVertex2f(axis_x_min, axis_y_max);
	glEnd();

	/// Display the maximum probability value
	/// place text in proper position relative to graph
	float maxProb = FLT_MIN; /// find max probability density value
	for (int i = 0; i < numIntervals; i++)
		if (prob[i] > maxProb)
			maxProb = prob[i];
	glRasterPos2f((axis_x_min) + 10, axis_y_max);
	printString("Probability Density");
	/// Produce value string to be displayed
	stringstream probdens;
	probdens << maxProb;
	string probDensValue = probdens.str();
	glRasterPos2f(axis_x_min + 10, axis_y_max - 15);
	printString(probDensValue);
	
	/// Draw probability histogram
	/// set color
	glColor3f(0.0, 1.0, 0.0); /// histogram intervals will be green
	/// create the intervals
	float intWidth = (axis_x_max - axis_x_min) / numIntervals; /// width of interval
	float intHeight = (axis_y_max/maxProb) * 0.9; /// height of interval multiplied by scaling factor
	for (int i = 1; i < numIntervals + 1; i++)
	{
		glBegin(GL_LINE_LOOP);
			glVertex2f(axis_x_min + (intWidth * i), axis_y_min);
			glVertex2f(axis_x_min + (intWidth * i), axis_y_min + (prob[i]*intHeight));
			glVertex2f(axis_x_min + (intWidth * i) - intWidth, axis_y_min + (prob[i] * intHeight));
			glVertex2f(axis_x_min + (intWidth * i) - intWidth, axis_y_min);
		glEnd();
	}

	/// Draw the theoretical distribution using thicker lines
	glLineWidth(3);
	glColor3f(1.0, 0.0, 0.0); /// theoretical distribution will be red
	glBegin(GL_LINE_STRIP);
	/// these scaling factors project the actual distributions onto the world coordinates so that 
	/// they best match in size the histogram
	float curveScaleXn = 100; /// normal distribution scale
	float curveScaleYn = 1300;
	float curveScaleXx = 100; /// exponential distribution scale
	float curveScaleYx = 900;
	if (curveType == 0) /// normal distribution
	{
		for (int j = 0; j < numCurvePoints; j++)
		{
			glVertex2f(axis_x_min + (curveXn[j] + 3.5)*curveScaleXn, axis_y_min + curveYn[j]*curveScaleYn);
		}
	}
	else /// exponential distribution
	{
		for (int j = 0; j < numCurvePoints; j++)
		{
			glVertex2f(axis_x_min + curveXx[j] * curveScaleXx, axis_y_min + curveYx[j] * curveScaleYx);
		}
	}
	glEnd();

	/// Compute the top-left position of the annotation
	float topLeftX = axis_x_max * 0.7;
	float topLeftY = axis_y_max * 0.9;

	/// make the histogram-related text green
	glColor3f(0.0, 1.0, 0.0);

	/// File Name UPPER RIGHT
	glRasterPos2f(topLeftX, topLeftY);
	printString("File: " + currentFile);

	/// Minimum UPPER RIGHT
	stringstream minval;
	minval << "Min: " << minimum;
	string minUpperRight = minval.str();
	glRasterPos2f(topLeftX, topLeftY - 20.0);
	printString(minUpperRight);

	/// Maximum UPPER RIGHT
	stringstream maxval;
	maxval << "Max: " << maximum;
	string maxUpperRight = maxval.str();
	glRasterPos2f(topLeftX, topLeftY - 40.0);
	printString(maxUpperRight);

	/// Number of Intervals UPPER RIGHT
	stringstream numIntv;
	numIntv << "Num. of Intervals: " << numIntervals;
	string intervalUpperRight = numIntv.str();
	glRasterPos2f(topLeftX, topLeftY - 60.0);
	printString(intervalUpperRight);

	/// make the theoretical distribution-related text red
	glColor3f(1.0, 0.0, 0.0);

	/// Draw theoretical distributions
	stringstream theorD1;
	stringstream theorD2;
	string whichDistro1;
	string whichDistro2;

	if (curveType == 0)
	{
		theorD1 << "Distribution: Normal";
		whichDistro1 = theorD1.str();
		theorD2 << "Mu: " << mu << " Sigma: " << sigma;
		whichDistro2 = theorD2.str();
	}
	else
	{
		theorD1 << "Distribution: Exponential";
		whichDistro1 = theorD1.str();
		theorD2 << "Lambda: " << lambda;
		whichDistro2 = theorD2.str();
	}
	glRasterPos2f(topLeftX, topLeftY - 80.0);
	printString(whichDistro1);
	glRasterPos2f(topLeftX, topLeftY - 100.0);
	printString(whichDistro2);

	/// Print results of normality test
	glColor3f(0.0, 0.5, 0.5); /// cyan text for normality test
	glRasterPos2f(topLeftX, topLeftY - 120.0);
	if (isNormal == true)
	{
		printString("The distribution is normal");
	}
	else
		printString("The distribution is not normal");

	glFlush();
	glutSwapBuffers();
}

void computeProbability(int numIntervals)
{
	/// This function computes the probability for the histogram. It determines
	/// the number of endPoints and their x axis locations as well as the probability
	/// density for each interval, which is the y axis height. X and y values are stored
	/// in arrays endPoints and prob, respectively.
	/// \param numIntervals The number of histogram intervals to be displayed.

	/// Delete previously allocated memory
	if (endPoints != NULL)
	{
		delete endPoints;
	}

	if (prob != NULL)
	{
		delete prob;
	}

	/// establish arrays for endpoints and probabilities 
	endPoints = new float[numIntervals];
	prob = new float[numIntervals];

	/// Determine the end points for each interval (update the array endPoints)
	float range = maximum - minimum;
	for (int i = 0; i < numIntervals; i++)
	{
		endPoints[i] = minimum + ((range / numIntervals) * i);
	}

	/// Compute the probability for each interval (update the array prob)
	for (int i = 0; i < numIntervals; i++)
	{
		int counter = 0;
		for (int j = 0; j < numDataPoints; j++)
		{
			if (data_ptr[j] <= endPoints[i] && data_ptr[j] > endPoints[i-1])
			{
				counter += 1;
			}
		}
		float pdf = (float)counter / numDataPoints;
		prob[i] = pdf;
	}

}

// TODO revisit world dimensions calculations. Should they be based on the dimensions of the data?
void readFile(string fileName) 
{
	/// This function reads in the contents of the data file desired and 
	/// stores the data in the array data_ptr, and then invokes the
	/// functions to compute the histogram, theoretical probability
	/// distributions, calculate world dimensions, and conduct the test for normality. 
	/// \param fileName Name of the input .DAT file.

	ifstream inFile(fileName);

	if (!inFile.is_open())
	{
		cout << fileName << " couldn't be opened.\n";
		system("pause");
		exit(1);
	}

	inFile >> numDataPoints;

	/// Memory management
	if (data_ptr != NULL)
	{
		delete data_ptr;
	}
	data_ptr = new float[numDataPoints];

	minimum = FLT_MAX;
	maximum = -FLT_MAX;

	/// Read the data and compute the data set minimum and maximum
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

	/// Compute the limits for the axes and world
	world_x_min = 0;
	world_x_max = (float) width;
	world_y_min = 0;
	world_y_max = (float) height;
	axis_x_min = world_x_min + 40.0;
	axis_x_max = world_x_max - 40.0;
	axis_y_min = world_y_min + 30.0; 
	axis_y_max = world_y_max - 30.0;

	/// set current file name
	currentFile = fileName;

	/// Compute the histogram
	computeProbability(numIntervals);

	/// reset theoretical distro parameters to defaults
	float mu = 0, sigma = 1;  /// Normal distribution
	float lambda = 1;	      /// Exponential distribution

	/// Compute the theoretical distribution and test for normality
	computeNormalFunc(mu, sigma);
	computeExponentialFunc(lambda);
	testForDistro();
}

void init(void)
{
	/// This function initializes the program.

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(1.0, 1.0, 1.0); /// draw in white
	readFile("Normal.dat"); /// initialize with the normal distribution file
}

void keyboard(unsigned char key, int x, int y)
{
	/// This function is the OpenGL keyboard callback. It assigns the Q key
	/// the command to exit the program.
	/// \param key, x, y Refer to the Q key in all cases.

	if (key == 'q' || key == 'Q' || key == 27)
	{
		exit(0);
	}
}

void specialKey(int key, int x, int y) 
{
	/// This function assigns actions to the arrow keys. They control the 
	/// change in the mu, sigma, and lambda parameters for the theoretical 
	/// distributions and recalculate and redisplay the theoretical curves 
	/// once pressed. 
	/// \param key, x, y Refers to the arrow keys.

	if (key == GLUT_KEY_LEFT) /// decrease mu and recompute
	{
		mu -= parameterStep;
		computeNormalFunc(mu, sigma);
		computeExponentialFunc(lambda);
		testForDistro();
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_RIGHT) /// increase mu and recompute
	{
		mu += parameterStep;
		computeNormalFunc(mu, sigma);
		computeExponentialFunc(lambda);
		testForDistro();
		glutPostRedisplay();
	}
	if (key == GLUT_KEY_UP) /// increase sigma or lambda and recompute
	{
		if (curveType == 0)
		{
			sigma += parameterStep;
			computeNormalFunc(mu, sigma);
			computeExponentialFunc(lambda);
			testForDistro();
			glutPostRedisplay();
		}
		else
			lambda +=parameterStep;
			computeNormalFunc(mu, sigma);
			computeExponentialFunc(lambda);
			testForDistro();
			glutPostRedisplay();
	}
	if (key == GLUT_KEY_DOWN) /// decrease sigma or lambda and recompute
	{
		if (curveType == 0)
		{
			sigma -= parameterStep;
			computeNormalFunc(mu, sigma);
			computeExponentialFunc(lambda);
			testForDistro();
			glutPostRedisplay();
		}
		else
			lambda -= parameterStep;
			computeNormalFunc(mu, sigma);
			computeExponentialFunc(lambda);
			testForDistro();
			glutPostRedisplay();
	}
}

void topMenuFunc(int id) 
{
	/// This function implements the exit program option from the right-click menu.
	/// \param id Input from the menu action.
	exit(0);
}

void fileMenuFunction(int id)
{
	/// This function implements the file menu program option from the menu. 
	/// It allows the user to select which file he wants to load and display.
	/// It invokes the readFile function on whichever file is chosen.
	/// \param id Input from the menu action.

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
	/// Update projection since the data has changed
	glutPostRedisplay();
}

void distributionMenuFunction(int id)
{
	/// This function implements the distribution menu program option from the menu.
	/// It allows the user to select which theoretical distribution he wants 
	/// to display.
	/// \param id Input from the menu action.

	switch (id)
	{
	case 1: curveType = 0; /// normal
		break;
	case 2: curveType = 1; /// exponential
		break;
	}
	/// redraw the theoretical distribution curve
	glutPostRedisplay();
}

void histogramMenuFunction(int id)
{
	/// This function implements the histogram menu program option from the menu.
	/// It allows the user to select the number of intervals he wants to display.
	/// \param id Input from the menu action.
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
	/// Update the projection since the histogram has changed due to the
	/// change of the number of bars
	glutPostRedisplay();
}

void parameterStepMenuFunction(int id)
{
	/// This function implements the parameter step option from the menu.
	/// It allows the user to select the size of parameter step change 
	/// whenever an arrow key is pressed.
	/// \param id Input from the menu action.

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
	/// This function implements the selection of a menu option from the main menu.
	/// \param id Input from the menu action.

	switch (id)
	{
	case 5: topMenuFunc(1);
		break;
	}
}

void createMenu()
{
	/// This function creates the menus for the program's control menu.

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
	/// This function is the OpenGL reshape callback. It is invoked whenever
	/// the program window's size is changed and recalculate the display to 
	/// maintain proportions at the new window size.
	/// \param w,h These parameters are inputs for the window width and height

	/// Set matrix mode and projection
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
	/// This is the program main function. 
	/// \param argc, argv These are command line arguments, not implemented in this program.

	///Program initialization
	init();
	///GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("MSIM541 Programming Assignment 1");
	///Set up callbacks
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	createMenu();
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	///Enter the GLUT main loop
	glutMainLoop();
}