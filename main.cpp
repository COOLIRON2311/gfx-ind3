#include "main.h"

void Init()
{
	// Point light
	pl.pos = glm::vec3(-3.12f, 8.27f, -2.83f);
	pl.ambient = glm::vec3(0.1f);
	pl.diffuse = glm::vec3(1.0f);
	pl.specular = glm::vec3(1.0f);
	pl.atten = glm::vec3(0.2f);

	// Directional light
	dl.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	dl.ambient = glm::vec3(0.5f);
	dl.diffuse = glm::vec3(0.5f);
	dl.specular = glm::vec3(0.5f);

	// Spot light
	sl.pos = glm::vec3(-5.0f, -8.37f, -5.0f);
	sl.direction = glm::vec3(1.0f);
	sl.ambient = glm::vec3(1.0f);
	sl.diffuse = glm::vec3(1.0f);
	sl.specular = glm::vec3(1.0f);
	sl.cutoff = 12.5f;
	sl.atten = glm::vec3(0.1f, 0.1f, 0.1f);

	// Material
	mat.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.emission = glm::vec3(0.0f, 0.0f, 0.0f);
	mat.shininess = 1.0f;

	if (!bg.openFromFile("music/big_rock.ogg"))
	{
		music_playing = false;
		cerr << "Can't load music" << endl;
	}
	else
	{
		music_playing = false; // true
		bg.setLoop(true);
		// bg.play();
	}

	//�������� �������� �������
	glEnable(GL_DEPTH_TEST);
	// �������������� �������
	InitShader();
	// �������������� ��������� �����
	InitVBO();
	InitTextures();
}

int main()
{
	setlocale(LC_ALL, "Russian");
	sf::Window window(sf::VideoMode(1000, 1000), "World of Tonks", sf::Style::Default, sf::ContextSettings(24));
	SetIcon(window);
	window.setVerticalSyncEnabled(true); // ������������ �������������
	window.setActive(true); // ������������� �������� OpenGL
	glewInit(); // �������������� GLEW
	Init(); // �������������� �������
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) // ���� ������������ ������ ����
			{
				window.close(); // ��������� ����
				goto EXIT_IS_RIGHT_HERE; // ������� �� �����
			}
			else if (event.type == sf::Event::Resized) // ���� ������������ ������� ������ ����
			{
				glViewport(0, 0, event.size.width, event.size.height); // ������������� ������� ������
			}
			else if (event.type == sf::Event::KeyPressed) // ���� ������������ ����� �������
			{
				// Rotation
				if (event.key.code == sf::Keyboard::Up)
				{
					cam.FWD(tonk->dir);
					tonk->MoveForward();
				}
				if (event.key.code == sf::Keyboard::Down)
				{
					cam.BWD(tonk->dir);
					tonk->MoveBackward();
				}
				if (event.key.code == sf::Keyboard::Right)
				{
					tonk->RotateRight();
					cam.RotRight(tonk->center, tonk->dir);
				}
				if (event.key.code == sf::Keyboard::Left)
				{
					tonk->RotateLeft();
					cam.RotLeft(tonk->center, tonk->dir);
				}

				// Movement
				if (event.key.code == sf::Keyboard::W)
				{
					cam.W();
				}
				if (event.key.code == sf::Keyboard::S)
				{
					cam.S();
				}
				if (event.key.code == sf::Keyboard::A)
				{
					cam.A();
				}
				if (event.key.code == sf::Keyboard::D)
				{
					cam.D();
				}

				if (event.key.code == sf::Keyboard::F1)
				{
					cam.Perspective();
				}

				if (event.key.code == sf::Keyboard::F2)
				{
					cam.Ortho();
				}

				if (event.key.code == sf::Keyboard::Escape)
				{
					cam.Reset();
				}

				if (event.key.code == sf::Keyboard::Num1)
				{
					pl.Config();
				}
				if (event.key.code == sf::Keyboard::Num2)
				{
					dl.Config();
				}
				if (event.key.code == sf::Keyboard::Num3)
				{
					sl.Config();
				}
				if (event.key.code == sf::Keyboard::Space)
				{
					music_playing = !music_playing;
					if (music_playing)
						bg.play();
					else
						bg.pause();
				}
			}

		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ������� ����� ����� � ����� �������
		Draw(window); // ������
		window.display(); // ������� �� �����
	}
EXIT_IS_RIGHT_HERE: // ����� ������
	Release(); // ������� �������
	return 0; // ������� �� ���������
}

void LoadObject(int i, const char* path)
{
	GLuint id;
	glGenBuffers(1, &id); // ���������� ��������� �����
	vector<Vertex> data;
	load_obj(path, data);
	Object obj(id, data);
	objects.push_back(obj);
	checkOpenGLerror();
}

void InitVBO()
{
	LoadObject(0, "models/Field.obj");
	LoadObject(1, "models/Tank.obj");
	LoadObject(2, "models/ChrTree.obj");
	LoadObject(3, "models/Barrel.obj");
	LoadObject(4, "models/Tree.obj");
	LoadObject(5, "models/Stone.obj");
	// Player tank
	tonk = new PlayerTank(objects[1]);
	//tonk->ry = -90;
	// Christmas tree
	objects[2].dx = 10;
	objects[2].dz = 10;
	for (auto& obj : objects)
		obj.Update();
}

