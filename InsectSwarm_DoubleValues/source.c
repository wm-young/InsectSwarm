#if !defined(Linux)
  #include <windows.h>           //Not Linux must be windows
#else
	#include <stdlib.h>
#endif

#include <stdio.h>
#include <glut.h>
#include <math.h>


#define X 0
#define Y 1
#define Z 2
#define POINTS 1
#define WIRE_FRAME 2
#define POLYGON 3
#define TRUE 1
#define FALSE 0
#define MAX_INSECT_DIST 25
#define MIN_INSECT_DIST 5
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

//the pixel structure
typedef struct {
	GLubyte r, g, b;
} color;

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
	printf("General:\n");
	printf("	X:Rotate on X-axis\n");
	printf("	Y:Rotate on Y-axis\n");
	printf("	Z:Rotate on Z-axis\n");
	printf("	R:Reset Alignment\n");
	printf("Speed:\n");
	printf("	S:Random\n");
	printf("Life Span:\n");
	printf("	L: Toggle On/Off(Default 10 secs)\n");
	printf("Insects:\n");
	printf("	A: Add an Insect\n");
	printf("********************************************\n");
}
/*Draws the world in the display. The world is a box with a ground and 
  4 walls: 2 visible,2 invisible walls*/
void drawWorld(void) {

  //float c[][3] = {{1.0,0,0},{0,1.0,0},{1.0,1.0,1.0},
//		 {0,0,1.0},{.6,0,.6},{0,.6,.6}};

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


	curr = leader;
	switch(insectModelType)
	{
	case 1://Points
		for(i=0;i<population;i++)
		{
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
			glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
			  //Face 1
			  glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]);
			 glEnd();

			 //Face 2
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]);
			 glEnd();

			 //Face 3
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]-2);
			 glEnd();

			 //Face 4
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]-2);
			 glEnd();

			 //Face 5
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
			 glEnd();

			 //Face 6
			 glBegin(GL_LINES);
				glVertex3d(curr->pos[X],curr->pos[Y]-1,curr->pos[Z]);
				glVertex3d(curr->pos[X]-1,curr->pos[Y]-1,curr->pos[Z]);
				glVertex3d(curr->pos[X]-1,curr->pos[Y]-1,curr->pos[Z]-1);
				glVertex3d(curr->pos[X],curr->pos[Y]-1,curr->pos[Z]-1);
			 glEnd();
		}
		break;
	case 3://Polygon
		for(i=0;i<population;i++)
		{
			  glColor3ub(curr->colour.r,curr->colour.g,curr->colour.b);
			  //Face 1
			  glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]);
			 glEnd();

			 //Face 2
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]);
			 glEnd();

			 //Face 3
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y]-2,curr->pos[Z]-2);
			 glEnd();

			 //Face 4
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y]-2,curr->pos[Z]-2);
			 glEnd();

			 //Face 5
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]);
				glVertex3d(curr->pos[X],curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]-2);
				glVertex3d(curr->pos[X]-2,curr->pos[Y],curr->pos[Z]);
			 glEnd();

			 //Face 6
			 glBegin(GL_QUADS);
				glVertex3d(curr->pos[X],curr->pos[Y]-1,curr->pos[Z]);
				glVertex3d(curr->pos[X]-1,curr->pos[Y]-1,curr->pos[Z]);
				glVertex3d(curr->pos[X]-1,curr->pos[Y]-1,curr->pos[Z]-1);
				glVertex3d(curr->pos[X],curr->pos[Y]-1,curr->pos[Z]-1);
			 glEnd();
			  curr = curr->next;
		}
		break;
	}
	/*Works to draws insect as a point
	*/
	
	
	glPointSize(5);
	glBegin(GL_POINTS);
		glVertex3i(target.coords[X],target.coords[Y],target.coords[Z]);
	 glEnd();
	glFlush();
	
}//drawInsects

//Returns the distance between the given insect and the leader target
double pointToTargetDistance(particle* point,destination dest)
{
	return sqrt((double)((dest.coords[X]-point->pos[X])*(dest.coords[X]-point->pos[X]) + (dest.coords[Y]-point->pos[Y])*(dest.coords[Y]-point->pos[Y])) + (dest.coords[Z]-point->pos[Z])*(dest.coords[Z]-point->pos[Z]));
}//pointToTargetDistance

