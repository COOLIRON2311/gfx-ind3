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

ostream& operator<<(ostream& os, const glm::vec3 v)
{
	return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

// ������� ��� �������� ������
void checkOpenGLerror();

class Object
{
protected:
	const float RL = 1.0f; // laser collider radius
	const float RB = 1.0f; // bullet collider radius
	void ApplyTransform()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
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
			
			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
		center = glm::vec3(x, y, z) / (float)size();
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
	}

	size_t size()
	{
		return vertices.size();
	}

	Object* copy()
	{
		GLuint new_id;
		glGenBuffers(1, &new_id); // ���������� ��������� �����
		return new Object(new_id, vertices, dx, dz, ry);
	}

	void Update()
	{
		ApplyTransform();
	}

	bool HitBullet(const glm::vec3& bulletPos)
	{
		if (glm::distance(center, bulletPos) < RB)
			return true;
		return false;
	}

	glm::vec3 HitLaser(const glm::vec3 pos, const glm::vec3 d)
	{
		static const float inf = 1e10;
		glm::vec3 T = pos - center; // ������ �� ������ ������� �� ����� ��������� ������
		float A = 1.0f; // ����������� A ������������� ���������
		float B = 2.0f * glm::dot(T, d); // ����������� B ������������� ���������
		float C = glm::dot(T, T) - RL * RL; // ����������� C ������������� ���������
		float delta = B * B - 4.0f * A * C; // ������������ ������������� ���������
		float dist = inf; // ���������� �� ����� ��������� ������
		glm::vec3 hit_pos = glm::vec3(0.0f);
		if (delta > -1e-4) // ���� ������������ ������ ����, �� ���� ��� �����
		{
			delta = max(0.0f, delta); // ����������� �� ��������� ����
			float sdelta = sqrt(delta); // ������ �� �������������
			float ratio = 0.5f / A; // ����������� ��� ���������� ������
			float ret1 = ratio * (-B - sdelta); // ������ ������
			dist = ret1; // �������� ������ ������
			if (dist < inf) // ���� ������ ������ ���������
			{
				float old_dist = dist; // ��������� ������ ����������
				glm::vec3 new_pos = pos + d * dist; // ��������� ����� �������
				T = new_pos - center; // ��������� ����� ������ �� ������ ������� �� ����� ��������� ������
				A = 1.0f; // ����������� A ������������� ���������
				B = 2.0f * glm::dot(T, d); // ����������� B ������������� ���������
				C = glm::dot(T, T) - RL * RL; // ����������� C ������������� ���������
				delta = B * B - 4.0f * A * C; // ������������ ������������� ���������
				if (delta > 0) // ���� ������������ ������ ����, �� ���� ��� �����
				{
					sdelta = sqrt(delta); // ������ �� �������������
					ratio = 0.5f / A; // ����������� ��� ���������� ������
					ret1 = ratio * (-B - sdelta); // ������ ������
					if (ret1 > 0.0f) // ���� ������ ������ ���������
					{
						dist = ret1; // �������� ������ ������
						hit_pos = new_pos + ratio * (-B - sdelta) * d; // ��������� ����� ��������� ������
					}
				}
				else
				{
					dist = inf; // ���� ������������ ������ ����, �� ��� ������
				}

			}
		}
		// cout << "pos: " << pos << ", d: " << d << ", dist: " << dist << ", hit_pos: " << hit_pos << endl;
		if (dist < inf)
		{
			if (glm::dot(hit_pos - pos, -d) < 0.0f)
			{
				return pos;
			}
			else
				return hit_pos; // ���������� ����� ��������� ������
		}
		else
			return pos; // ���� ��� �� ����� � ������, �� ���������� �������� �������
	}

	void destroy()
	{
		glDeleteBuffers(1, &id);
	}
};


class PlayerTank
{
	void setCenter()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		for (int i = 0; i < size(); i++)
		{
			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		center = glm::vec3(x, y, z) / (float)size();
	}

public:
	float dx;
	float dz;
	float ry;
	GLuint id;
	glm::vec4 dir;
	glm::vec3 center;
	vector<Vertex> vertices;
	const float speed = 0.2f;
	const float rotspeed = 2.0f;
	
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
	}

	void MoveBackward()
	{
		dz = 1.0f;
		Move();
	}

	void RotateLeft()
	{
		ry = rotspeed;
		Rotate();
	}

	void RotateRight()
	{
		ry = -rotspeed;
		Rotate();
	}

	void Move()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(speed * dir.x * dz, 0.0f, speed * dir.z * dz));
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
			v = model * v;
			vertices[i].x = v.x;
			vertices[i].y = v.y;
			vertices[i].z = v.z;

			x += vertices[i].x;
			y += vertices[i].y;
			z += vertices[i].z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
		center = glm::vec3(x, y, z) / (float)size();
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