void InitTextures()
{
	LoadTexture(GL_TEXTURE0, textures[0], "textures/Field.png");
	LoadTexture(GL_TEXTURE1, textures[1], "textures/Tank.png");
	LoadTexture(GL_TEXTURE2, textures[2], "textures/ChrTree.png");
	LoadTexture(GL_TEXTURE3, textures[3], "textures/Barrel.png");
	LoadTexture(GL_TEXTURE4, textures[4], "textures/Tree.png");
	LoadTexture(GL_TEXTURE5, textures[5], "textures/Stone.png");
	LoadTexture(GL_TEXTURE6, textures[6], "textures/EnTank.png");
}

void LoadAttrib(GLuint prog, GLint& attrib, const char* attr_name)
{
	attrib = glGetAttribLocation(prog, attr_name);
	if (attrib == -1)
	{
		std::cout << "could not bind attrib " << attr_name << std::endl;
		return;
	}
}

void LoadUniform(GLuint prog, GLint& attrib, const char* attr_name)
{
	attrib = glGetUniformLocation(prog, attr_name);
	if (attrib == -1)
	{
		std::cout << "could not bind uniform " << attr_name << std::endl;
		return;
	}
}

void LoadTexture(GLenum tex_enum, GLuint& tex, const char* path)
{
	glGenTextures(1, &tex); // ���������� ��������
	glActiveTexture(tex_enum);
	glBindTexture(GL_TEXTURE_2D, tex); // ����������� ��������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // ������������� ��������� ��������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sf::Image img;
	if (!img.loadFromFile(path))
	{
		std::cout << "could not load texture " << path << std::endl;
		return;
	}

	sf::Vector2u size = img.getSize();
	int width = size.x;
	int height = size.y;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void InitShader()
{
	GLuint PhongVShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PhongVShader, 1, load_shader("shaders/phong.vert"), NULL);
	glCompileShader(PhongVShader);
	std::cout << "phong vertex shader \n";
	ShaderLog(PhongVShader);

	GLuint PhongFShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PhongFShader, 1, load_shader("shaders/phong.frag"), NULL);
	glCompileShader(PhongFShader);
	std::cout << "phong fragment shader \n";
	ShaderLog(PhongFShader);

	// ������� ��������� ���������
	Programs[0] = glCreateProgram(); // Phong


	// ����������� ������� � ���������
	glAttachShader(Programs[0], PhongVShader);
	glAttachShader(Programs[0], PhongFShader);

	// ������� ��������� ���������
	glLinkProgram(Programs[0]);
	
	int link1;
	glGetProgramiv(Programs[0], GL_LINK_STATUS, &link1);

	// ��������� �� ������
	if (!link1)
	{
		std::cout << "could not link shader program" << std::endl;
		return;
	}

	LoadAttrib(Programs[0], Phong_coord, "coord");
	LoadAttrib(Programs[0], Phong_texcoord, "texcoord");
	LoadAttrib(Programs[0], Phong_normal, "normal");
	LoadUniform(Programs[0], Phong_mvp, "mvp");
	LoadUniform(Programs[0], Phong_viewPos, "viewPos");

	checkOpenGLerror();
}

void Draw(sf::Window& window)
{
	GLuint tex_loc;
	// Field
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 0);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, objects[0].id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, objects[0].size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0); // ��������� ��������� ���������

	// Tonk
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 1);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, tonk->id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, tonk->size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0); // ��������� ��������� ���������
	
	// Christmas tree
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 2);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, objects[2].id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, objects[2].size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0); // ��������� ��������� ���������
	

	checkOpenGLerror(); // ��������� �� ������
}

void Release()
{
	ReleaseShader(); // ������� �������
	ReleaseVBO(); // ������� �����
}

void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0); // ���������� �����
	for (int i = 0; i < objects.size(); i++)
	{
		glDeleteBuffers(1, &objects[i].id); // ������� �����
	}
}

void ReleaseShader()
{
	glUseProgram(0); // ��������� ��������� ���������
	for (int i = 0; i < Programs.size(); i++)
	{
		glDeleteProgram(Programs[i]); // ������� ��������� ���������
	}
}

void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
		std::cout << "InfoLog: " << infoLog.data() << std::endl;
		exit(1);
	}
}

void checkOpenGLerror()
{
	GLenum errCode;
	const GLubyte* errString;
	if ((errCode = glGetError()) != GL_NO_ERROR)
	{
		errString = gluErrorString(errCode);
		std::cout << "OpenGL error: (" << errCode << ") " << errString << std::endl;
	}
}

void SetIcon(sf::Window& wnd)
{
	sf::Image image;
	if (!image.loadFromFile("icon.png"))
	{
		std::cout << "error load icon \n";
		return;
	}

	wnd.setIcon(image.getSize().x, image.getSize().y, image.getPixelsPtr());
}
