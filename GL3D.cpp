#include "Model.hpp"

void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::cerr << "OpenGL ";
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			std::cerr << "High"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			std::cerr << "Medium"; break;
		case GL_DEBUG_SEVERITY_LOW:
			std::cerr << "Low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			std::cerr << "Info"; break;
		default: break;
	}
	std::cerr << ", ";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
			std::cerr << "Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			std::cerr << "Deprecation"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			std::cerr << "Undefined behaviour/bug"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			std::cerr << "Performance"; break;
		case GL_DEBUG_TYPE_PORTABILITY:
			std::cerr << "Portability"; break;
		default:
			std::cerr << "Unknown (" << type << ")";
	}
	std::cerr << ", " << message << " (" << id << ")" << std::endl;
}

static float CameraXOffset = 0.0f;
static float CameraYOffset = 0.0f;
static float CameraZOffset = 0.0f;

void KeyCallback(GLFWwindow* aWindow, int aKey, int aScancode, int aAction, int aModifiers) {
	if(aAction == GLFW_RELEASE || glfwGetInputMode(aWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) return;

	switch(aKey) {
		case GLFW_KEY_ESCAPE:
			glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		case GLFW_KEY_W:
			CameraZOffset -= 0.05f;
			break;
		case GLFW_KEY_A:
			CameraXOffset -= 0.05f;
			break;
		case GLFW_KEY_S:
			CameraZOffset += 0.05f;
			break;
		case GLFW_KEY_D:
			CameraXOffset += 0.05f;
			break;
		case GLFW_KEY_R:
			CameraYOffset += 0.05f;
			break;
		case GLFW_KEY_F:
			CameraYOffset -= 0.05f;
			break;
		case GLFW_KEY_Q:
			CameraXOffset = 0.0f;
			CameraYOffset = 0.0f;
			CameraZOffset = 0.0f;
			break;
		case GLFW_KEY_X:
			glfwDestroyWindow(aWindow);
			std::exit(0);
	}
}

const static double FOV = 45.0;

static bool FirstMove = true;
static double LastX = 400, LastY = 400;

static double Pitch = 0.0;
static double Yaw = -90.0; //so we start oriented correctly: 0 is to the right of X axis, 90 is back

glm::vec3 Direction;

void MouseCallback(GLFWwindow* aWindow, double aX, double aY) {
	if(glfwGetInputMode(aWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) return;

	if(FirstMove) {
		LastX = aX;
		LastY = aY;
		FirstMove = false;
	}

	double DX = aX - LastX;
	double DY = LastY - aY;

	LastX = aX;
	LastY = aY;

	double Sensitivity = 0.1f;
	DX *= Sensitivity;
	DY *= Sensitivity;

	Yaw   += DX;
	Pitch += DY;

	//limits
	if(Pitch > 89.0f) {
		Pitch = 89.0f;
	}
	if(Pitch < -89.0f) {
		Pitch = -89.0f;
	}

	Direction = glm::vec3();
	Direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Direction.y = sin(glm::radians(Pitch));
	Direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Direction = glm::normalize(Direction);
}

void MouseKeyCallback(GLFWwindow* aWindow, int aKey, int aAction, int aModifiers) {
	if(aKey == GLFW_MOUSE_BUTTON_RIGHT && aAction == GLFW_PRESS) {
		std::cout << "Window clicked!";
		glfwSetInputMode(aWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		return;
	}

}

int main() {
    if(glfwInit() != GLFW_TRUE) {
		std::cerr << "GLFW init fail! " << glfwGetError(NULL) << "\n";
		return -1;
	};

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL 3D", NULL, NULL);
    glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseKeyCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, MouseCallback);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "GLAD failed to initialize. \n";
		return -1;
	}

	glDebugMessageCallback(GLDebugCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glEnable(GL_BLEND); //texture blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST); //depth testing
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE); //backface culling
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_MULTISAMPLE); //anti aliasing

	//imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 450 core");

	Model m(std::filesystem::path("./Untitled2.glb"));

	GLint samplers[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		30, 31
	};

	Shader s("vertBase.glsl", "fragBase.glsl");
	s.bind();
	glUniform1iv(16, 32, &samplers[0]);

	Shader sa("vertAnim.glsl", "fragAnim.glsl");
	sa.bind();
	glUniform1iv(16, 32, &samplers[0]);

	glm::mat4 matrix;
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians((float)FOV), 800.0f/800.0f, 0.1f, 100.0f);
	glm::mat4 view = glm::mat4(1.0f);

	bool renderBase = false;
	bool overrideAnimTime = false;
	float animTime = 0.0;
    while (!glfwWindowShouldClose(window)) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//camera
		glm::vec3 camera_pos = glm::vec3(CameraXOffset, CameraYOffset, CameraZOffset);
		view = glm::lookAt(camera_pos, camera_pos + Direction, glm::vec3(0.0, 1.0, 0.0));

		model = glm::mat4(1.0f);
		matrix = proj * view * model;

		if(!overrideAnimTime) {
			animTime = std::fmod(glfwGetTime(), 1.5);
		}
		m.setStateAtTime(animTime);

		if(renderBase) {
			//base model
			s.bind();
			m.draw(matrix);
		}

		//animated model
		sa.bind();
		m.draw(matrix);

		//gui for control and debugging
		ImGui::Begin("Anim control");
		ImGui::Checkbox("Render base model", &renderBase);
		ImGui::Checkbox("Override time", &overrideAnimTime);
		ImGui::SliderFloat("Anim seconds", &animTime, 0, 3.3333);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
