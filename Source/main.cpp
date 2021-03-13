#include "../Include/Common.h"

//For GLUT to handle 
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3

#ifndef max
# define max(a,b) (((a)>(b))?(a):(b))
# define min(a,b) (((a)<(b))?(a):(b))
#endif

#define deg2rad(x) ((x)*((3.1415926f)/(180.0f)))

using namespace glm;
using namespace std;

// Default window size
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
// current window size
int screenWidth = WINDOW_WIDTH, screenHeight = WINDOW_HEIGHT;

vector<string> filenames; // .obj filename list

float timer_cnt = 0;
//bool timer_enabled = true;
unsigned int timer_speed = 16;

typedef struct
{
	GLuint diffuseTexture;
} PhongMaterial;

typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object

	int materialId;
	int vertexCount;
	GLuint m_texture;
} Shape;

struct model
{
	vec3 position = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);
	vec3 rotation = vec3(0, 0, 0);	// Euler form

	vector<Shape> shapes;
};
vector<model> models;

struct camera
{
	vec3 position;
	vec3 center;
	vec3 up_vector;
};
camera main_camera;

struct project_setting
{
	GLfloat nearClip, farClip;
	GLfloat fovy;
	GLfloat aspect; //µøµ¡ªø¼e¤ñ
};
project_setting proj;

Shape m_shape_list[10];

int cur_idx = 0; // represent which model should be rendered now
int move_x = 0;
int move_y = 0;
int move_z = 0;
int change_dir = 1;
bool mousePressL = false;
bool mousePressR = false;
bool mousePressU = false;
bool mousePressD = false;
int reflect_x = 1;
int reflect_y = 1;
int reflect_z = 1;
int camera_x = 0;
int camera_y = 1;
bool pause = false;
int pauseFlag = 0;
int leftArm = 1;
int rightArm = 1;
int leftLeg = 1;
int rightLeg = 1;
bool rotate_flag = false;
bool time_flag = false;
bool rotate90_flag = false;
vec3 rotate_axis;
GLfloat head_y = 2.7f;
GLfloat setTime = 0.0f;
GLfloat pauseTime = 0;
GLfloat stopTime = 0;
GLfloat body_degree = 0;
GLfloat armBaseL_degree;
GLfloat armL1_degree;
GLfloat armL2_degree;
GLfloat armBaseR_degree;
GLfloat armR1_degree;
GLfloat armR2_degree;
GLfloat legL1_degree;
GLfloat legL2_degree;
GLfloat legR1_degree;
GLfloat legR2_degree;
GLfloat legcL_degree;
GLfloat legfL_degree;
GLfloat legcR_degree;
GLfloat legfR_degree;
GLfloat lastTime = 0;
GLfloat num = 0;
GLuint body_tex;

vector<string> model_list{ "Capsule.obj", "Cube.obj","Cylinder.obj","Plane.obj", "Sphere.obj" };

GLuint program;

GLuint iLocP;
GLuint iLocV;
GLuint iLocM;
GLuint cur_color;
GLuint texture_color;
GLuint Tex_material;

void initParameter()
{
	proj.nearClip = 0.001;
	proj.farClip = 1000.0;
	proj.fovy = 100;
	proj.aspect = (float)(WINDOW_WIDTH) / (float)WINDOW_HEIGHT; // adjust width for side by side view

	main_camera.position = vec3(0.0f, 0.1f, 3.0f);
	main_camera.center = vec3(0.0f, 0.0f, 1.0f);
	main_camera.up_vector = vec3(0.0f, 1.0f, 0.0f);
}

// Load shader file to program
char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

// Free shader file
void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

static string GetBaseDir(const string& filepath) {
	if (filepath.find_last_of("/\\") != std::string::npos)
		return filepath.substr(0, filepath.find_last_of("/\\"));
	return "";
}

