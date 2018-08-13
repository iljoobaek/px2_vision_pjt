// Include standard headers
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/imgproc/imgproc.hpp>

// Include GLFW
#include "glfw3.h"
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GL/glut.h>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "objloader.hpp"

// Include header files to read buffer from camera
#include "eglconsumer_main.h"

using namespace glm;

#define HEIGHT 604
#define WIDTH 3840

int i;
int prev;
int change_mesh = 0;
int mesh = 0;
CvCapture* capture1;
CvCapture* capture2;
CvCapture* capture3;
CvCapture* capture4;
int video_demo = 1;
int px2 = 0;


std::vector<std::vector<glm::vec3> > v(100);
std::vector<std::vector<glm::vec2> > u(100);
std::vector<std::vector<glm::vec3> > n(100);
std::vector<std::string> obj_names;

void init_obj_names() {
	obj_names.push_back("rect_new_dimen_front.obj");
	obj_names.push_back("rect_new_dimen_left.obj");
	obj_names.push_back("rect_new_dimen_rear.obj");
	obj_names.push_back("rect_new_dimen_right.obj");

	obj_names.push_back("rectangular_mesh_measured_front.obj");
	obj_names.push_back("rectangular_mesh_measured_left.obj");
	obj_names.push_back("rectangular_mesh_measured_rear.obj");
	obj_names.push_back("rectangular_mesh_measured_right.obj");

	obj_names.push_back("elliptical_mesh_rectcutout_front.obj");
	obj_names.push_back("elliptical_mesh_side_flat_rectcutout_left.obj");
	obj_names.push_back("elliptical_mesh_rectcutout_rear.obj");
	obj_names.push_back("elliptical_mesh_side_flat_rectcutout_right.obj");

	obj_names.push_back("cadillac_chassis.obj");
}

void load_all_obj() {
	for (int j = 0; j < (int) obj_names.size(); j++) {
		bool res = loadOBJ((obj_names[j]).c_str(), v[j], u[j], n[j]);
	}
}

