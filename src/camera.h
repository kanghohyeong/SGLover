#ifndef __CAMERA_H__
#define __CAMERA_H__
#include "cgmath.h"
#include "cgut.h"		// slee's OpenGL utility
#include "colosseum.h"
#include "cyl.h"
// camera 움직임, 회전 그에 따른 탱크 객체 이동을 정의한 헤더입니다. tank 객체가 생성되어 있습니다.

extern float t;	// 전체 시간, 단 정지 기능을 위해 buffer가 빠진 값

struct tank_t
{
	float	radius=0.5f;	// radius
	float	height=0.5f;	// height
	vec3	pos;			// position of tank
	vec4	color;			// RGBA color in [0,1]
	uint	NTESS=29;
	float	mass = 7.0f * radius*radius*height;	// tank 질량
	mat4	model_matrix;	// modeling transformation
	mat4 model_matrix_head;	//머리 matrix
	mat4 model_matrix_arm;	//팔

	// public functions
	void	update_tank( float t, const vec3& eye, const vec3& at );
	void	update_tank_head(float t);
	void	update_tank_arm();
};

#define cosrand() cos((float)rand()) 

// tank 객체 생성
tank_t tank = {0.3f, 0.3f, vec3(0), vec4(0.5f,0.5f,0.5f,1.0f), 30};

//*************************************
struct camera
{
	// 바닥이 보여 눈은 1.0h 만큼 올렸지만, tank 바닥은 그대로 지면에 닿아있게 조절했음
	vec3	eye = vec3( 0.5f, colosseum.pos.y + colosseum.height * 0.5f + 2.0f * tank.height, 0.0f );
	vec3	at = vec3( 0.0f, colosseum.pos.y + colosseum.height * 0.5f + 2.0f * tank.height, -20.0f );
	vec3	up = vec3( 0.0f, 1.0f, 0.0f );
	mat4	view_matrix = mat4::look_at( eye, at, up );;

	float	fovy = PI/4.0f; // must be in radian
	float	aspect;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
	
	bool	b_tracking = false;
	vec2	m0;				// the last mouse position

	bool is_tracking() const { return b_tracking; }
	
	void begin( vec2 m );
	void end() { b_tracking = false; }

	// time buffer
	float t0=0;
	vec3 eye0;
	vec3 at0;
	
	// **** speed
	float	speed = 2.8f / sqrt(tank.mass);	// velocity of tank
	float	max_speed = 1.3f * speed;	// max velocity of tank
	float	camera_speed = 1.3f;
	//float	max_camera_speed = 1.3f * camera_speed;	// max velocity of camera

	// ************ WASD  ***************//
	bool b_W = false;
	bool b_A = false;
	bool b_S = false;
	bool b_D = false;
	bool b_LEFT = false;
	bool b_RIGHT = false;

	bool is_W() const { return b_W; }
	bool is_A() const { return b_A; }
	bool is_S() const { return b_S; }
	bool is_D() const { return b_D; }
	bool is_LEFT() const { return b_LEFT; }
	bool is_RIGHT() const { return b_RIGHT; }

	//굳이 함수 call을 해서 overhead를 늘릴 필요는 사실 없음
	void begin_Camera(char key, int flags) {
		t0=t; 
		eye0 = eye;
		at0 = at;

		if (key == 'W') b_W = true;
		else if (key == 'A') b_A = true;
		else if (key == 'S') b_S = true;
		else if (key == 'D') b_D = true;
		else if (key == 'L' ) b_LEFT = true;
		else if (key == 'R') b_RIGHT = true;

		//end_Camera 당시에 켜져 있던 key들 다시 키기.
		//다른 키 눌렸을 때 갱신용임.
		if (flags & 0x1) {
			b_W = true;
		}
		if (flags & 0x10) {
			b_A = true;
		}
		if (flags & 0x100) {
			b_S = true;
		}
		if (flags & 0x1000) {
			b_D = true;
		}
		if (flags & 0x10000) {
			b_LEFT = true;
		}
		if (flags & 0x100000) {
			b_RIGHT = true;
		}

	}


