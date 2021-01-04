/*COSC 3P98: Assignment 3
 *Insect Swarm
 *Author: Mike Young
 *Student ID: 4245718
 *The goal of this assignment was to create an insect swarm out of graphical particles
 *and make them behave more life-like. All the options can be accessed through the GLUT
 *menu, the options in the text menu should do a lot of the major options as well, but
 *it's lacking some, such as only being able to add one insect where as the GLUT menu
 *you can add 5 at a time.You can access the GLUT menu by pressing the middle mouse
 *button.*/

#if !defined(Linux)
  #include <windows.h>           //Not Linux must be windows
#else
	#include <stdlib.h>
#endif

#include <stdio.h>
#include <glut.h>
#include <math.h>
#include <time.h>
#include <FreeImage.h>

#define X 0
#define Y 1
#define Z 2
#define POINTS 1
#define EXPLORATION 1
#define SWARMING 0
#define WIRE_FRAME 2
#define POLYGON 3
#define TRUE 1
#define FALSE 0
#define MAX_INSECT_DIST 25
#define MIN_INSECT_DIST 15
#define INSECT_SIZE 3
#define GOURAUD 1
#define FLAT 0
int shading;
int insectState;
int trailModeEnabled;
int smoothTurning;
int lifeSpan;
int insectModelType;
int limitedLife;
int population;
int speed;
int world_width = 100;
int world_height = 100;
int world_depth = 100;
int worldPoints[][3] = {{100,100,100}, {100,-100,100}, {-100,-100,100}, {-100,100,100},
                 {100,100,-100}, {100,-100,-100}, {-100,-100,-100}, {-100,100,-100}};

int worldEdges[][4] = {{0,3,2,1},
			  {3,7,6,2},
			  {7,4,5,6},
			  {4,0,1,5}, 
			  {0,4,7,3},
			  {1,2,6,5}};

struct glob {
   float angle[3];
   int axis;
};

typedef struct{
	double pos[3];
}position;

//trail of a particle
typedef struct{
	position path[50]; 
}trail;
trail insectTrail;

//pixel structure
typedef struct pix
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} pixel;
pixel* insectTexture;
//the colour structure
typedef struct {
	GLubyte r, g, b;
} color;

//the global structure
typedef struct {
	pixel *data;
	int w, h;
} texture;

//represents the target location for the leader to swarm towards
typedef struct {
	int coords[3];//coordinates for the desination
} destination;
destination target;

//Particle
typedef struct particle
{
	double pos[3];//position vector
	double dir[3];//direction vector
	int speed;
	struct particle* next;
	int isLeader;
	int target[3];
	color colour;
}particle;
particle* head,*leader;

//redraws the image
void redraw(void) {
   glClear(GL_COLOR_BUFFER_BIT);
}//redraw

struct glob global= {{0.0,0.0,0.0},Y};

//Displays the commands the user can enter to interact with the insect swarm
void showMenu()
{
	printf("********************************************\n");
	printf("********\n");
	printf("* MENU *\n");
	printf("********\n");
	printf("Use the glut menu(middle mouse button) to find more precise options for the insect swarm.\n\n");
	printf("To Start:\n");
	printf("	I: Initialize\n");
	printf("General:\n");
	printf("	X: Rotate on X-axis\n");
	printf("	Y: Rotate on Y-axis\n");
	printf("	Z: Rotate on Z-axis\n");
	printf("	R: Reset Alignment\n");
	printf("Speed:\n");
	printf("	S: Random\n");
	printf("Life Span:\n");
	printf("	L: Toggle On/Off(Default 10 secs)\n");
	printf("Insects:\n");
	printf("	A: Add an Insect\n");
	printf("	V: Swarming Mode\n");
	printf("	B: Exploration Mode\n");
	printf("Shading:\n");
	printf("	H: Flat\n");
	printf("	G: Gouarud\n");
	printf("Smooth Turning:\n");
	printf("	T: Toggle On/Off\n");
	printf("Trail Mode:\n");
	printf("	M: Toggle On/Off\n");
	printf("********************************************\n");
}

//Initialize lighting
void initLighting() {
   GLfloat ambient[] = {0.1, 0.1, 0.1, 1.0};
   GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat specular[] = {1.0, 1.0, 1.0, 1.0};
   GLfloat position[] = {1.0, 1.0, 1.0, 0.0};
   GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
   GLfloat local_view[] = {0.0};

   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
   glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

   glEnable(GL_LIGHTING);   /* turns on lighting */
   glEnable(GL_LIGHT0);     /* turns on light 0  */
   glEnable(GL_NORMALIZE);
}//initLighting