void load_VBO_v(GLuint &vertex_buffer) {

	//GLuint vertex_buffer;
	//glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, (v[4 * mesh].size() + v[(4 * mesh) + 1].size() + v[(4 * mesh) + 2].size() + v[(4 * mesh) + 3].size() + v[(int) obj_names.size() - 1].size()) * sizeof(glm::vec3), 0, GL_STATIC_DRAW);
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, v[(4 * mesh)].size() * sizeof(glm::vec3), &v[(4 * mesh)][0]);
	glBufferSubData(GL_ARRAY_BUFFER, v[(4 * mesh)].size() * sizeof(glm::vec3), v[(4 * mesh) + 1].size() * sizeof(glm::vec3), &v[(4 * mesh) + 1][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (v[(4 * mesh)].size() + v[(4 * mesh) + 1].size()) * sizeof(glm::vec3), v[(4 * mesh) + 2].size() * sizeof(glm::vec3), &v[(4 * mesh) + 2][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (v[(4 * mesh)].size() + v[(4 * mesh) + 1].size() + v[(4 * mesh) + 2].size()) * sizeof(glm::vec3), v[(4 * mesh) + 3].size() * sizeof(glm::vec3), &v[(4 * mesh) + 3][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (v[(4 * mesh)].size() + v[(4 * mesh) + 1].size() + v[(4 * mesh) + 2].size() + v[(4 * mesh) + 3].size()) * sizeof(glm::vec3), v[(int) obj_names.size() - 1].size() * sizeof(glm::vec3), &v[(int) obj_names.size() - 1][0]);	

	////printf("145\n");
	//return vertex_buffer;
	
}

void load_VBO_u(GLuint &uv_buffer) {

	//GLuint uv_buffer;
	//glGenBuffers(1, &uv_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, (u[4 * mesh].size() + u[(4 * mesh) + 1].size() + u[(4 * mesh) + 2].size() + u[(4 * mesh) + 3].size() + u[(int) obj_names.size() - 1].size()) * sizeof(glm::vec2), 0, GL_STATIC_DRAW);
	
	glBufferSubData(GL_ARRAY_BUFFER, 0, u[(4 * mesh)].size() * sizeof(glm::vec2), &u[(4 * mesh)][0]);
	glBufferSubData(GL_ARRAY_BUFFER, u[(4 * mesh)].size() * sizeof(glm::vec2), u[(4 * mesh) + 1].size() * sizeof(glm::vec2), &u[(4 * mesh) + 1][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (u[(4 * mesh)].size() + u[(4 * mesh) + 1].size()) * sizeof(glm::vec2), u[(4 * mesh) + 2].size() * sizeof(glm::vec2), &u[(4 * mesh) + 2][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (u[(4 * mesh)].size() + u[(4 * mesh) + 1].size() + u[(4 * mesh) + 2].size()) * sizeof(glm::vec2), u[(4 * mesh) + 3].size() * sizeof(glm::vec2), &u[(4 * mesh) + 3][0]);
	glBufferSubData(GL_ARRAY_BUFFER, (u[(4 * mesh)].size() + u[(4 * mesh) + 1].size() + u[(4 * mesh) + 2].size() + u[(4 * mesh) + 3].size()) * sizeof(glm::vec2), u[(int) obj_names.size() - 1].size() * sizeof(glm::vec2), &u[(int) obj_names.size() - 1][0]);	

	//return uv_buffer;
}


GLuint ConvertIplToTexture(cv::Mat &image_mat, int feed)
{
	IplImage image_t = image_mat;
	IplImage* image = &image_t; 

	GLuint texture;
	int startX, startY, sizeX, sizeY;

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	/*glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,image->width,image->height,
	GL_BGR,GL_UNSIGNED_BYTE,image->imageData);*/
	//printf("%d\n", feed);
	
    /*
    switch (feed) {
		case 1:
			startX = 0;
			startY = 0;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 2:
			startX = (image->width) / 2;
			startY = (image->height) / 2;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 3:
			startX = 0;
			startY = (image->height) / 2;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 4:
			startX = (image->width) / 2;
			startY = 0;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
	}
	//printf("%d\n", __LINE__);

	//printf("sizeX = %d, sizeY = %d\n", sizeX, sizeY);
	CvRect cropRect = cvRect(startX, startY, sizeX, sizeY); // ROI in source image
	//printf("%d\n", __LINE__);
 
	cvSetImageROI(image, cropRect);
	IplImage *target_im = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);
	//printf("%d\n", __LINE__);
	cvCopy(image, target_im, NULL); // Copies only crop region
	//printf("%d\n", __LINE__);
	cvResetImageROI(image);
    
    */

    
	IplImage *target_im = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);
	cvCopy(image, target_im, NULL); // Copies only crop region

	//printf("target_im_width = %d, target_im_height = %d\n", target_im->width, target_im->height);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, target_im->width, target_im->height, 0, GL_BGR, GL_UNSIGNED_BYTE, target_im->imageData);
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
	
	cvReleaseImage(&target_im);

	return texture;
}
GLuint ConvertIplToTexture(IplImage *image, int feed)
{
	GLuint texture;
	int startX, startY, sizeX, sizeY;

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	/*glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,image->width,image->height,
	GL_BGR,GL_UNSIGNED_BYTE,image->imageData);*/
	//printf("%d\n", feed);
	switch (feed) {
		case 1:
			startX = 0;
			startY = 0;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 2:
			startX = (image->width) / 2;
			startY = (image->height) / 2;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 3:
			startX = 0;
			startY = (image->height) / 2;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
		case 4:
			startX = (image->width) / 2;
			startY = 0;
			sizeX = (image->width) / 2;
			sizeY = (image->height) / 2;
			break;
	}
	//printf("%d\n", __LINE__);

	//printf("sizeX = %d, sizeY = %d\n", sizeX, sizeY);
	CvRect cropRect = cvRect(startX, startY, sizeX, sizeY); // ROI in source image
	//printf("%d\n", __LINE__);
 
	cvSetImageROI(image, cropRect);
	IplImage *target_im = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);
	//printf("%d\n", __LINE__);
	cvCopy(image, target_im, NULL); // Copies only crop region
	//printf("%d\n", __LINE__);
	cvResetImageROI(image);

	//printf("target_im_width = %d, target_im_height = %d\n", target_im->width, target_im->height);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, target_im->width, target_im->height, 0, GL_BGR, GL_UNSIGNED_BYTE, target_im->imageData);
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
	
	cvReleaseImage(&target_im);

	return texture;
}

GLuint ConvertIplToTexturePX2(IplImage *image)
{
	GLuint texture;
	int startX, startY, sizeX, sizeY;

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	
	cvFlip(image, NULL, -1);

	//printf("target_im_width = %d, target_im_height = %d\n", target_im->width, target_im->height);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, image->width, image->height, 0, GL_BGR, GL_UNSIGNED_BYTE, image->imageData);
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

GLuint ConvertIplToTexturePX2(cv::Mat &image_mat)
{
	IplImage image_t = image_mat;
	IplImage* image = &image_t; 
	
    GLuint texture;
	int startX, startY, sizeX, sizeY;

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	
	cvFlip(image, NULL, -1);

	//printf("target_im_width = %d, target_im_height = %d\n", target_im->width, target_im->height);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, image->width, image->height, 0, GL_BGR, GL_UNSIGNED_BYTE, image->imageData);
	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	switch(key) {
		case GLFW_KEY_1:	// default view
			prev = i;
			i = 0;
			break;
		case GLFW_KEY_2:	// top view
			prev = i;
			i = 1;
			break;
		case GLFW_KEY_3:	// default view from rear left
			prev = i;
			i = 2;
			break;
		case GLFW_KEY_4:	// default view from rear right
			prev = i;
			i = 3;
			break;
		case GLFW_KEY_5:	// right door
			prev = i;
			i = 4;
			break;
		case GLFW_KEY_6:	// front to back view
			prev = i;
			i = 5;
			break;
		case GLFW_KEY_7:	// left door
			prev = i;
			i = 6;
			break;
		case GLFW_KEY_8:	// mesh 0
			change_mesh = 1;
			mesh = 0;
			break;
		case GLFW_KEY_9:	// mesh 1
			change_mesh = 1;
			mesh = 1;
			break;
		case GLFW_KEY_0:	// mesh 2
			change_mesh = 1;
			mesh = 2;
			break;
	}
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1920, 1208, "3D Surround View", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1920/2, 1208/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	//GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	init_obj_names();
	load_all_obj();
	/*// Read our .obj file
	std::vector<glm::vec3> vertices1;
	std::vector<glm::vec2> uvs1;
	std::vector<glm::vec3> normals1; // Won't be used at the moment.
	bool res1 = loadOBJ("elliptical_mesh_rectcutout_front.obj", vertices1, uvs1, normals1);

	// Read our .obj fil
	std::vector<glm::vec3> vertices2;
	std::vector<glm::vec2> uvs2;
	std::vector<glm::vec3> normals2; // Won't be used at the moment.
	bool res2 = loadOBJ("elliptical_mesh_side_flat_rectcutout_left.obj", vertices2, uvs2, normals2);

	// Read our .obj file
	std::vector<glm::vec3> vertices3;
	std::vector<glm::vec2> uvs3;
	std::vector<glm::vec3> normals3; // Won't be used at the moment.
	bool res3 = loadOBJ("elliptical_mesh_rectcutout_rear.obj", vertices3, uvs3, normals3);

	// Read our .obj file
	std::vector<glm::vec3> vertices4;
	std::vector<glm::vec2> uvs4;
	std::vector<glm::vec3> normals4; // Won't be used at the moment.
	bool res4 = loadOBJ("elliptical_mesh_side_flat_rectcutout_right.obj", vertices4, uvs4, normals4);

	// Read our .obj file
	std::vector<glm::vec3> vertices5;
	std::vector<glm::vec2> uvs5;
	std::vector<glm::vec3> normals5; // Won't be used at the moment.
	bool res5 = loadOBJ("cadillac_measured.obj", vertices5, uvs5, normals5);*/

	char * vid_common = "highway.mp4";
	char * vid_front = "output_AB.h264_6_0";
	char * vid_left = "output_AB.h264_6_1";
	char * vid_rear = "output_AB.h264_6_2";
	char * vid_right = "output_AB.h264_6_3";

	if (video_demo) {
		if (!px2) {
			capture1 = cvCreateFileCapture(vid_common);
			capture2 = cvCreateFileCapture(vid_common);
			capture3 = cvCreateFileCapture(vid_common);
			capture4 = cvCreateFileCapture(vid_common);
		}
		else if(px2) {
			capture1 = cvCreateFileCapture(vid_front);
			capture2 = cvCreateFileCapture(vid_left);
			capture3 = cvCreateFileCapture(vid_rear);
			capture4 = cvCreateFileCapture(vid_right);
		}
		
	}
	/*capture1 = cvCaptureFromCAM(CV_CAP_ANY);
	capture2 = cvCaptureFromCAM(CV_CAP_ANY);
	capture3 = cvCaptureFromCAM(CV_CAP_ANY);
	capture4 = cvCaptureFromCAM(CV_CAP_ANY);*/
	/*IplImage *image = cvLoadImage("front_origin.bmp");
	Texture1 = ConvertIplToTexture(image);
	cvReleaseImage(&image);*/

	////printf("131\n");

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	load_VBO_v(vertexbuffer);
	/*glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	////printf("138\n");
	glBufferData(GL_ARRAY_BUFFER, (vertices1.size() + vertices2.size() + vertices3.size() + vertices4.size() + vertices5.size()) * sizeof(glm::vec3), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices1.size() * sizeof(glm::vec3), &vertices1[0]);
	////printf("142\n");
	glBufferSubData(GL_ARRAY_BUFFER, vertices1.size() * sizeof(glm::vec3), vertices2.size() * sizeof(glm::vec3), &vertices2[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (vertices1.size() + vertices2.size()) * sizeof(glm::vec3), vertices3.size() * sizeof(glm::vec3), &vertices3[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (vertices1.size() + vertices2.size() + vertices3.size()) * sizeof(glm::vec3), vertices4.size() * sizeof(glm::vec3), &vertices4[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (vertices1.size() + vertices2.size() + vertices3.size() + vertices4.size()) * sizeof(glm::vec3), vertices5.size() * sizeof(glm::vec3), &vertices5[0]);	
*/

	////printf("145\n");
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	load_VBO_u(uvbuffer);

//////////////////////////////////////////////////////////////////////

	std::vector<glm::vec3> v_wheel_fl;
	std::vector<glm::vec2> u_wheel_fl;
	std::vector<glm::vec3> n_wheel_fl;
	bool res1 = loadOBJ("front_left_wheel.obj", v_wheel_fl, u_wheel_fl, n_wheel_fl);

	GLuint vertexbuffer_wheel_fl;
	glGenBuffers(1, &vertexbuffer_wheel_fl);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_fl);
	glBufferData(GL_ARRAY_BUFFER, v_wheel_fl.size() * sizeof(glm::vec3), &v_wheel_fl[0], GL_STATIC_DRAW);

	GLuint uvbuffer_wheel_fl;
	glGenBuffers(1, &uvbuffer_wheel_fl);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_fl);
	glBufferData(GL_ARRAY_BUFFER, u_wheel_fl.size() * sizeof(glm::vec2), &u_wheel_fl[0], GL_STATIC_DRAW);

//////////////////////////////////////////////////////////////////////

	std::vector<glm::vec3> v_wheel_fr;
	std::vector<glm::vec2> u_wheel_fr;
	std::vector<glm::vec3> n_wheel_fr;
	bool res2 = loadOBJ("front_right_wheel.obj", v_wheel_fr, u_wheel_fr, n_wheel_fr);

	GLuint vertexbuffer_wheel_fr;
	glGenBuffers(1, &vertexbuffer_wheel_fr);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_fr);
	glBufferData(GL_ARRAY_BUFFER, v_wheel_fr.size() * sizeof(glm::vec3), &v_wheel_fr[0], GL_STATIC_DRAW);

	GLuint uvbuffer_wheel_fr;
	glGenBuffers(1, &uvbuffer_wheel_fr);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_fr);
	glBufferData(GL_ARRAY_BUFFER, u_wheel_fr.size() * sizeof(glm::vec2), &u_wheel_fr[0], GL_STATIC_DRAW);

