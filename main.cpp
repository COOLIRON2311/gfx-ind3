#define GLM_SWIZZLE
#include "main.h"

void Init()
{
	// Point light
	pl.enabled = false;
	pl.pos = glm::vec3(-3.12f, 8.27f, -2.83f);
	pl.ambient = glm::vec3(0.1f);
	pl.diffuse = glm::vec3(0.8f);
	pl.specular = glm::vec3(0.8f);
	pl.atten = glm::vec3(0.5f);

	// Directional light
	dl.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	dl.ambient = glm::vec3(1.0f);
	dl.diffuse = glm::vec3(1.0f);
	dl.specular = glm::vec3(1.0f);

	// Spot light
	sl.enabled = false;
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

	// Laser
	projectile.ambient = glm::vec3(1.0f, 0.0f, 0.0f);
	projectile.diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
	projectile.specular = glm::vec3(1.0f, 0.0f, 0.0f);
	projectile.emission = glm::vec3(1.0f, 0.0f, 0.0f);
	projectile.shininess = 1.0f;

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

	laser_sfx.openFromFile("music/laser.ogg");
	bullet_sfx.openFromFile("music/bullet.ogg");
	bullet_fired = false;

	//Включаем проверку глубины
	glEnable(GL_DEPTH_TEST);
	// Инициализируем шейдеры
	InitShader();
	// Инициализируем вершинный буфер
	InitVBO();
	InitTextures();
}

