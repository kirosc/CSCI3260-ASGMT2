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
#include "Dependencies/stb_image/stb_image.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

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

GLuint groundVAO;
GLuint groundVBO;
GLuint groundEBO;

Model ground;

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
	//TODO: Use mouse to do interactive events and animation

}

void motion_callback(int x, int y)
{
	//TODO: Use mouse to do interactive events and animation
}

void keyboard_callback(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation

}

void special_callback(int key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation

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

GLuint loadTexture(const char* texturePath)
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

	GLuint textureID = 0;
	//TODO: Create one OpenGL texture and set the texture parameter 


	stbi_image_free(data);
	std::cout << "Load " << texturePath << " successfully!" << std::endl;
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture

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

	// Vertex position
	GLint posAttrib = glGetAttribLocation(programID, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));

	// Set up view transformation
	mat4 view = lookAt(
		vec3(0.0f, -1.5f, 0.8f),	// Position of the camera
		vec3(0.0f,  0.0f, 0.0f),	// The point to be centered on-screen
		vec3(0.0f,  0.0f, 1.0f)		// The up axis
	);
	GLint uniView = glGetUniformLocation(programID, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, value_ptr(view));

	// Create a perspective projection matrix
	// Specify FOV, aspect ratio, near and far
	mat4 proj = glm::perspective(glm::radians(40.0f), 512.0f / 512.0f, 1.0f, 10.0f);
	GLint uniProj = glGetUniformLocation(programID, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, value_ptr(proj));
}

void paintGL(void)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures

	glBindVertexArray(groundVAO);
	glDrawElements(GL_TRIANGLES, ground.indices.size(), GL_UNSIGNED_INT, 0); // Rendering

	glFlush();
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
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Assignment 2");

	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Paint the background color once
	glutDisplayFunc(paintGL);

	glutMouseFunc(mouse_callback);
	glutMotionFunc(motion_callback);
	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(special_callback);

	glutMainLoop();

	return 0;
}

void mouseWheel_callback(int wheel, int direction, int x, int y)
{
	// Optional.
}