//////////////////////////////////////////////////////////////////////

	std::vector<glm::vec3> v_wheel_rl;
	std::vector<glm::vec2> u_wheel_rl;
	std::vector<glm::vec3> n_wheel_rl;
	bool res3 = loadOBJ("rear_left_wheel.obj", v_wheel_rl, u_wheel_rl, n_wheel_rl);

	GLuint vertexbuffer_wheel_rl;
	glGenBuffers(1, &vertexbuffer_wheel_rl);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_rl);
	glBufferData(GL_ARRAY_BUFFER, v_wheel_rl.size() * sizeof(glm::vec3), &v_wheel_rl[0], GL_STATIC_DRAW);

	GLuint uvbuffer_wheel_rl;
	glGenBuffers(1, &uvbuffer_wheel_rl);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_rl);
	glBufferData(GL_ARRAY_BUFFER, u_wheel_rl.size() * sizeof(glm::vec2), &u_wheel_rl[0], GL_STATIC_DRAW);

//////////////////////////////////////////////////////////////////////

	std::vector<glm::vec3> v_wheel_rr;
	std::vector<glm::vec2> u_wheel_rr;
	std::vector<glm::vec3> n_wheel_rr;
	bool res4 = loadOBJ("rear_right_wheel.obj", v_wheel_rr, u_wheel_rr, n_wheel_rr);

	GLuint vertexbuffer_wheel_rr;
	glGenBuffers(1, &vertexbuffer_wheel_rr);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_rr);
	glBufferData(GL_ARRAY_BUFFER, v_wheel_rr.size() * sizeof(glm::vec3), &v_wheel_rr[0], GL_STATIC_DRAW);

	GLuint uvbuffer_wheel_rr;
	glGenBuffers(1, &uvbuffer_wheel_rr);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_rr);
	glBufferData(GL_ARRAY_BUFFER, u_wheel_rr.size() * sizeof(glm::vec2), &u_wheel_rr[0], GL_STATIC_DRAW);
	/*glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, (uvs1.size() + uvs2.size() + uvs3.size() + uvs4.size() + uvs5.size()) * sizeof(glm::vec2), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, uvs1.size() * sizeof(glm::vec2), (void *) &uvs1[0]);
	glBufferSubData(GL_ARRAY_BUFFER, uvs1.size() * sizeof(glm::vec2), uvs2.size() * sizeof(glm::vec2), (void *) &uvs2[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (uvs1.size() + uvs2.size()) * sizeof(glm::vec2), uvs3.size() * sizeof(glm::vec2), (void *) &uvs3[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (uvs1.size() + uvs2.size() + uvs3.size()) * sizeof(glm::vec2), uvs4.size() * sizeof(glm::vec2), (void *) &uvs4[0]);
	glBufferSubData(GL_ARRAY_BUFFER, (uvs1.size() + uvs2.size() + uvs3.size() + uvs4.size()) * sizeof(glm::vec2), uvs5.size() * sizeof(glm::vec2), (void *) &uvs5[0]);
*/
	
	////printf("154\n");