//Initializes texture
void initTexture()
{
	int i,j;
	int tile;
	pixel* texMap;
	texMap=(pixel*)malloc(sizeof(pixel)*100*100);
	for (i=0;i<100;i++)
	{
		for (j=0;j<100;j++)
		{
			tile=(1+(int)(i/50)-(int)(j/50))%2;
			texMap[i*100+j].g=texMap[i*100+j].r=255*tile;
			texMap[i*100+j].b=255*(1-tile);
			texMap[i*100+j].a=255;
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);	// Use texture number 0

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, 100, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, texMap);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 100, 100, GL_RGBA, GL_UNSIGNED_BYTE, texMap);

	glEnable(GL_TEXTURE_2D);

	free(texMap);
}//initTexture

/*Draws the world in the display. The world is a box with a ground and 
  4 walls: 2 visible,2 invisible walls*/
void drawWorld(void) {

  //float c[][3] = {{1.0,0,0},{0,1.0,0},{1.0,1.0,1.0},
//		 {0,0,1.0},{.6,0,.6},{0,.6,.6}};
float norm[][3] = {{0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0}};
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glRotatef(global.angle[X], 1.0, 0.0, 0.0);
  glRotatef(global.angle[Y], 0.0, 1.0, 0.0);
  glRotatef(global.angle[Z], 0.0, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glColor3f(1.0,0.0,0.0);
     glBegin(GL_LINE_STRIP);
        glVertex3iv(worldPoints[worldEdges[0][0]]);
        glVertex3iv(worldPoints[worldEdges[0][1]]);
        glVertex3iv(worldPoints[worldEdges[0][2]]);
        glVertex3iv(worldPoints[worldEdges[0][3]]);
     glEnd();

     glBegin(GL_LINE_STRIP);
        glVertex3iv(worldPoints[worldEdges[1][0]]);
        glVertex3iv(worldPoints[worldEdges[1][1]]);
        glVertex3iv(worldPoints[worldEdges[1][2]]);
        glVertex3iv(worldPoints[worldEdges[1][3]]);
     glEnd();

	 //backgound 1
	 glColor3f(0.1,0.45,1.0);//sky blue
     glBegin(GL_QUADS);
        glVertex3iv(worldPoints[worldEdges[2][0]]);
        glVertex3iv(worldPoints[worldEdges[2][1]]);
		glColor3f(0.0,0.17,0.0);//grass
        glVertex3iv(worldPoints[worldEdges[2][2]]);
        glVertex3iv(worldPoints[worldEdges[2][3]]);
     glEnd();

  //background 2
  glColor3f(0.1,0.45,1.0);//sky blue
     glBegin(GL_QUADS);
        glVertex3iv(worldPoints[worldEdges[3][0]]);
        glVertex3iv(worldPoints[worldEdges[3][1]]);
		glColor3f(0.0,0.17,0.0);//grass
        glVertex3iv(worldPoints[worldEdges[3][2]]);
        glVertex3iv(worldPoints[worldEdges[3][3]]);
     glEnd();

	 glColor3f(1.0,0.0,0.0);
     glBegin(GL_LINE_STRIP);
        glVertex3iv(worldPoints[worldEdges[4][0]]);
        glVertex3iv(worldPoints[worldEdges[4][1]]);
        glVertex3iv(worldPoints[worldEdges[4][2]]);
        glVertex3iv(worldPoints[worldEdges[4][3]]);
     glEnd();

  	 glColor3f(0.5,0.2,0.0);//brown
	 glBegin(GL_QUADS);

	 		glColor3f(0.0,0.2,0.0);//grass
        glVertex3iv(worldPoints[worldEdges[5][0]]);
		glColor3f(0.5,0.2,0.0);//brown
        glVertex3iv(worldPoints[worldEdges[5][1]]);
		glColor3f(0.0,0.2,0.0);//grass
        glVertex3iv(worldPoints[worldEdges[5][2]]);
        glVertex3iv(worldPoints[worldEdges[5][3]]);
     glEnd();
  glFlush();
}//drawWorld

//draws all the particles in the list
void drawInsects()
{
	int i;
	particle* curr;
	int insect[][3] = {{1,1,1}, {1,-1,1}, {-1,-1,1}, {-1,1,1},
                 {1,1,-1}, {1,-1,-1}, {-1,-1,-1}, {-1,1,-1}};
	float norm[][3] = {{0,0,1.0},{-1.0,0,0},{0,0,-1.0},{1.0,0,0},{0,1.0,0},{0,-1.0,0}};

	curr = leader;
	switch(insectModelType)
	{
	case 1://Points
		for(i=0;i<population;i++)
		{
			//glNormal3fv(norm[i]);
			glPointSize(5);
			glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);

			glBegin(GL_POINTS);
				glVertex3f(curr->pos[X],curr->pos[Y],curr->pos[Z]);
			glEnd();

			curr = curr->next;
		}
		break;
	case 2://Wireframe
		for(i=0;i<population;i++)
		{
			//glNormal3fv(norm[i]);
			glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
			  //Face 1
			  glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
			 glEnd();

			 //Face 2
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
			 glEnd();

			 //Face 3
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();

			 //Face 4
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();

			 //Face 5
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
			 glEnd();

			 //Face 6
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();
		}
		break;
	case 3://Polygon
		for(i=0;i<population;i++)
		{
			//glNormal3fv(norm[i]);
			  glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
			  //Face 1
			  glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
			 glEnd();
			 glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
				
			 //Face 2
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
			 glEnd();
			 glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);

			 //Face 3
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();
			 glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);

			 //Face 4
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();
			 glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);

			 //Face 5
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y],curr->pos[Z]);
			 glEnd();
			 glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);

			 //Face 6
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y]-1,curr->pos[Z]);
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]);
				if(shading == GOURAUD)
				{
					glColor3f(1.0,0.0,1.0);//grass
				}
				glVertex3d(curr->pos[X]-INSECT_SIZE,curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
				glVertex3d(curr->pos[X],curr->pos[Y]-INSECT_SIZE,curr->pos[Z]-INSECT_SIZE);
			 glEnd();
			  glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
			  curr = curr->next;
		}
		break;
	}
	
	
	glPointSize(5);
	glBegin(GL_POINTS);
		glVertex3i(target.coords[X],target.coords[Y],target.coords[Z]);
	 glEnd();
	glFlush();
	
}//drawInsects

