#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// TODO Dodanie map wektorów normalnych/bump mapping
// TODO Jeden z nich g³adki - kula, torus lub powierzchnia Beziera

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemapTexture(std::vector<std::string> faces);
void changeCameraId(int id);
Camera getCurrentCamera();
void ChangeCameraDir(Camera_Movement direction, float deltaTime);
glm::vec3 CountLightFront();

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera cameraGlobal(glm::vec3(0.0f, 15.0f, 11.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -45.0f);
Camera cameraMovingObj(glm::vec3(0.0f, 0.0f, 0.0f));
Camera cameraFPP(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
int cameraId = 0;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
bool isDay = true;
bool isBlinn = false;
bool isSpotlightCurrCamera = true;
float lightYaw = 0.0f;
float lightPitch = 0.0f;

// moving cube
bool isMovingObj = true;
float movingObjTime = NULL;
float movingObjRadius = 5.0f;
float movingObjSpeed = 0.75f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader zprogram
	// ------------------------------------
	Shader lightingShader("multiple_lights.vs", "multiple_lights.fs");
	Shader lightingShaderBlinn("multiple_lights.vs", "multiple_lights_blinn.fs");
	Shader lightCubeShader("light_cube.vs", "light_cube.fs");
	Shader sphereShader("sphere.vs", "sphere.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float cubeVertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};
	// positions all containers
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// first, configure the cube's VAO (and VBO)
	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Cubemaps
	std::vector<std::string> faces
	{
		"resources/textures/right.jpg",
		"resources/textures/left.jpg",
		"resources/textures/top.jpg",
		"resources/textures/bottom.jpg",
		"resources/textures/front.jpg",
		"resources/textures/back.jpg"
	};
	std::vector<std::string> facesNight
	{
		"resources/textures/right_night.jpg",
		"resources/textures/left_night.jpg",
		"resources/textures/top_night.jpg",
		"resources/textures/bottom_night.jpg",
		"resources/textures/front_night.jpg",
		"resources/textures/back_night.jpg"
	};

	unsigned int cubemapTexture = loadCubemapTexture(faces);
	unsigned int cubemapTextureNight = loadCubemapTexture(facesNight);
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

#pragma region Sphere
	const float PI = acos(-1.0f);

	int sectorCount = 36;
	int stackCount = 18;
	float sphereRadius = 5.0f;
	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / sphereRadius;    // normal
	float s, t;                                     // texCoord

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	std::vector<GLfloat> sphereVertices;
	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = sphereRadius * cosf(stackAngle);             // r * cos(u)
		z = sphereRadius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position
			x = xy * cosf(sectorAngle);
			sphereVertices.push_back(x);      // x = r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);
			sphereVertices.push_back(y);      // y = r * cos(u) * sin(v)
			sphereVertices.push_back(z);

			// normalized vertex normal
			sphereVertices.push_back(x * lengthInv);
			sphereVertices.push_back(y * lengthInv);
			sphereVertices.push_back(z * lengthInv);

		}
	}

	std::vector<unsigned int> indices;

	// indices
	//  k1--k1+1
	//  |  / |
	//  | /  |
	//  k2--k2+1
	unsigned int k1, k2;
	for (int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding 1st and last stacks
			if (i != 0)
			{
				indices.push_back(k1);   // k1---k2---k1+
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stackCount - 1))
			{
				indices.push_back(k1 + 1); // k1+1---k2---k2+1
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	unsigned int sphereVBO, sphereVAO, sphereEBO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(sphereVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

#pragma endregion

	// shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);


	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ambient of directional light
		float ambientValue = isDay ? 0.05f : 0.005f;

		// count for moving object and camera
		if (movingObjTime == NULL || isMovingObj)
		{
			movingObjTime += movingObjSpeed * deltaTime;
		}
		float cubeX = static_cast<float>(sin(movingObjTime) * movingObjRadius);
		float cubeZ = static_cast<float>(cos(movingObjTime) * movingObjRadius);
		float lightX = static_cast<float>(sin(movingObjTime + movingObjSpeed * 0.08f) * movingObjRadius);
		float lightZ = static_cast<float>(cos(movingObjTime + movingObjSpeed * 0.08f) * movingObjRadius);
		glm::vec3 movingObjPos = glm::vec3(cubeX, 0.0f, -10.0f + cubeZ);
		glm::vec3 movingLightPos = glm::vec3(lightX, 0.0f, -10.0f + lightZ);
		// update camera pos
		cameraMovingObj.Position = movingObjPos + glm::vec3(0.0f, 0.6f, 0.0f);
		cameraMovingObj.Yaw = -glm::degrees(movingObjTime);
		cameraMovingObj.updateCameraVectors();

		// be sure to activate shader when setting uniforms/drawing objects
		Shader currLightingShader = isBlinn ? lightingShaderBlinn : lightingShader;
		currLightingShader.use();
		currLightingShader.setVec3("viewPos", getCurrentCamera().Position);
		currLightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
		currLightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		currLightingShader.setFloat("material.shininess", 32.0f);

#pragma region UnifromsLight

		/*
		   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
		   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
		   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
		   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
		*/
		// directional light
		currLightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		currLightingShader.setVec3("dirLight.ambient", ambientValue, ambientValue, ambientValue);
		currLightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		currLightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// point light 1
		currLightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		currLightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		currLightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		currLightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		currLightingShader.setFloat("pointLights[0].constant", 1.0f);
		currLightingShader.setFloat("pointLights[0].linear", 0.09f);
		currLightingShader.setFloat("pointLights[0].quadratic", 0.032f);
		// point light 2
		currLightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		currLightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		currLightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		currLightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		currLightingShader.setFloat("pointLights[1].constant", 1.0f);
		currLightingShader.setFloat("pointLights[1].linear", 0.09f);
		currLightingShader.setFloat("pointLights[1].quadratic", 0.032f);
		// point light 3
		currLightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		currLightingShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		currLightingShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		currLightingShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		currLightingShader.setFloat("pointLights[2].constant", 1.0f);
		currLightingShader.setFloat("pointLights[2].linear", 0.09f);
		currLightingShader.setFloat("pointLights[2].quadratic", 0.032f);
		// point light 4
		currLightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		currLightingShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		currLightingShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		currLightingShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		currLightingShader.setFloat("pointLights[3].constant", 1.0f);
		currLightingShader.setFloat("pointLights[3].linear", 0.09f);
		currLightingShader.setFloat("pointLights[3].quadratic", 0.032f);
		// spotLight
		float x = isSpotlightCurrCamera && cameraId != 1 ? 1.0f : 0.0f;
		currLightingShader.setVec3("spotLight[0].position", getCurrentCamera().Position);
		currLightingShader.setVec3("spotLight[0].direction", getCurrentCamera().Front);
		currLightingShader.setVec3("spotLight[0].ambient", 0.0f, 0.0f, 0.0f);
		currLightingShader.setVec3("spotLight[0].diffuse", 1.0f * x, 1.0f * x, 1.0f * x);
		currLightingShader.setVec3("spotLight[0].specular", 1.0f * x, 1.0f * x, 1.0f * x);
		currLightingShader.setFloat("spotLight[0].constant", 1.0f * x);
		currLightingShader.setFloat("spotLight[0].linear", 0.09f * x);
		currLightingShader.setFloat("spotLight[0].quadratic", 0.032f * x);
		currLightingShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(12.5f)));
		currLightingShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(15.0f)));
		
		currLightingShader.setVec3("spotLight[1].position", movingLightPos);
		currLightingShader.setVec3("spotLight[1].direction", CountLightFront());
		currLightingShader.setVec3("spotLight[1].ambient", 0.0f, 0.0f, 0.0f);
		currLightingShader.setVec3("spotLight[1].diffuse", 1.0f, 1.0f, 1.0f);
		currLightingShader.setVec3("spotLight[1].specular", 1.0f, 1.0f, 1.0f);
		currLightingShader.setFloat("spotLight[1].constant", 1.0f);
		currLightingShader.setFloat("spotLight[1].linear", 0.09f);
		currLightingShader.setFloat("spotLight[1].quadratic", 0.032f);
		currLightingShader.setFloat("spotLight[1].cutOff", glm::cos(glm::radians(12.5f)));
		currLightingShader.setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(15.0f)));

