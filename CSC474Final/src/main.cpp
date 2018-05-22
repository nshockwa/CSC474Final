// Core libraries
#include <iostream>
#include <fstream>
#include <cmath>

// Third party libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <thread>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Local headers
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "Camera.h"

#define MESHSIZE 100

using namespace std;
using namespace glm;

double get_last_elapsed_time() {
	static double lasttime = glfwGetTime();
	double actualtime = glfwGetTime();
	double difference = actualtime - lasttime;
	lasttime = actualtime;
	return difference;
}

class Application : public EventCallbacks {
public:
	WindowManager *windowManager = nullptr;
    Camera *camera = nullptr;

    std::shared_ptr<Shape> shape;
	std::shared_ptr<Program> phongShader, prog, heightshader, skyprog, linesshader;
    
    double gametime = 0;
    bool wireframeEnabled = false;
    bool mousePressed = false;
    bool mouseCaptured = false;
    glm::vec2 mouseMoveOrigin = glm::vec2(0);
    glm::vec3 mouseMoveInitialCameraRot;

    GLuint VertexArrayID;
    GLuint MeshPosID, MeshTexID, IndexBufferIDBox;
    GLuint Texture,AudioTex, AudioTexBuf;
    GLuint Texture2,HeightTex;

    Application() {
        camera = new Camera();
    }
    