/////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<glm::vec3> cam_array;
	std::vector<glm::vec3> look_array;

	cam_array.push_back(glm::vec3(0,0,-4));			// default view
	look_array.push_back(glm::vec3(0,9,-10));

	cam_array.push_back(glm::vec3(0,-0.0001,-5));			// top view
	look_array.push_back(glm::vec3(0,0,-6));

	cam_array.push_back(glm::vec3(0,0,-5));		// default view from rear left
	look_array.push_back(glm::vec3(2,5,-7));

	cam_array.push_back(glm::vec3(0,0,-5));		// default view from rear right
	look_array.push_back(glm::vec3(-2,5,-7));

	cam_array.push_back(glm::vec3(0,0,-4));		// right door
	look_array.push_back(glm::vec3(-2,0,-5));

	cam_array.push_back(glm::vec3(0,0,-4));			// front to back view
	look_array.push_back(glm::vec3(0,-9,-10));

	cam_array.push_back(glm::vec3(0,0,-4));		// left door
	look_array.push_back(glm::vec3(2,0,-5));

	
	GLuint Texture5 = loadBMP_custom("cadillac.bmp");

	IplImage *frame1;
	IplImage *frame2;
	IplImage *frame3;
	IplImage *frame4;

	// replace by cv::Mat
	cv::Mat f1;
	cv::Mat f2;
	cv::Mat f3;
	cv::Mat f4;

	float rot = 0.0f;
	glm::vec3 myRotationAxis( 1, 0, 0);
	glm::vec3 center_fl(0.0f, 0.0f, 0.0f);
	glm::vec3 center_fr(0.0f, 0.0f, 0.0f);
	glm::vec3 center_rl(0.0f, 0.0f, 0.0f);
	glm::vec3 center_rr(0.0f, 0.0f, 0.0f);
	
	float lastTime = glfwGetTime();
	int nbFrames = 0;	