// Load .obj model
void My_LoadModels(string model_path, int i)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, model_path.c_str());
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
}
	if (!ret) {
		exit(1);
	}

	cout << i << endl;

	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			m_shape_list[i].vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &m_shape_list[i].vao);
	glBindVertexArray(m_shape_list[i].vao);

	glGenBuffers(1, &m_shape_list[i].vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape_list[i].vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();


	cout << "Load " << m_shape_list[i].vertexCount << " vertices" << endl;

	if (i == 2) {
		texture_data tdata = loadImg("universe.jpg");

		glGenTextures(1, &m_shape_list[i].m_texture);
		glBindTexture(GL_TEXTURE_2D, m_shape_list[i].m_texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete tdata.data;
	}
}

// OpenGL initialization
void My_Init()
{
	glClearColor(0.5f, 0.3f, 0.7f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	initParameter();

	// Create Shader Program
	program = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	// Compile these shaders
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Logging
	shaderLog(vertexShader);
	shaderLog(fragmentShader);

	// Assign the program we created before with these shaders
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Get the id of inner variable 'um4p' and 'um4mv' in shader programs
	iLocP = glGetUniformLocation(program, "um4p");
	iLocV = glGetUniformLocation(program, "um4v");
	iLocM = glGetUniformLocation(program, "um4m");
	cur_color = glGetUniformLocation(program, "part_color");
	texture_color = glGetUniformLocation(program, "tex_color");

	// Tell OpenGL to use this shader program now
	glUseProgram(program);

	int i = 0;
	for (string model_path : model_list) {
		My_LoadModels(model_path, i);
		i++;
	}
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
	// Bind a vertex array for OpenGL (OpenGL will apply operation only to the vertex array objects it bind)
	//glBindVertexArray(m_shape.vao);

	// clear canvas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(program);

	//    mat4 T_head, T_body, T_arm_connect, T_arm_base, R_head, R_body, R_arm_connect, R_arm_base, S_body, S_head, S_arm_connect, S_arm_base;
	vec4 model_color;
	vec4 texture;
	texture = vec4(1.0, 1.0, 1.0, 1.0);
	GLfloat degree;

	mat4 project_matrix;
	// perspective(fov, aspect_ratio, near_plane_distance, far_plane_distance)
	// ps. fov = field of view, it represent how much range(degree) is this camera could see
	project_matrix = perspective(deg2rad(proj.fovy), proj.aspect, proj.nearClip, proj.farClip);

	mat4 view_matrix;
	// lookAt(camera_position, camera_viewing_vector, up_vector)
	// up_vector represent the vector which define the direction of 'up'
	view_matrix = lookAt(main_camera.position, main_camera.center, main_camera.up_vector);

	// // /// /// /// // // // // / /// // // //// // // // // / /  body model
	mat4 T_body, R_body, S_body, T_move, Reflect, R_body90;

	//models[2].position = vec3(0, 0, -3);

	T_body = translate(mat4(1.0), vec3(0, 0, -3));
	T_move = translate(mat4(1.0), vec3(move_x, move_y, move_z));
	Reflect = scale(mat4(1.0), vec3(reflect_x, reflect_y, reflect_z));
	S_body = scale(mat4(1.0), vec3(2.7, 1.7, 2.7));
	//Build rotation matrix
//    cout << lastTime << endl;
	if (pauseTime != 0) {
		lastTime = timer_cnt - pauseTime;
		pauseTime = 0;
	}
	else if (time_flag) {
		lastTime = timer_cnt - stopTime;
		stopTime = 0;
		time_flag = false;
	}

	if (rotate_flag)
		body_degree = timer_cnt - lastTime;
	//    lastTime = 0;
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_body = rotate(mat4(1.0), body_degree, rotate_axis);
	vec3 rotate90_axis;
	rotate90_axis = vec3(0.0, 0.0, 1.0);
	GLfloat rotate_deg = radians(90.0f);
	R_body90 = rotate(mat4(1.0), rotate_deg, rotate90_axis);
	mat4 model_matrix_body;
	mat4 model_tmp_body;
	if (rotate90_flag) {
		// render object
		model_tmp_body = T_body * T_move * Reflect * R_body90 * R_body;
		model_matrix_body = model_tmp_body * S_body;
	}
	else {
		// render object
		model_tmp_body = T_body * T_move * Reflect * R_body;
		model_matrix_body = model_tmp_body * S_body;
	}


	// body model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_body));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.5, 0.8, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));
	glUniform4fv(texture_color, 1, value_ptr(texture));

	glBindVertexArray(m_shape_list[2].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[2].vertexCount);

	texture = vec4(0.0, 0.0, 0.0, 0.0);
	// // /// /// /// // // // // / /// // // //// // // // // / / head model
	mat4 T_head, R_head, S_head;
	//models[1].position = vec3(0, head_y, 0);
	//    cout << setTime << endl;
	if (setTime > 1.2) {
		if (head_y < 3.5)
			head_y += 0.15;
		else
			head_y -= 0.6;
		setTime = 0.0f;
	}
	else {
		setTime += 0.3;
	}
	T_head = translate(mat4(1.0), vec3(0, head_y, 0));
	S_head = scale(mat4(1.0), vec3(1.6, 1.6, 1.6));
	//Build rotation matrix
	degree = 0;
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_head = rotate(mat4(1.0), degree, rotate_axis);

	// render object
	mat4 model_tmp_head = R_head * T_head;
	mat4 model_matrix_head = model_tmp_body * model_tmp_head * S_head;

	// head model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_head));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.5, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));
	glUniform4fv(texture_color, 1, value_ptr(texture));

	glBindVertexArray(m_shape_list[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[1].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left eye model
	mat4 T_eyeL, R_eyeL, S_eyeL;
	//models[4].position = vec3(-0.45, 0.2, 0.8);
	T_eyeL = translate(mat4(1.0), vec3(-0.4, 0.1, 0.8));
	S_eyeL = scale(mat4(1.0), vec3(0.4, 0.4, 0.4));
	//Build rotation matrix
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_eyeL = rotate(mat4(1.0), degree, rotate_axis);

	// render object
	mat4 model_tmp_eyeL = R_eyeL * T_eyeL;
	mat4 model_matrix_eyeL = model_tmp_body * model_tmp_head * model_tmp_eyeL * S_eyeL;

	// eye model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_eyeL));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.8, 0.8, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right eye model
	mat4 T_eyeR, R_eyeR, S_eyeR;
	//models[4].position = vec3(0.45, 0.2, 0.8);
	T_eyeR = translate(mat4(1.0), vec3(0.4, 0.1, 0.8));
	S_eyeR = scale(mat4(1.0), vec3(0.4, 0.4, 0.4));
	//Build rotation matrix
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_eyeR = rotate(mat4(1.0), degree, rotate_axis);

	// render object
	mat4 model_tmp_eyeR = R_eyeR * T_eyeR;
	mat4 model_matrix_eyeR = model_tmp_body * model_tmp_head * model_tmp_eyeR * S_eyeR;

	// eye model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_eyeR));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.8, 0.8, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left arm base
	mat4 T_arm_base, R_arm_base, S_arm_base;
	//models[1].position = vec3(-1.75, 1.36, 0);
	T_arm_base = translate(mat4(1.0), vec3(-1.75, 1.36, 0));
	S_arm_base = scale(mat4(1.0), vec3(2.2, 0.7, 1.5));
	//Build rotation matrix
	if (armBaseL_degree > 0.1) {
		leftArm = 1;
	}
	else if (armBaseL_degree < -0.2) {
		leftArm = 0;
	}
	if (leftArm)
		armBaseL_degree -= 0.004;
	else
		armBaseL_degree += 0.004;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_arm_base = rotate(mat4(1.0), armBaseL_degree, rotate_axis);

	mat4 model_tmp_arm_base = R_arm_base * T_arm_base;
	mat4 model_matrix_arm_base = model_tmp_body * model_tmp_arm_base * S_arm_base;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_base));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.1, 0.5, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[1].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left arm connect

	mat4 T_arm_connect, R_arm_connect, S_arm_connect;
	//models[4].position = vec3(-0.4, -0.5, 0);
	T_arm_connect = translate(mat4(1.0), vec3(-0.4, -0.5, 0));
	S_arm_connect = scale(mat4(1.0), vec3(0.5, 0.5, 0.5));
	//Build rotation matrix
	degree = 0;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_arm_connect = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_arm_connect = R_arm_connect * T_arm_connect;
	mat4 model_matrix_arm_connect = model_tmp_body * model_tmp_arm_base * model_tmp_arm_connect * S_arm_connect;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_connect));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left arm1

	mat4 T_arm1, R_arm1, S_arm1;
	//models[0].position = vec3(0, -0.8, 0);
	T_arm1 = translate(mat4(1.0), vec3(0, -0.8, 0));
	S_arm1 = scale(mat4(1.0), vec3(0.6, 0.7, 0.7));
	//Build rotation matrix
	if (leftArm)
		armL1_degree -= 0.015;
	else
		armL1_degree += 0.015;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_arm1 = rotate(mat4(1.0), armL1_degree, rotate_axis);

	mat4 model_tmp_arm1 = R_arm1 * T_arm1;
	mat4 model_matrix_arm1 = model_tmp_body * model_tmp_arm_base * model_tmp_arm_connect * model_tmp_arm1 * S_arm1;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm1));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(1, 1, 1, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[0].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left arm connect2

	mat4 T_arm_connect2, R_arm_connect2, S_arm_connect2;
	//models[4].position = vec3(0, -0.75, 0);
	T_arm_connect2 = translate(mat4(1.0), vec3(0, -0.75, 0));
	S_arm_connect2 = scale(mat4(1.0), vec3(0.5, 0.5, 0.5));
	//Build rotation matrix
	degree = 0;
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_arm_connect2 = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_arm_connect2 = R_arm_connect2 * T_arm_connect2;
	mat4 model_matrix_arm_connect2 = model_tmp_body * model_tmp_arm_base * model_tmp_arm_connect * model_tmp_arm1 * model_tmp_arm_connect2 * S_arm_connect2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_connect2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left arm2

	mat4 T_arm2, R_arm2, S_arm2;
	//models[0].position = vec3(0, -0.85, 0);
	T_arm2 = translate(mat4(1.0), vec3(0, -0.85, 0));
	S_arm2 = scale(mat4(1.0), vec3(0.6, 0.7, 0.7));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (leftArm)
		armL2_degree -= 0.025;
	else {
		if (armL2_degree > 0)
			armL2_degree += 0.01;
		else
			armL2_degree += 0.025;
	}

	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_arm2 = rotate(mat4(1.0), armL2_degree, rotate_axis);

	mat4 model_tmp_arm2 = R_arm2 * T_arm2;
	mat4 model_matrix_arm2 = model_tmp_body * model_tmp_arm_base * model_tmp_arm_connect * model_tmp_arm1 * model_tmp_arm_connect2 * model_tmp_arm2 * S_arm2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(1, 1, 1, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[0].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left hand

	mat4 T_hand, R_hand, S_hand;
	//models[4].position = vec3(0, -0.9, 0);
	T_hand = translate(mat4(1.0), vec3(0, -0.9, 0));
	S_hand = scale(mat4(1.0), vec3(0.8, 0.8, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_hand = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_hand = R_hand * T_hand;
	mat4 model_matrix_hand = model_tmp_body * model_tmp_arm_base * model_tmp_arm_connect * model_tmp_arm1 * model_tmp_arm_connect2 * model_tmp_arm2 * model_tmp_hand * S_hand;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_hand));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.0, 0.0, 0.0, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right arm base
	mat4 T_arm_base2, R_arm_base2, S_arm_base2;
	//models[1].position = vec3(1.75, 1.36, 0);
	T_arm_base2 = translate(mat4(1.0), vec3(1.75, 1.36, 0));
	S_arm_base2 = scale(mat4(1.0), vec3(2.2, 0.7, 1.5));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (armBaseR_degree > 0.2) {
		rightArm = 1;
	}
	else if (armBaseR_degree < -0.1) {
		rightArm = 0;
	}
	if (rightArm)
		armBaseR_degree -= 0.004;
	else
		armBaseR_degree += 0.004;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_arm_base2 = rotate(mat4(1.0), armBaseR_degree, rotate_axis);

	mat4 model_tmp_arm_base2 = R_arm_base2 * T_arm_base2;
	mat4 model_matrix_arm_base2 = model_tmp_body * model_tmp_arm_base2 * S_arm_base2;


	// right arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_base2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.1, 0.5, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[1].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right arm connect

	mat4 T_arm_connect3, R_arm_connect3, S_arm_connect3;
	//models[4].position = vec3(0.4, -0.5, 0);
	T_arm_connect3 = translate(mat4(1.0), vec3(0.4, -0.5, 0));
	S_arm_connect3 = scale(mat4(1.0), vec3(0.5, 0.5, 0.5));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_arm_connect3 = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_arm_connect3 = R_arm_connect3 * T_arm_connect3;
	mat4 model_matrix_arm_connect3 = model_tmp_body * model_tmp_arm_base2 * model_tmp_arm_connect3 * S_arm_connect3;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_connect3));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right arm1

	mat4 T_arm3, R_arm3, S_arm3;
	//models[0].position = vec3(0, -0.8, 0);
	T_arm3 = translate(mat4(1.0), vec3(0, -0.8, 0));
	S_arm3 = scale(mat4(1.0), vec3(0.6, 0.7, 0.7));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (rightArm)
		armR1_degree -= 0.015;
	else
		armR1_degree += 0.015;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_arm3 = rotate(mat4(1.0), armR1_degree, rotate_axis);

	mat4 model_tmp_arm3 = R_arm3 * T_arm3;
	mat4 model_matrix_arm3 = model_tmp_body * model_tmp_arm_base2 * model_tmp_arm_connect3 * model_tmp_arm3 * S_arm3;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm3));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(1, 1, 1, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[0].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right arm connect2

	mat4 T_arm_connect4, R_arm_connect4, S_arm_connect4;
	//models[4].position = vec3(0, -0.75, 0);
	T_arm_connect4 = translate(mat4(1.0), vec3(0, -0.75, 0));
	S_arm_connect4 = scale(mat4(1.0), vec3(0.5, 0.5, 0.5));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_arm_connect4 = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_arm_connect4 = R_arm_connect4 * T_arm_connect4;
	mat4 model_matrix_arm_connect4 = model_tmp_body * model_tmp_arm_base2 * model_tmp_arm_connect3 * model_tmp_arm3 * model_tmp_arm_connect4 * S_arm_connect4;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm_connect4));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right arm2

	mat4 T_arm4, R_arm4, S_arm4;
	//models[0].position = vec3(0, -0.85, 0);
	T_arm4 = translate(mat4(1.0), vec3(0, -0.85, 0));
	S_arm4 = scale(mat4(1.0), vec3(0.6, 0.7, 0.7));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (rightArm) {
		if (armR2_degree < 0)
			armR2_degree -= 0.01;
		else
			armR2_degree -= 0.025;
	}
	else
		armR2_degree += 0.025;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_arm4 = rotate(mat4(1.0), armR2_degree, rotate_axis);

	mat4 model_tmp_arm4 = R_arm4 * T_arm4;
	mat4 model_matrix_arm4 = model_tmp_body * model_tmp_arm_base2 * model_tmp_arm_connect3 * model_tmp_arm3 * model_tmp_arm_connect4 * model_tmp_arm4 * S_arm4;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_arm4));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(1, 1, 1, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[0].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[0].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right hand

	mat4 T_hand2, R_hand2, S_hand2;
	//models[4].position = vec3(0, -0.9, 0);
	T_hand2 = translate(mat4(1.0), vec3(0, -0.9, 0));
	S_hand2 = scale(mat4(1.0), vec3(0.8, 0.8, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_hand2 = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_hand2 = R_hand2 * T_hand2;
	mat4 model_matrix_hand2 = model_tmp_body * model_tmp_arm_base2 * model_tmp_arm_connect3 * model_tmp_arm3 * model_tmp_arm_connect4 * model_tmp_arm4 * model_tmp_hand2 * S_hand2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_hand2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.0, 0.0, 0.0, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left leg connect

	mat4 T_leg_connectL, R_leg_connectL, S_leg_connectL;
	//models[4].position = vec3(-0.65, -2.05, 0);
	T_leg_connectL = translate(mat4(1.0), vec3(-0.65, -2.05, 0));
	S_leg_connectL = scale(mat4(1.0), vec3(0.6, 0.6, 0.6));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_leg_connectL = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_leg_connectL = R_leg_connectL * T_leg_connectL;
	mat4 model_matrix_leg_connectL = model_tmp_body * model_tmp_leg_connectL * S_leg_connectL;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_leg_connectL));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left leg

	mat4 T_legL, R_legL, S_legL;
	//models[2].position = vec3(0, -0.7, 0);
	T_legL = translate(mat4(1.0), vec3(0, -0.7, 0));
	S_legL = scale(mat4(1.0), vec3(0.8, 0.55, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (legL1_degree > 0.8) {
		leftLeg = 1;
	}
	else if (legL1_degree < -1) {
		leftLeg = 0;
	}
	if (leftLeg)
		legL1_degree -= 0.025;
	else
		legL1_degree += 0.025;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_legL = rotate(mat4(1.0), legL1_degree, rotate_axis);

	mat4 model_tmp_legL = R_legL * T_legL;
	mat4 model_matrix_legL = model_tmp_body * model_tmp_leg_connectL * model_tmp_legL * S_legL;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_legL));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[2].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[2].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left leg connect2

	mat4 T_leg_connectL2, R_leg_connectL2, S_leg_connectL2;
	//models[4].position = vec3(0, -0.85, 0);
	T_leg_connectL2 = translate(mat4(1.0), vec3(0, -0.85, 0));
	S_leg_connectL2 = scale(mat4(1.0), vec3(0.6, 0.6, 0.6));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (leftLeg)
		legcL_degree -= 0.005;
	else
		legcL_degree += 0.005;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_leg_connectL2 = rotate(mat4(1.0), legcL_degree, rotate_axis);

	mat4 model_tmp_leg_connectL2 = R_leg_connectL2 * T_leg_connectL2;
	mat4 model_matrix_leg_connectL2 = model_tmp_body * model_tmp_leg_connectL * model_tmp_legL * model_tmp_leg_connectL2 * S_leg_connectL2;

		// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_leg_connectL2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left leg2

	mat4 T_legL2, R_legL2, S_legL2;
	//models[2].position = vec3(0, -0.7, 0);
	T_legL2 = translate(mat4(1.0), vec3(0, -0.7, 0));
	S_legL2 = scale(mat4(1.0), vec3(0.8, 0.55, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (leftLeg)
		legL2_degree -= 0.001;
	else
		legL2_degree += 0.001;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_legL2 = rotate(mat4(1.0), legL2_degree, rotate_axis);

	mat4 model_tmp_legL2 = R_legL2 * T_legL2;
	mat4 model_matrix_legL2 = model_tmp_body * model_tmp_leg_connectL * model_tmp_legL * model_tmp_leg_connectL2 * model_tmp_legL2 * S_legL2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_legL2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[2].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[2].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / left feet

	mat4 T_feet, R_feet, S_feet;
	//models[1].position = vec3(0, -1.15, 0);
	T_feet = translate(mat4(1.0), vec3(0, -1.15, 0));
	S_feet = scale(mat4(1.0), vec3(1.2, 1.2, 1.27));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (leftLeg)
		legfL_degree -= 0.008;
	else
		legfL_degree += 0.008;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_feet = rotate(mat4(1.0), legfL_degree, rotate_axis);

	mat4 model_tmp_feet = R_feet * T_feet;
	mat4 model_matrix_feet = model_tmp_body * model_tmp_leg_connectL * model_tmp_legL * model_tmp_leg_connectL2 * model_tmp_legL2 * model_tmp_feet * S_feet;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_feet));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.7, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[1].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / /  right leg connect

	mat4 T_leg_connectR, R_leg_connectR, S_leg_connectR;
	//models[4].position = vec3(0.65, -2.05, 0);
	T_leg_connectR = translate(mat4(1.0), vec3(0.65, -2.05, 0));
	S_leg_connectR = scale(mat4(1.0), vec3(0.6, 0.6, 0.6));
	//Build rotation matrix
//    degree = glfwGetTime();
	rotate_axis = vec3(0.0, 1.0, 0.0);
	R_leg_connectR = rotate(mat4(1.0), degree, rotate_axis);

	mat4 model_tmp_leg_connectR = R_leg_connectR * T_leg_connectR;
	mat4 model_matrix_leg_connectR = model_tmp_body * model_tmp_leg_connectR * S_leg_connectR;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_leg_connectR));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right leg

	mat4 T_legR, R_legR, S_legR;
	//models[2].position = vec3(0, -0.7, 0);
	T_legR = translate(mat4(1.0), vec3(0, -0.7, 0));
	S_legR = scale(mat4(1.0), vec3(0.8, 0.55, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();

	if (legR1_degree > 1) {
		rightLeg = 1;
	}
	else if (legR1_degree < -0.8) {
		rightLeg = 0;
	}
	if (rightLeg)
		legR1_degree -= 0.025;
	else
		legR1_degree += 0.025;

	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_legR = rotate(mat4(1.0), legR1_degree, rotate_axis);

	mat4 model_tmp_legR = R_legR * T_legR;
	mat4 model_matrix_legR = model_tmp_body * model_tmp_leg_connectR * model_tmp_legR * S_legR;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_legR));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[2].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[2].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right leg connect2

	mat4 T_leg_connectR2, R_leg_connectR2, S_leg_connectR2;
	//models[4].position = vec3(0, -0.85, 0);
	T_leg_connectR2 = translate(mat4(1.0), vec3(0, -0.85, 0));
	S_leg_connectR2 = scale(mat4(1.0), vec3(0.6, 0.6, 0.6));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (rightLeg)
		legcR_degree -= 0.005;
	else
		legcR_degree += 0.005;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_leg_connectR2 = rotate(mat4(1.0), legcR_degree, rotate_axis);


	mat4 model_tmp_leg_connectR2 = R_leg_connectR2 * T_leg_connectR2;
	mat4 model_matrix_leg_connectR2 = model_tmp_body * model_tmp_leg_connectR * model_tmp_legR * model_tmp_leg_connectR2 * S_leg_connectR2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_leg_connectR2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.7, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[4].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[4].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right leg2

	mat4 T_legR2, R_legR2, S_legR2;
	//models[2].position = vec3(0, -0.7, 0);
	T_legR2 = translate(mat4(1.0), vec3(0, -0.7, 0));
	S_legR2 = scale(mat4(1.0), vec3(0.8, 0.55, 0.8));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (rightLeg)
		legR2_degree -= 0.001;
	else
		legR2_degree += 0.001;
	rotate_axis = vec3(-1.0, 0.0, 0.0);
	R_legR2 = rotate(mat4(1.0), legR2_degree, rotate_axis);


	mat4 model_tmp_legR2 = R_legR2 * T_legR2;
	mat4 model_matrix_legR2 = model_tmp_body * model_tmp_leg_connectR * model_tmp_legR * model_tmp_leg_connectR2 * model_tmp_legR2 * S_legR2;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_legR2));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.3, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[2].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[2].vertexCount);

	// // /// /// /// // // // // / /// // // //// // // // // / / right feet

	mat4 T_feetR, R_feetR, S_feetR;
	//models[1].position = vec3(0, -1.15, 0);
	T_feetR = translate(mat4(1.0), vec3(0, -1.15, 0));
	S_feetR = scale(mat4(1.0), vec3(1.2, 1.2, 1.27));
	//Build rotation matrix
