#include <iostream>
#include <glew.h>
#include <glfw3.h>
#include <Glaux.h>
#include <glut.h>
#include <vec3.hpp> 
#include <vec4.hpp> 
#include <mat4x4.hpp> 
#include <gtc/matrix_transform.hpp>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <string>
#include <map>
#include <algorithm>
#include <glm.hpp>

#define M 7

#define WIDTH 800
#define HEIGHT 600

using namespace std;
using namespace glm;

GLFWwindow* window;

vec3 cameraPos = vec3(4, 0, 4);
vector<const char*> files =
{
	"b1.obj",
	"b4.obj",
	"b2.obj",
	"b3.obj",
	"b6.obj",
	"b5.obj",
	"b7.obj"
};
bool keys[1024];
bool post = false;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_P)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
	if (key == GLFW_KEY_O)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
	
}
void do_post()
{
	if (keys[GLFW_KEY_P])
	{
		post = true;
	}
	if (keys[GLFW_KEY_O])
	{
		post = false;
	}
}
GLuint GetBMPPicture(const char * imagepath) {

	cout << "Image: " << imagepath << endl;

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char* data;

	FILE* file = fopen(imagepath, "rb");
	if (!file)
	{
		cout << imagepath << "could not be opened" << endl;
		return 0;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		cout << "Not a correct BMP file" << endl;
		fclose(file);
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M')
	{
		cout << "Not a correct BMP file" << endl;
		fclose(file);
		return 0;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = width * height * 3;
	if (dataPos == 0)
		dataPos = 54;

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;
}

bool GetOBJ(const char* path, vector<vec3> & out_vertices, vector<vec2>& out_uvs,
	vector<vec3> & out_normals)
{
	cout << "Loading OBJ file: " << path << endl;

	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<vec3> temp_vertices;
	vector<vec2> temp_uvs;
	vector<vec3> temp_normals;

	FILE* file = fopen(path, "r");
	if (file == NULL)
	{
		cout << "Impossible to open the file!" << endl;
		return false;
	}

	while (1)
	{
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (strcmp(lineHeader, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else
			if (strcmp(lineHeader, "vt") == 0)
			{
				vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else
				if (strcmp(lineHeader, "vn") == 0)
				{
					glm::vec3 normal;
					fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
					temp_normals.push_back(normal);
				}
				else
					if (strcmp(lineHeader, "f") == 0)
					{
						std::string vertex1, vertex2, vertex3;
						unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
						int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
						if (matches != 9)
						{
							cout << "File can't be read by parser" << endl;
							fclose(file);
							return false;
						}
						vertexIndices.push_back(vertexIndex[0]);
						vertexIndices.push_back(vertexIndex[1]);
						vertexIndices.push_back(vertexIndex[2]);
						uvIndices.push_back(uvIndex[0]);
						uvIndices.push_back(uvIndex[1]);
						uvIndices.push_back(uvIndex[2]);
						normalIndices.push_back(normalIndex[0]);
						normalIndices.push_back(normalIndex[1]);
						normalIndices.push_back(normalIndex[2]);

					}
	}
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		vec3 vertex = temp_vertices[vertexIndex - 1];
		vec2 uv = temp_uvs[uvIndex - 1];
		vec3 normal = temp_normals[normalIndex - 1];

		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}
	fclose(file);
	return true;
}

void quicksort(vector<float>& s, vector<vec3> v[M], vector<vec2> u[M],
	vector<vec3> n[M], int first, int last)
{
	float mid, count;
	vector<vec3> tmp_v;
	vector<vec2> tmp_u;
	vector<vec3> tmp_n;
	int f = first, l = last;
	mid = s[(f + l) / 2];
	do
	{
		while (s[f] < mid) f++;
		while (s[l] > mid) l--;
		if (f <= l) 
		{
			count = s[f];
			s[f] = s[l];
			s[l] = count;

			tmp_v = v[f];
			v[f] = v[l];
			v[l] = tmp_v;

			tmp_u = u[f];
			u[f] = u[l];
			u[l] = tmp_u;

			tmp_n = n[f];
			n[f] = n[l];
			n[l] = tmp_n;
			f++;
			l--;
		}
	} while (f < l);
	if (first < l) quicksort(s,v, u, n, first, l);
	if (f < last) quicksort(s, v, u, n, f, last);
}
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
	string Line;

	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path, ios::in);
	Line = "";
	while (getline(VertexShaderStream, Line))
		VertexShaderCode += "\n" + Line;
	VertexShaderStream.close();

	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path, ios::in);
	Line = "";
	while (getline(FragmentShaderStream, Line))
		FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();

	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Comp_Grapf", NULL, NULL);
	if (window == NULL) 
	{
		fprintf(stderr, "Failed to open GLFW window");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) 
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, HEIGHT / 2, WIDTH / 2);

	glfwPollEvents();
	glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);
	glfwSetKeyCallback(window, key_callback);

	glClearColor(0.2f, 0.0f, 0.4f, 0.0f);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	GLuint VertexArrayID[M];
	glGenVertexArrays(M, VertexArrayID);

	GLuint quadVAO;
	glGenVertexArrays(1, &quadVAO);

	GLuint programID = LoadShaders("main.vertex", "main.fragment");
	GLuint screenprog = LoadShaders("posteffect.vertex", "posteffect.fragment");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	GLuint Texture = GetBMPPicture("glass.bmp");
	GLuint TextureID = glGetUniformLocation(programID, "my_texture");
	GLuint TextureID1 = glGetUniformLocation(screenprog, "screenTexture");

	vector<vec3> vertices[M];
	vector<vec2> uvs[M];
	vector<vec3> normals[M];
	bool res[M];
	for (int i = 0; i < M; i++)
	{
		res[i] = GetOBJ(files[i], vertices[i], uvs[i], normals[i]);
	}
	GLfloat quad_vertices[] = 
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};
	GLfloat quad_uvs[] = 
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	GLuint vertexbuffer1;
	glGenBuffers(1, &vertexbuffer1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	GLuint uvbuffer1;
	glGenBuffers(1, &uvbuffer1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_uvs), quad_uvs, GL_STATIC_DRAW);

	vector<float> sumx(M);
	vector<float> sumy(M);
	vector<float> sumz(M);
	for (int j = 0; j < M; j++)
	{
		for (int i = 0; i < vertices[j].size(); i++)
		{
			sumx[j] = sumx[j] + vertices[j][i].x;
			sumy[j] = sumy[j] + vertices[j][i].y;
			sumz[j] = sumz[j] + vertices[j][i].z;
		}
		sumx[j] = sumx[j] / vertices[j].size();
		sumy[j] = sumy[j] / vertices[j].size();
		sumz[j] = sumz[j] / vertices[j].size();
	}
	vector<vec3> billboard(M);
	for (int i = 0; i < M; i++)
	{
		billboard[i] = vec3(sumx[i], sumy[i], sumz[i]);
	}
	vector<float> distance(M);
	for (int i = 0; i < M; i++)
	{
		distance[i] = 0;
	}
	GLuint vertexbuffer[M];
	glGenBuffers(M, vertexbuffer);
	GLuint uvbuffer[M];
	glGenBuffers(M, uvbuffer);
	GLuint normalbuffer[M];
	glGenBuffers(M, normalbuffer);
	for (int i = 0; i < M; i++)
	{
		distance[i] = length(cameraPos - billboard[i]);
	}
	for (int i = 0; i < M; i++)
	{
		cout << distance[i] << endl;
	}

	quicksort(distance, vertices, uvs, normals, 0, M - 1);
	cout << endl;
	for (int i = 0; i < M; i++)
	{
		cout << distance[i] << endl;
	}
	for (int i = 0; i < M; i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(vec3), &vertices[i][0], GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, uvs[i].size() * sizeof(vec2), &uvs[i][0], GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, normals[i].size() * sizeof(vec3), &normals[i][0], GL_STREAM_DRAW);
	}

	GLuint LightID = glGetUniformLocation(programID, "light_pos");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mat4 Projection = perspective(radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	mat4 View = lookAt(cameraPos, vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 Model = mat4(1.0f);
	mat4 MVP = Projection * View * Model;

	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// присоедиение текстуры к объекту текущего кадрового буфера
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR framebuffer" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	do 
	{
		glfwPollEvents();
		do_post();
		if (post) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glClearColor(0.4f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			for (int i = M - 1; i >= 0; i--)
			{

				glUseProgram(programID);
				glBindVertexArray(VertexArrayID[i]);

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

				vec3 lightPos = vec3(6, -7, 3);
				glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture);
				glUniform1i(TextureID, 0);

				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClearColor(0.4f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUseProgram(screenprog);
			glBindVertexArray(quadVAO);

			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texColorBuffer);
			glUniform1i(TextureID1, 0);

			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer1);
			glVertexAttribPointer(
				0,
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer1);
			glVertexAttribPointer(
				1,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
		else
		{
			glClearColor(0.4f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
			glEnable(GL_DEPTH_TEST);

			for (int i = M - 1; i >= 0; i--)
			{

				glUseProgram(programID);
				glBindVertexArray(VertexArrayID[i]);

				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
				glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);

				vec3 lightPos = vec3(4, 4, 4);
				glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, Texture);
				glUniform1i(TextureID, 0);

				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[i]);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[i]);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glEnableVertexAttribArray(2);
				glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[i]);
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

				glDrawArrays(GL_TRIANGLES, 0, vertices[i].size());

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				glDisableVertexAttribArray(2);
			}
		}
		glfwSwapBuffers(window);
		//glfwPollEvents();

	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDeleteBuffers(M, vertexbuffer);
	glDeleteBuffers(M, uvbuffer);
	glDeleteBuffers(M, normalbuffer);
	glDeleteBuffers(1, &vertexbuffer1);
	glDeleteBuffers(1, &uvbuffer1);
	glDeleteProgram(programID);
	glDeleteProgram(screenprog);
	glDeleteTextures(M, &Texture);
	glDeleteTextures(1, &texColorBuffer);
	glDeleteVertexArrays(M, VertexArrayID);
	glDeleteFramebuffers(1, &framebuffer);
	glfwTerminate();

	return 0;
}