	int end_Camera() {
		int flags=0;

		if (b_W) {
			b_W = false;
			flags += 0x1;
		}
		if (b_A) {
			b_A = false;
			flags += 0x10;
		}
		if (b_S) {
			b_S = false;
			flags += 0x100;
		}
		if (b_D) {
			b_D = false;
			flags += 0x1000;
		}
		if (b_LEFT) {
			b_LEFT = false;
			flags += 0x10000;
		}
		if (b_RIGHT) {
			b_RIGHT = false;
			flags += 0x100000;
		}

		return flags;
	}


	//camera의 eye, at, up 값을 받아서 바꾸고 view matrix를 반환하는 함수임
	void update_Camera();


};

camera	cam;

inline void camera::begin( vec2 m )
{
	b_tracking = true;			// enable camera tracking
	m0 = m;						// save current mouse position
}

// WASD는 n벡터의 시작점을 바꾸는 것과 같음
// 즉 eye와 at을 같이 바꿔주면 됨
inline void camera::update_Camera()
{
	vec3 n = (cam.at - cam.eye).normalize();
	vec3 u = cam.up;
	vec3 v = (n.cross(u)).normalize();

	float dt = t - t0;

	vec3 eye_cumulative;
	vec3 at_cumulative;
	if (b_W || b_A || b_S || b_D || b_LEFT || b_RIGHT) {
		eye_cumulative = eye0;
		at_cumulative = at0;
	}

	if (b_W) {
		eye_cumulative= eye_cumulative + n * dt * speed;
		at_cumulative = at_cumulative + n * dt * speed;
		
	}
	if (b_A) {
		eye_cumulative = eye_cumulative - v * dt * speed;
		at_cumulative = at_cumulative - v * dt * speed;

	}
	if (b_S) {
		eye_cumulative = eye_cumulative - n * dt * speed;
		at_cumulative = at_cumulative - n * dt * speed;
		
	}
	if (b_D) {
		eye_cumulative = eye_cumulative + v * dt * speed;
		at_cumulative = at_cumulative + v * dt * speed;
	}
	//at = at_cumulative;

	if (b_LEFT) {

		vec4 at4 = vec4(at_cumulative, 1);
		vec4 atb = (mat4::translate(eye_cumulative) * mat4::rotate(up, camera_speed * dt) * (mat4::translate(-1*eye_cumulative))) * at4;

		at_cumulative = vec3(atb.x, atb.y, atb.z);

		
	}
	if (b_RIGHT) {
		
		vec4 at4 = vec4(at_cumulative, 1);
		vec4 atb = (mat4::translate(eye_cumulative) * mat4::rotate(up, -1* camera_speed * dt) * (mat4::translate(-1 * eye_cumulative))) * at4;

		at_cumulative = vec3(atb.x, atb.y, atb.z);
	}



	if (b_W || b_A || b_S || b_D || b_LEFT || b_RIGHT) {
		eye = eye_cumulative;
		at = at_cumulative;
	}

}
/*
inline void camera::update_A()
{
	vec3 u = (at- eye).normalize();
	vec3 v = (u.cross(up)).normalize();
	float dt = t - ta;

	eye -= v * min(max_speed,dt * speed);
	at -= v * min(max_speed,dt * speed);
}


inline void camera::update_S()
{
	vec3 u = (at- eye).normalize();
	//vec3 v = (u.cross(up)).normalize();
	float dt = t - ts;

	eye -= u * min(max_speed,dt * speed);
	at -= u * min(max_speed,dt * speed);
}


inline void camera::update_D()
{
	vec3 u = (at- eye).normalize();
	vec3 v = (u.cross(up)).normalize();
	float dt = t - td;

	eye += v * min(max_speed,dt * speed);
	at += v * min(max_speed,dt * speed);
}

// n를 up을 축으로 회전시키는 것임, (축이 v일 수도 있음)
// eye를 고정하고 at을 바꾸는 것 트랙볼 반대라고 보면 됨
inline void camera::update_LEFT()
{
	vec4 at4 = vec4(at, 1);
	float dt = t - tl;

	//eye를 중심으로 이동(translate), up기준 회전(rotate), 다시 eye 원래 위치로 이동
	vec4 atb = (mat4::translate(eye) * mat4::rotate(up, min(camera_speed * dt, max_camera_speed)) * (mat4::translate(-eye) * at4));
	
	at = vec3(atb.x, atb.y, atb.z);
}

inline void camera::update_RIGHT()
{
	vec4 at4 = vec4(at, 1);
	float dt = t - tr;

	//eye를 중심으로 이동(translate), up기준 회전(rotate), 다시 eye 원래 위치로 이동
	vec4 atb = (mat4::translate(eye) * mat4::rotate(-up, min(camera_speed * dt, max_camera_speed)) * (mat4::translate(-eye) * at4));

	at = vec3(atb.x, atb.y, atb.z);
}
*/

