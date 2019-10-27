/*********************************************************
FILE : main.cpp (csci3260 2019-2020 Assignment 2)
*********************************************************/
/*********************************************************
Student Information
Student ID:
Student Name:
*********************************************************/

#define BUFFER_OFFSET(i) ((char *)NULL + (i * sizeof(float)))
#include "Dependencies/glew/glew.h"
#include "Dependencies/freeglut/freeglut.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include "Dependencies/stb_image/stb_image.h"
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// Function Declaration
GLuint loadTexture(const char* texturePath, const int idx);

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

using namespace std;
using glm::vec3;
using glm::mat4;

GLint programID;
GLint uniTrans;

GLuint textureID[4];
GLuint groundTexture;
GLuint catTexture;
GLuint dogTexture;
GLuint penguinTexture;

GLuint groundVAO;
GLuint groundVBO;
GLuint groundEBO;
GLuint catVAO;
GLuint catVBO;
GLuint catEBO;
GLuint dogVAO;
GLuint dogVBO;
GLuint dogEBO;
GLuint penguinVAO;
GLuint penguinVBO;
GLuint penguinEBO;

float translate_delta = 0.01f;
float rotate_delta = 0.1f;
float brightness_delta = 0.1f;
float cameraX_delta = 0.01f;
float cameraY_delta = 0.01f;
float cameraZ_delta = 0.05f;

int translate_press_num = 0;
int rotate_press_num = 0;
int brightness_press_num = 7;
int cameraX_move_num = 0;
int cameraY_move_num = 50; // Y: 0.5
int cameraZ_move_num = 20; // Z: 1.0

int lastMouseX, lastMouseY;
bool mouseClicked = false;

Model ground;
Model cat;
Model dog;
Model penguin;

//a series utilities for setting shader parameters 
void setMat4(const std::string& name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}
void setVec4(const std::string& name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string& name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string& name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string& name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void mouse_callback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			lastMouseX = x;
			lastMouseY = y;
			mouseClicked = true;
		}
		else
		{
			mouseClicked = false;
		}
	}
}

void motion_callback(int x, int y)
{
	if (mouseClicked)
	{
		if (y < lastMouseY)
		{
			cameraY_move_num++;
		}
		else if (y > lastMouseY)
		{
			cameraY_move_num--;
		}


		if (x < lastMouseX)
		{
			cameraX_move_num--;
		}
		else if (x > lastMouseX)
		{
			cameraX_move_num++;
		}

		printf("X: %.3f, Y: %.3f\n", cameraX_delta * cameraX_move_num, cameraY_delta * cameraY_move_num);
		lastMouseX = x;
		lastMouseY = y;
	}
}

void mouseWheel_callback(int wheel, int direction, int x, int y)
{
	//					Zoom-in				 Zoom-out
	direction > 0 ? cameraZ_move_num-- : cameraZ_move_num++;
}

void keyboard_callback(unsigned char key, int x, int y)
{
	if (key == 'w')
	{
		brightness_press_num++;
	}
	else if (key == 's')
	{
		brightness_press_num--;
	}
	else if (key == 'a')
	{
	}
	else if (key == 'd')
	{
	}
	else if (key == '1')
	{
		catTexture = loadTexture("./resources/cat/cat_01.jpg", 1);
	}
	else if (key == '2')
	{
		catTexture = loadTexture("./resources/cat/cat_02.jpg", 1);
	}
	else if (key == '3')
	{
		groundTexture = loadTexture("./resources/floor/floor_diff.jpg", 0);
	}
	else if (key == '4')
	{
		groundTexture = loadTexture("./resources/floor/floor_spec.jpg", 0);
	}
	else if (key == 27)
	{
		// Exit the program gracefully
		glutLeaveMainLoop();
	}
}