//returns the distance between two insects
double insectToInsectDistance(particle* insect1,particle* insect2)
{
	return sqrt((double)((insect2->pos[X]-insect1->pos[X])*(insect2->pos[X]-insect1->pos[X]) + (insect2->pos[Y]-insect1->pos[Y])*(insect2->pos[Y]-insect1->pos[Y])) + (insect2->pos[Z]-insect1->pos[Z])*(insect2->pos[Z]-insect1->pos[Z]));
}//insectToInsectDistance

//Determines whether or not the given particle is approaching the target destination
int approachTarget(particle* insect)
{
	double dist;
	dist = pointToTargetDistance(insect,target);
	//printf("Distance:%3.2Lf\n",dist);
	if(pointToTargetDistance(insect,target)<20)
	{
		return TRUE;
	}

	return FALSE;
}//approachTarget

//creates a new target for the leader insect to travel to
void createNewTarget()
{
	target.coords[X] = (double)(rand()%199 - world_width);
	target.coords[Y] = (double)(rand()%199 - world_height);
	target.coords[Z] = (double)(rand()%199 - world_depth);
				//printf("NEW Target pos:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf)\n", target.coords[X],target.coords[Y],target.coords[Z]);
}//createNewTarget

//determines if the given insect is too close to another
int tooCloseToInsect(particle* insect)
{
	particle* curr;
	curr = leader;

	while(curr)
	{
		if(insectToInsectDistance(insect,curr)<=MIN_INSECT_DIST)
		{
				printf("Too close to an insect.\n");
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
	if(curr->pos[X]+curr->pos[X] >= world_width-2 || curr->pos[X]+curr->pos[X] < -world_width+2)
	{
		return FALSE;
	}
	if(curr->pos[Y]+curr->pos[Y] >= world_height-2 || curr->pos[Y]+curr->pos[Y] < -world_height+2)
	{
		return FALSE;
	}
	if(curr->pos[Z]+curr->pos[Z] >= world_depth-2 || curr->pos[Z]+curr->pos[Z] < -world_depth+2)
	{
		return FALSE;
	}
	return TRUE;
}//withinWorldBound


//Returns the magnitude of the vector represented by the 3 x,y,z paramters.
double magnitude(double x,double y,double z)
{
	return sqrt((x*x+y*y+z+z));
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
		if(distance<smallestDistance)
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
	double xCoord,yCoord,zCoord;
	xCoord = neighbour->pos[X] - moveMe->pos[X];
	yCoord = neighbour->pos[Y] - moveMe->pos[Y];
	zCoord = neighbour->pos[Z] - moveMe->pos[Z];
	moveMe->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
	moveMe->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
	moveMe->dir[Z] = (yCoord/magnitude(xCoord,yCoord,zCoord));
}//adjustBreathingSpace

/* Updates the given particales position vector, direction vector, and anything
   else that needs updating*/
void updateParticle(particle* curr)
{
	double xCoord,yCoord,zCoord,mag;
	particle* neighbour;//represent the closest insect to the current
	neighbour= findClosestInsect(curr);

	if(curr->isLeader)
	{
		xCoord = target.coords[X] - curr->pos[X];
		yCoord = target.coords[Y] - curr->pos[Y];
		zCoord = target.coords[Z] - curr->pos[Z];
		//printf("xCoord=%4.2Lf, yCoord=%4.2Lf,zCoord=%4.2Lf\n",xCoord,yCoord,zCoord);
		//If the leader is approaching the target, make a new target
		if(approachTarget(curr)==TRUE)
		{
			createNewTarget();
			mag = magnitude(xCoord,yCoord,zCoord);
			//printf("magnitude=%4.2Lf\n",mag);
			curr->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
			curr->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
			curr->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
		}
		mag = magnitude(xCoord,yCoord,zCoord);
			//printf("magnitude=%4.2Lf\n",mag);
		curr->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));

		curr->pos[X] = (curr->pos[X])+(curr->dir[X]);
		curr->pos[Y] = (curr->pos[Y])+(curr->dir[Y]);
		curr->pos[Z] = (curr->pos[Z])+(curr->dir[Z]);

		////Set the directions values to the new normalized values

		//printf("Target pos:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf)\n", target.coords[X],target.coords[Y],target.coords[Z]);
	  /* printf("LEADER  pos:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf),leader:%d\n", curr->pos[X],curr->pos[Y],curr->pos[Z],curr->isLeader);
	    printf("LEADER  dir:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf),leader:%d\n", curr->dir[X],curr->dir[Y],curr->dir[Z],curr->isLeader);
		printf("-------------------------------------------------------------------\n");*/

	}
	else
	{
		xCoord = leader->pos[X] - curr->pos[X];
		yCoord = leader->pos[Y] - curr->pos[Y];
		zCoord = leader->pos[Z] - curr->pos[Z];
		//printf("xCoord=%4.2f, yCoord=%4.2f,zCoord=%4.2f\n",xCoord,yCoord,zCoord);
		mag = magnitude(xCoord,yCoord,zCoord);
		//printf("magnitude=%4.2f\n",mag);
		//if(tooCloseToInsect(curr))
		//{
		//	adjustBreathingSpace(curr,neighbour);
		//}
		//else if(tooFarFromInsect(curr))
		//{
		//	//move towards closest insect
		//	curr->dir[X]=-curr->dir[X];
		//	curr->dir[Y]=-curr->dir[Y];
		//	curr->dir[Z]=-curr->dir[Z];
		//}

		//if(withinWorldBounds(curr)==FALSE)
		//{
		//	curr->dir[X]=-curr->dir[X];
		//	curr->dir[Y]=-curr->dir[Y];
		//	curr->dir[Z]=-curr->dir[Z];
		//}
		//else
		//{
		//curr->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
		//curr->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
		//curr->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
		//}*/
		curr->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
		/*Update Insect Position*/
		curr->pos[X] = (curr->pos[X])+(curr->dir[X]);
		curr->pos[Y] = (curr->pos[Y])+(curr->dir[Y]);
		curr->pos[Z] = (curr->pos[Z])+(curr->dir[Z]);
		/************************/

		/*printf("WORKER  pos:(x,y,z)=(%5.2Lf,%5.2Lf,%5.2Lf),leader:%d\n", curr->pos[X],curr->pos[Y],curr->pos[Z],curr->isLeader);
	    printf("WORKER  dir:(x,y,z)=(%5.2Lf,%5.2Lf,%5.2Lf),leader:%d\n", curr->dir[X],curr->dir[Y],curr->dir[Z],curr->isLeader);
		printf("-------------------------------------------------------------------\n");*/
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
		//printf("Updating:(x,y,z)=(%d,%d,%d),leader:%d\n", curr->pos[X],curr->pos[Y],curr->pos[Z],curr->isLeader);
		updateParticle(curr);
		/*printf("WORKER  pos:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf),leader:%d\n", curr->pos[X],curr->pos[Y],curr->pos[Z],curr->isLeader);
	    printf("WORKER  dir:(x,y,z)=(%3.2Lf,%3.2Lf,%3.2Lf),leader:%d\n", curr->dir[X],curr->dir[Y],curr->dir[Z],curr->isLeader);
		printf("-------------------------------------------------------------------\n");*/
		curr=curr->next;
	}
	
	glFlush();
	glutSwapBuffers();
}//renderFrame