//    degree = glfwGetTime();
	if (rightLeg)
		legfR_degree -= 0.008;
	else
		legfR_degree += 0.008;
	rotate_axis = vec3(1.0, 0.0, 0.0);
	R_feetR = rotate(mat4(1.0), legfR_degree, rotate_axis);


	mat4 model_tmp_feetR = R_feetR * T_feetR;
	mat4 model_matrix_feetR = model_tmp_body * model_tmp_leg_connectR * model_tmp_legR * model_tmp_leg_connectR2 * model_tmp_legR2 * model_tmp_feetR * S_feetR;

	// left arm  model
	glUniformMatrix4fv(iLocM, 1, GL_FALSE, value_ptr(model_matrix_feetR));
	glUniformMatrix4fv(iLocV, 1, GL_FALSE, value_ptr(view_matrix));
	glUniformMatrix4fv(iLocP, 1, GL_FALSE, value_ptr(project_matrix));
	// give model color
	model_color = vec4(0.1, 0.2, 0.7, 1.0);
	glUniform4fv(cur_color, 1, value_ptr(model_color));

	glBindVertexArray(m_shape_list[1].vao);
	glDrawArrays(GL_TRIANGLES, 0, m_shape_list[1].vertexCount);

	glutSwapBuffers();
}

// Setting up viewing matrix
void My_Reshape(int width, int height)
{
	proj.aspect = (float)(width) / (float)height;
	screenWidth = width;
	screenHeight = height;

	glViewport(0, 0, screenWidth, screenHeight);
}

