

//
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<ao/ao.h>
#include<mpg123.h>
#define BITS 8
#include<pthread.h>
#include<bits/stdc++.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

	using namespace std;

	struct VAO {
		GLuint VertexArrayID;
		GLuint VertexBuffer;
		GLuint ColorBuffer;

		GLenum PrimitiveMode;
		GLenum FillMode;
		int NumVertices;
	};
	typedef struct VAO VAO;

	struct GLMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		GLuint MatrixID;
	} Matrices;

	GLuint programID;

	/* Function to load Shaders - Use it as it is */
	GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

		// Create the shaders
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the Vertex Shader code from the file
		std::string VertexShaderCode;
		std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
		if(VertexShaderStream.is_open())
		{
			std::string Line = "";
			while(getline(VertexShaderStream, Line))
				VertexShaderCode += "\n" + Line;
			VertexShaderStream.close();
		}

		// Read the Fragment Shader code from the file
		std::string FragmentShaderCode;
		std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
		if(FragmentShaderStream.is_open()){
			std::string Line = "";
			while(getline(FragmentShaderStream, Line))
				FragmentShaderCode += "\n" + Line;
			FragmentShaderStream.close();
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Vertex Shader
		printf("Compiling shader : %s\n", vertex_file_path);
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> VertexShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

		// Compile Fragment Shader
		printf("Compiling shader : %s\n", fragment_file_path);
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

		// Link the program
		fprintf(stdout, "Linking program\n");
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return ProgramID;
	}

	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	void quit(GLFWwindow *window)
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		//    exit(EXIT_SUCCESS);
	}


	/* Generate VAO, VBOs and return VAO handle */
	struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
	{
		struct VAO* vao = new struct VAO;
		vao->PrimitiveMode = primitive_mode;
		vao->NumVertices = numVertices;
		vao->FillMode = fill_mode;

		// Create Vertex Array Object
		// Should be done after CreateWindow and before any other GL calls
		glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
		glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
		glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

		glBindVertexArray (vao->VertexArrayID); // Bind the VAO
		glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
		glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
		glVertexAttribPointer(
				0,                  // attribute 0. Vertices
				3,                  // size (x,y,z)
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
		glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
		glVertexAttribPointer(
				1,                  // attribute 1. Color
				3,                  // size (r,g,b)
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
				);

		return vao;
	}

	/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
	struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
	{
		GLfloat* color_buffer_data = new GLfloat [3*numVertices];
		for (int i=0; i<numVertices; i++) {
			color_buffer_data [3*i] = red;
			color_buffer_data [3*i + 1] = green;
			color_buffer_data [3*i + 2] = blue;
		}

		return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
	}

	/* Render the VBOs handled by VAO */
	void draw3DObject (struct VAO* vao)
	{
		// Change the Fill Mode for this object
		glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

		// Bind the VAO to use
		glBindVertexArray (vao->VertexArrayID);

		// Enable Vertex Attribute 0 - 3d Vertices
		glEnableVertexAttribArray(0);
		// Bind the VBO to use
		glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

		// Enable Vertex Attribute 1 - Color
		glEnableVertexAttribArray(1);
		// Bind the VBO to use
		glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

		// Draw the geometry !
		glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
	}

	/**************************
	 * Customizable functions *
	 **************************/
	 mpg123_handle *mh;
	     unsigned char *buffer;
	     size_t buffer_size;
	     size_t done;
	     int err;

	     int driver;
	     ao_device *dev;

	     ao_sample_format format;
	     int channels, encoding;
	     long rate;
			 pthread_t mythread;

 float a=2.0,b=2.5,c=0;
 float shiftright=2.0,shiftleft=2.5,shiftup=2.5,shiftdown=2.0,ztranslate=0.0,initialx=-2.25,initialy=2.25,zoom_camera=2;
 int pos=1,upkeypressed=0,downkeypressed=0,rightkeypressed=0,leftkeypressed=0,fall=0,level=1,won=0,toggle=0,moves=0,count_camera=1,counter=1,leftmouseclick=0;
	VAO *yellowtile,*orangetile,*whitetile,*greentile, *block;
	float rotationBlock=0,translationBlock=0;
	float camera_rotation_angle = 0;
	float rectangle_rotation = 0;
	float triangle_rotation = 0;
	float triangle_rot_dir = 1;
	float rectangle_rot_dir = 1;
	bool triangle_rot_status = true;
	bool rectangle_rot_status = true;
	float xchange=0.0f,ychange=0.0f;
	int level1[11][11]={0},level2[11][11]={0};
	double xi;double xl;double yi;double yl;
	//glm::mat4 modelMatrix;
	glm::mat4 modelMatrix=glm::translate (glm::vec3(initialx, initialy, 0));
	//modelMatrix=translateBlock1;
	/* Executed when a regular key is pressed/released/held-down */
	/* Prefered for Keyboard events */
void* playsound(void *x)
{

	while(1)
	{

		if( rightkeypressed==1)
		{
			double last_update_time = glfwGetTime(), current_time;
			ao_initialize();
			driver = ao_default_driver_id();
			mpg123_init();
			mh = mpg123_new(NULL, &err);
			buffer_size = mpg123_outblock(mh);
			buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
			string s="./example.mp3";
			/* open the file and get the decoding format */
			mpg123_open(mh,&s[0]);
			mpg123_getformat(mh, &rate, &channels, &encoding);

			/* set the output format and open the output device */
			format.bits = mpg123_encsize(encoding) * BITS;
			format.rate = rate;
			format.channels = channels;
			format.byte_format = AO_FMT_NATIVE;
			format.matrix = 0;
			dev = ao_open_live(driver, &format, NULL);
			 while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
			        ao_play(dev, (char *)buffer, done);
		}
		if( leftkeypressed==1)
		{
			double last_update_time = glfwGetTime(), current_time;
			ao_initialize();
			driver = ao_default_driver_id();
			mpg123_init();
			mh = mpg123_new(NULL, &err);
			buffer_size = mpg123_outblock(mh);
			buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
			string s="./example.mp3";
			/* open the file and get the decoding format */
			mpg123_open(mh,&s[0]);
			mpg123_getformat(mh, &rate, &channels, &encoding);

			/* set the output format and open the output device */
			format.bits = mpg123_encsize(encoding) * BITS;
			format.rate = rate;
			format.channels = channels;
			format.byte_format = AO_FMT_NATIVE;
			format.matrix = 0;
			dev = ao_open_live(driver, &format, NULL);
			 while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
			        ao_play(dev, (char *)buffer, done);
		}
		if(downkeypressed==1)
		{
			double last_update_time = glfwGetTime(), current_time;
			ao_initialize();
			driver = ao_default_driver_id();
			mpg123_init();
			mh = mpg123_new(NULL, &err);
			buffer_size = mpg123_outblock(mh);
			buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
			string s="./example.mp3";
			/* open the file and get the decoding format */
			mpg123_open(mh,&s[0]);
			mpg123_getformat(mh, &rate, &channels, &encoding);

			/* set the output format and open the output device */
			format.bits = mpg123_encsize(encoding) * BITS;
			format.rate = rate;
			format.channels = channels;
			format.byte_format = AO_FMT_NATIVE;
			format.matrix = 0;
			dev = ao_open_live(driver, &format, NULL);
			 while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
			        ao_play(dev, (char *)buffer, done);
		}
		if( upkeypressed==1)
		{
			double last_update_time = glfwGetTime(), current_time;
			ao_initialize();
			driver = ao_default_driver_id();
			mpg123_init();
			mh = mpg123_new(NULL, &err);
			buffer_size = mpg123_outblock(mh);
			buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
			string s="./example.mp3";
			/* open the file and get the decoding format */
			mpg123_open(mh,&s[0]);
			mpg123_getformat(mh, &rate, &channels, &encoding);

			/* set the output format and open the output device */
			format.bits = mpg123_encsize(encoding) * BITS;
			format.rate = rate;
			format.channels = channels;
			format.byte_format = AO_FMT_NATIVE;
			format.matrix = 0;
			dev = ao_open_live(driver, &format, NULL);
			 while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
			        ao_play(dev, (char *)buffer, done);
		}

	}
}
	void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		// Function is called first on GLFW_PRESS.

		if (action == GLFW_RELEASE) {
			switch (key) {
				case GLFW_KEY_C:
					rectangle_rot_status = !rectangle_rot_status;
					break;
				case GLFW_KEY_P:
					triangle_rot_status = !triangle_rot_status;
					break;
				case GLFW_KEY_X:
					// do something ..
					break;
				default:
					break;
			}
		}
		else if (action == GLFW_PRESS) {
			switch (key) {
				case GLFW_KEY_ESCAPE:
					quit(window);
					break;
				case GLFW_KEY_D:
					camera_rotation_angle-=15*M_PI/180.0;
					break;
				case GLFW_KEY_A:
					camera_rotation_angle+=15*M_PI/180.0;
					break;
				case GLFW_KEY_RIGHT:
					rightkeypressed=1;
					moves++;
					break;
				case GLFW_KEY_LEFT:
					leftkeypressed=1;
					moves++;
					break;
				case GLFW_KEY_UP:
					upkeypressed=1;
					moves++;
					break;
				case GLFW_KEY_DOWN:
					downkeypressed=1;
					moves++;
					break;
				case GLFW_KEY_SPACE:
					count_camera++;
					break;
				default:
					break;
			}
		}
	}

	/* Executed for character input (like in text boxes) */
	void keyboardChar (GLFWwindow* window, unsigned int key)
	{
		switch (key) {
			case 'Q':
			case 'q':
				quit(window);
				break;
			default:
				break;
		}
	}

	/* Executed when a mouse button is pressed/released */
	void mouseButton (GLFWwindow* window, int button, int action, int mods)
	{
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				if(action== GLFW_PRESS)
				{
					leftmouseclick =1;
					break;
				}
				else if (action == GLFW_RELEASE)
				{
					leftmouseclick=0;
				break;
			}
			case GLFW_MOUSE_BUTTON_RIGHT:
				if (action == GLFW_RELEASE) {
					rectangle_rot_dir *= -1;
				}
				break;
			default:
				break;
		}
	}
	void mouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
    if (yoffset == -1) {
        if (zoom_camera > 0.33) {
            zoom_camera = zoom_camera / 1.1;
        }
    }
    else if (yoffset == 1) {
        if (zoom_camera < 3.0) {
            zoom_camera = zoom_camera * 1.1;
        }
    }
	}



	/* Executed when window is resized to 'width' and 'height' */
	/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
	void reshapeWindow (GLFWwindow* window, int width, int height)
	{
		int fbwidth=width, fbheight=height;
		/* With Retina display on Mac OS X, GLFW's FramebufferSize
		   is different from WindowSize */
		glfwGetFramebufferSize(window, &fbwidth, &fbheight);

		GLfloat fov = 90.0f;

		// sets the viewport of openGL renderer
		glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

		// set the projection matrix as perspective
		/* glMatrixMode (GL_PROJECTION);
		   glLoadIdentity ();
		   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
		// Store the projection matrix in a variable for future use
		// Perspective projection for 3D views
		Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

		// Ortho projection for 2D views
		//Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
	}



	// Creates the triangle object used in this sample code
	void createYellowTile ()
	{
		/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

		/* Define vertex array as used in glBegin (GL_TRIANGLES) */
		static const GLfloat vertex_buffer_data [] = {
			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,0.0, // vertex 2
			-0.25,-0.25,0.0,
			-0.25,0.25,0.0,
			0.25,0.25,0.0,

			0.25, 0.25,-0.05, // vertex 0
			0.25,-0.25,-0.05, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			0.25,0.25,-0.05,

			-0.25, 0.25,0.0, // vertex 0
			-0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0,

			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			0.25,0.25,-0.05,
			0.25,0.25,0.0,

			-0.25, -0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			-0.25,-0.25,-0.05,
			-0.25,-0.25,0.0,

			-0.25, 0.25,0.0, // vertex 0
			0.25,0.25,0.0, // vertex 1
			0.25,0.25,-0.05, // vertex 2
			0.25,0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0


		};

		static const GLfloat color_buffer_data [] = {
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0,
			1,194/255.0,0
				};

		// create3DObject creates and returns a handle to a VAO that can be used later
		yellowtile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
	}
	void createOrangeTile ()
	{
		/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

		/* Define vertex array as used in glBegin (GL_TRIANGLES) */
		static const GLfloat vertex_buffer_data [] = {
			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,0.0, // vertex 2
			-0.25,-0.25,0.0,
			-0.25,0.25,0.0,
			0.25,0.25,0.0,

			0.25, 0.25,-0.05, // vertex 0
			0.25,-0.25,-0.05, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			0.25,0.25,-0.05,

			-0.25, 0.25,0.0, // vertex 0
			-0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0,

			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			0.25,0.25,-0.05,
			0.25,0.25,0.0,

			-0.25, -0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			-0.25,-0.25,-0.05,
			-0.25,-0.25,0.0,

			-0.25, 0.25,0.0, // vertex 0
			0.25,0.25,0.0, // vertex 1
			0.25,0.25,-0.05, // vertex 2
			0.25,0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0


		};

		static const GLfloat color_buffer_data [] = {
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,
			1,153.0/255.0,0,


		};


		// create3DObject creates and returns a handle to a VAO that can be used later
		orangetile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
		//redtile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color2_buffer_data, GL_LINE);
	}
	void createWhiteTile ()
	{
		/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

		/* Define vertex array as used in glBegin (GL_TRIANGLES) */
		static const GLfloat vertex_buffer_data [] = {
			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,0.0, // vertex 2
			-0.25,-0.25,0.0,
			-0.25,0.25,0.0,
			0.25,0.25,0.0,

			0.25, 0.25,-0.05, // vertex 0
			0.25,-0.25,-0.05, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			0.25,0.25,-0.05,

			-0.25, 0.25,0.0, // vertex 0
			-0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0,

			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			0.25,0.25,-0.05,
			0.25,0.25,0.0,

			-0.25, -0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			-0.25,-0.25,-0.05,
			-0.25,-0.25,0.0,

			-0.25, 0.25,0.0, // vertex 0
			0.25,0.25,0.0, // vertex 1
			0.25,0.25,-0.05, // vertex 2
			0.25,0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0


		};

		static const GLfloat color_buffer_data [] = {
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,
			1,1,1,


		};


		// create3DObject creates and returns a handle to a VAO that can be used later
		whitetile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
		//redtile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color2_buffer_data, GL_LINE);
	}
	void createGreenTile ()
	{
		/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

		/* Define vertex array as used in glBegin (GL_TRIANGLES) */
		static const GLfloat vertex_buffer_data [] = {
			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,0.0, // vertex 2
			-0.25,-0.25,0.0,
			-0.25,0.25,0.0,
			0.25,0.25,0.0,

			0.25, 0.25,-0.05, // vertex 0
			0.25,-0.25,-0.05, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			0.25,0.25,-0.05,

			-0.25, 0.25,0.0, // vertex 0
			-0.25,-0.25,0.0, // vertex 1
			-0.25,-0.25,-0.05, // vertex 2
			-0.25,-0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0,

			0.25, 0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			0.25,0.25,-0.05,
			0.25,0.25,0.0,

			-0.25, -0.25,0.0, // vertex 0
			0.25,-0.25,0.0, // vertex 1
			0.25,-0.25,-0.05, // vertex 2
			0.25,-0.25,-0.05,
			-0.25,-0.25,-0.05,
			-0.25,-0.25,0.0,

			-0.25, 0.25,0.0, // vertex 0
			0.25,0.25,0.0, // vertex 1
			0.25,0.25,-0.05, // vertex 2
			0.25,0.25,-0.05,
			-0.25,0.25,-0.05,
			-0.25,0.25,0.0


		};

		static const GLfloat color_buffer_data [] = {
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0,
			0,1,0


		};


		// create3DObject creates and returns a handle to a VAO that can be used later
		greentile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
		//redtile = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color2_buffer_data, GL_LINE);
	}
	// Creates the rectangle object used in this sample code
	void createblock()
	{
		// GL3 accepts only Triangles. Quads are not supported
		static const GLfloat vertex_buffer_data [] = {
			-0.25f,-0.25f,0.0f, // triangle 1 : begin
			-0.25f,-0.25f, 1.0f,
			-0.25f, 0.25f, 1.0f, // triangle 1 : end
			0.25f, 0.25f,0.0f, // triangle 2 : begin
			-0.25f,-0.25f,0.0f,
			-0.25f, 0.25f,0.0f, // triangle 2 : end
			0.25f,-0.25f, 1.0f,
			-0.25f,-0.25f,0.0f,
			0.25f,-0.25f,0.0f,
			0.25f, 0.25f,0.0f,
			0.25f,-0.25f,0.0f,
			-0.25f,-0.25f,0.0f,
			-0.25f,-0.25f,0.0f,
			-0.25f, 0.25f, 1.0f,
			-0.25f, 0.25f,0.0f,
			0.25f,-0.25f, 1.0f,
			-0.25f,-0.25f, 1.0f,
			-0.25f,-0.25f,0.0f,
			-0.25f, 0.25f, 1.0f,
			-0.25f,-0.25f, 1.0f,
			0.25f,-0.25f, 1.0f,
			0.25f, 0.25f, 1.0f,
			0.25f,-0.25f,0.0f,
			0.25f, 0.25f,0.0f,
			0.25f,-0.25f,0.0f,
			0.25f, 0.25f, 1.0f,
			0.25f,-0.25f, 1.0f,
			0.25f, 0.25f, 1.0f,
			0.25f, 0.25f,0.0f,
			-0.25f, 0.25f,0.0f,
			0.25f, 0.25f, 1.0f,
			-0.25f, 0.25f,0.0f,
			-0.25f, 0.25f, 1.0f,
			0.25f, 0.25f, 1.0f,
			-0.25f, 0.25f, 1.0f,
			0.25f,-0.25f, 1.0f
		};

		static const GLfloat color_buffer_data [] = {
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3,
			0.3,0.3,0.3

		};

		// create3DObject creates and returns a handle to a VAO that can be used later
		block = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
	}

	int theta=0;
	/* Render the scene with openGL */
	/* Edit this function according to your assignment */
	void draw ()
	{
		// clear the color and depth in the frame buffer
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use the loaded shader program
		// Don't change unless you know what you are doing
		glUseProgram (programID);
		if(count_camera%5==1) ///top view
		{
			glm::vec3 eye ( 0, 0, 5);
			glm::vec3 target (0, 0, 0);
			glm::vec3 up (0, 1, 0);
			 Matrices.view = glm::lookAt( eye, target, up );
			 camera_rotation_angle=0;
		}
		else if(count_camera%5==2)  ///tower view
		{
			glm::vec3 eye ( 3, 0, 2);
			glm::vec3 target (0, 0, 0);
			glm::vec3 up (0, 1, 0);
			 Matrices.view = glm::lookAt( eye, target, up );
			 camera_rotation_angle=0;
		}
		else if(count_camera%5==3) ///follow cam view
		{
			glm::vec3 eye (-1*(shiftleft+shiftright)/2.0-1.0 ,(shiftdown+shiftup)/2.0 , 2);
			glm::vec3 target (0, 0, 0);
			glm::vec3 up (0, 1, 0);
			 Matrices.view = glm::lookAt( eye, target, up );
			 camera_rotation_angle=0;
		}
		else if(count_camera%5==4)  /// block view
		{
			glm::vec3 eye (-1*shiftright+0.25,(shiftup+shiftdown)/2.0 , 0.5);
			glm::vec3 target (shiftright, -1*(shiftup+shiftdown)/2.0, 0);
			glm::vec3 up (0, 1, 1);
			 Matrices.view = glm::lookAt( eye, target, up );
			 camera_rotation_angle=0;
		}
		else if(count_camera%5==0)  /// helicopter view
		{
			glm::vec3 eye (zoom_camera*sin(camera_rotation_angle), -1*zoom_camera*cos(camera_rotation_angle), zoom_camera);
			glm::vec3 target (0, 0, 0);
			glm::vec3 up (0, 0, 1);
			 Matrices.view = glm::lookAt( eye, target, up );
		}
		// Compute Camera matrix (view)
		 // Rotating Camera for 3D
		//  Don't change unless you are sure!!
	 //Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

		// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
		//  Don't change unless you are sure!!
		glm::mat4 VP = Matrices.projection * Matrices.view;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		// For each model you render, since the MVP will be different (at least the M part)
		//  Don't change unless you are sure!!
		glm::mat4 MVP;	// MVP = Projection * View * Model
		int flag=0;
		// Load identity to model matrix
		int i,j;ychange=0.0f;xchange=0.0f;
		for(i=1;i<=10;i++)
		{
			for(j=1;j<=10;j++)
			{
				//simple bridge tile
				if(i==4 and j==10 and level==1)
				flag=3;
				else if(i==1 and j==4 and level==2)
				flag=3;
				else if(i==3 and j==10 and level==2)
				flag=3;
				else if(i==2 and j==2 and level==2)
				flag=2;

				//normal tiles
				else if((i+j)%2==0)
				flag=1;
				else
				flag=0;
				Matrices.model = glm::mat4(1.0f);
				glm::mat4 translateTile = glm::translate (glm::vec3(-2.25+float(j/2.0)-0.5, 2.25-float(i/2.0)+0.5, 0.0f)); // glTranslatef
				glm::mat4 rotateTile = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
				glm::mat4 tileTransform = translateTile * rotateTile;
				Matrices.model *= tileTransform;
				MVP = VP * Matrices.model; // MVP = p * V * M
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				if(level==1)
				{
					if(level1[i][j]==1)
					{
						if(flag==3)
						draw3DObject(whitetile);
						else if(flag==1)
						draw3DObject(orangetile);
						else
						draw3DObject(yellowtile);
					}
				}
				else if(level==2)
				{
					if(level2[i][j]==1 or level2[i][j]==3)
					{
						if(flag==3)
						draw3DObject(whitetile);
						else if(flag==2)
						draw3DObject(greentile);
						else if(flag==1)
						draw3DObject(orangetile);
						else
						draw3DObject(yellowtile);
					}
				}
			}
		}

		// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
		// glPopMatrix ();

		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBlockz = glm::translate (glm::vec3(0, 0, -0.1));
		glm::mat4 translateBlock2 = glm::translate (glm::vec3(shiftright, 0, 0));
		glm::mat4 translateBlock3 = glm::translate (glm::vec3(-shiftright, 0, 0));
		glm::mat4 translateBlock4 = glm::translate (glm::vec3(shiftleft, 0, 0));
		glm::mat4 translateBlock5 = glm::translate (glm::vec3(-shiftleft, 0, 0));
		glm::mat4 translateBlock6 = glm::translate (glm::vec3(0,-shiftdown, 0));
		glm::mat4 translateBlock7 = glm::translate (glm::vec3(0, +shiftdown, 0));
		glm::mat4 translateBlock8 = glm::translate (glm::vec3(0, -shiftup, 0));
		glm::mat4 translateBlock9 = glm::translate (glm::vec3(0, +shiftup, 0));
		//cout<<rotationBlock<<endl;
		 // rotate about vector (-1,1,1)

		if(rightkeypressed==1)
		{
			rotationBlock=5;
			glm::mat4 rotateBlock = glm::rotate((float)(rotationBlock*M_PI/180.0f), glm::vec3(0,1,0));
			if(fall==0)
			{
			Matrices.model *= (translateBlock3 * rotateBlock*translateBlock2*modelMatrix);
			modelMatrix=(translateBlock3 * rotateBlock*translateBlock2*modelMatrix);
			}
			// rightkeypressed=0;


		}
		else if(leftkeypressed==1)
		{
			rotationBlock=-5;
			//cout<<"hi"<<endl;
			glm::mat4 rotateBlock = glm::rotate((float)(rotationBlock*M_PI/180.0f), glm::vec3(0,1,0));
			if(fall==0)
			{
			Matrices.model *= (translateBlock5 * rotateBlock*translateBlock4*modelMatrix);
			modelMatrix=(translateBlock5 * rotateBlock*translateBlock4*modelMatrix);
			}
		}
		else if(downkeypressed==1)
		{
			rotationBlock=5;
			glm::mat4 rotateBlock = glm::rotate((float)(rotationBlock*M_PI/180.0f), glm::vec3(1,0,0));
			if(fall==0)
			{
				Matrices.model *= (translateBlock7 * rotateBlock*translateBlock6*modelMatrix);
				modelMatrix=(translateBlock7* rotateBlock*translateBlock6*modelMatrix);
			}
		}
		else if(upkeypressed==1)
		{
			rotationBlock=-5;
			glm::mat4 rotateBlock = glm::rotate((float)(rotationBlock*M_PI/180.0f), glm::vec3(1,0,0));
			if(fall==0)
			{
			Matrices.model *= (translateBlock9 * rotateBlock*translateBlock8*modelMatrix);
			modelMatrix=(translateBlock9* rotateBlock*translateBlock8*modelMatrix);
			}
		}
		else if(fall==0)
		{
			rotationBlock=0;
			glm::mat4 rotateBlock = glm::rotate((float)(rotationBlock*M_PI/180.0f), glm::vec3(0,1,0));
			if(fall==0)
			{
			Matrices.model *= (translateBlock5 * rotateBlock*translateBlock4*modelMatrix);
			modelMatrix=(translateBlock5 * rotateBlock*translateBlock4*modelMatrix);
			}
		}
		//MVP = VP * Matrices.model;
		//glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	 if(fall==1)
	 {
		Matrices.model=(translateBlockz)*modelMatrix;
		modelMatrix=translateBlockz*modelMatrix;
}
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(block);
		// draw3DObject draws the VAO given to it using current MVP matrix
		//draw3DObject(block);

		rotationBlock=0;
	// Increment angles
		float increments = 1;

	}

	/* Initialise glfw window, I/O callbacks and the renderer to use */
	/* Nothing to Edit here */
	GLFWwindow* initGLFW (int width, int height)
	{
		GLFWwindow* window; // window desciptor/handle

		glfwSetErrorCallback(error_callback);
		if (!glfwInit()) {
			//        exit(EXIT_FAILURE);
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

		if (!window) {
			glfwTerminate();
			//        exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
		glfwSwapInterval( 1 );

		/* --- register callbacks with GLFW --- */

		/* Register function to handle window resizes */
		/* With Retina display on Mac OS X GLFW's FramebufferSize
		   is different from WindowSize */
		glfwSetFramebufferSizeCallback(window, reshapeWindow);
		glfwSetWindowSizeCallback(window, reshapeWindow);

		/* Register function to handle window close */
		glfwSetWindowCloseCallback(window, quit);

		/* Register function to handle keyboard input */
		glfwSetKeyCallback(window, keyboard);      // general keyboard input
		glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

		/* Register function to handle mouse click */
		glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
		glfwSetScrollCallback(window,mouseScroll);
		return window;
	}

	/* Initialize the OpenGL rendering properties */
	/* Add all the models to be created here */
	void initGL (GLFWwindow* window, int width, int height)
	{
		/* Objects should be created before any other gl function and shaders */
		// Create the models
		createOrangeTile (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
		createblock ();
		createYellowTile();
		createWhiteTile();
		createGreenTile();
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
		// Get a handle for our "MVP" uniform
		Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


		reshapeWindow (window, width, height);

		// Background color of the scene
		glClearColor (135/255.0f, 206/255.0f, 250/255.0f, 0.0f); // R, G, B, A
		glClearDepth (1.0f);

		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);

		cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
		cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
		cout << "VERSION: " << glGetString(GL_VERSION) << endl;
		cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	}

	int main (int argc, char** argv)
	{
		int i;
		int width = 600;
		int height = 600;
		///level1 tiles////
	  level1[1][1]=1,level1[1][2]=1,level1[1][3]=1;
		for(i=1;i<=6;i++)
		level1[2][i]=1;
		for(i=1;i<=9;i++)
		level1[3][i]=1;
		for(i=2;i<=10;i++)
		level1[4][i]=1;
		level1[5][6]=1;level1[5][7]=1;level1[5][8]=2;level1[5][9]=1,level1[5][10]=1;
		level1[6][7]=1;level1[6][8]=1;level1[6][9]=1;

		/////Level2 Tiles////
		for(i=1;i<=4;i++)
		{
		level2[1][i]=1;
		level2[2][i]=1;
		level2[3][i]=1;
		level2[4][i]=1;
		}
		level2[2][2]=3;
		level2[1][7]=1;level2[1][8]=1;
		level2[2][7]=1;level2[2][8]=1;
		level2[3][7]=1;level2[3][8]=1;level2[3][9]=1;level2[3][10]=1;
		level2[4][7]=1;level2[4][8]=1;level2[4][9]=2;level2[4][10]=1;
		level2[5][7]=1;level2[5][8]=1;level2[5][9]=1;level2[5][10]=1;
		level2[6][8]=1;level2[6][9]=1;level2[6][10]=1;



		int j;
		GLFWwindow* window = initGLFW(width, height);


		initGL (window, width, height);

		double last_update_time = glfwGetTime(), current_time;

	pthread_t mythread;
        pthread_create(&mythread, NULL, playsound,(void*)NULL);


		/* Draw in loop */
		while (!glfwWindowShouldClose(window)) {

			if(leftmouseclick==1)
			{
			glfwGetCursorPos(window, &xi, &yi);
      xi = (xi / 75.0) - 4.0;
      yi = -(yi / 75.0) + 4.0;

			if(xl>xi)
			camera_rotation_angle-=2.0*M_PI/180.0;
			else
			camera_rotation_angle+=2.0*M_PI/180.0;
			glfwGetCursorPos(window, &xl, &yl);
			xl = (xl / 75.0) - 4.0;
      yl = -(yl / 75.0) + 4.0;
		}
			// OpenGL Draw commands
			draw();

			// Swap Frame Buffer in double buffering
			glfwSwapBuffers(window);

			// Poll for Keyboard and mouse events
			glfwPollEvents();
			if(leftkeypressed)
			{
				theta-=5;
				if(theta<-90)
				{
					theta=0;
					leftkeypressed=0;
					toggle=1;
					if(pos==1)
					{
						shiftleft+=1.0;
						shiftright+=0.5;
						pos=2;
					}
					else if(pos==2)
					{
						shiftleft+=0.5;
						shiftright+=1.0;
						pos=1;
					}
					else if(pos==3)
					{
						shiftright+=0.5;
						shiftleft+=0.5;
					}
				}
			}
			if(rightkeypressed)
			{
				theta=theta+5;
				if(theta>90)
				{

					rightkeypressed=0;
					theta=0;
					toggle=1;
					if(pos==1)
					{
					shiftright-=1.0;
					shiftleft-=0.5;
					pos=2;
					}
					else if(pos==2)
					{
					shiftright-=0.5;
					shiftleft-=1.0;
					pos=1;
					}
					else if(pos==3)
					{
						shiftright-=0.5;
						shiftleft-=0.5;
						pos=3;
					}
					//break;
				}
			}

			if(upkeypressed)
			{
				theta-=5;
				if(theta<-90)
				{
					theta=0;
					upkeypressed=0;
					toggle=1;
					if(pos==1)
					{
						shiftup+=1.0;
						shiftdown+=0.5;
						pos=3;
					}
					else if(pos==2)
					{
						shiftup+=0.5;
						shiftdown+=0.5;
					}
					else if(pos==3)
					{
						shiftup+=0.5;
						shiftdown+=1.0;
						pos=1;
					}
				}
			}
			if(downkeypressed)
			{
				theta+=5;
				if(theta>90)
				{
					theta=0;
					downkeypressed=0;
					toggle=1;
					if(pos==1)
					{
						shiftdown-=1.0;
						shiftup-=0.5;
						pos=3;
					}
					else if(pos==2)
					{
						shiftup-=0.5;
						shiftdown-=0.5;
					}
					else if(pos==3)
					{
						shiftdown-=0.5;
						shiftup-=1.0;
						pos=1;
					}
				}
			}
			if(fall!=1 and won==0)
			{
			for(i=1;i<=10;i++)
			{
				for(j=1;j<=10;j++)
				{
						if(level==1)
						{
							if(shiftleft>2.5 or shiftleft<-2.5 or shiftright<-2.5 or shiftright>2.5 or shiftup>2.5 or shiftup<-2.5 or shiftdown<-2.5 or shiftdown>2.5)
							{
								//cout<<"Game Over"<<endl;
								fall=1;
								//exit(0);
							}
							if(abs(shiftup-shiftdown)==abs(shiftleft-shiftright))
							{
								if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and (level1[i][j]==0 or (i==4 and j==10)) and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
								{
									//cout<<"Game Over"<<endl;
									fall=1;
									//exit(0);
								}
								if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and level1[i][j]==2 and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
								{
									//cout<<"Game won"<<endl;
									fall=1;
									won=1;
									//exit(0);
								}

							}
							else if(abs(shiftup-shiftdown)>abs(shiftleft-shiftright))
							{
								if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and (level1[i][j]==0 or level1[i+1][j]==0) and shiftup==3-0.5*i and shiftdown==2-0.5*i)
								{
									//cout<<"Game Over"<<endl;
									fall=1;
									//exit(0);
								}
							}
						else if(abs(shiftup-shiftdown)<abs(shiftleft-shiftright))
						{
							if(shiftleft==3-0.5*j and shiftright==2.0-0.5*j and (level1[i][j]==0 or level1[i][j+1]==0) and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								//cout<<"Game Over"<<endl;
								fall=1;
								//exit(0);
							}
						}
					}
					else if(level==2)
					{
						if(shiftleft>2.5 or shiftleft<-2.5 or shiftright<-2.5 or shiftright>2.5 or shiftup>2.5 or shiftup<-2.5 or shiftdown<-2.5 or shiftdown>2.5)
						{
							//cout<<"Game Over"<<endl;
							fall=1;
							//exit(0);
						}
						if(abs(shiftup-shiftdown)==abs(shiftleft-shiftright))
						{
							if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and (level2[i][j]==0 or (i==1 and j==4) or (i==3 and j==10)) and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								//cout<<"Game Over"<<endl;
								fall=1;
								//exit(0);
							}
							if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and level2[i][j]==3 and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								//cout<<"Game Over"<<endl;
								if(toggle==1)
								{
									counter++;
									toggle=0;
								}
								// else if(toggle==1 and check_toggle!=moves)
								// {
								// 	level2[4][5]=0;
								// 	level2[4][6]=0;
								// 	toggle=0;
								// 	check_toggle=moves;
								// }
							}
							if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and level2[i][j]==2 and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								//cout<<"Game won"<<endl;
								fall=1;
								won=1;
								//exit(0);
							}

						}
						else if(abs(shiftup-shiftdown)>abs(shiftleft-shiftright))
						{
							if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and (level2[i][j]==0 or level2[i+1][j]==0) and shiftup==3-0.5*i and shiftdown==2-0.5*i)
							{
								//cout<<"Game Over"<<endl;
								fall=1;
								//exit(0);
							}
							if(shiftleft==3-0.5*j and shiftright==2.5-0.5*j and (level2[i][j]==3 or level2[i+1][j]==3) and shiftup==3-0.5*i and shiftdown==2-0.5*i)
							{
								if(toggle==1)
								{
									counter++;
									toggle=0;
								}
								// else if(toggle==1 and check_toggle!=moves)
								// {
								// 	level2[4][5]=0;
								// 	level2[4][6]=0;
								// 	toggle=0;
								// 	check_toggle=moves;
								// 	cout<<"Out"<<endl;
								// }
							}
						}
						else if(abs(shiftup-shiftdown)<abs(shiftleft-shiftright))
						{
							if(shiftleft==3-0.5*j and shiftright==2.0-0.5*j and (level2[i][j]==0 or level2[i][j+1]==0) and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								//cout<<"Game Over"<<endl;
								fall=1;
								//exit(0);
							}
							if(shiftleft==3-0.5*j and shiftright==2.0-0.5*j and (level2[i][j]==3 or level2[i][j+1]==3) and shiftup==3-0.5*i and shiftdown==2.5-0.5*i)
							{
								if(toggle==1)
								{
									counter++;
									toggle=0;
								}
								// else if(toggle==1 and check_toggle!=moves)
								// {
								// 	level2[4][5]=0;
								// 	level2[4][6]=0;
								// 	toggle=0;
								// 	check_toggle=moves;
								// }
							}
						}
					}
				}
			}
		}
		if(counter%2==0)
		{
			level2[4][5]=1;
			level2[4][6]=1;
		}
		else
		{
			level2[4][5]=0;
			level2[4][6]=0;
		}
			if(fall==1)
			{
				if(ztranslate<15.0)
				ztranslate+=1.0;
				else
				{
					ztranslate=0;
					if(won==0)
					{
					cout<<"Game Over"<<endl;
					fall=0;
					exit(0);
					}
					else if(won==1 and level==1)
					{
					//cout<<"Game Won"<<endl;
					fall=0;
					won=0;
					level=2;
					initialx=-1.75;initialy=2.25;
					shiftleft=2.0;
					shiftright=1.5;
					shiftup=2.5;
					shiftdown=2.0;
					modelMatrix=glm::translate (glm::vec3(initialx, initialy, 0));
					}
					else if(won==1 and level==2)
					{
						cout<<"Game Won"<<endl;
						cout<<moves<<endl;
						exit(0);
					}
				}
			}
						// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
			current_time = glfwGetTime(); // Time in seconds
			if ((current_time - last_update_time) >= 3) { // atleast 0.5s elapsed since last frame
				// do something every 0.5 seconds ..
				last_update_time = current_time;
			}


		}
		free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();

		glfwTerminate();
		//    exit(EXIT_SUCCESS);
	}
