#pragma once
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex
{
	//coords
	GLfloat x;
	GLfloat y;
	GLfloat z;

	// texture coords
	GLfloat s;
	GLfloat t;

	//normals
	GLfloat nx;
	GLfloat ny;
	GLfloat nz;
};

// Функция для проверки ошибок
void checkOpenGLerror();

class Object
{
protected:
	const float R = 3.0f;
	void ApplyTransform()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(dx, 0.0f, dz));
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
			v = model * v;
			vertices[i].x = v.x;
			vertices[i].y = v.y;
			vertices[i].z = v.z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
	}

	void setCenter()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		for (int i = 0; i < vertices.size(); i++)
		{
			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		x /= vertices.size();
		y /= vertices.size();
		z /= vertices.size();
		center = glm::vec3(x, y, z);
	}

public:
	float dx;
	float dz;
	float ry;
	bool hit;
	GLuint id;
	glm::vec3 center;
	vector<Vertex> vertices;

	Object(GLuint id, const vector<Vertex>& vertices, float dx = 0.0f, float dz = 0.0f, float ry = 0.0f)
	{
		this->id = id;
		this->dx = dx;
		this->dz = dz;
		this->ry = ry;
		hit = false;
		this->vertices = vertices;
		ApplyTransform();
		setCenter();
	}

	size_t size()
	{
		return vertices.size();
	}

	Object* copy()
	{
		GLuint new_id;
		glGenBuffers(1, &new_id); // Генерируем вершинный буфер
		return new Object(new_id, vertices, dx, dz, ry);
	}

	void Update()
	{
		ApplyTransform();
		setCenter();
	}

	void HitBullet(const glm::vec3& bulletPos)
	{
		if (glm::distance(center, bulletPos) < R)
		{
			hit = true;
		}
	}

	glm::vec3 HitLaser(const glm::vec3 pos, const glm::vec3 dir)
	{
		glm::vec3 n = glm::normalize(dir);
		glm::vec3 p = pos;
		glm::vec3 c = center;
		float d = glm::distance(p, c);
		float t = glm::dot(c - p, n);
		glm::vec3 q = p + t * n;
		if (glm::distance(q, c) < R)
		{
			hit = true;
			return q;
		}
		// if there is no hit return ray origin
		return pos;
	}

	void destroy()
	{
		glDeleteBuffers(1, &id);
	}
};


class PlayerTank
{
	const GLfloat speed = 0.1f;
	void setCenter()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		for (int i = 0; i < vertices.size(); i++)
		{
			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		x /= vertices.size();
		y /= vertices.size();
		z /= vertices.size();
		center = glm::vec3(x, y, z);
	}

public:
	float dx;
	float dz;
	float ry;
	GLuint id;
	glm::vec4 dir;
	glm::vec3 center;
	vector<Vertex> vertices;
	
	PlayerTank(Object& obj)
	{
		dx = 0.0f;
		dz = 0.0f;
		ry = 0.0f;
		id = obj.id;
		vertices = obj.vertices;
		dir = { 0.0f, 0.0f, 1.0f, 0.0f};
		setCenter();
	}

	size_t size()
	{
		return vertices.size();
	}

	void MoveForward()
	{
		dz = -1.0f;
		Move();
		setCenter();
	}

	void MoveBackward()
	{
		dz = 1.0f;
		Move();
		setCenter();
	}

	void RotateLeft()
	{
		ry = 1.0f;
		Rotate();
	}

	void RotateRight()
	{
		ry = -1.0f;
		Rotate();
	}

	void Move()
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(speed * dir.x * dz, 0.0f, speed * dir.z * dz));
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
			v = model * v;
			vertices[i].x = v.x;
			vertices[i].y = v.y;
			vertices[i].z = v.z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
	}
	
	void Rotate()
	{
		glm::vec3 pos = center;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(pos.x, 0.0f, pos.z));
		model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-pos.x, 0.0f, -pos.z));
		dir = model * dir;
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
			v = model * v;
			vertices[i].x = v.x;
			vertices[i].y = v.y;
			vertices[i].z = v.z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
	}
};