#pragma endregion

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(getCurrentCamera().Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = getCurrentCamera().GetViewMatrix();
		currLightingShader.setMat4("projection", projection);
		currLightingShader.setMat4("view", view);

		// world transformation
		glm::mat4 model = glm::mat4(1.0f);
		currLightingShader.setMat4("model", model);

		// render containers
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 1; i < 9; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			currLightingShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		// Draw moving cube
		glm::mat4 modelMoving = glm::mat4(1.0f);
		modelMoving = glm::translate(modelMoving, movingObjPos);
		modelMoving = glm::rotate(modelMoving, movingObjTime, glm::vec3(0.0f, 1.0f, 0.0f));
		currLightingShader.setMat4("model", modelMoving);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// also draw the lamp object(s)
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(lightCubeVAO);
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lightCubeShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// Draw sphere
		//std::cout << "Start: " << glGetError() << std::endl;
		sphereShader.use();
		sphereShader.setVec3("viewPos", getCurrentCamera().Position);
		sphereShader.setFloat("material.shininess", 32.0f);
		sphereShader.setVec3("material.objectColor", 1.0f, 0.5f, 0.31f);

#pragma region UnifromsLight

		/*
		   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
		   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
		   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
		   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
		*/
		// directional light
		sphereShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		sphereShader.setVec3("dirLight.ambient", ambientValue, ambientValue, ambientValue);
		sphereShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		sphereShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		// point light 1
		sphereShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		sphereShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		sphereShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		sphereShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("pointLights[0].constant", 1.0f);
		sphereShader.setFloat("pointLights[0].linear", 0.09f);
		sphereShader.setFloat("pointLights[0].quadratic", 0.032f);
		// point light 2
		sphereShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		sphereShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		sphereShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		sphereShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("pointLights[1].constant", 1.0f);
		sphereShader.setFloat("pointLights[1].linear", 0.09f);
		sphereShader.setFloat("pointLights[1].quadratic", 0.032f);
		// point light 3
		sphereShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		sphereShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		sphereShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		sphereShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("pointLights[2].constant", 1.0f);
		sphereShader.setFloat("pointLights[2].linear", 0.09f);
		sphereShader.setFloat("pointLights[2].quadratic", 0.032f);
		// point light 4
		sphereShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		sphereShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		sphereShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		sphereShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("pointLights[3].constant", 1.0f);
		sphereShader.setFloat("pointLights[3].linear", 0.09f);
		sphereShader.setFloat("pointLights[3].quadratic", 0.032f);
		// spotLight
		sphereShader.setVec3("spotLight[0].position", getCurrentCamera().Position);
		sphereShader.setVec3("spotLight[0].direction", getCurrentCamera().Front);
		sphereShader.setVec3("spotLight[0].ambient", 0.0f, 0.0f, 0.0f);
		sphereShader.setVec3("spotLight[0].diffuse", 1.0f, 1.0f, 1.0f);
		sphereShader.setVec3("spotLight[0].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("spotLight[0].constant", 1.0f);
		sphereShader.setFloat("spotLight[0].linear", 0.09f);
		sphereShader.setFloat("spotLight[0].quadratic", 0.032f);
		sphereShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(12.5f)));
		sphereShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(15.0f)));

		sphereShader.setVec3("spotLight[1].position", movingLightPos);
		sphereShader.setVec3("spotLight[1].direction", cameraMovingObj.Front);
		sphereShader.setVec3("spotLight[1].ambient", 0.0f, 0.0f, 0.0f);
		sphereShader.setVec3("spotLight[1].diffuse", 1.0f, 1.0f, 1.0f);
		sphereShader.setVec3("spotLight[1].specular", 1.0f, 1.0f, 1.0f);
		sphereShader.setFloat("spotLight[1].constant", 1.0f);
		sphereShader.setFloat("spotLight[1].linear", 0.09f);
		sphereShader.setFloat("spotLight[1].quadratic", 0.032f);
		sphereShader.setFloat("spotLight[1].cutOff", glm::cos(glm::radians(12.5f)));
		sphereShader.setFloat("spotLight[1].outerCutOff", glm::cos(glm::radians(15.0f)));