//Returns the distance between the given insect and the leader target
double pointToTargetDistance(particle* point,int x,int y,int z)
{
	return sqrt((double)((x-point->pos[X])*(x-point->pos[X]) + (y-point->pos[Y])*(y-point->pos[Y])) + (z-point->pos[Z])*(z-point->pos[Z]));
}//pointToTargetDistance

//returns the distance between two insects
double insectToInsectDistance(particle* insect1,particle* insect2)
{
	return sqrt((double)((insect2->pos[X]-insect1->pos[X])*(insect2->pos[X]-insect1->pos[X]) + (insect2->pos[Y]-insect1->pos[Y])*(insect2->pos[Y]-insect1->pos[Y])) + (insect2->pos[Z]-insect1->pos[Z])*(insect2->pos[Z]-insect1->pos[Z]));
}//insectToInsectDistance

//Creates a new target for a given particle
void createNewTarget(particle* curr)
{
	curr->target[X] = (double)(rand()%199 - world_width);
	curr->target[Y] = (double)(rand()%199 - world_height);
	curr->target[Z] = (double)(rand()%199 - world_depth);
}//createNewTarget

//sets a new target for all the insects
void setInsectTargets()
{
	particle* curr;
	curr = leader;
	while(curr)
	{
		createNewTarget(curr);
		curr=curr->next;
	}
}//setInsectTargets

//Determines whether or not the given particle is approaching the target destination
int approachTarget(particle* insect)
{
	double dist;
	dist = pointToTargetDistance(insect,insect->target[X],insect->target[Y],insect->target[Z]);
	if(dist<20)
	{
		return TRUE;
	}
	return FALSE;
}//approachTarget


//determines if the given insect is too close to another
int tooCloseToInsect(particle* insect)
{
	particle* curr;
	curr = leader;

	while(curr)
	{
		if(insectToInsectDistance(insect,curr)<=MIN_INSECT_DIST)
		{
			return TRUE;
		}
		curr= curr->next;
	}
	return FALSE;
}//tooCloseToInsect

//Determines if the given insect is too far away from an insect
int tooFarFromInsect(particle* insect)
{
	particle* curr;
	curr = leader;
	while(curr)
	{
		if(insectToInsectDistance(insect,curr)>=MAX_INSECT_DIST)
		{
			return TRUE;
		}
		curr= curr->next;
	}
	return FALSE;
}//tooFarFromInsect


//Determines if a given insect's next move will stay in the world
int withinWorldBounds(particle* curr)
{
	if(curr->pos[X]+curr->dir[X] >= world_width-INSECT_SIZE || curr->pos[X]+curr->dir[X] < -world_width+INSECT_SIZE)
	{
		return FALSE;
	}
	if(curr->pos[Y]+curr->dir[Y] >= world_height-INSECT_SIZE || curr->pos[Y]+curr->dir[Y] < -world_height+INSECT_SIZE)
	{
		return FALSE;
	}
	if(curr->pos[Z]+curr->dir[Z] >= world_depth-INSECT_SIZE || curr->pos[Z]+curr->dir[Z] < -world_depth+INSECT_SIZE)
	{
		return FALSE;
	}
	return TRUE;
}//withinWorldBound