//Creates the linked list representing each insect.
void initiateInsects()
{
	int i;
	double xCoord,yCoord,zCoord;
	char num[6] = "";
	particle* curr;

	/*Initialize the first target*/
	/*target.coords[X] = rand()%((world_height+world_height)-1) - world_width;
	target.coords[Y] = rand()%((world_height+world_height)-1) - world_height;
	target.coords[Z] = rand()%((world_height+world_height)-1) - world_depth;*/
	target.coords[X] = 0.0;
	target.coords[Y] = 0.0;
	target.coords[Z] = 0.0;
	/*****************************/
	printf("Enter the amount of particles you desire:\n");
	fgets(num,6,stdin);
	showMenu();
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
	
	/*printf("xCoord=%d,yCoord=%d,zCoord=%d\n",xCoord,yCoord,zCoord);
	mag = sqrt((double)(xCoord*xCoord) + (yCoord*yCoord) + (zCoord*zCoord));
	dirX = xCoord/mag;
	dirY = yCoord/mag;
	dirZ = zCoord/mag;
	setDirCoords(leader,dirX,dirY,dirZ);*/

	leader->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
	leader->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
	leader->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
	
	leader->colour.r = 255;
	leader->colour.b = 0;
	leader->colour.g = 0;
	leader->speed = speed;
	leader->isLeader = TRUE;
	//printf("Initial TargetPosition(x,y,z)=(%f,%f,%f),leader:%f\n", target.coords[X],target.coords[Y],target.coords[Z]);
	//printf("Initial Position(x,y,z)=(%f,%f,%f),leader:%d\n", leader->pos[X],leader->pos[Y],leader->pos[Z],leader->isLeader);
   // printf("Initial Direction(x,y,z)=(%f,%f,%f),leader:%d\n", leader->dir[X],leader->dir[Y],leader->dir[Z],leader->isLeader);
	leader->next = head;
	head = leader;
	//leader=head;
	/*****************/

	for(i=1;i<population;i++)
	{
		curr = (particle*)malloc(sizeof(particle));
		curr->pos[X] = rand()%69 - world_width;
		curr->pos[Y] = rand()%69 - world_height;
		curr->pos[Z] = rand()%69 - world_depth;
		xCoord = leader->pos[X] - curr->pos[X];
		yCoord = leader->pos[Y] - curr->pos[Y];
		zCoord = leader->pos[Z] - curr->pos[Z];
		curr->dir[X] = (xCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Y] = (yCoord/magnitude(xCoord,yCoord,zCoord));
		curr->dir[Z] = (zCoord/magnitude(xCoord,yCoord,zCoord));
		curr->colour.r = 255;
		curr->colour.b = 255;
		curr->colour.g = 255;
		curr->speed = speed;
		curr->isLeader = FALSE;
		curr->next = NULL;
		//printf("Initial Worker Position(x,y,z)=(%f,%f,%d),leader:%d\n", curr->pos[X],curr->pos[Y],curr->pos[Z],curr->isLeader);
		//printf("Initial Worker Direction(x,y,z)=(%d,%d,%d),leader:%d\n", curr->dir[X],curr->dir[Y],curr->dir[Z],curr->isLeader);
		//head = curr;
		head->next = curr;
		head = head->next;
	}
}//initiateInsects

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
		//addParticle(key);
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
	}
}