int main()
{
	setlocale(LC_ALL, "Russian");
	srand(time(NULL));
	sf::Window window(sf::VideoMode(1000, 1000), "World of Tonks", sf::Style::Default, sf::ContextSettings(24));
	SetIcon(window);
	window.setVerticalSyncEnabled(true); // Вертикальная синхронизация
	window.setActive(true); // Устанавливаем контекст OpenGL
	glewInit(); // Инициализируем GLEW
	Init(); // Инициализируем ресурсы
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) // Если пользователь закрыл окно
			{
				window.close(); // Закрываем окно
				goto EXIT_IS_RIGHT_HERE; // Выходим из цикла
			}
			else if (event.type == sf::Event::Resized) // Если пользователь изменил размер окна
			{
				glViewport(0, 0, event.size.width, event.size.height); // Устанавливаем область вывода
			}
			else if (event.type == sf::Event::KeyPressed) // Если пользователь нажал клавишу
			{
				// Rotation
				if (event.key.code == sf::Keyboard::W)
				{
					cam.FWD(tonk->dir, tonk->speed);
					tonk->MoveForward();
					sl.pos = tonk->center;
					sl.direction = -tonk->dir;
				}
				if (event.key.code == sf::Keyboard::S)
				{
					cam.BWD(tonk->dir, tonk->speed);
					tonk->MoveBackward();
					sl.pos = tonk->center;
					sl.direction = -tonk->dir;
				}
				if (event.key.code == sf::Keyboard::D)
				{
					tonk->RotateRight();
					cam.RotRight(tonk->center, tonk->dir);
					sl.pos = tonk->center;
					sl.direction = -tonk->dir;
				}
				if (event.key.code == sf::Keyboard::A)
				{
					tonk->RotateLeft();
					cam.RotLeft(tonk->center, tonk->dir);
					sl.pos = tonk->center;
					sl.direction = -tonk->dir;
				}

				// Movement
				if (event.key.code == sf::Keyboard::Numpad8)
				{
					cam.W();
				}
				if (event.key.code == sf::Keyboard::Numpad2)
				{
					cam.S();
				}
				if (event.key.code == sf::Keyboard::Numpad6)
				{
					cam.D();
				}
				if (event.key.code == sf::Keyboard::Numpad4)
				{
					cam.A();
				}

				// Debug camera movement
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

				// Music
				if (event.key.code == sf::Keyboard::M)
				{
					music_playing = !music_playing;
					if (music_playing)
						bg.play();
					else
						bg.pause();
				}

				// Light
				if (event.key.code == sf::Keyboard::LAlt)
				{
					sl.enabled = !sl.enabled;
				}

				// Reset
				if (event.key.code == sf::Keyboard::Escape)
				{
					for (auto& t : enemy_tanks)
						t->hit = false;

					for (auto& b : barrels)
						b->hit = false;

					for (auto& t : trees)
						t->hit = false;

					for (auto& r : rocks)
						r->hit = false;
				}
			}
			// Weapons
			if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
			{
				// Laser
				if (laser_frames == 0)
				{
					laser_sfx.stop();
					laser_sfx.play();
					const int frames = 10;
					glm::vec3 p;

					// check if laser ray hit something
					for (auto& t : enemy_tanks)
					{
						if (!t->hit)
						{
							p = t->HitLaser(tonk->center, tonk->dir);
							if (p != tonk->center)
							{
								t->hit = true;
								goto zap_end;
							}
						}
					}

					for (auto& b : barrels)
					{
						if (!b->hit)
						{
							p = b->HitLaser(tonk->center, tonk->dir);
							if (p != tonk->center)
							{
								b->hit = true;
								goto zap_end;
							}
						}
					}

					for (auto& t : trees)
					{
						if (!t->hit)
						{
							p = t->HitLaser(tonk->center, tonk->dir);
							if (p != tonk->center)
							{
								t->hit = true;
								goto zap_end;
							}
						}
					}

					for (auto& r : rocks)
					{
						if (!r->hit)
						{
							p = r->HitLaser(tonk->center, tonk->dir);
							if (p != tonk->center)
							{
								r->hit = true;
								goto zap_end;
							}
						}
					}
					// if there is no hit, then p is point on same line as tonk->center and tonk->dir 1000 units away
					p = tonk->center + -tonk->dir.xyz() * 1000.0f;
				zap_end:
					laser_frames = frames;
					zap(tonk->center, p);
					// cout << endl;
				}
			}
			
			// Bullet
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (!bullet_fired)
				{
					bullet = objects[6].copy();
					bullet_sfx.stop();
					bullet_sfx.play();
					// Bullet
					pl.enabled = true;
					pl.pos = tonk->center;

					bullet_fired = true;
					bullet->dx = tonk->center.x;
					bullet->dz = tonk->center.z;
					bul_dir = -tonk->dir;
					bullet->Update();
				}
			}
		}
		
		if (bullet_fired)
		{
			if (glm::distance(bullet->center, tonk->center) > bul_max_dst)
			{
				bullet_fired = false;
				pl.enabled = false;
				bullet->destroy();
				delete bullet;
			}
			else
			{
				update_bullet();
				
				// check if bullet hit something
				for (auto& t : enemy_tanks)
				{
					if (!t->hit)
					{
						if (t->HitBullet(bullet->center))
						{
							t->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}

				for (auto& b : barrels)
				{
					if (!b->hit)
					{
						if (b->HitBullet(bullet->center))
						{
							b->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}

				for (auto& t : trees)
				{
					if (!t->hit)
					{
						if (t->HitBullet(bullet->center))
						{
							t->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}

				for (auto& r : rocks)
				{
					if (!r->hit)
					{
						if (r->HitBullet(bullet->center))
						{
							r->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очищаем буфер цвета и буфер глубины
		Draw(window); // Рисуем
		window.display(); // Выводим на экран
	}
EXIT_IS_RIGHT_HERE: // Метка выхода
	Release(); // Очищаем ресурсы
	return 0; // Выходим из программы
}

void LoadObject(int i, const char* path)
{
	GLuint id;
	glGenBuffers(1, &id); // Генерируем вершинный буфер
	vector<Vertex> data;
	load_obj(path, data);
	Object obj(id, data);
	objects.push_back(obj);
	checkOpenGLerror();
}

bool intersects(Object* obj)
{
	const float R = 3.0f;
	obj->Update();
	
	if (glm::distance(obj->center, tonk->center) < R)
	{
		return true;
	}

	if (glm::distance(obj->center, objects[2].center) < R)
	{
		return true;
	}
	
	for (auto& t : enemy_tanks)
	{
		if (glm::distance(obj->center, t->center) < R)
			return true;
	}

	for (auto& b : barrels)
	{
		if (glm::distance(obj->center,b->center) < R)
			return true;
	}

	for (auto& t : trees)
	{
		if (glm::distance(obj->center, t->center) < R)
			return true;
	}

	for (auto& r : rocks)
	{
		if (glm::distance(obj->center, r->center) < R)
			return true;
	}

	return false;
}

void InitVBO()
{
	glGenBuffers(1, &laser_vbo);
	LoadObject(0, "models/Field.obj");
	LoadObject(1, "models/Tank.obj");
	LoadObject(2, "models/ChrTree.obj");
	LoadObject(3, "models/Barrel.obj");
	LoadObject(4, "models/Tree.obj");
	LoadObject(5, "models/Stone.obj");
	LoadObject(6, "models/Bullet.obj");
	// Player tank
	tonk = new PlayerTank(objects[1]);
	sl.pos = tonk->center;
	sl.direction = -tonk->dir;

	// Christmas tree
	objects[2].dx = 10;
	objects[2].dz = 10;
	for (auto& obj : objects)
		obj.Update();

	// Enemy tanks
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[1].copy();
		do {
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->ry = rand() % 360;
		} while (intersects(o));
		enemy_tanks.push_back(o);
	}

	// Barrels
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[3].copy();
		do {
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->ry = rand() % 360;
		} while (intersects(o));
		barrels.push_back(o);
	}

	// Trees
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[4].copy();
		do {
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->ry = rand() % 360;
		} while (intersects(o));
		trees.push_back(o);
	}

	// Rocks
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[5].copy();
		do {
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->ry = rand() % 360;
		} while (intersects(o));
		rocks.push_back(o);
	}
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
	LoadTexture(GL_TEXTURE7, textures[7], "textures/laser.jpg");
	LoadTexture(GL_TEXTURE8, textures[8], "textures/bullet.jpg");
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
	glGenTextures(1, &tex); // Генерируем текстуру
	glActiveTexture(tex_enum);
	glBindTexture(GL_TEXTURE_2D, tex); // Привязываем текстуру
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Устанавливаем параметры текстуры
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

	// Создаем шейдерную программу
	Programs[0] = glCreateProgram(); // Phong


	// Прикрепляем шейдеры к программе
	glAttachShader(Programs[0], PhongVShader);
	glAttachShader(Programs[0], PhongFShader);

	// Линкуем шейдерную программу
	glLinkProgram(Programs[0]);

	int link1;
	glGetProgramiv(Programs[0], GL_LINK_STATUS, &link1);

	// Проверяем на ошибки
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
	glUseProgram(0); // Отключаем шейдерную программу

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
	glUseProgram(0); // Отключаем шейдерную программу

	// Laser
	if (laser_frames > 0)
	{
		laser_frames--;
		glUseProgram(Programs[0]);
		tex_loc = glGetUniformLocation(Programs[0], "tex");
		pl.Load(Programs[0]);
		dl.Load(Programs[0]);
		sl.Load(Programs[0]);
		projectile.Load(Programs[0]);
		glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
		glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
		glUniform1i(tex_loc, 7);
		glEnableVertexAttribArray(Phong_coord);
		glEnableVertexAttribArray(Phong_texcoord);
		glEnableVertexAttribArray(Phong_normal);
		glBindBuffer(GL_ARRAY_BUFFER, laser_vbo);
		glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(Phong_coord);
		glDisableVertexAttribArray(Phong_texcoord);
		glDisableVertexAttribArray(Phong_normal);
		glUseProgram(0); // Отключаем шейдерную программу
	}

	// Bullet
	if (bullet_fired)
	{
		glUseProgram(Programs[0]);
		tex_loc = glGetUniformLocation(Programs[0], "tex");
		pl.Load(Programs[0]);
		dl.Load(Programs[0]);
		sl.Load(Programs[0]);
		projectile.Load(Programs[0]);
		glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
		glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
		glUniform1i(tex_loc, 8);
		glEnableVertexAttribArray(Phong_coord);
		glEnableVertexAttribArray(Phong_texcoord);
		glEnableVertexAttribArray(Phong_normal);
		glBindBuffer(GL_ARRAY_BUFFER, bullet->id);
		glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, bullet->size());
		glDisableVertexAttribArray(Phong_coord);
		glDisableVertexAttribArray(Phong_texcoord);
		glDisableVertexAttribArray(Phong_normal);
		glUseProgram(0); // Отключаем шейдерную программу
	}

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
	glUseProgram(0); // Отключаем шейдерную программу


	// Enemy tanks
	for (int i = 0; i < enemy_tanks.size(); i++)
	{
		if (!enemy_tanks[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 6);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, enemy_tanks[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, enemy_tanks[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0); // Отключаем шейдерную программу
		}
	}

	// Barrels
	for (int i = 0; i < barrels.size(); i++)
	{
		if (!barrels[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 3);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, barrels[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, barrels[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0); // Отключаем шейдерную программу
		}
	}

	// Trees
	for (int i = 0; i < trees.size(); i++)
	{
		if (!trees[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 4);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, trees[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, trees[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0); // Отключаем шейдерную программу
		}
	}

	// Rocks
	for (int i = 0; i < rocks.size(); i++)
	{
		if (!rocks[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.MVP()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 5);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, rocks[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, rocks[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0); // Отключаем шейдерную программу
		}
	}


	checkOpenGLerror(); // Проверяем на ошибки
}

void Release()
{
	ReleaseShader(); // Очищаем шейдеры
	ReleaseVBO(); // Очищаем буфер
}

void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Отвязываем буфер
	for (int i = 0; i < objects.size(); i++)
	{
		glDeleteBuffers(1, &objects[i].id); // Удаляем буфер
	}
	for (int i = 0; i < barrels.size(); i++)
	{
		barrels[i]->destroy();
		delete barrels[i];
	}
	for (int i = 0; i < trees.size(); i++)
	{
		trees[i]->destroy();
		delete trees[i];
	}
	for (int i = 0; i < rocks.size(); i++)
	{
		rocks[i]->destroy();
		delete rocks[i];
	}
	for (int i = 0; i < enemy_tanks.size(); i++)
	{
		enemy_tanks[i]->destroy();
		delete enemy_tanks[i];
	}
}

void ReleaseShader()
{
	glUseProgram(0); // Отключаем шейдерную программу
	for (int i = 0; i < Programs.size(); i++)
	{
		glDeleteProgram(Programs[i]); // Удаляем шейдерную программу
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