//Returns the magnitude of the vector represented by the 3 x,y,z paramters.
double magnitude(double x,double y,double z)
{
	double mag = sqrt((x*x+y*y+z*z));
	return mag;
}//magnitude

//finds the closest insect given a specific insect
particle* findClosestInsect(particle* insect)
{
	particle* curr;
	particle* closestInsect;
	double distance,smallestDistance;
	closestInsect = leader;
	curr = leader;
	smallestDistance = insectToInsectDistance(insect,curr);
	while(curr)
	{
		distance=insectToInsectDistance(insect,curr);
		if(distance<smallestDistance && (curr->pos[X]!=insect->pos[X] && curr->pos[Y]!=insect->pos[Y] && curr->pos[Z]!=insect->pos[Z]))
		{
			smallestDistance = distance;
			closestInsect = curr;
		}
		curr = curr->next;
	}
	return closestInsect;
}//findClosestInsect

//Adjusts the breathing space between two insects
void adjustBreathingSpace(particle* moveMe,particle* neighbour)
{
	double xCoord,yCoord,zCoord,mag;
	xCoord = neighbour->pos[X] - moveMe->pos[X];
	yCoord = neighbour->pos[Y] - moveMe->pos[Y];
	zCoord = neighbour->pos[Z] - moveMe->pos[Z];
	mag = magnitude(xCoord,yCoord,zCoord);
	moveMe->dir[X] = -(xCoord/mag);
	moveMe->dir[Y] = -(yCoord/mag);
	moveMe->dir[Z] = -(yCoord/mag);
}//adjustBreathingSpace

/*Sets the direction vection of the current insect to the
 *direction of the closest insect that is more than the 
 *MAX_INSECT_DIST away*/
void moveTowardsClosest(particle* curr, particle* target)
{
	double xCoord,yCoord,zCoord,mag;
	xCoord = target->pos[X] - curr->pos[X];
	yCoord = target->pos[Y] - curr->pos[Y];
	zCoord = target->pos[Z] - curr->pos[Z];
	mag = magnitude(xCoord,yCoord,zCoord);
	curr->dir[X] = (xCoord/mag);
	curr->dir[Y] = (yCoord/mag);
	curr->dir[Z] = (zCoord/mag);
}//moveTowardsClosest

/*Determines if the next move will be a clear move, 
 * meaning there isn't an insect in the next position*/
int clearToMove(double position[3])
{
	particle* curr;
	curr = leader;
	while(curr)
		{
			if(position[X] == curr->pos[X] && position[Y] == curr->pos[Y] && position[Z] == curr->pos[Z])
			{
				FALSE;
			}
			curr = curr->next;
		}
	return TRUE;
}

/* Updates the given particales position vector, direction vector, and anything
   else that needs updating*/
