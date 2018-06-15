// Core libraries
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <thread>

// Third party libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Local headers
#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "Camera.h"
#include "line.h"
#include "ControlPoint.h"
#include "bone.h"


#define MESHSIZE 100		// terrain
#define	FRAMES 61			// plane animation

using namespace std;
using namespace glm;

ofstream ofile;
string resourceDir = "../resources/";
//ifstream ifile_1;
int renderstate = 1;
int realspeed = 0;
ControlPoint *Path1_CP = nullptr;
ControlPoint *CamPath_CP = nullptr;


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

    std::shared_ptr<Shape> shape, dbone, dragon, skull;
	std::shared_ptr<Program> dboneShader, phongShader, prog, heightshader, skyprog, linesshader, pplane;

    double gametime = 0;
    bool wireframeEnabled = false;
    bool mousePressed = false;
    bool mouseCaptured = false;
	bool switchAnim = false;
	bool slowMo = false;
	bool speedUp = false;
    glm::vec2 mouseMoveOrigin = glm::vec2(0);
    glm::vec3 mouseMoveInitialCameraRot;


		//FBX animation
		GLuint VAO, VBO, VBO2;
		bone *root = NULL;
		int boneCount = 0;
		vector<mat4> animMats;
		int currentKeyframe = 0;
		mat4 animmat[200];
		mat4 animbones[200];
		int animmatsize=0;
		all_animations all_animation;

    // terrain
    GLuint VertexArrayID;
    GLuint MeshPosID, MeshTexID, IndexBufferIDBox;
    GLuint TextureID, Texture2ID, HeightTexID, AudioTex, AudioTexBuf;

   	// paths
	Line path1_render, campath_render, campath_inverse_render;
	vector<vec3> path1, campath, campath_inverse, path1_cardinal, camcardinal, camcardinal_inverse;

	// pos, lookat, up - data
	vector<mat3> path1_controlpts, campath_controlpts;

	// toggle plane camera perspective
	int cam_persp = 0;		// toggle camera perspective
	int back_count = 0; // keep track of how many nodes I added

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
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
			vec3 dir,pos,up;
			camera->getUpRotPos(up, dir, pos);
			cout << "point position:" << pos.x << "," << pos.y<< "," << pos.z << endl;
			cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			vec3 dir, pos, up;
			camera->getUpRotPos(up, dir, pos);
			cout << endl;
			back_count++;
			cout << "backspace count: " << back_count << endl;
			cout << "point position:" << pos.x << "," << pos.y << "," << pos.z << endl;
			cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			cout << "point saved into ofile!" << endl << endl;
			cout << endl;

			Path1_CP->addPoint(pos, up, dir, resourceDir + "/path1.txt");
			
			path1.push_back(Path1_CP->points[Path1_CP->getSize() - 1][0]);		// add point to line
			path1_render.re_init_line(path1);
			cardinal_curve(path1_cardinal, path1, FRAMES, 1.0);
			path1_render.re_init_line(path1_cardinal);

		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			switchAnim = !switchAnim;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			slowMo = !slowMo;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			speedUp = !speedUp;
		}

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
        if (key == GLFW_KEY_TAB && action == GLFW_RELEASE)
		{
			cam_persp = !cam_persp;
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
        string str = resourceDirectory + "/sky1.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &TextureID);
        glActiveTexture(GL_TEXTURE0);	// 1st texture unit - must set active unit to avoid overwriting textures
        glBindTexture(GL_TEXTURE_2D, TextureID);
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
        glGenTextures(1, &Texture2ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Texture2ID);
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
        glGenTextures(1, &HeightTexID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, HeightTexID);
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

    // would be cool if we could read the paths from a file instead?
	void initPaths(const std::string& resourceDirectory) {

		//--------- FIX THIS --------------------
		// campath
		campath_render.init();
		for (int i = 0; i < CamPath_CP->points.size(); i++) {
			campath.push_back(CamPath_CP->points[i][0]);
			//campath_inverse.push_back(campath_controlpts[i][0] * -1.0f);
			//cout << "campath: " << campath_controlpts[i][0].x << " " << campath_controlpts[i][0].y << " " << campath_controlpts[i][0].z << endl;
		}
		campath_render.re_init_line(campath);
		cardinal_curve(camcardinal, campath, FRAMES, 1.0);
		campath_render.re_init_line(camcardinal);
		cout << "cam path has: " << campath.size() << " points" << endl;

		// campath - inverse (drawing purposes)
		/*campath_inverse_render.init();
		campath_inverse_render.re_init_line(campath_inverse);
		cardinal_curve(camcardinal_inverse, campath_inverse, FRAMES, 1.0);
		campath_inverse_render.re_init_line(camcardinal_inverse);*/

		// new path renderer
		path1_render.init();
		for (int i = 0; i < Path1_CP->points.size(); i++) {
			path1.push_back(Path1_CP->points[i][0]);
			//	cout << path1_controlpts[i][0].x << " " << path1_controlpts[i][0].y << " " << path1_controlpts[i][0].z << endl;
		}
		path1_render.re_init_line(path1);
		cardinal_curve(path1_cardinal, path1, FRAMES, 1.0);
		path1_render.re_init_line(path1_cardinal);

		cout << "path 1 has: " << path1.size() << " points\n" << endl;
	}

	void initAnim(const std::string& resourceDirectory) {
		// Read FBX file
			std::vector<glm::vec3> boneVertices;
			std::vector<unsigned int> indexBuffer;
			for (int ii = 0; ii < 200; ii++)
				animbones[ii]=animmat[ii] = mat4(1);
			//readtobone(resourceDirectory + "/test.fbx", &all_animation, &root);
			readtobone(resourceDirectory + "/CompleteRiggedDragonFly.fbx", &all_animation, &root);
			readtobone(resourceDirectory + "/CompleteRiggedDragonRun.fbx", &all_animation, NULL);
//        readtobone(&root, (resourceDirectory + "/test.fbx").c_str(), animations);
//        readtobone(&root, (resourceDirectory + "/axisneurontestfile_binary.fbx").c_str());
			root->write_to_VBOs(glm::vec3(0), boneVertices, indexBuffer);
			root->set_animations(&all_animation,animmat, animbones,animmatsize);
//        root->findAnimations(animations[0]);
//        root->assignMatrix(&animMats);
			boneCount = boneVertices.size();

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, boneVertices.size() * sizeof(glm::vec3), boneVertices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

			glGenBuffers(1, &VBO2);
			glBindBuffer(GL_ARRAY_BUFFER, VBO2);
			glBufferData(GL_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), indexBuffer.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(3);
			glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 0, (const void *)0);
}
	void initGeom(const std::string& resourceDirectory) {
		init_terrain_mesh();

		// load sphere.obj

        shape = make_shared<Shape>();
        shape->loadMesh(resourceDirectory + "/sphere.obj");
        shape->resize();
        shape->init();
				initAnim(resourceDirectory);

				skull = make_shared<Shape>();
				skull->loadMesh(resourceDirectory + "/demonskull.obj");
				skull->resize();
				skull->init();

		dbone = make_shared<Shape>();
		dbone->loadMesh(resourceDirectory + "/bone.obj");
		dbone->resize();
		dbone->init();
		// load dragon.obj
		dragon = make_shared<Shape>();
		dragon->loadMesh(resourceDirectory + "/FA18.obj");
		dragon->resize();
		dragon->init();

        init_terrain_tex(resourceDirectory);
        initPaths(resourceDirectory);

	}

	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL programs
        phongShader = std::make_shared<Program>();
        phongShader->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
        phongShader->init();

				dboneShader = std::make_shared<Program>();
        dboneShader->setShaderNames(resourceDirectory + "/dbone.vert", resourceDirectory + "/dbone.frag");
        dboneShader->init();

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

        pplane = std::make_shared<Program>();
        pplane->setShaderNames(resourceDirectory + "/plane.vert", resourceDirectory + "/plane.frag");
        pplane->init();

		// Initialize Control Points
		Path1_CP = new ControlPoint();
		Path1_CP->loadPoints(resourceDirectory + "/path1.txt");

		CamPath_CP = new ControlPoint();
		CamPath_CP->loadPoints(resourceDirectory + "/campath.txt");

	}

    glm::mat4 getPerspectiveMatrix() {
        float fov = 3.14159f / 4.0f;
        float aspect = windowManager->getAspect();
        return glm::perspective(fov, aspect, 0.01f, 10000.0f);
    }

    mat4 linint_between_two_orientations(vec3 ez_lookto_1, vec3 ey_up_1, vec3 ez_lookto_2, vec3 ey_up_2, float t) {
		mat4 m1, m2;
		quat q1, q2;
		vec3 ex, ey, ez;

		t = ((-cos(t* 3.14)) + 1) / 2.0;	// smooth transition

		// get rotation matrix ---------
		ey = ey_up_1;											// control point 1
		ez = ez_lookto_1;
		ex = cross(ey, ez);
		m1[0][0] = ex.x;	m1[1][0] = ey.x;	m1[2][0] = ez.x;	m1[3][0] = 0;
		m1[0][1] = ex.y;	m1[1][1] = ey.y;	m1[2][1] = ez.y;	m1[3][1] = 0;
		m1[0][2] = ex.z;	m1[1][2] = ey.z;	m1[2][2] = ez.z;	m1[3][2] = 0;
		m1[0][3] = 0;		m1[1][3] = 0;		m1[2][3] = 0;		m1[3][3] = 1.0f;
		ey = ey_up_2;											// control point 2
		ez = ez_lookto_2;
		ex = cross(ey, ez);
		m2[0][0] = ex.x;	m2[1][0] = ey.x;	m2[2][0] = ez.x;	m2[3][0] = 0;
		m2[0][1] = ex.y;	m2[1][1] = ey.y;	m2[2][1] = ez.y;	m2[3][1] = 0;
		m2[0][2] = ex.z;	m2[1][2] = ey.z;	m2[2][2] = ez.z;	m2[3][2] = 0;
		m2[0][3] = 0;		m2[1][3] = 0;		m2[2][3] = 0;		m2[3][3] = 1.0f;

		// Convert to quarternion -------
		q1 = quat(m1);
		q2 = quat(m2);

		// Interpolate ------------------
		quat qt = slerp(q1, q2, t);
		qt = normalize(qt);

		// Convert back to Matrix -------
		mat4 mt = mat4(qt);

		return mt;
	}

	mat4 TranslateObjAlongPath(float frametime,  vector<vec3> path, vector<mat3> controlpts) {
		mat4 TransPlane1, RotPlane1;
		// Translate Plane Along Path
		if (controlpts.size() < 3) return mat4(0.0);
		static float sumft = 0; // sum of frame times
		sumft += frametime;
		float f = sumft * FRAMES;
		int frame = f;
		if (frame >= path.size() - 1) {								// loop through path
			sumft = 0;
			frame = 0;
		}

		// Rotate Plane Along Path
		vec3 ez1, ey1, ez2, ey2;							// zbase and ybase vectors for two contorl points
		static float t = 0.0;								// t for interpoltation
		t = (float)(frame % (FRAMES - 1)) / (float)(FRAMES - 1);

		ey1 = ey2 = controlpts[frame / (FRAMES - 1)][1];				// ey1 - up 
		ez1 = ez2 = controlpts[frame / (FRAMES - 1)][2];				// ez1 - look at

		if ((frame / (FRAMES - 1)) + 1 < controlpts.size()) {		// check if the next control pt exists
			ey2 = controlpts[(frame / (FRAMES - 1)) + 1][1];		// ey2 - up
			ez2 = controlpts[(frame / (FRAMES - 1)) + 1][2];		// ez2 - look at
		}
		RotPlane1 = linint_between_two_orientations(ez1, ey1, ez2, ey2, t);
		TransPlane1 = glm::translate(glm::mat4(1.0f), path[frame]);

		return TransPlane1 * RotPlane1;
	}

	mat4 CamPathView(float frametime) {
		// Translate Plane Along Path
		static float sumft = 0; // sum of frame times
		sumft += frametime;
		float f = sumft * FRAMES;
		int frame = f;
		if (frame >= camcardinal.size() - 1) {								// loop through path
			sumft = 0;
			frame = 0;
		}

		camera->pos = camcardinal[frame];

		float t = 0.0;
		t = (float)(frame % (FRAMES - 1)) / (float)(FRAMES - 1);
		vec3 ez1, ey1, ez2, ey2;
		ez1 = ez2 = campath_controlpts[frame / (FRAMES - 1)][1];				// ez1 - up
		ey1 = ey2 = campath_controlpts[frame / (FRAMES - 1)][2];				// ey1 - lookat

		if ((frame / (FRAMES - 1)) + 1 < campath_controlpts.size()) {		// check if the next control pt exists
			ez2 = campath_controlpts[(frame / (FRAMES - 1)) + 1][1];		// ez2 - up
			ey2 = campath_controlpts[(frame / (FRAMES - 1)) + 1][2];		// ey2 - lookat
		}

		mat4 RotPlane = linint_between_two_orientations(ez1, ey1, ez2, ey2, t);
		mat4 TransPlane = glm::translate(mat4(1.0), camcardinal[frame]);
		return transpose(RotPlane) * TransPlane;
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
		if (cam_persp) {
			V = CamPathView(frametime);
		}

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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureID);
		glDisable(GL_DEPTH_TEST);
        shape->draw(skyprog,false);
		glEnable(GL_DEPTH_TEST);
        skyprog->unbind();

        /************* draw terrain ******************/
        glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -9.0f, -50));
        M = TransY;

        vec3 offset = camera->pos;
			offset.y = 0; offset.x = (int)offset.x;	offset.z = (int)offset.z;
        vec3 bg = vec3(254. / 255., 225. / 255., 168. / 255.);
        if (renderstate == 2)
            bg = vec3(49. / 255., 88. / 255., 114. / 255.);

        heightshader->bind();
        heightshader->setMVP(&M[0][0], &V[0][0], &P[0][0]);
        glUniform3fv(heightshader->getUniform("camoff"), 1, &offset[0]);
        glUniform3fv(heightshader->getUniform("campos"), 1, &camera->pos[0]);
        glUniform3fv(heightshader->getUniform("bgcolor"), 1, &bg[0]);
        glUniform1i(heightshader->getUniform("renderstate"), renderstate);
        glBindVertexArray(VertexArrayID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, HeightTexID);
        glDrawArrays(GL_TRIANGLES, 0, MESHSIZE*MESHSIZE * 6);
        heightshader->unbind();

		//cout << camera->pos.x << " " << camera->pos.y << " " << camera->pos.z << " " << endl;

		// draw control points
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		mat4 RotateCP = rotate(mat4(1.0), radians(-180.0f), vec3(1, 0, 0));
		mat4 ScaleCP = scale(mat4(1.0), vec3(0.5));
		int activate_red = 0;
		float size =0.5, red = 0.0, green = 0.0;
		//Path1_CP->buildModelMat();

		for (int i = 0; i < Path1_CP->modelMats.size(); i++) {
			M = Path1_CP->modelMats[i] *RotateCP * ScaleCP;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform1f(prog->getUniform("red"), red);
			glUniform1f(prog->getUniform("green"), green);
			dragon->draw(prog, false);
			if (activate_red) {
				red += 0.2;
			} else{
				green += 0.2;
			}
			if (red >= 1.0 || green >= 1.0) {
				activate_red = !activate_red;
				red = 0.0; green = 0.0;
			}
		}

		activate_red = 0;
		size = 0.1, red = 0.0, green = 0.0;
		for (int i = 0; i < campath_controlpts.size(); i++) {
			mat4 transCP = translate(mat4(1.0), (campath_controlpts[i][0]*-1.0f));
			M = transCP * S;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform1f(prog->getUniform("red"), red);
			glUniform1f(prog->getUniform("green"), green);
			shape->draw(prog, false);

			if (activate_red) {
				red += 0.2;
			}
			else {
				green += 0.2;
			}
			if (red >= 1.0 || green >= 1.0) {
				activate_red = !activate_red;
				red = 0.0; green = 0.0;
			}
		}

		// Draw the Dragon -------------------------------------------------------------------
		// pplane->bind();
		//
		// glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);				// send constant attributes to shaders
		// glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		// glUniform3fv(pplane->getUniform("campos"), 1, &camera->pos[0]);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, Texture2ID);
		//
		// // initalize transformation of dragon
		// glm::mat4 T = glm::translate(glm::mat4(1.0f), vec3(0, 0, -3));
		// glm::mat4 ScalePlane = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
		// float sangle = -3.1415926 / 2.;
		// glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(1, 0, 0));
		// sangle = 3.1415926;
		// glm::mat4 RotateZPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 0, 1));
		// glm::mat4 RotateYPlane = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 1, 0));
		//
		// M =  T * TranslateObjAlongPath(frametime/2.0, path1_cardinal, Path1_CP->points)* RotateZPlane;
		// glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		// dragon->draw(pplane, false);
		// pplane->unbind();

		// Draw the line path --------------------------------------------------------------

		glm::vec3 linecolor = glm::vec3(1, 0, 0);
		path1_render.draw(P, V, linecolor);

		linecolor = glm::vec3(0, 0, 0);
		campath_inverse_render.draw(P, V, linecolor);


		//anim ish *******************************************************
		static double totaltime_ms=0;
		totaltime_ms += frametime*1000.0;
		static double totaltime_untilframe_ms = 0;
		totaltime_untilframe_ms += frametime*1000.0;

		P = getPerspectiveMatrix();
	V = camera->getViewMatrix();
	M = glm::mat4(1);

	// Setup Animation
	string animationName = "";
	int animIdx = 0;
	if (!switchAnim)
	{
		animationName = "ArmatureAction";
		animIdx = 0;
	}
	else
	{
		animationName = "ArmatureAction2";
		animIdx = 1;
	}
	for (int ii = 0; ii < 200; ii++)
			animmat[ii] = mat4(1);
	long long animationDuration = root->getDuration("ArmatureAction");
	int keyFrameCount = root->getKeyFrameCount("ArmatureAction");
	int anim_step_width_ms = animationDuration / keyFrameCount;
	static float frame = 0;
	//static float fframe = 0;
	if (totaltime_untilframe_ms >= anim_step_width_ms)
	{
			totaltime_untilframe_ms = 0;
			if (slowMo)
			{
				frame += 0.1f;
			}
			else if (speedUp)
			{
				frame+=3;
			}
			else
			{
				frame++;
			}
	}
	if (frame >= 24)
	{
		frame = 0;
	}

	static float inter = 0;

	root->play_animation(frame, animationName, inter);
	if (switchAnim && inter < 1)
	{
		inter += frametime;
	}
	else if (!switchAnim && inter > 0)
	{
		inter -= frametime;
	}

	/**************/
	/* DRAW SHAPE */
	/**************/
	S = glm::scale(glm::mat4(1), glm::vec3(1.0f));
	glm::mat4	T = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	glm::mat4 pathMB = TranslateObjAlongPath(frametime/258.0, path1_cardinal, Path1_CP->points); //bones
	glm::mat4 pathML = TranslateObjAlongPath(frametime/2.0, path1_cardinal, Path1_CP->points); // lines
	M = pathML * S;
	phongShader->bind();
	phongShader->setMVP(&M[0][0], &V[0][0], &P[0][0]);
	phongShader->setMatrixArray("Manim", 200, &animmat[0][0][0]);


	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, boneCount-4);
	phongShader->unbind();

	dboneShader->bind();
	for (int i=0;i<129;i++)
	{
		if (i==10)
	  {
			glm::mat4 R = glm::rotate(mat4(1),glm::radians(180.0f), glm::vec3(0,1,0))*  glm::rotate(mat4(1),glm::radians(90.0f), glm::vec3(0,0,1));
		 	M =  pathMB * animbones[10]*  R *  scale(mat4(1), vec3(0.6, 0.6, 0.6));
		 	dboneShader->setMVP(&M[0][0], &V[0][0], &P[0][0]);
		 	skull ->draw(dboneShader,false);
	  }
		else
		{
			M = pathMB * animbones[i]*  translate(mat4(1), vec3(0.5, 0, 0))*scale(mat4(1), vec3(0.4, 0.4, 0.4));
			dboneShader->setMVP(&M[0][0], &V[0][0], &P[0][0]);
			dbone->draw(dboneShader,false);
		}
	}

};
};

int main(int argc, char **argv) {

	// setup resource directory
	if (argc >= 2)
		resourceDir = argv[1];


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