//********** tank 움직임 파트 *************
inline void tank_t::update_tank( float t, const vec3& eye, const vec3& at )
{
	vec3 n = (eye - at).normalize();
	// tank pos랑 정확히 일치시키면 자기몸이 안 보여요 좀 떼어놓은 겁니다.
	pos = eye - 7.0f * radius * n;
	pos.y = eye.y - 1.5f * height;

	// 카메라 회전에 따라 몸체도 회전시키기 위함. n,ref 둘다 단위벡터임. y축 기준 회전
	vec3 ref = vec3(0,0,1.0f);
	float theta = 0;
	if (n.x >= 0) theta = acos(ref.dot(n));
	else theta = -acos(ref.dot(n));
	float c = cos(theta), s = sin(theta);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, height, 0, 0,
		0, 0, radius, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c, 0, s, 0,
		0, 1, 0, 0,
		-s, 0, c, 0,
		0, 0, 0, 1
	};

	mat4 rotation_cloth_matrix =
	{
		cos(PI/2), 0, sin(PI/2), 0,
		0, 1, 0, 0,
		-sin(PI/2), 0, cos(PI/2), 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, pos.x,
		0, 1, 0, pos.y,
		0, 0, 1, pos.z,
		0, 0, 0, 1
	};

	/*
	mat4 scale_matrix_2 =
	{
		0.3f, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0.3f, 0,
		0, 0, 0, 1
	};
	*/


	model_matrix = translate_matrix *rotation_cloth_matrix* rotation_matrix * scale_matrix;
}

inline void	tank_t::update_tank_head(float t)
{

	mat4 translate_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, height*1.2f,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		cos(t), 0, sin(t), 0,
		0, 1, 0, 0,
		-sin(t), 0, cos(t), 0,
		0, 0, 0, 1
	};

	mat4 scale_matrix =
	{
		0.8f, 0, 0, 0,
		0, 0.8f, 0, 0,
		0, 0, 0.8f, 0,
		0, 0, 0, 1
	};
	
	model_matrix_head = translate_matrix * model_matrix * scale_matrix * rotation_matrix;


}

inline void	tank_t::update_tank_arm()
{

	mat4 translate_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, height * 0.3f,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, cos(PI / 2), -sin(PI / 2), 0,
		0, sin(PI / 2), cos(PI / 2), 0,
		0, 0, 0, 1
	};

	mat4	scale_matrix =
	{
		0.8f,0,0,0,
		0,1.5f,0,0,
		0,0,0.8f,0,
		0,0,0,1

	};

	model_matrix_arm = translate_matrix * model_matrix * rotation_matrix * scale_matrix;


}

#endif // __CAMERA_H__