void updateParticle(particle* curr)
{
	double xCoord,yCoord,zCoord,mag,neighbour_dist;
	double tempPos[3];
	particle* neighbour;//represent the closest insect to the current
	neighbour = findClosestInsect(curr);
	neighbour_dist = insectToInsectDistance(curr, neighbour);

	/*Draw Trail Mode*/
	if(trailModeEnabled == TRUE)
	{
		glPointSize(1);
		glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
		glBegin(GL_POINTS);
				glVertex3f(curr->pos[X],curr->pos[Y],curr->pos[Z]);
		glEnd();
	}

	if(curr->isLeader)
	{
		/*If the leader is approaching the target, make a new target*/
		if(approachTarget(curr)==TRUE)
		{
			createNewTarget(curr);
			xCoord = curr->target[X] - curr->pos[X];
			yCoord = curr->target[Y] - curr->pos[Y];
			zCoord = curr->target[Z] - curr->pos[Z];
			mag = magnitude(xCoord,yCoord,zCoord);

			curr->dir[X] = (xCoord/mag);
			curr->dir[Y] = (yCoord/mag);
			curr->dir[Z] = (zCoord/mag);
			if(withinWorldBounds(curr)==FALSE)
			{
				curr->dir[X]=-curr->dir[X];
				curr->dir[Y]=-curr->dir[Y];
				curr->dir[Z]=-curr->dir[Z];
			}
		}

		xCoord = curr->target[X] - curr->pos[X];
		yCoord = curr->target[Y] - curr->pos[Y];
		zCoord = curr->target[Z] - curr->pos[Z];
		mag = magnitude(xCoord,yCoord,zCoord);
		curr->dir[X] = (xCoord/mag);
		curr->dir[Y] = (yCoord/mag);
		curr->dir[Z] = (zCoord/mag);

		if(smoothTurning == TRUE)
			{
				curr->dir[X] = curr->dir[X] + 0.3*(xCoord);
				curr->dir[Y] = curr->dir[Y] + 0.3*(yCoord);
				curr->dir[Z] = curr->dir[Z] + 0.3*(zCoord);
			}
	
		curr->pos[X] = (curr->pos[X])+(curr->dir[X])*speed;
		curr->pos[Y] = (curr->pos[Y])+(curr->dir[Y])*speed;
		curr->pos[Z] = (curr->pos[Z])+(curr->dir[Z])*speed;
	}
	else
	{
		if(insectState == SWARMING)
		{
			curr->target[X] = leader->pos[X];
			curr->target[Y] = leader->pos[Y];
			curr->target[Z] = leader->pos[Z];
		}
		else if(insectState == EXPLORATION)
		{
			
		}
		xCoord = curr->target[X] - curr->pos[X];
		yCoord = curr->target[Y] - curr->pos[Y];
		zCoord = curr->target[Z] - curr->pos[Z];
		mag = magnitude(xCoord,yCoord,zCoord);
		if(neighbour_dist <=MIN_INSECT_DIST)
		{
			/*Move away from neighbour*/
			xCoord = neighbour->pos[X] - curr->pos[X];
			yCoord = neighbour->pos[Y] - curr->pos[Y];
			zCoord = neighbour->pos[Z] - curr->pos[Z];

			curr->dir[X] = -(xCoord/mag);
			curr->dir[Y] = -(yCoord/mag);
			curr->dir[Z] = -(zCoord/mag);
		}
		else if(neighbour_dist >=MAX_INSECT_DIST)
		{
			/*Move towards neighbour*/
			moveTowardsClosest(curr,neighbour);
			curr->dir[X] = (xCoord/mag);
			curr->dir[Y] = (yCoord/mag);
			curr->dir[Z] = (zCoord/mag);
		}
		else
		{
			curr->dir[X] = (xCoord/mag);
			curr->dir[Y] = (yCoord/mag);
			curr->dir[Z] = (zCoord/mag);
		}
		/*If the leader is approaching the target, make a new target*/
		if(approachTarget(curr)==TRUE)
		{
			createNewTarget(curr);
			xCoord = curr->target[X] - curr->pos[X];
			yCoord = curr->target[Y] - curr->pos[Y];
			zCoord = curr->target[Z] - curr->pos[Z];
			mag = magnitude(xCoord,yCoord,zCoord);

			curr->dir[X] = (xCoord/mag);
			curr->dir[Y] = (yCoord/mag);
			curr->dir[Z] = (zCoord/mag);
		}
		if(withinWorldBounds(curr)==FALSE)
		{
			curr->dir[X]=-curr->dir[X];
			curr->dir[Y]=-curr->dir[Y];
			curr->dir[Z]=-curr->dir[Z];
		}
			
		if(smoothTurning == TRUE)
		{
			curr->dir[X] = curr->dir[X] + 0.3*(xCoord);
			curr->dir[Y] = curr->dir[Y] + 0.3*(yCoord);
			curr->dir[Z] = curr->dir[Z] + 0.3*(zCoord);
		}

		tempPos[X] = (curr->pos[X])+(curr->dir[X])*speed;
		tempPos[Y] = (curr->pos[Y])+(curr->dir[Y])*speed;
		tempPos[Z] = (curr->pos[Z])+(curr->dir[Z])*speed;

		if(clearToMove(tempPos)==FALSE)
		{
			/*Don't update the position*/
		}
		else
		{
			/*Update Insect Position*/
			curr->pos[X] = (curr->pos[X])+(curr->dir[X])*speed;
			curr->pos[Y] = (curr->pos[Y])+(curr->dir[Y])*speed;
			curr->pos[Z] = (curr->pos[Z])+(curr->dir[Z])*speed;
			/************************/
		}
	}
}//updateParticle

/* This is what happens in one frame of animation. Each particle's
   position is updated depending on their current direction vector.*/
void renderFrame(void)
{
	particle* curr;
	curr = leader;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	drawWorld();
	drawInsects();
	while(curr)
	{
		updateParticle(curr);
		curr=curr->next;
	}
	glFlush();
	glutSwapBuffers();
}//renderFrame