///////////////////////////////////////////////////////////////////////////////////////////
	
	cv::Mat matInputBuffer(HEIGHT,WIDTH,CV_8UC4);			
	unsigned char* inputBuffer = matInputBuffer.data;
	eglconsumer_main();
	sleep(1);


	do{
		//usleep(10000);
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) {
			printf("%d fps\n", nbFrames);
			nbFrames = 0;
			lastTime += 1.0;
		}
 
		if (change_mesh) {
			change_mesh = 0;
			load_VBO_v(vertexbuffer);
			load_VBO_u(uvbuffer);
		}
		
		// Load the texture
		//GLuint Texture1 = loadBMP_custom("front_origin.bmp");
		GLuint Texture1;
		GLuint Texture2;
		GLuint Texture3;
		GLuint Texture4;
		
		if (video_demo && !px2) {

			printf("Get ImgBuffer:%d\n", getImgBuffer(inputBuffer));

			cv::Mat temp_re;
			cvtColor(matInputBuffer, temp_re, CV_RGBA2BGR);
			resize(temp_re, temp_re, cv::Size(2560,360));
						
			f1 = temp_re(cv::Rect(0,0,640,360));
			f2 = temp_re(cv::Rect(640,0,640,360));
			f3 = temp_re(cv::Rect(1280,0,640,360));
			f4 = temp_re(cv::Rect(1920,0,640,360));
			
			Texture1 = ConvertIplToTexture(f1, 1);
			Texture2 = ConvertIplToTexture(f2, 2);
			Texture3 = ConvertIplToTexture(f3, 3);
			Texture4 = ConvertIplToTexture(f4, 4);
            
            // Try to find out differences between these below and ConvertIplToTexture()
            /*
            Texture1 = ConvertIplToTexturePX2(f1);
            Texture2 = ConvertIplToTexturePX2(f2);
            Texture3 = ConvertIplToTexturePX2(f3);
            Texture4 = ConvertIplToTexturePX2(f4);
            */			

            printf("After Texture %d\n", __LINE__);
		}

		if (video_demo && px2) {
			
			// cv::Mat to store the data
			cv::Mat temp(HEIGHT, WIDTH, CV_16UC1, inputBuffer);
			temp.convertTo(temp, CV_8UC1, 0.015625);     //proper 12bit to 8bit conversion value
			// convert colorspace or not ??
			
			// resize or not ??
			cv::Mat temp_re;
			resize(temp, temp_re, cv::Size(2560, 360));
			
			f1 = temp_re(cv::Rect(0,0,640,360));
			f2 = temp_re(cv::Rect(640,0,640,360));
			f3 = temp_re(cv::Rect(1280,0,640,360));
			f4 = temp_re(cv::Rect(1920,0,640,360));
				

			/*
			frame1 = cvQueryFrame(capture1);
			if (frame1 != NULL) {
				////printf("%d\n", __LINE__);
				Texture1 = ConvertIplToTexturePX2(frame1);
			} else {
				break;
			}

			frame2 = cvQueryFrame(capture2);
			if (frame2 != NULL) {
				////printf("%d\n", __LINE__);
				Texture2 = ConvertIplToTexturePX2(frame2);
			}

			frame3 = cvQueryFrame(capture3);
			if (frame3 != NULL) {
				////printf("%d\n", __LINE__);
				Texture3 = ConvertIplToTexturePX2(frame3);
			}

			frame4= cvQueryFrame(capture4);
			if (frame4 != NULL) {
				////printf("%d\n", __LINE__);
				Texture4 = ConvertIplToTexturePX2(frame4);
			}
			*/
		}

		if (!video_demo) {
			Texture1 = loadBMP_custom("front_highway_flip.bmp");
			Texture2 = loadBMP_custom("left_highway_flip.bmp");
			Texture3 = loadBMP_custom("rear_highway_flip.bmp");
			Texture4 = loadBMP_custom("right_highway_flip.bmp");
		}
		

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		glfwSetKeyCallback(window, key_callback);

		// Compute the MVP matrix from keyboard and mouse input
		//computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(60.0f), 3840.0f / 2160.0f, 0.1f, 100.0f);
  
		// Or, for an ortho camera :
		//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
		
		// Create quaternions from the rotation matrices produced by glm::lookAt
		glm::quat quat_start (glm::lookAt (cam_array.at(prev), look_array.at(prev), glm::vec3(0,0,1)));
		glm::quat quat_end   (glm::lookAt (cam_array.at(i), look_array.at(i), glm::vec3(0,0,1)));

		// Interpolate half way from original view to the new.
		float interp_factor = 0.5; // 0.0 == original, 1.0 == new

		// First interpolate the rotation
		glm::quat  quat_interp = glm::slerp (quat_start, quat_end, interp_factor);

		// Then interpolate the translation
		glm::vec3  pos_interp  = glm::mix   (cam_array.at(prev),  cam_array.at(i),  interp_factor);

		glm::mat4  ViewMatrix = glm::mat4_cast (quat_interp); // Setup rotation
		ViewMatrix [3]        = glm::vec4 (pos_interp, 1.0);  // Introduce translation
		  
		// Camera matrix
		/*glm::mat4 ViewMatrix = glm::lookAt(
		    cam_array.at(i), // Camera is at (4,3,3), in World Space
		    look_array.at(i), // and looks at the origin
		    glm::vec3(0,0,1)  // Head is up (set to 0,0,-1 to look upside-down)
		    );*/
		  
		// Model matrix : an identity matrix (model will be at the origin)
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		/*glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;*/
		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture1);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, v[(4 * mesh)].size() );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);		





		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) (v[(4 * mesh)].size() * sizeof(glm::vec3))             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*) (u[(4 * mesh)].size() * sizeof(glm::vec2))                          // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, v[(4 * mesh) + 1].size() );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

