#include <math.h>
#include <GL/glut.h>
#include <opencv/highgui.h>

GLfloat angle = 0.0; 
GLuint listIndex;
GLuint texture;
CvCapture* capture;

GLuint ConvertIplToTexture(IplImage *image)
{
  GLuint texture;

  glGenTextures(1,&texture);
  glBindTexture(GL_TEXTURE_2D,texture);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  gluBuild2DMipmaps(GL_TEXTURE_2D,3,image->width,image->height,
  GL_BGR,GL_UNSIGNED_BYTE,image->imageData);

 return texture;
}

GLvoid DrawCube()
{
	IplImage* frame;
	frame = cvQueryFrame( capture );
	if (frame != NULL)
		texture = ConvertIplToTexture(frame);;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
    glCallList(listIndex);
	glDisable(GL_TEXTURE_2D);
}


void init(){
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();  
	gluLookAt(0.0, 0.0, 2.0, 0.0, 0.0, -0.5, 0.0, -1.0, 0.0);

	//glRotatef(angle, 0.0, 1.0, 0.0);
	
	DrawCube();
	
	angle += 1.0;
	glutSwapBuffers();
}

void reshape(int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}


void keyboard (unsigned char key, int x, int y)
{
	switch(key){
	case 27: case 'q':
		exit (0);
		break;
	}
}


void initialize_opencv(){
	capture = cvCreateFileCapture("video1.mp4");
	//capture = cvCaptureFromCAM(CV_CAP_ANY);
	//IplImage *image = cvLoadImage("/home/luca/Scrivania/Uni/2 Magistrale/2° Anno/1 Ambienti Virtuali, Interattiva e Videogiochi/OpenGL/WorkSpace/Open Portal/texture/cube.png");
	//texture = ConvertIplToTexture(image);
	//cvReleaseImage(&image);

	GLfloat vert[48] =
     {-0.5f, 0.0f, 0.5f,   0.5f, 0.0f, 0.5f,   0.5f, 1.0f, 0.5f,  -0.5f, 1.0f, 0.5f,
      -0.5f, 1.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f,
       0.5f, 0.0f, 0.5f,   0.5f, 0.0f, -0.5f,  0.5f, 1.0f, -0.5f,  0.5f, 1.0f, 0.5f,
       -0.5f, 0.0f, -0.5f,  -0.5f, 0.0f, 0.5f,  -0.5f, 1.0f, 0.5f, -0.5f, 1.0f, -0.5f
      };

    GLfloat texcoords[32] = { 0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
                  0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
                  0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0,
                  0.0,0.0, 1.0,0.0, 1.0,1.0, 0.0,1.0
                };

	GLubyte cubeIndices[24] = {0,1,2,3, 4,5,6,7, 3,2,5,4, 7,6,1,0,  8,9,10,11, 12,13,14,15};

	listIndex = glGenLists(1);
    glNewList(listIndex, GL_COMPILE);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
		glVertexPointer(3, GL_FLOAT, 0, vert);

		glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, cubeIndices);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEndList();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1920, 1080);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Cubi Rotanti con luce");

	glEnable(GL_CULL_FACE);
	initialize_opencv();
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
}