//Creates the linked list representing each insect.
void initiateInsects()
{
	int i;
	double xCoord,yCoord,zCoord,mag;
	char num[6] = "";
	particle* curr;

	/*Initialize the first target*/
	target.coords[X] = 0.0;
	target.coords[Y] = 0.0;
	target.coords[Z] = 0.0;
	/*****************************/
	printf("Enter the amount of particles you desire:\n");
	fgets(num,6,stdin);
	population = atoi(num);

	head = NULL;

	/*Leader Particle*/
	leader = (particle*)malloc(sizeof(particle));
	leader->pos[X] = rand()%((world_height+world_height)-1) - world_width;
	leader->pos[Y] = rand()%((world_height+world_height)-1) - world_height;
	leader->pos[Z] = rand()%((world_height+world_height)-1) - world_depth;
	
	xCoord = target.coords[X] - leader->pos[X];
	yCoord = target.coords[Y] - leader->pos[Y];
	zCoord = target.coords[Z] - leader->pos[Z];
	mag = magnitude(xCoord,yCoord,zCoord);
	leader->dir[X] = (xCoord/mag);
	leader->dir[Y] = (yCoord/mag);
	leader->dir[Z] = (zCoord/mag);
	leader->colour.r = 255;
	leader->colour.b = 0;
	leader->colour.g = 0;
	leader->speed = speed;
	leader->isLeader = TRUE;
	leader->target[X] = 0.0;
	leader->target[Y] = 0.0;
	leader->target[Z] = 0.0;
	leader->next = head;
	head = leader;
	/*****************/

	/***The rest of the insects***/
	for(i=1;i<population;i++)
	{
		curr = (particle*)malloc(sizeof(particle));
		curr->pos[X] = rand()%69 - world_width;
		curr->pos[Y] = rand()%69 - world_height;
		curr->pos[Z] = rand()%69 - world_depth;
		xCoord = leader->pos[X] - curr->pos[X];
		yCoord = leader->pos[Y] - curr->pos[Y];
		zCoord = leader->pos[Z] - curr->pos[Z];
		mag = magnitude(xCoord,yCoord,zCoord);
		curr->dir[X] = (xCoord/mag);
		curr->dir[Y] = (yCoord/mag);
		curr->dir[Z] = (zCoord/mag);
		curr->colour.r = 255;
		curr->colour.b = 255;
		curr->colour.g = 255;
		curr->speed = speed;
		curr->isLeader = FALSE;
		curr->target[X] = leader->pos[X];
		curr->target[Y] = leader->pos[Y];
		curr->target[Z] = leader->pos[Z];
		curr->next = NULL;

		head->next = curr;
		head = head->next;
	}
	/**************************/
}//initiateInsects

/* Adds a particle to the insect swarm.
   Can add either 1 or 5 at a time depending on the key.*/
void addParticle(int key)
{
	int i;
	double xCoord,yCoord,zCoord;
	particle* curr;
	particle* newInsect;

	if(population <= 0)
	{
		printf("Cannot add particles if the insect swarm hasn't been initialized yet.\n");
	}
	else
	{
		curr = leader;
		//iterate to end of list
		while(curr->next)
		{
			curr = curr->next;
		}
		
		//depending on which was chosen add 1 or 5
		switch(key)
		{
			case'6':
				newInsect = (particle*)malloc(sizeof(particle));
				newInsect->pos[X] = rand()%69 - world_width;
				newInsect->pos[Y] = rand()%69 - world_height;
				newInsect->pos[Z] = rand()%69 - world_depth;
				xCoord = leader->pos[X] - curr->pos[X];
				yCoord = leader->pos[Y] - curr->pos[Y];
				zCoord = leader->pos[Z] - curr->pos[Z];
				newInsect->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
				newInsect->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
				newInsect->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
				newInsect->colour.r = 255;
				newInsect->colour.b = 255;
				newInsect->colour.g = 255;
				newInsect->speed = speed;
				newInsect->isLeader = FALSE;
				newInsect->target[X] = leader->pos[X];
				newInsect->target[Y] = leader->pos[Y];
				newInsect->target[Z] = leader->pos[Z];
				newInsect->next = NULL; 
				curr->next = newInsect;
				population++;
				break;
			case'7':
				for(i=0;i<5;i++)
				{
					newInsect = (particle*)malloc(sizeof(particle));
					newInsect->pos[X] = rand()%69 - world_width;
					newInsect->pos[Y] = rand()%69 - world_height;
					newInsect->pos[Z] = rand()%69 - world_depth;
					xCoord = leader->pos[X] - curr->pos[X];
					yCoord = leader->pos[Y] - curr->pos[Y];
					zCoord = leader->pos[Z] - curr->pos[Z];
					newInsect->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
					newInsect->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
					newInsect->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
					newInsect->colour.r = 255;
					newInsect->colour.b = 255;
					newInsect->colour.g = 255;
					newInsect->speed = speed;
					newInsect->isLeader = FALSE;
					newInsect->target[X] = leader->pos[X];
					newInsect->target[Y] = leader->pos[Y];
					newInsect->target[Z] = leader->pos[Z];
					newInsect->next = NULL; 
					curr->next = newInsect;
					curr = curr->next;
					population++;
				}
				break;
		}
	}
}//addParticle