////////////////////////////////////////////////////////////////////////////////

		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) ((v[(4 * mesh)].size() + v[(4 * mesh) + 1].size()) * sizeof(glm::vec3))             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*) ((u[(4 * mesh)].size() + u[(4 * mesh) + 1].size()) * sizeof(glm::vec2))                          // array buffer offset
		);

		glDrawArrays(GL_TRIANGLES, 0, v[(4 * mesh) + 2].size() );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


////////////////////////////////////////////////////////////////////////////////

		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture4);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) ((v[(4 * mesh)].size() + v[(4 * mesh) + 1].size() + v[(4 * mesh) + 2].size()) * sizeof(glm::vec3))             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*) ((u[(4 * mesh)].size() + u[(4 * mesh) + 1].size() + u[(4 * mesh) + 2].size()) * sizeof(glm::vec2))                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v[(4 * mesh) + 3].size() );
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

///////////////////////////////////////////////////////////////////////////////////

		glBindTexture(GL_TEXTURE_2D, Texture5);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*) ((v[(4 * mesh)].size() + v[(4 * mesh) + 1].size() + v[(4 * mesh) + 2].size() + v[(4 * mesh) + 3].size()) * sizeof(glm::vec3))             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*) ((u[(4 * mesh)].size() + u[(4 * mesh) + 1].size() + u[(4 * mesh) + 2].size() + u[(4 * mesh) + 3].size()) * sizeof(glm::vec2))                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v[(int) obj_names.size() - 1].size() );
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