void special_callback(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		translate_press_num++;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		translate_press_num--;
	}
	else if (key == GLUT_KEY_LEFT)
	{
		rotate_press_num--;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		rotate_press_num++;
	}
}

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot handle all obj files.

	struct Vertice {
		// struct for identify if a vertice has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const Vertice& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const Vertice& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	// temple variables to store v, vt and vn
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	// check for duplicates
	std::map<Vertice, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			Vertice vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertice never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertice
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

GLuint loadTexture(const char* texturePath, const int idx)
{
	// tell stb_image.h to flip loaded texture's on the y-axis.
	stbi_set_flip_vertically_on_load(true);
	// load the texture data into "data"
	int Width, Height, BPP;
	unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
	// Please pay attention to the format when sending the data to GPU
	GLenum format = 3;
	switch (BPP) {
	case 1: format = GL_RED; break;
	case 3: format = GL_RGB; break;
	case 4: format = GL_RGBA; break;
	}
	if (!data) {
		std::cout << "Failed to load texture: " << texturePath << std::endl;
		exit(1);
	}

	glGenTextures(4, textureID);
	glBindTexture(GL_TEXTURE_2D, textureID[idx]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	std::cout << "Load " << texturePath << " successfully!" << std::endl;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID[idx];
}

// Transform the object based on the given name
void transform(string name) {
	mat4 model = mat4(1.0f);

	if (name == "ground")
	{
		model *= glm::translate(mat4(1.0f), vec3(0.0f, 0.05f, 0.0f));
		model *= glm::scale(mat4(1.0f), vec3(0.05f, 0.05f, 0.0625f)); // Apply first
	}
	else if (name == "cat")
	{
		float catAngle = rotate_delta * rotate_press_num * -45.0f;
		float catX = translate_delta * translate_press_num * sin(catAngle * M_PI / 180);
		float catZ = translate_delta * translate_press_num * cos(catAngle * M_PI / 180);

		model = glm::translate(mat4(1.0f), vec3(catX, 0.0f, catZ)) *
				glm::rotate(mat4(1.0f), rotate_delta * rotate_press_num * glm::radians(-45.0f), vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(mat4(1.0f), glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)) * // Front facing
				glm::scale(mat4(1.0f), vec3(0.005f)); // Scale down
	}
	else if (name == "dog")
	{
		model = glm::translate(mat4(1.0f), vec3(0.2f, 0.0f, 0.0f)) *
			glm::scale(mat4(1.0f), vec3(0.005f)); // Scale down
	}
	else if (name == "penguin")
	{
		model = glm::translate(mat4(1.0f), vec3(-0.2f, 0.0f, 0.0f)) *
			glm::scale(mat4(1.0f), vec3(0.002f)); // Scale down
	}
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, value_ptr(model));
}

void sendDataToOpenGL()
{
	// Ground
	ground = loadOBJ("resources/floor/floor.obj");
	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, ground.vertices.size() * sizeof (Vertex), &ground.vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &groundEBO); // Element array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.indices.size() * sizeof(unsigned int), &ground.indices[0], GL_STATIC_DRAW);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(programID, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

	GLint texAttrib = glGetAttribLocation(programID, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	GLint normAttrib = glGetAttribLocation(programID, "normal");
	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	groundTexture = loadTexture("./resources/floor/floor_diff.jpg", 0);

	// Cat
	cat = loadOBJ("resources/cat/cat.obj");
	glGenVertexArrays(1, &catVAO);
	glBindVertexArray(catVAO);
	glGenBuffers(1, &catVBO);
	glBindBuffer(GL_ARRAY_BUFFER, catVBO);
	glBufferData(GL_ARRAY_BUFFER, cat.vertices.size() * sizeof(Vertex), &cat.vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &catEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, catEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cat.indices.size() * sizeof(unsigned int), &cat.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	catTexture = loadTexture("./resources/cat/cat_01.jpg", 1);

	// Dog
	dog = loadOBJ("resources/dog/dog.obj");
	glGenVertexArrays(1, &dogVAO);
	glBindVertexArray(dogVAO);
	glGenBuffers(1, &dogVBO);
	glBindBuffer(GL_ARRAY_BUFFER, dogVBO);
	glBufferData(GL_ARRAY_BUFFER, dog.vertices.size() * sizeof(Vertex), &dog.vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &dogEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dogEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, dog.indices.size() * sizeof(unsigned int), &dog.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	dogTexture = loadTexture("./resources/dog/dog.jpg", 2);

	// Penguin
	penguin = loadOBJ("resources/penguin/penguin.obj");
	glGenVertexArrays(1, &penguinVAO);
	glBindVertexArray(penguinVAO);
	glGenBuffers(1, &penguinVBO);
	glBindBuffer(GL_ARRAY_BUFFER, penguinVBO);
	glBufferData(GL_ARRAY_BUFFER, penguin.vertices.size() * sizeof(Vertex), &penguin.vertices[0], GL_STATIC_DRAW);
	glGenBuffers(1, &penguinEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, penguinEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, penguin.indices.size() * sizeof(unsigned int), &penguin.indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	penguinTexture = loadTexture("./resources/penguin/penguin.jpg", 2);

	// Get reference of texture
	GLuint textureID = glGetUniformLocation(programID, "tex");
	glUniform1i(textureID, 0);
}

void paintGL(void)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Get reference of shader variable
	uniTrans = glGetUniformLocation(programID, "model");
	GLint uniAmbient = glGetUniformLocation(programID, "ambient");
	GLint uniLightPos = glGetUniformLocation(programID, "lightPos");
	GLint uniLightColor = glGetUniformLocation(programID, "lightColor");
	GLint viewPos = glGetUniformLocation(programID, "viewPos");

	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures

	// Lighting
	// Ambient
	vec3 ambient(0.5f, 0.5f, 0.5f);
	glUniform3fv(uniAmbient, 1, value_ptr(ambient));

	// Diffuse
	float brightness = brightness_delta * brightness_press_num;

	vec3 lightPos(0.0f, 0.2f, 0.0f);
	vec3 lightColor(brightness);
	glUniform3fv(uniLightPos, 1, value_ptr(lightPos));
	glUniform3fv(uniLightColor, 1, value_ptr(lightColor));

	// Specular
	vec3 cameraPos(cameraX_delta * cameraX_move_num,
				   cameraY_delta * cameraY_move_num,
				   cameraZ_delta * cameraZ_move_num);
	glUniform3fv(viewPos, 1, value_ptr(cameraPos));

	// Ground
	transform("ground");
	// Load textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, groundTexture);
	// Rendering
	glBindVertexArray(groundVAO);
	glDrawElements(GL_TRIANGLES, ground.indices.size(), GL_UNSIGNED_INT, 0);

	// Cat
	transform("cat");

	glBindTexture(GL_TEXTURE_2D, catTexture);

	glBindVertexArray(catVAO);
	glDrawElements(GL_TRIANGLES, cat.indices.size(), GL_UNSIGNED_INT, 0);

	// Dog
	transform("dog");

	glBindTexture(GL_TEXTURE_2D, dogTexture);

	glBindVertexArray(dogVAO);
	glDrawElements(GL_TRIANGLES, dog.indices.size(), GL_UNSIGNED_INT, 0);

	// Penguin
	transform("penguin");

	glBindTexture(GL_TEXTURE_2D, penguinTexture);

	glBindVertexArray(penguinVAO);
	glDrawElements(GL_TRIANGLES, penguin.indices.size(), GL_UNSIGNED_INT, 0);

	// Set up view transformation
	mat4 view = lookAt(
		cameraPos,				// Position of the camera
		vec3(0.0f, 0.0f, 0.0f),	// The point to be centered on-screen
		vec3(0.0f, 1.0f, 0.0f)	// The up axis
	);
	GLint uniView = glGetUniformLocation(programID, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, value_ptr(view));

	// Create a perspective projection matrix
	// Specify FOV, aspect ratio, near and far
	mat4 proj = glm::perspective(glm::radians(40.0f), 512.0f / 512.0f, 0.01f, 10.0f);
	GLint uniProj = glGetUniformLocation(programID, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, value_ptr(proj));

	glutSwapBuffers();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutSetOption(GLUT_MULTISAMPLE, 4);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Assignment 2");

	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Paint the background color once
	glutDisplayFunc(paintGL);

	glutMouseFunc(mouse_callback);
	glutMotionFunc(motion_callback);
	glutMouseWheelFunc(mouseWheel_callback);
	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(special_callback);

	glutMainLoop();

	return 0;
}