//changes the speed of the particles based on what the user entered
void changeSpeed (char value)
{
	switch(value)
	{
	case '+':
	case '>':
	case '.':
		if(speed < 5)
		{
			speed++;
		}
		else
		{
			printf("The speed cannot go any higher.\n",speed);
		}
		break;
	case '-':
	case '<':
	case ',':
		if(speed > 0)
		{
			speed--;
		}
		else
		{
			printf("The speed cannot go any lower.\n",speed);
		}
		break;
	case '0':
		speed = 0;
		break;
	case '1':
		speed = 1;
		break;
	case '2':
		speed = 2;
		break;
	case '3':
		speed = 3;
		break;
	case '4':
		speed = 4;
		break;
	case '5':
		speed = 5;
		break;
	case 's':
		speed = rand()%6;
		break;
	}
	printf("The current speed is:%d\n",speed);
}//changeSpeed

/**************************************************************************/
/**************************User Input Handling*****************************/
/**************************************************************************/
//This function handles the events when a option is clicked from the menu
void processMenuEvents(int key) {

	switch (key) 
	{
	case 'q':
		exit(0);
		break;
	case 's':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '+':
	case '-':
	case '<'://decrease speed
	case '>'://increase speed
		changeSpeed(key);
		break;
	case '6'://add one particle
	case '7'://add 5 particles
		addParticle(key);
		break;
	case 'c':
		initiateInsects();
		break;
	case 'r':
		global.angle[X] = 0.0;
		global.angle[Y] = 0.0;
	    global.angle[Z] = 0.0;
	    glPopMatrix();
	    glPushMatrix();
		 break;
	case 'w':
		insectModelType = WIRE_FRAME;
		break;
	case 'o':
		insectModelType = POLYGON;
		break;
	case 'p':
		insectModelType = POINTS;
		break;
	case 'h':
		shading = FLAT;
		printf("Shading is set to: FLAT\n");
		break;
	case 'g':
		shading = GOURAUD;
		printf("Shading is set to: GOURAUD\n");
		break;
	case 'm':
		trailModeEnabled = !trailModeEnabled;
		if(trailModeEnabled == TRUE)
		{
			printf("Trail Mode has been enabled.\n");	
		}
		else
		{
			printf("Trail Mode has been disabled.\n");	
		}
		break;
	case 't':
		smoothTurning = !smoothTurning;
		if(smoothTurning == TRUE)
		{
			printf("Smooth turning is on.\n");	
		}
		else
		{
			printf("Smooth turning is off.\n");	
		}
		break;
	case 'v':
		insectState = SWARMING;
		printf("Insect state is set to Swarming.\n");
		break;
	case 'b':
		insectState = EXPLORATION;
		setInsectTargets();
		printf("Insect state is set to Exploration.\n");
		break;
	}
}