void My_Timer(int val)
{
	timer_cnt += 0.03f;
	glutPostRedisplay();
	if (!pause)
	{
		glutTimerFunc(timer_speed, My_Timer, val);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (key == 'z')
	{
		printf("press z!\n");
		if (!pause) {
			if (!rotate90_flag) {
				reflect_y *= -1;
				if (reflect_y == -1) {
					move_y -= 2;
				}
				else {
					move_y += 2;
				}
			}
			else {
				reflect_x *= -1;
				if (reflect_x == -1) {
					move_x -= 2;
				}
				else {
					move_x += 2;
				}
			}
		}
	}
	else if (key == 'x')
	{
		if (!pause) {
			rotate90_flag = !rotate90_flag;
		}
	}
	else if (key == 'c')
	{
		printf("press c!\n");
		pause = !pause;
		if (pause) {
			cout << "yes" << endl;
			pauseTime = body_degree;
		}
		else {
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
	}
	else if (key == 'v')
	{
		if (!pause) {
			rotate_flag = !rotate_flag;
			if (!rotate_flag) {
				stopTime = body_degree;
			}
			else {
				time_flag = true;
			}
		}
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{
	case MENU_TIMER_START:
		if (pause)
		{
			pause = false;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		pause = true;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !pause) {
		printf("Click Mouse Left button!\n");
		if (x <= 400 && y > 200 && y <= 400) {
			if (!camera_x)
				move_x -= 1;
			else
				move_y += 1;
		}
		else if (x > 400 && y > 200 && y <= 400) {
			if (!camera_x)
				move_x += 1;
			else
				move_y -= 1;
		}
		else if (x > 400 && y > 400) {
			if (!camera_x) {
				move_x += 1;
				move_y -= 1;
			}
			else {
				move_x -= 1;
				move_y -= 1;
			}
		}
		else if (x > 400 && y <= 200) {
			if (!camera_x) {
				move_x += 1;
				move_y += 1;
			}
			else {
				move_x += 1;
				move_y -= 1;
			}
		}
		else if (x <= 400 && y > 400) {
			if (!camera_x) {
				move_x -= 1;
				move_y -= 1;
			}
			else {
				move_x -= 1;
				move_y += 1;
			}
		}
		else if (x <= 400 && y <= 200) {
			if (!camera_x) {
				move_x -= 1;
				move_y += 1;
			}
			else {
				move_x += 1;
				move_y += 1;
			}
		}
	}
}

void My_newMenu(int id)
{

}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Robot"); // You cannot use OpenGL functions before this line;
								  // The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	dumpInfo();
	My_Init();

	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_new = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);
	glutMouseFunc(My_Mouse);

	// Enter main event loop.
	glutMainLoop();

	return 0;
}