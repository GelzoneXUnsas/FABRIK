/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#define RESOURCEDIR  "../../resources"

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"

#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> plane;

ofstream file;

vec3 point_1 = vec3(-8.0f, -10.0f, -20.0f);
vec3 point_2 = vec3(-10.0f, -8.0f, -20.0f);
vec3 point_3 = vec3(-4.0f, -5.0f, -20.0f);
vec3 point_4 = vec3(3.0f, 2.0f, -20.0f);

vector<vec3> line;
vector<vec3> actLine;
vector<float> distances;

static vec3 objPoint = vec3(-4.0f, 6.0f, -20.0f);
static vec3 startPoint = point_1;

const int maxIterations = 100;
const float distanceThreshold = 0.01f;
static float ct = 0;
static bool done = false;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}


class camera
{
public:
    glm::vec3 pos, rot;
    int w, a, s, d, q, e, z, c;
    camera()
    {
        w = a = s = d = q = e = z = c = 0;
        pos = glm::vec3(0, 0, -15);
        rot = glm::vec3(0, 0, 0);
    }
    glm::mat4 process(double ftime)
    {
        float speed = 0;

        float fwdspeed = 10;

        if (w == 1)
        {
            speed = fwdspeed*ftime;
        }
        else if (s == 1)
        {
            speed = -fwdspeed*ftime;
        }
        float yangle=0;
        if (a == 1)
            yangle = -ftime;
        else if(d==1)
            yangle = ftime;
        rot.y += yangle;
        float zangle = 0;
        if (q == 1)
            zangle = -3 * ftime;
        else if (e == 1)
            zangle = 3 * ftime;
        rot.z += zangle;
        float xangle = 0;
        if (z == 1)
            xangle = -0.3 * ftime;
        else if (c == 1)
            xangle = 0.3 * ftime;
        rot.x += xangle;

        glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
        glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
        glm::vec4 dir = glm::vec4(0, 0, speed,1);
        R = Rz *Rx * R;
        dir = dir*R;
        pos += glm::vec3(dir.x, dir.y, dir.z);
        glm::mat4 T = glm::translate(glm::mat4(1), pos);
        return R*T;
    }
    void get_dirpos(vec3 &up,vec3 &dir,vec3 &position)
        {
        position = pos;
        glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
        glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
        glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
        glm::vec4 dir4 = glm::vec4(0, 0, 1, 0);
        R = Rz *Rx * R;
        dir4 = dir4*R;
        dir = vec3(dir4);
        glm::vec4 up4 = glm::vec4(0, 1, 0, 0);
        up4 = R*vec4(0, 1, 0, 0);
        up4 = vec4(0, 1, 0, 0)*R;
        up = vec3(up4);
        }
    
};


camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> psky, prog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;

	//texture data
	GLuint Texture;
	GLuint Texture2;

	//line
	Line linerender;
    
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        if (key ==GLFW_KEY_1 && action == GLFW_RELEASE)
        {
            objPoint = vec3(-4.0f, 6.0f, -20.0f);
            done = false;
        }
        if (key ==GLFW_KEY_2 && action == GLFW_RELEASE)
        {
            objPoint = vec3(0.0f, 3.5f, -20.0f);
            done = false;
        }
        if (key ==GLFW_KEY_3 && action == GLFW_RELEASE)
        {
            objPoint = vec3(6.0f, 0.0f, -20.0f);
            done = false;
        }
        
        if (key == GLFW_KEY_W && action == GLFW_PRESS)
        {
            mycam.w = 1;
        }
        if (key == GLFW_KEY_W && action == GLFW_RELEASE)
        {
            mycam.w = 0;
        }
        if (key == GLFW_KEY_S && action == GLFW_PRESS)
        {
            mycam.s = 1;
        }
        if (key == GLFW_KEY_S && action == GLFW_RELEASE)
        {
            mycam.s = 0;
        }
        if (key == GLFW_KEY_A && action == GLFW_PRESS)
        {
            mycam.a = 1;
        }
        if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        {
            mycam.a = 0;
        }
        if (key == GLFW_KEY_D && action == GLFW_PRESS)
        {
            mycam.d = 1;
        }
        if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        {
            mycam.d = 0;
        }
        if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        {
            mycam.q = 1;
        }
        if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
        {
            mycam.q = 0;
        }
        if (key == GLFW_KEY_E && action == GLFW_PRESS)
        {
            mycam.e = 1;
        }
        if (key == GLFW_KEY_E && action == GLFW_RELEASE)
        {
            mycam.e = 0;
        }
        if (key == GLFW_KEY_Z && action == GLFW_PRESS)
        {
            mycam.z = 1;
        }
        if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
        {
            mycam.z = 0;
        }
        if (key == GLFW_KEY_C && action == GLFW_PRESS)
        {
            mycam.c = 1;
        }
        if (key == GLFW_KEY_C && action == GLFW_RELEASE)
        {
            mycam.c = 0;
        }
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
            {
            vec3 dir,pos,up;
            mycam.get_dirpos(up, dir, pos);
            cout << "point position:" << pos.x << "," << pos.y<< "," << pos.z << endl;
            cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
            cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
            }
        if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
            {
            vec3 dir, pos, up;
            mycam.get_dirpos(up, dir, pos);
            cout << endl;
            cout << "point position:" << pos.x << "," << pos.y << "," << pos.z << endl;
            cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
            cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
            cout << "point saved into file!" << endl << endl;
            file << "point position:" << pos.x << "," << pos.y << "," << pos.z << endl;
            file << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
            file << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
            }
        
    }

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
    
    mat4 get_ori(float x_angle, float y_angle, float z_angle){
        float sangle = -3.1415926 / 2.;
        glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), sangle + glm::radians(x_angle), vec3(1,0,0));
        glm::mat4 RotateYPlane = glm::rotate(glm::mat4(1.0f), glm::radians(y_angle), vec3(0,1,0));
        glm::mat4 RotateZPlane = glm::rotate(glm::mat4(1.0f), glm::radians(z_angle), vec3(0,0,1));
        return RotateYPlane*RotateZPlane*RotateXPlane;
    }

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{

		// Initialize mesh.
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();
        
		linerender.init();
        
        line.push_back(point_1);
        line.push_back(point_2);
        line.push_back(point_3);
        line.push_back(point_4);

		linerender.re_init_line(line);
        
        actLine = line;
        
        for (int i = 0; i < line.size() - 1; i++)
        {
            float d = glm::distance(line[i],line[i + 1]);
            distances.push_back(d);
        }
        
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.

		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("campos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");
        
        prog = std::make_shared<Program>();
        prog->setVerbose(true);
        prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
        if (!prog->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        prog->addUniform("P");
        prog->addUniform("V");
        prog->addUniform("M");
        prog->addUniform("campos");
        prog->addAttribute("vertPos");
        prog->addAttribute("vertNor");
        prog->addAttribute("vertTex");
	}
    
    template <class T>
    T cos_interp(float t, T a, T b){
        float ct = 1.0f - (cos(t * 3.1415926)+ 1.0f) / 2.0f;
        return a * (1-ct) + b*ct;
    }
    
    float cos_interp_float(float t){
        return 1.0f - (cos(t * 3.1415926)+ 1.0f) / 2.0f;
    }

    void BRIK(vector<vec3> &line)
    {
        
        float sum = 0.0f;
        for (float d : distances) {
            sum += d;
        }
        
        if (sum < glm::distance(line[0], objPoint)){
            printf("can't reach\n");
            return;
        }
        
        line[line.size() - 1] = objPoint;

        // Perform the Backward Reaching step
        for (int i = (int)line.size() - 2; i >= 0; i--)
        {
            glm::vec3 currentDir = glm::normalize(line[i] - line[i + 1]);
            float targetDist = distances[i];
            line[i] = line[i + 1] + currentDir * targetDist;
        }
        
        if (glm::distance(line[0],startPoint) <= distanceThreshold){
            done = true;
        }
    }
    
    void FRIK(vector<vec3> &line)
    {
        
        line[0] = startPoint;

        // Perform the Forward Reaching step
        for (int i = 0; i < (int)line.size() - 2; i++)
        {
            glm::vec3 currentDir = glm::normalize(line[i + 1] - line[i]);
            float targetDist = distances[i];
            line[i + 1] = line[i] + currentDir * targetDist;
        }
        
        if (glm::distance(line[line.size() - 1], objPoint) <= distanceThreshold){
            done = true;
        }
    }
    
	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);
        
//        mycam.pos.z += frametime * 4;

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones
		float sangle = 3.1415926 / 2.;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;

		// Draw the sky using GLSL.
		psky->bind();		
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		//shape->draw(psky);			//render!!!!!!!
		glEnable(GL_DEPTH_TEST);
        
        TransSky = glm::translate(glm::mat4(1.0f), objPoint);
        SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
        if (done){
            SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
        }
        M = TransSky * SSky;
        glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(psky->getUniform("campos"), 1, &mycam.pos[0]);
        shape->draw(psky);
        
		psky->unbind();
        
        
        float wt = 1.0f;
        
        ct += frametime;
        
        //-----------------------------------------------
        
        //draw the points
        prog->bind();
        
        BRIK(actLine);
        FRIK(actLine);
        
        for (int i = 0; i < line.size(); i++)
        {
            line[i] = line[i] * (1 - ct/wt) + actLine[i] * ct/wt;
        }
        
        if (ct > wt)
        {
            line = actLine;
            ct = 0;
        }
        
        for (int i = 0; i < line.size(); i++)
        {
            glm::mat4 Trans = glm::translate(glm::mat4(1.0f), line[i]);
            glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
            if (i == 0)
            {
                Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
            }
            M = Trans * Scale;
            glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
            glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
            glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
            shape->draw(prog);
        }
        
        prog->unbind();
        
		//draw the lines
        linerender.re_init_line(line);
		glm::vec3 linecolor = glm::vec3(1, 1, 1);
		linerender.draw(P, V, linecolor);

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = RESOURCEDIR; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
    file.open("pathinfo.txt");
    if (!file.is_open())
        {
        cout << "warning! could not open pathinfo.txt file!" << endl;
        }

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}
    
    file.close();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