//GLUT Menu to better interact with the graphics
void createGLUTMenus() 
{
	int menu,rotateWorld_submenu,addParticle_submenu,particleSpeed_submenu,particleModel_submenu,shading_submenu,particleState_submenu;

	//Rotate World submenu
	rotateWorld_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("X",'x');
	glutAddMenuEntry("Y",'y');
	glutAddMenuEntry("Z",'z');

	//Particle State submenu
	particleState_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Swarming",'v');
	glutAddMenuEntry("Exploration",'b');

	//Shading submenu
	shading_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Flat",'h');
	glutAddMenuEntry("Gouraud",'g');

	//Particle Model Submenu
	particleModel_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Wire Frame",'w');
	glutAddMenuEntry("Polygon",'o');
	glutAddMenuEntry("Points",'p');

	//Add Particle submenu
	addParticle_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("1",'6');
	glutAddMenuEntry("5",'7');
	
	//Particle Speed
	particleSpeed_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Random(0-5)",'s');
	glutAddMenuEntry("+1",'+');
	glutAddMenuEntry("-1",'-');
	glutAddMenuEntry("Speed: 1",'1');
	glutAddMenuEntry("Speed: 2",'2');
	glutAddMenuEntry("Speed: 3",'3');
	glutAddMenuEntry("Speed: 4",'4');
	glutAddMenuEntry("Speed: 5",'5');

	//Main menu
	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Start",'c');
	glutAddMenuEntry("Reset",'r');
	
	glutAddSubMenu("Rotate World",rotateWorld_submenu);
	glutAddSubMenu("Particle State",particleState_submenu);
    glutAddSubMenu("Add Particle",addParticle_submenu);
	glutAddSubMenu("Particle Speed",particleSpeed_submenu);
	glutAddSubMenu("Particle Model",particleModel_submenu);
	glutAddSubMenu("Shading",shading_submenu);

	glutAddMenuEntry("Smooth Turning[Toggle]",'t');
	glutAddMenuEntry("Trail Mode[Toggle]",'m');
	glutAddMenuEntry("Quit",'q');

	glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

//keyboard commands
void keyboard(unsigned char key, int x, int y) {

   switch (key)
   {
      case 'x':
      case 'X':
		global.axis = X;
		break;
	  case 'a':
      case 'A':
		addParticle(6);
		break;
      case 'y':
      case 'Y':
		global.axis = Y;
		break;
      case 'z':
      case 'Z':
		global.axis = Z;
		break;

	  case 'r':
      case 'R':
		global.angle[X] = 0.0;
		global.angle[Y] = 0.0;
	    global.angle[Z] = 0.0;
	    glPopMatrix();
	    glPushMatrix();
		  break;
	  case 'g':
		shading = GOURAUD;
		break;
	  case 'h':
		shading = FLAT;
		break;
	  case 'm':
	  case 'M':
		trailModeEnabled = !trailModeEnabled;
		break;

	  case 'i':
      case 'I':
		initiateInsects();
		break;

	  case 's':
      case 'S':
	  case '<':
      case '>':
      case ',':
      case '.':
		changeSpeed(key);
		break;

	  case 't':
		smoothTurning = !smoothTurning;
		if(smoothTurning == TRUE)
		{
			printf("Smooth turning is on.\n");	
		}
		else
		{
			printf("Smooth turning is off.\n");	
		}
		break;
		break;

      case 0x1B:
      case 'q':
      case 'Q':
  		exit(0);

	  case GLUT_KEY_LEFT:
		  global.angle[global.axis] = global.angle[global.axis] + 0.2;
		  break;
      case GLUT_KEY_RIGHT:
			global.angle[global.axis] = global.angle[global.axis] - 0.2;
		break;
	  case 'v':
	  case 'V':
		insectState = SWARMING;
		printf("Insect state is set to Swarming.\n");
		break;
	  case 'b':
	  case 'B':
		insectState = EXPLORATION;
		setInsectTargets();
		printf("Insect state is set to Exploration.\n");
		break;
   }
}//keyboard

void mouse(int btn, int state, int x, int y) 
{
if (state == GLUT_DOWN) {
      if (btn==GLUT_LEFT_BUTTON) {
  	  global.angle[global.axis] = global.angle[global.axis] + 0.007;
      }
      else if (btn == GLUT_RIGHT_BUTTON) {
	  global.angle[global.axis] = global.angle[global.axis] - 0.007;
      }
   }
}//mouse
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/


void test()
{
	int x,i;
	
	for(i=0;i<100;i++)
	{
		srand(i);
		x = rand()%((world_height+world_height)-1) - world_height;
		printf("x= %d\n",x);
	}
}

int main(int argc, char **argv) {

  time_t times;
  times = time(NULL);
  population = 0;
  limitedLife = FALSE;
  lifeSpan = 15;
  insectModelType = POLYGON;
  shading = FLAT;
  speed=1;
  trailModeEnabled = FALSE;
  insectState = SWARMING;
  showMenu();
  glutInit(&argc, argv);
  glutInitWindowSize(500, 500);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Insect Swarm");
  glutMouseFunc(mouse);
  glutKeyboardFunc(keyboard);
  glutDisplayFunc(redraw);
  glutIdleFunc(renderFrame);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //gluPerspective(60.0, 1.5, 0.0, 100.0);
  //gluLookAt(0.0, 250.0, 250.0, 10.0, 100.0, 100.0, 0.0, 1.0, 0.0);
  glOrtho(-190.0, 190.0, -190.0, 190.0, -190.0, 190.0);
  glRotatef(30.0, 1.0, 0.0, 0.0); 
  glRotatef(30.0, 0.0, 1.0, 0.0); 
  glEnable(GL_DEPTH_TEST);//used for z-buffering
 // initLighting();
  //initTexture();
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  createGLUTMenus();
  glCullFace(GL_BACK);
  glutMainLoop();
  return (0);
}

