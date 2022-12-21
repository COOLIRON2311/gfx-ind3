#pragma once
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <vector>
#include <corecrt_math_defines.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <array>
#include <locale>
#include "camera.h"
#include "lights.h"
#include "object.h"

using namespace std;

// Camera
Camera cam;

// Music
sf::Music bg;
bool music_playing;

// Stuff
vector<Object> objects;
array<GLuint, 1> Programs;
array<GLuint, 9> textures;
vector<Object*> enemy_tanks;
vector<Object*> trees;
vector<Object*> rocks;
vector<Object*> barrels;
PlayerTank* tonk;

// Attribs
GLint Phong_coord;
GLint Phong_texcoord;
GLint Phong_normal;
GLint Phong_mvp;
GLint Phong_viewPos;

// Lights
PointLight pl;
DirLight dl;
SpotLight sl;
Material mat;

// Laser
Material projectile;
sf::Music laser_sfx;
GLuint laser_vbo;
int laser_frames;

// Bullet
sf::Music bullet_sfx;
Object* bullet;
bool bullet_fired;
// glm::vec3 bul_origin;
glm::vec3 bul_dir;
const float bul_speed = 1.0f;
const float bul_max_dst = 50.0f;

// Функция для установки иконки приложения
void SetIcon(sf::Window& wnd);

void ShaderLog(unsigned int shader);
// Функция для загрузки шейдеров
void InitShader();
void LoadAttrib(GLuint prog, GLint& attrib, const char* attr_name);
void LoadUniform(GLuint prog, GLint& attrib, const char* attr_name);
void LoadTexture(GLenum tex_enum, GLuint& tex, const char* path);
// Функция для инициализации вершинного буфера
void InitVBO();
// Функция для инициализации ресурсов
void InitTextures();
void Init();
// Функция для отрисовки
void Draw(sf::Window& window);
// Функция для очистки шейдеров
void ReleaseShader();
// Функция для очистки вершинного буфера
void ReleaseVBO();
// Функция для очистки ресурсов
void Release();

int load_obj(const char* filename, vector<Vertex>& out)
{
	vector<glm::vec3> vertices;
	vector<glm::vec3> normals;
	vector<glm::vec2> uvs;
	
    ifstream in(filename, ios::in);
    if (!in)
    {
        cerr << "Can't open obj " << filename << endl;
        return 0;
    }

    string line;
    while (getline(in, line))
    {
		string s = line.substr(0, 2);
		if (s == "v ")
		{
			istringstream s(line.substr(2));
			glm::vec3 v;
			s >> v.x;
			s >> v.y;
			s >> v.z;
			vertices.push_back(v);
		}
		else if (s == "vt")
		{
			istringstream s(line.substr(3));
			glm::vec2 uv;
			s >> uv.x;
			s >> uv.y;
			uvs.push_back(uv);
		}
		else if (s == "vn")
		{
			istringstream s(line.substr(3));
			glm::vec3 n;
			s >> n.x;
			s >> n.y;
			s >> n.z;
			normals.push_back(n);
		}
		else if (s == "f ")
		{
			istringstream s(line.substr(2));
			string s1, s2, s3;
			s >> s1;
			s >> s2;
			s >> s3;
			unsigned int v1, v2, v3, uv1, uv2, uv3, n1, n2, n3;
			sscanf_s(s1.c_str(), "%d/%d/%d", &v1, &uv1, &n1);
			sscanf_s(s2.c_str(), "%d/%d/%d", &v2, &uv2, &n2);
			sscanf_s(s3.c_str(), "%d/%d/%d", &v3, &uv3, &n3);
			Vertex ve1 = { vertices[v1 - 1].x, vertices[v1 - 1].y, vertices[v1 - 1].z, uvs[uv1 - 1].x, -uvs[uv1 - 1].y, normals[n1 - 1].x, normals[n1 - 1].y, normals[n1 - 1].z };
			Vertex ve2 = { vertices[v2 - 1].x, vertices[v2 - 1].y, vertices[v2 - 1].z, uvs[uv2 - 1].x, -uvs[uv2 - 1].y, normals[n2 - 1].x, normals[n2 - 1].y, normals[n2 - 1].z };
			Vertex ve3 = { vertices[v3 - 1].x, vertices[v3 - 1].y, vertices[v3 - 1].z, uvs[uv3 - 1].x, -uvs[uv3 - 1].y, normals[n3 - 1].x, normals[n3 - 1].y, normals[n3 - 1].z };
			out.push_back(ve1);
			out.push_back(ve2);
			out.push_back(ve3);
		}
	}
	return out.size();
}

const GLchar** load_shader(const char* path)
{
	ifstream file(path, ios::in);
	string src;

	while (file.good())
	{
		string line;
		getline(file, line);
		src.append(line + "\n");
	}
;
	char* out = new char[src.length() + 1];
	strcpy_s(out, src.length() + 1, src.c_str());
	return (const GLchar**) & out;
}

// load points a and b into laser_vbo
void zap(const glm::vec3 a, const glm::vec3 b)
{
	Vertex v[2];
	v[0].x = a.x;
	v[0].y = a.y;
	v[0].z = a.z;
	v[1].x = b.x;
	v[1].y = b.y;
	v[1].z = b.z;
	glBindBuffer(GL_ARRAY_BUFFER, laser_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
}