///////////////////////////////////////////////////////////////////////////////////////
		if (rot < -3.88f)
			rot = 0;

		rot = rot - 1.296f;

		center_fl = glm::vec3(0.0f, 0.0f, 0.0f);
		
		for (int i = 0; i < v_wheel_fl.size(); i++)
		{
			center_fl += v_wheel_fl[i];
		}
		center_fl = center_fl / (float) v_wheel_fl.size();
		glm::mat4 ModelMatrix_fl = glm::mat4(1.0f);
		ModelMatrix_fl = glm::translate( ModelMatrix_fl, center_fl);
		ModelMatrix_fl = glm::rotate( ModelMatrix_fl, rot, myRotationAxis );
		ModelMatrix_fl = glm::translate( ModelMatrix_fl, (-1.0f) * center_fl);

		glm::mat4 MVP_fl = ProjectionMatrix * ViewMatrix * ModelMatrix_fl;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_fl[0][0]);

		glBindTexture(GL_TEXTURE_2D, Texture5);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_fl);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_fl);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                        // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v_wheel_fl.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

/////////////////////////////////////////////////////////////////////////////////////////

		center_fr = glm::vec3(0.0f, 0.0f, 0.0f);
		
		for (int i = 0; i < v_wheel_fr.size(); i++)
		{
			center_fr += v_wheel_fr[i];
		}
		center_fr = center_fr / (float) v_wheel_fr.size();
		glm::mat4 ModelMatrix_fr = glm::mat4(1.0f);
		ModelMatrix_fr = glm::translate( ModelMatrix_fr, center_fr);
		ModelMatrix_fr = glm::rotate( ModelMatrix_fr, rot, myRotationAxis );
		ModelMatrix_fr = glm::translate( ModelMatrix_fr, (-1.0f) * center_fr);

		glm::mat4 MVP_fr = ProjectionMatrix * ViewMatrix * ModelMatrix_fr;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_fr[0][0]);

		glBindTexture(GL_TEXTURE_2D, Texture5);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_fr);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_fr);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                        // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v_wheel_fr.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