#pragma endregion

		// view/projection transformations
		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);

		// world transformation
		glBindVertexArray(sphereVAO);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f));
		sphereShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, indices.data());

		//std::cout << "HERE: " << glGetError() << std::endl;

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(getCurrentCamera().GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		if (isDay)
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		else
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureNight);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightCubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &sphereEBO);
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		ChangeCameraDir(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		ChangeCameraDir(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		ChangeCameraDir(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		ChangeCameraDir(RIGHT, deltaTime);

	if (cameraId == 2)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraFPP.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraFPP.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraFPP.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraFPP.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		changeCameraId(0);
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		changeCameraId(1);
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		changeCameraId(2);
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		isDay = !isDay;
	if (key == GLFW_KEY_B && action == GLFW_PRESS)
		isBlinn = !isBlinn;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		isMovingObj = !isMovingObj;
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		isSpotlightCurrCamera = !isSpotlightCurrCamera;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (cameraId != 2) return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	cameraFPP.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (cameraId == 2)
		cameraFPP.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemapTexture(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void changeCameraId(int id)
{
	cameraId = id;
}

Camera getCurrentCamera()
{
	switch (cameraId)
	{
	case 0:
		return cameraGlobal;
	case 1:
		return cameraMovingObj;
	default:
		return cameraFPP;
	}
}

void ChangeCameraDir(Camera_Movement direction, float deltaTime)
{
	float velocity = 5.0f * deltaTime;
	if (direction == FORWARD)
		lightPitch += velocity;
	if (direction == BACKWARD)
		lightPitch -= velocity;
	if (direction == LEFT)
		lightYaw += velocity;
	if (direction == RIGHT)
		lightYaw -= velocity;


	if (lightPitch > 89.0f)
		lightPitch = 89.0f;
	if (lightPitch < -89.0f)
		lightPitch = -89.0f;


	if (lightYaw > 89.0f)
		lightYaw = 89.0f;
	if (lightYaw < -89.0f)
		lightYaw = -89.0f;
}

glm::vec3 CountLightFront()
{
	glm::vec3 front;
	front.x = cos(glm::radians(cameraMovingObj.Yaw + lightYaw)) * cos(glm::radians(cameraMovingObj.Pitch + lightPitch));
	front.y = sin(glm::radians(cameraMovingObj.Pitch + lightPitch));
	front.z = sin(glm::radians(cameraMovingObj.Yaw + lightYaw)) * cos(glm::radians(cameraMovingObj.Pitch + lightPitch));
	return glm::normalize(front);
}