    ~Application() {
        delete camera;
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
		// Movement
        if (key == GLFW_KEY_W && action != GLFW_REPEAT) camera->vel.z = (action == GLFW_PRESS) * -0.2f;
        if (key == GLFW_KEY_S && action != GLFW_REPEAT) camera->vel.z = (action == GLFW_PRESS) * 0.2f;
        if (key == GLFW_KEY_A && action != GLFW_REPEAT) camera->vel.x = (action == GLFW_PRESS) * -0.2f;
        if (key == GLFW_KEY_D && action != GLFW_REPEAT) camera->vel.x = (action == GLFW_PRESS) * 0.2f;
        // Rotation
        if (key == GLFW_KEY_I && action != GLFW_REPEAT) camera->rotVel.x = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_K && action != GLFW_REPEAT) camera->rotVel.x = (action == GLFW_PRESS) * -0.02f;
        if (key == GLFW_KEY_J && action != GLFW_REPEAT) camera->rotVel.y = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_L && action != GLFW_REPEAT) camera->rotVel.y = (action == GLFW_PRESS) * -0.02f;
        if (key == GLFW_KEY_U && action != GLFW_REPEAT) camera->rotVel.z = (action == GLFW_PRESS) * 0.02f;
        if (key == GLFW_KEY_O && action != GLFW_REPEAT) camera->rotVel.z = (action == GLFW_PRESS) * -0.02f;
        // Polygon mode (wireframe vs solid)
        if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            wireframeEnabled = !wireframeEnabled;
            glPolygonMode(GL_FRONT_AND_BACK, wireframeEnabled ? GL_LINE : GL_FILL);
        }
        // Hide cursor (allows unlimited scrolling)
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            mouseCaptured = !mouseCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, mouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            resetMouseMoveInitialValues(window);
        }
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
        mousePressed = (action != GLFW_RELEASE);
        if (action == GLFW_PRESS) {
            resetMouseMoveInitialValues(window);
        }
    }
    
    void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
        if (mousePressed || mouseCaptured) {
            float yAngle = (xpos - mouseMoveOrigin.x) / windowManager->getWidth() * 3.14159f;
            float xAngle = (ypos - mouseMoveOrigin.y) / windowManager->getHeight() * 3.14159f;
            camera->setRotation(mouseMoveInitialCameraRot + glm::vec3(-xAngle, -yAngle, 0));
        }
    }

	void resizeCallback(GLFWwindow *window, int in_width, int in_height) { }
    
    // Reset mouse move initial position and rotation
    void resetMouseMoveInitialValues(GLFWwindow *window) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseMoveOrigin = glm::vec2(mouseX, mouseY);
        mouseMoveInitialCameraRot = camera->rot;
    }

    void init_terrain_mesh()
    {

        //generate the VAO
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &MeshPosID);
        glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
        glm::vec3 *vertices = new glm::vec3[MESHSIZE * MESHSIZE * 6];
        for (int x = 0; x < MESHSIZE; x++)
        {
            for (int z = 0; z < MESHSIZE; z++)
            {
                vertices[x * 6 + z*MESHSIZE * 6 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
                vertices[x * 6 + z*MESHSIZE * 6 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);//RD
                vertices[x * 6 + z*MESHSIZE * 6 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
                vertices[x * 6 + z*MESHSIZE * 6 + 3] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
                vertices[x * 6 + z*MESHSIZE * 6 + 4] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
                vertices[x * 6 + z*MESHSIZE * 6 + 5] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);//LU

            }
    
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 6, vertices, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        delete[] vertices;
        //tex coords
        float t = 1. / MESHSIZE;
        vec2 *tex = new vec2[MESHSIZE * MESHSIZE * 6];
        for (int x = 0; x<MESHSIZE; x++)
            for (int y = 0; y < MESHSIZE; y++)
            {
                tex[x * 6 + y*MESHSIZE * 6 + 0] = vec2(0.0, 0.0)+ vec2(x, y)*t; //LD
                tex[x * 6 + y*MESHSIZE * 6 + 1] = vec2(t, 0.0)+ vec2(x, y)*t;   //RD
                tex[x * 6 + y*MESHSIZE * 6 + 2] = vec2(t, t)+ vec2(x, y)*t;     //RU
                tex[x * 6 + y*MESHSIZE * 6 + 3] = vec2(0.0, 0.0) + vec2(x, y)*t;    //LD
                tex[x * 6 + y*MESHSIZE * 6 + 4] = vec2(t, t) + vec2(x, y)*t;        //RU
                tex[x * 6 + y*MESHSIZE * 6 + 5] = vec2(0.0, t)+ vec2(x, y)*t;   //LU
            }
        glGenBuffers(1, &MeshTexID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 6, tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        delete[] tex;
        glGenBuffers(1, &IndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
        GLuint *elements = new GLuint[MESHSIZE * MESHSIZE * 8];
    //  GLuint ele[10000];
        int ind = 0,i=0;
        for (i = 0; i<(MESHSIZE * MESHSIZE * 8); i+=8, ind+=6)
            {
            elements[i + 0] = ind + 0;
            elements[i + 1] = ind + 1;
            elements[i + 2] = ind + 1;
            elements[i + 3] = ind + 2;
            elements[i + 4] = ind + 2;
            elements[i + 5] = ind + 5;
            elements[i + 6] = ind + 5;
            elements[i + 7] = ind + 0;
            }           
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*MESHSIZE * MESHSIZE * 8, elements, GL_STATIC_DRAW);
        delete[] elements;
        glBindVertexArray(0);
    }

    void init_terrain_tex(const std::string& resourceDirectory) {

        int width, height, channels;
        char filepath[1000];

        //texture 1
        string str = resourceDirectory + "/sky.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        //texture 2
        str = resourceDirectory + "/sky1.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &Texture2);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        //texture 3
        str = resourceDirectory + "/height.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &HeightTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, HeightTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);


        //[TWOTEXTURES]
        //set the 2 textures to the correct samplers in the fragment shader:
        GLuint Tex1Location = glGetUniformLocation(prog->getPID(), "tex");//tex, tex2... sampler in the fragment shader
        GLuint Tex2Location = glGetUniformLocation(prog->getPID(), "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(prog->getPID());
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);

        Tex1Location = glGetUniformLocation(heightshader->getPID(), "tex");//tex, tex2... sampler in the fragment shader
        Tex2Location = glGetUniformLocation(heightshader->getPID(), "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(heightshader->getPID());
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);

        Tex1Location = glGetUniformLocation(skyprog->getPID(), "tex");//tex, tex2... sampler in the fragment shader
        Tex2Location = glGetUniformLocation(skyprog->getPID(), "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(skyprog->getPID());
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);
        
        Tex1Location = glGetUniformLocation(linesshader->getPID(), "tex");//tex, tex2... sampler in the fragment shader
        Tex2Location = glGetUniformLocation(linesshader->getPID(), "tex2");
        // Then bind the uniform samplers to texture units:
        glUseProgram(linesshader->getPID());
        glUniform1i(Tex1Location, 0);
        glUniform1i(Tex2Location, 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


	void initGeom(const std::string& resourceDirectory) {
        shape = make_shared<Shape>();
        shape->loadMesh(resourceDirectory + "/sphere.obj");
        shape->resize();
        shape->init();

        init_terrain_mesh();
        init_terrain_tex(resourceDirectory);

	}
	
	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
        
		// Initialize the GLSL programs
        phongShader = std::make_shared<Program>();
        phongShader->setShaderNames(resourceDirectory + "/phong.vert", resourceDirectory + "/phong.frag");
        phongShader->init();

        skyprog = std::make_shared<Program>();
        skyprog->setShaderNames(resourceDirectory + "/sky.vert", resourceDirectory + "/sky.frag");
        skyprog->init();

        prog = std::make_shared<Program>();
        prog->setShaderNames(resourceDirectory + "/shader.vert", resourceDirectory + "/shader.frag");
        prog->init();

        heightshader = std::make_shared<Program>();
        heightshader->setShaderNames(resourceDirectory + "/height.vert", resourceDirectory + "/height.frag", resourceDirectory + "/height.geom");
        heightshader->init();

        linesshader = std::make_shared<Program>();
        linesshader->setShaderNames(resourceDirectory + "/lines_height.vert", resourceDirectory + "/lines_height.frag", resourceDirectory + "/lines_height.geom");
        linesshader->init();

	}
    
    glm::mat4 getPerspectiveMatrix() {
        float fov = 3.14159f / 4.0f;
        float aspect = windowManager->getAspect();
        return glm::perspective(fov, aspect, 0.01f, 10000.0f);
    }

	void render() {
		double frametime = get_last_elapsed_time();
		gametime += frametime;

		// Clear framebuffer.
		glClearColor(0.3f, 0.7f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks.
		glm::mat4 V, M, P;
        P = getPerspectiveMatrix();
        V = camera->getViewMatrix();
        M = glm::mat4(1);
        
        /*************** DRAW SHAPE ***************
        M = glm::translate(glm::mat4(1), glm::vec3(0, 0, -3));
        phongShader->bind();
        phongShader->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        shape->draw(phongShader, false);
        phongShader->unbind();
        */

        /************* draw sky sphere ******************/
		static float w = 0.6;
        glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
        float angle = 3.1415926 / 2.0;
        glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), -camera->pos);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
        M = TransZ *RotateY * RotateX * S;

        skyprog->bind();
        skyprog->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        shape->draw(skyprog,false);
        skyprog->unbind();

		//cout << camera->pos.x << " " << camera->pos.y << " " << camera->pos.z << " " << endl;
	}
};

int main(int argc, char **argv) {
	std::string resourceDir = "../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

    // Initialize window.
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1280, 720);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);
    
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle())) {
        // Update camera position.
        application->camera->update();
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