//GLUT Menu to better interact with the graphics
void createGLUTMenus() 
{
	int menu,rotateWorld_submenu,addParticle_submenu,particleSpeed_submenu,particleModel_submenu;

	//Rotate World submenu
	rotateWorld_submenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("X",'x');
	glutAddMenuEntry("Y",'y');
	glutAddMenuEntry("Z",'z');

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
    glutAddSubMenu("Add Particle",addParticle_submenu);
	glutAddSubMenu("Particle Speed",particleSpeed_submenu);
	glutAddSubMenu("Particle Model",particleModel_submenu);

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

      case 0x1B:
      case 'q':
      case 'Q':
  		exit(0);

	  case GLUT_KEY_LEFT:
		  global.angle[global.axis] = global.angle[global.axis] + 0.01;
		  break;
      case GLUT_KEY_RIGHT:
			global.angle[global.axis] = global.angle[global.axis] - 0.01;
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
}


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

  population = 0;
  limitedLife = FALSE;
  lifeSpan = 15;
  insectModelType = POLYGON;
  /*Test shit*/
  printf("%3.2Lf\n",0.739999999999998437/0.739999999999998437);
  /************/
	
  printf("Press 'I' to start the insect swarm and enter the amount of particles you would like.\n");
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
  /*Get working****/
  //gluPerspective(60.0, 1.5, 0.0, 100.0);
  //gluLookAt(0.0, 250.0, 250.0, 10.0, 100.0, 100.0, 0.0, 1.0, 0.0);
  /*****************/
  glOrtho(-190.0, 190.0, -190.0, 190.0, -190.0, 190.0);
  glRotatef(30.0, 1.0, 0.0, 0.0); 
  glRotatef(30.0, 0.0, 1.0, 0.0); 
  glEnable(GL_DEPTH_TEST);//used for z-buffering
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  createGLUTMenus();
  glutMainLoop();
  return (0);
}