/////////////////////////////////////////////////////////////////////////////////////////

		center_rl = glm::vec3(0.0f, 0.0f, 0.0f);
		
		for (int i = 0; i < v_wheel_rl.size(); i++)
		{
			center_rl += v_wheel_rl[i];
		}
		center_rl = center_rl / (float) v_wheel_rl.size();
		glm::mat4 ModelMatrix_rl = glm::mat4(1.0f);
		ModelMatrix_rl = glm::translate( ModelMatrix_rl, center_rl);
		ModelMatrix_rl = glm::rotate( ModelMatrix_rl, rot, myRotationAxis );
		ModelMatrix_rl = glm::translate( ModelMatrix_rl, (-1.0f) * center_rl);

		glm::mat4 MVP_rl = ProjectionMatrix * ViewMatrix * ModelMatrix_rl;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_rl[0][0]);

		glBindTexture(GL_TEXTURE_2D, Texture5);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_rl);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_rl);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                        // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v_wheel_rl.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

/////////////////////////////////////////////////////////////////////////////////////////

		center_rr = glm::vec3(0.0f, 0.0f, 0.0f);
		
		for (int i = 0; i < v_wheel_rr.size(); i++)
		{
			center_rr += v_wheel_rr[i];
		}
		center_rr = center_rr / (float) v_wheel_rr.size();
		glm::mat4 ModelMatrix_rr = glm::mat4(1.0f);
		ModelMatrix_rr = glm::translate( ModelMatrix_rr, center_rr);
		ModelMatrix_rr = glm::rotate( ModelMatrix_rr, rot, myRotationAxis );
		ModelMatrix_rr = glm::translate( ModelMatrix_rr, (-1.0f) * center_rr);

		glm::mat4 MVP_rr = ProjectionMatrix * ViewMatrix * ModelMatrix_rr;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP_rr[0][0]);

		glBindTexture(GL_TEXTURE_2D, Texture5);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_wheel_rr);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0             // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_wheel_rr);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                        // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, v_wheel_rr.size() );


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		glDeleteTextures(1, &Texture1);
		glDeleteTextures(1, &Texture2);
		glDeleteTextures(1, &Texture3);
		glDeleteTextures(1, &Texture4);

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	
	//glDeleteTextures(1, &Texture5);
	glDeleteVertexArrays(1, &VertexArrayID);
	cam_array.clear();
	look_array.clear();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

