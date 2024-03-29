#include "mat4.h"
#include "vec3.h"
#include "plane.h"
#include "quaternion.h"
#include "../os/msg.h"

// ZXY -> object transformation
// der Vektor nach vorne (0,0,1) wird
// 1. um die z-Achse gedreht (um sich selbst)
// 2. um die x-Achse gedreht (nach oben oder unten "genickt")
// 3. um die y-Achse gedreht (nach links oder rechts)
// (alle Drehungen um Achsen, die nicht veraendert werden!!!)

// YXZ -> Kamera-/Projektions-Transformation
// der Vektor nach vorne (0,0,1) wird
// ... aber in die jeweils andere Richtung (-ang)

// YXZ-Matrix ist die Inverse zur ZXY-Matrix!!!


//------------------------------------------------------------------------------------------------//
//                                           matrices                                             //
//------------------------------------------------------------------------------------------------//


const float f_m_id[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
const mat4 mat4::ID = mat4(f_m_id);


mat4 MatrixMultiply2(const mat4 &m2, const mat4 &m1);

mat4::mat4(const float f[16]) {
	for (int i=0;i<16;i++)
		e[i]=f[i];
}

mat4::mat4(const vec3 &a, const vec3 &b, const vec3 &c) {
	_00 = a.x;	_01 = b.x;	_02 = c.x;	_03 = 0;
	_10 = a.y;	_11 = b.y;	_12 = c.y;	_13 = 0;
	_20 = a.z;	_21 = b.z;	_22 = c.z;	_23 = 0;
	_30 = 0;	_31 = 0;	_32 = 0;	_33 = 1;
}

mat4 mat4::operator + (const mat4 &m) const {
	mat4 r;
	for (int i=0;i<16;i++)
		r.e[i]=e[i]+m.e[i];
	return r;
}

mat4 mat4::operator - (const mat4 &m) const {
	mat4 r;
	for (int i=0;i<16;i++)
		r.e[i]=e[i]-m.e[i];
	return r;
}

mat4 mat4::operator * (const mat4 &m) const {
	return MatrixMultiply2(*this, m);
}

mat4 mat4::operator *= (const mat4 &m) {
	mat4 r = (*this * m);
	*this = r;
	return *this;
}

vec3 mat4::operator * (const vec3 &v) const {
	return vec3(v.x*_00 + v.y*_01 + v.z*_02 + _03,
	              v.x*_10 + v.y*_11 + v.z*_12 + _13,
	              v.x*_20 + v.y*_21 + v.z*_22 + _23);
}

mat4 mat4::operator * (float f) const {
	mat4 r;
	for (int i=0;i<16;i++)
		r.e[i] = e[i] * f;
	return r;
}

vec3 mat4::transform(const vec3 &v) const {
	return *this * v;
}
vec3 mat4::transform_normal(const vec3 &v) const {
	return vec3(v.x*_00 + v.y*_01 + v.z*_02,
	              v.x*_10 + v.y*_11 + v.z*_12,
	              v.x*_20 + v.y*_21 + v.z*_22);
}
vec3 mat4::untransform(const vec3 &v) const {
	return inverse() * v;
}

vec3 mat4::project(const vec3 &v) const {
	return (*this * v) / (v.x*_30 + v.y*_31 + v.z*_32 + _33);
}

vec3 mat4::unproject(const vec3 &v) const {
	return (*this * v) / (v.x*_30 + v.y*_31 + v.z*_32 + _33);
}

string mat4::str() const {
	//return format("(%f, %f, %f, %f; %f, %f, %f, %f; %f, %f, %f, %f; %f, %f, %f, %f)", _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);
	return format("(%f, %f, %f, %f\n %f, %f, %f, %f\n %f, %f, %f, %f\n %f, %f, %f, %f)", _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33);
}


//#define _ps(a,b,i,j)	(a.__e[0][i]*b.__e[j][0] + a.__e[1][i]*b.__e[j][1] + a.__e[2][i]*b.__e[j][2] + a.__e[3][i]*b.__e[j][3])


// combining two transformation matrices (first do m1, then m2:   m = m2 * m1 )
mat4 MatrixMultiply2(const mat4 &m2, const mat4 &m1)
{
	// m_ij = (sum k) m2_ik * m1_kj
	mat4 m;
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			m.__e[j][i] = m2.__e[0][i]*m1.__e[j][0] + m2.__e[1][i]*m1.__e[j][1] + m2.__e[2][i]*m1.__e[j][2] + m2.__e[3][i]*m1.__e[j][3];
	/*_m._00=_ps(m2,m1,0,0);	_m._01=_ps(m2,m1,0,1);	_m._02=_ps(m2,m1,0,2);	_m._03=_ps(m2,m1,0,3);
	_m._10=_ps(m2,m1,1,0);	_m._11=_ps(m2,m1,1,1);	_m._12=_ps(m2,m1,1,2);	_m._13=_ps(m2,m1,1,3);
	_m._20=_ps(m2,m1,2,0);	_m._21=_ps(m2,m1,2,1);	_m._22=_ps(m2,m1,2,2);	_m._23=_ps(m2,m1,2,3);
	_m._30=_ps(m2,m1,3,0);	_m._31=_ps(m2,m1,3,1);	_m._32=_ps(m2,m1,3,2);	_m._33=_ps(m2,m1,3,3);*/
	return m;
}

float mat4::determinant() const {
	return
		_30*_21*_12*_03-
		_20*_31*_12*_03-
		_30*_11*_22*_03+
		_10*_31*_22*_03+
		_20*_11*_32*_03-
		_10*_21*_32*_03-
		_30*_21*_02*_13+
		_20*_31*_02*_13+
		_30*_01*_22*_13-
		_00*_31*_22*_13-
		_20*_01*_32*_13+
		_00*_21*_32*_13+
		_30*_11*_02*_23-
		_10*_31*_02*_23-
		_30*_01*_12*_23+
		_00*_31*_12*_23+
		_10*_01*_32*_23-
		_00*_11*_32*_23-
		_20*_11*_02*_33+
		_10*_21*_02*_33+
		_20*_01*_12*_33-
		_00*_21*_12*_33-
		_10*_01*_22*_33+
		_00*_11*_22*_33;
}

// inverting the transformation
mat4 mat4::inverse() const {
	mat4 mo;
	float x = determinant();

	/*msg_write("Matrix Inverse");
	mout(mi);
	msg_write(f2s(x,3));*/

	if (x == 0){
		msg_write("matrix.inverse():  not invertible");
		return ID;
	}

	mo._00= (-_31*_22*_13 +_21*_32*_13 +_31*_12*_23 -_11*_32*_23 -_21*_12*_33 +_11*_22*_33)/x;
	mo._10= ( _30*_22*_13 -_20*_32*_13 -_30*_12*_23 +_10*_32*_23 +_20*_12*_33 -_10*_22*_33)/x;
	mo._20= (-_30*_21* _13 +_20*_31*_13 +_30*_11*_23 -_10*_31*_23 -_20*_11*_33 +_10*_21* _33)/x;
	mo._30=( _30*_21* _12 -_20*_31*_12 -_30*_11*_22 +_10*_31*_22 +_20*_11*_32 -_10*_21* _32)/x;
	mo._01= ( _31*_22*_03 -_21*_32*_03 -_31*_02*_23 +_01*_32*_23 +_21*_02*_33 -_01*_22*_33)/x;
	mo._11= (-_30*_22*_03 +_20*_32*_03 +_30*_02*_23 -_00*_32*_23 -_20*_02*_33 +_00*_22*_33)/x;
	mo._21= ( _30*_21* _03 -_20*_31*_03 -_30*_01*_23 +_00*_31*_23 +_20*_01*_33 -_00*_21* _33)/x;
	mo._31=(-_30*_21* _02 +_20*_31*_02 +_30*_01*_22 -_00*_31*_22 -_20*_01*_32 +_00*_21* _32)/x;
	mo._02= (-_31*_12* _03 +_11*_32*_03 +_31*_02*_13  -_01*_32*_13 -_11*_02*_33 +_01*_12* _33)/x;
	mo._12= ( _30*_12* _03 -_10*_32*_03 -_30*_02*_13  +_00*_32*_13 +_10*_02*_33 -_00*_12* _33)/x;
	mo._22=(-_30*_11* _03 +_10*_31*_03 +_30*_01*_13  -_00*_31*_13 -_10*_01*_33 +_00*_11* _33)/x;
	mo._32=( _30*_11* _02 -_10*_31*_02 -_30*_01*_12  +_00*_31*_12 +_10*_01*_32 -_00*_11* _32)/x;
	mo._03= ( _21* _12* _03 -_11*_22*_03 -_21* _02*_13  +_01*_22*_13 +_11*_02*_23 -_01*_12* _23)/x;
	mo._13= (-_20* _12* _03 +_10*_22*_03 +_20* _02*_13  -_00*_22*_13 -_10*_02*_23 +_00*_12* _23)/x;
	mo._23=( _20* _11* _03 -_10*_21* _03 -_20* _01*_13  +_00*_21* _13 +_10*_01*_23 -_00*_11* _23)/x;
	mo._33=(-_20* _11* _02 +_10*_21* _02 +_20* _01*_12  -_00*_21* _12 -_10*_01*_22 +_00*_11* _22)/x;
	return mo;
}

// transposes a matrix
mat4 mat4::transpose() const {
	mat4 _m;
	_m._00=_00;	_m._01=_10;	_m._02=_20;	_m._03=_30;
	_m._10=_01;	_m._11=_11;	_m._12=_21;	_m._13=_31;
	_m._20=_02;	_m._21=_12;	_m._22=_22;	_m._23=_32;
	_m._30=_03;	_m._31=_13;	_m._32=_23;	_m._33=_33;
	return _m;
}

// translation by a vector ( m * v = v + t )
mat4 mat4::translation(const vec3 &t) {
	mat4 m;
	m._00=1;	m._01=0;	m._02=0;	m._03=t.x;
	m._10=0;	m._11=1;	m._12=0;	m._13=t.y;
	m._20=0;	m._21=0;	m._22=1;	m._23=t.z;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
	return m;
}

// rotation around the X axis (down)
mat4 mat4::rotation_x(float w) {
	mat4 m;
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=1;	m._01=0;	m._02=0;	m._03=0;
	m._10=0;	m._11=cw;	m._12=-sw;	m._13=0;
	m._20=0;	m._21=sw;	m._22=cw;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
	return m;
}

// rotation around the Y axis (to the right)
mat4 mat4::rotation_y(float w) {
	mat4 m;
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=cw;	m._01=0;	m._02=sw;	m._03=0;
	m._10=0;	m._11=1;	m._12=0;	m._13=0;
	m._20=-sw;	m._21=0;	m._22=cw;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
	return m;
}

// rotation around the Z axis (counter clockwise)
mat4 mat4::rotation_z(float w) {
	mat4 m;
	float sw=sinf(w);
	float cw=cosf(w);
	m._00=cw;	m._01=-sw;	m._02=0;	m._03=0;
	m._10=sw;	m._11=cw;	m._12=0;	m._13=0;
	m._20=0;	m._21=0;	m._22=1;	m._23=0;
	m._30=0;	m._31=0;	m._32=0;	m._33=1;
	return m;
}

// ZXY -> for objects
mat4 mat4::rotation(const vec3 &ang) {
	mat4 m;
	/*matrix x,y,z;
	MatrixRotationX(x,ang.x);
	MatrixRotationY(y,ang.y);
	MatrixRotationZ(z,ang.z);
	// m=y*x*z
	MatrixMultiply(m,y,x);
	MatrixMultiply(m,m,z);*/
	float sx=sinf(ang.x);
	float cx=cosf(ang.x);
	float sy=sinf(ang.y);
	float cy=cosf(ang.y);
	float sz=sinf(ang.z);
	float cz=cosf(ang.z);
	m._00= sx*sy*sz + cy*cz;	m._01= sx*sy*cz - cy*sz;	m._02= cx*sy;	m._03=0;
	m._10= cx*sz;				m._11= cx*cz;				m._12=-sx;		m._13=0;
	m._20= sx*cy*sz - sy*cz;	m._21= sx*cy*cz + sy*sz;	m._22= cx*cy;	m._23=0;
	m._30= 0;					m._31= 0;					m._32=0;		m._33=1;
	return m;
}

// rotation matrix from quaterion
mat4 mat4::rotation(const quaternion &q) {
	mat4 m;
	m._00 = 1 - 2*q.y*q.y - 2*q.z*q.z; m._01 =     2*q.x*q.y - 2*q.w*q.z; m._02 =     2*q.x*q.z + 2*q.w*q.y; m._03 = 0;
	m._10 =     2*q.x*q.y + 2*q.w*q.z; m._11 = 1 - 2*q.x*q.x - 2*q.z*q.z; m._12 =     2*q.y*q.z - 2*q.w*q.x; m._13 = 0;
	m._20 =     2*q.x*q.z - 2*q.w*q.y; m._21 =     2*q.y*q.z + 2*q.w*q.x; m._22 = 1 - 2*q.x*q.x - 2*q.y*q.y; m._23 = 0;
	m._30 = 0;                         m._31 = 0;                         m._32 = 0;                         m._33 = 1;
	/* [ 1 - 2y2 - 2z2        2xy - 2wz        2xz + 2wy
	         2xy + 2wz    1 - 2x2 - 2z2        2yz - 2wx
		     2xz - 2wy        2yz + 2wx    1 - 2x2 - 2y2 ] */
	return m;
}

// scale orthogonally in 3 dimensions
mat4 mat4::scale(float fx, float fy, float fz) {
	mat4 m;
	m._00 = fx; m._01 = 0;  m._02 = 0;  m._03 = 0;
	m._10 = 0;  m._11 = fy; m._12 = 0;  m._13 = 0;
	m._20 = 0;  m._21 = 0;  m._22 = fz; m._23 = 0;
	m._30 = 0;  m._31 = 0;  m._32 = 0;  m._33 = 1;
	return m;
}

mat4 mat4::scale(const vec3 &v) {
	return scale(v.x, v.y, v.z);
}

// create a transformation that reflects at a <plane pl>
mat4 mat4::reflection(const plane &pl) {
	mat4 m;
	vec3 n = pl.n;
	vec3 p = -n * pl.d;
	// mirror: matrix s from transforming the basis vectors:
	//    e_i' = e_i - 2 < n, e_i >
	//     or thinking of it as a tensor product (projection): s = id - 2n (x) n
	// translation by p: t_p
	// complete reflection is: r = t_p * s * t_(-p) = t_(2p) * s
	m._00 = 1 - 2 * n.x*n.x; m._01 =   - 2 * n.y*n.x; m._02 =   - 2 * n.z*n.x; m._03 = 2 * p.x;
	m._10 =   - 2 * n.x*n.y; m._11 = 1 - 2 * n.y*n.y; m._12 =   - 2 * n.z*n.y; m._13 = 2 * p.y;
	m._20 =   - 2 * n.x*n.z; m._21 =   - 2 * n.y*n.z; m._22 = 1 - 2 * n.z*n.z; m._23 = 2 * p.z;
	m._30 = 0;               m._31 = 0;               m._32 = 0;               m._33 = 1;
	return m;
}

// keep ALL axes aligned!
// z_sym=true:   P (x,y,z0) = (.,.,-1)   P (x,y,z1) = (.,.,+1)
// z_sym=false:  P (x,y,z0) = (.,., 0)   P (x,y,z1) = (.,.,+1)
// maps the POSITIVE z-half-space
mat4 mat4::perspective(float fovy, float aspect, float z_near, float z_far, bool z_sym) {
	mat4 m;
	float f = 1 / tan(fovy / 2);
	float ndz = z_far - z_near;
	if (z_sym) {
		m._00 = f / aspect; m._01 = 0; m._02 = 0;                      m._03 = 0;
		m._10 = 0;          m._11 = f; m._12 = 0;                      m._13 = 0;
		m._20 = 0;          m._21 = 0; m._22 = (z_near + z_far) / ndz; m._23 =-2 * z_near * z_far / ndz;
		m._30 = 0;          m._31 = 0; m._32 = 1;                      m._33 = 0;
	} else {
		m._00 = f / aspect; m._01 = 0; m._02 = 0;           m._03 = 0;
		m._10 = 0;          m._11 = f; m._12 = 0;           m._13 = 0;
		m._20 = 0;          m._21 = 0; m._22 = z_far / ndz; m._23 =-z_near * z_far / ndz;
		m._30 = 0;          m._31 = 0; m._32 = 1;           m._33 = 0;
	}
	return m;
}

