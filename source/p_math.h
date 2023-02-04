#ifndef P_MATH_H
#define P_MATH_H

#include <math.h>
#include <float.h>
#include <stdint.h>
#include <cmath>

typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef float    f32;
typedef double   f64;
typedef int8_t   b8;
typedef int32_t  b32;
typedef uint8_t  byte;

#define U64_MAX ((u64)UINT64_MAX)
#define U64_MIN ((u64)0)
#define U32_MAX ((u32)UINT32_MAX)
#define U32_MIN ((u32)0)
#define F32_MAX ((f32)FLT_MAX)
#define F32_MIN ((f32)FLT_MIN)
#define I32_MAX ((i32)INT32_MAX)
#define I32_MIN ((i32)INT32_MIN)

#define PI32 3.1415926535f

#define KB(b) ((b)   * 1024)
#define MB(b) (KB(b) * 1024)
#define GB(b) (MB(b) * 1024)

struct rectf32
{
    f32 x;
    f32 y;
    f32 width;
    f32 height;
};

struct recti32
{
    i32 x;
    i32 y;
    i32 width;
    i32 height;
};

struct vec2
{
    union
    {
        f32 e[2];
        struct { f32 x, y; };
        struct { f32 w, h; };
        struct { f32 v; };
        struct { f32 theta, phi; };
    };
};

struct vec2_4x {
    union {
        vec2 vecs[4];
        struct {
            vec2 v0;
            vec2 v1;
            vec2 v2;
            vec2 v3;
        };
    };
};

struct vec2i
{
    union
    {
        i32 e[2];
        struct { i32 x, y; };
        struct { i32 w, h; };
    };
    vec2 to_vec2(void) { return { (f32)this->x, (f32)this->y }; };
};

struct vec3
{
    union
    {
        f32 e[3];
        struct { f32 x, y, z; };
        struct { f32 r, g, b; };
        struct { vec2 xy; };
        struct { f32 p, theta, phi; };
    };
};

struct vec3i
{
    union
    {
        i32 e[3];
        struct { i32 x, y, z; };
        struct { i32 r, g, b; };
        struct { vec2i xy; };
    };
    vec3 to_vec3(void) { return {(f32)this->x, (f32)this->y, (f32)this->z}; }
};

struct vec4
{
    union
    {
        f32 e[4];
        struct { f32 x, y, z, w; };
        struct { f32 r, g, b, a; };
        struct { vec3 xyz; };
        struct { vec3 rgb; };
    };
};

struct mat3x3
{
    union
    {
        f32 e[9];
        f32 m[3][3];
    };
};

struct mat4x4
{
    union
    {
        f32 e[16];
        f32 m[4][4];
    };
};

#define P_INLINE inline

/*------------------*/
/*                  */
/*------------------*/
#define square(v) ((v) * (v))
#define clamp_min(v,m) ((v < m) ? m : v)
#define clamp_max(v,m) ((v > m) ? m : v)
#define clamp(v,min,max) (clamp_max(clamp_min(v, min), max))
#define clamp01(v) (clamp(v,0,1))
#define clamp_min_v(v, min)  ((v) = clamp_min((v), (min)))
#define clamp_max_v(v, max)  ((v) = clamp_max((v), (max)))
#define clamp_v(v, min, max) ((v) = clamp((v), (min), (max)))
#define clamp01_v(v) (clamp_v(v, 0, 1))
#define is_in_bounds(v,min,max) (((v) >= (min)) && ((v) <= (max)))
#define min_value(a, b) (((a) < (b)) ? (a) : (b))
#define max_value(a, b) (((a) > (b)) ? (a) : (b))

P_INLINE f32 prec(f32 v, u32 p);
P_INLINE f32 rad_to_deg(f32 radians);
P_INLINE f32 deg_to_rad(f32 degrees);
P_INLINE f32 square_root(f32 v);
P_INLINE f32 power(f32 v, f32 p);
P_INLINE f32 roundf32(f32 v);
P_INLINE i32 roundf32_to_i32(f32 v);
P_INLINE f32 lerp(f32 v0, f32 v1, f32 t);
P_INLINE f32 absolute(f32 v);
P_INLINE i32 absolute(i32 v);

/*------------------*/
/*       vec2       */
/*------------------*/
P_INLINE vec2 make_vec2(f32 e0, f32 e1);
P_INLINE vec2 make_vec2(f32 e = 0.0f);

vec2 operator-(vec2 v);
vec2 operator+(const vec2 v0, const vec2 v1);
vec2 operator-(const vec2 v0, const vec2 v1);
vec2 operator*(const vec2 v0, const vec2 v1);
vec2 operator/(const vec2 v0, const vec2 v1);
vec2 operator+(const vec2 v, const f32 n);
vec2 operator-(const vec2 v, const f32 n);
vec2 operator*(const vec2 v, const f32 n);
vec2 operator/(const vec2 v, const f32 n);
vec2 &operator+=(vec2 &v0, const vec2 v1);
vec2 &operator-=(vec2 &v0, const vec2 v1);
vec2 &operator*=(vec2 &v0, const vec2 v1);
vec2 &operator/=(vec2 &v0, const vec2 v1);
vec2 &operator+=(vec2 &v, const f32 n);
vec2 &operator-=(vec2 &v, const f32 n);
vec2 &operator*=(vec2 &v, const f32 n);
vec2 &operator/=(vec2 &v, const f32 n);
bool operator==(const vec2 v0, const vec2 v1);
bool operator!=(const vec2 v0, const vec2 v1);

P_INLINE vec2 vec_prec(vec2 v, u32 p);
P_INLINE vec2 vec_lerp(vec2 v0, vec2 v1, f32 t);
P_INLINE f32  vec_length(vec2 v);
P_INLINE f32  vec_atan2(vec2 v);
P_INLINE vec2 vec_vector(vec2 v0, vec2 v1);
P_INLINE vec2 vec_from_angle(f32 a);
P_INLINE vec2 vec_normalize(vec2 v);
P_INLINE f32  vec_dot(vec2 v0, vec2 v1);
P_INLINE f32  vec_distance(vec2 v0, vec2 v1);
P_INLINE vec2 vec_line_middle(vec2 v0, vec2 v1);
P_INLINE vec2 vec_rotate(vec2 v, f32 t);
P_INLINE vec2 vec_rotate_around_p(vec2 v, f32 t, vec2 o);

P_INLINE vec2 vec_line_normal(vec2 v0, vec2 v1, bool opposite = false);
P_INLINE vec2 vec_normal(vec2 v, bool opposite = false);

/*------------------*/
/*      vec2i       */
/*------------------*/
P_INLINE vec2i make_vec2i(i32 e0, i32 e1);
P_INLINE vec2i make_vec2i(i32 e = 0);

vec2i operator-(vec2i v);
vec2i operator+(const vec2i v0, const vec2i v1);
vec2i operator-(const vec2i v0, const vec2i v1);
vec2i operator*(const vec2i v0, const vec2i v1);
vec2i operator/(const vec2i v0, const vec2i v1);
vec2i operator+(const vec2i v, const i32 n);
vec2i operator-(const vec2i v, const i32 n);
vec2i operator*(const vec2i v, const i32 n);
vec2i operator/(const vec2i v, const i32 n);
vec2i &operator+=(vec2i &v0, const vec2i v1);
vec2i &operator-=(vec2i &v0, const vec2i v1);
vec2i &operator*=(vec2i &v0, const vec2i v1);
vec2i &operator/=(vec2i &v0, const vec2i v1);
vec2i &operator+=(vec2i &v, const i32 n);
vec2i &operator-=(vec2i &v, const i32 n);
vec2i &operator*=(vec2i &v, const i32 n);
vec2i &operator/=(vec2i &v, const i32 n);
bool operator==(const vec2i v0, const vec2i v1);
bool operator!=(const vec2i v0, const vec2i v1);

/*------------------*/
/*       vec3       */
/*------------------*/
P_INLINE vec3 make_vec3(f32 e0, f32 e1, f32 e2);
P_INLINE vec3 make_vec3(f32 e = 0.0f);
P_INLINE vec3 make_vec3(vec2 v, f32 n = 0.0f);

vec3 operator-(vec3 v);
vec3 operator+(const vec3 v0, const vec3 v1);
vec3 operator-(const vec3 v0, const vec3 v1);
vec3 operator*(const vec3 v0, const vec3 v1);
vec3 operator/(const vec3 v0, const vec3 v1);
vec3 operator+(const vec3 v, const f32 n);
vec3 operator-(const vec3 v, const f32 n);
vec3 operator*(const vec3 v, const f32 n);
vec3 operator/(const vec3 v, const f32 n);
vec3 &operator+=(vec3 &v0, const vec3 v1);
vec3 &operator-=(vec3 &v0, const vec3 v1);
vec3 &operator*=(vec3 &v0, const vec3 v1);
vec3 &operator/=(vec3 &v0, const vec3 v1);
vec3 &operator+=(vec3 &v, const f32 n);
vec3 &operator-=(vec3 &v, const f32 n);
vec3 &operator*=(vec3 &v, const f32 n);
vec3 &operator/=(vec3 &v, const f32 n);
bool operator==(const vec3 v0, const vec3 v1);
bool operator!=(const vec3 v0, const vec3 v1);

P_INLINE vec3 vec_lerp(vec3 v0, vec3 v1, f32 t);
P_INLINE f32  vec_distance(vec3 v0, vec3 v1);
P_INLINE f32  vec_length(vec3 v);
P_INLINE f32  vec_dot(vec3 v0, vec3 v1);
P_INLINE vec3 vec_normalize(vec3 v);
P_INLINE vec3 vec_cross(vec3 v0, vec3 v1);
P_INLINE vec3 vec_vector(vec3 v0, vec3 v1);
P_INLINE vec3 vec_spherical(vec3 v);
P_INLINE vec2 vec_spherical_angles(vec3 v);
P_INLINE vec3 vec_spherical_from_p_to_p(vec3 v0, vec3 v1);
P_INLINE vec3 vec_from_spherical(f32 p, f32 angle_theta, f32 angle_phi);
P_INLINE vec3 vec_from_spherical(vec3 spherical);
P_INLINE vec3 vec_from_spherical(f32 angle_theta, f32 angle_phi);
P_INLINE vec3 vec_from_spherical(vec2 spherical);

/*------------------*/
/*      vec3i       */
/*------------------*/
P_INLINE vec3i make_vec3i(i32 e0, i32 e1, i32 e2);
P_INLINE vec3i make_vec3i(i32 e = 0.0f);

vec3i operator-(vec3i v);
vec3i operator+(const vec3i v0, const vec3i v1);
vec3i operator-(const vec3i v0, const vec3i v1);
vec3i operator*(const vec3i v0, const vec3i v1);
vec3i operator/(const vec3i v0, const vec3i v1);
vec3i operator+(const vec3i v, const i32 n);
vec3i operator-(const vec3i v, const i32 n);
vec3i operator*(const vec3i v, const i32 n);
vec3i operator/(const vec3i v, const i32 n);
vec3i &operator+=(vec3i &v0, const vec3i v1);
vec3i &operator-=(vec3i &v0, const vec3i v1);
vec3i &operator*=(vec3i &v0, const vec3i v1);
vec3i &operator/=(vec3i &v0, const vec3i v1);
vec3i &operator+=(vec3i &v, const i32 n);
vec3i &operator-=(vec3i &v, const i32 n);
vec3i &operator*=(vec3i &v, const i32 n);
vec3i &operator/=(vec3i &v, const i32 n);
bool operator==(const vec3i v0, const vec3i v1);
bool operator!=(const vec3i v0, const vec3i v1);

/*------------------*/
/*       vec4       */
/*------------------*/
P_INLINE vec4 make_vec4(f32 e0, f32 e1, f32 e2, f32 e3);
P_INLINE vec4 make_vec4(f32 e = 0.0f);
P_INLINE vec4 make_vec4(vec3 v, f32 e = 0.0f);

vec4 operator-(vec4 v);
vec4 operator+(const vec4 v0, const vec4 v1);
vec4 operator-(const vec4 v0, const vec4 v1);
vec4 operator*(const vec4 v0, const vec4 v1);
vec4 operator/(const vec4 v0, const vec4 v1);
vec4 operator+(const vec4 v, const f32 n);
vec4 operator-(const vec4 v, const f32 n);
vec4 operator*(const vec4 v, const f32 n);
vec4 operator/(const vec4 v, const f32 n);
vec4 &operator+=(vec4 &v0, const vec4 v1);
vec4 &operator-=(vec4 &v0, const vec4 v1);
vec4 &operator*=(vec4 &v0, const vec4 v1);
vec4 &operator/=(vec4 &v0, const vec4 v1);
vec4 &operator+=(vec4 &v, const f32 n);
vec4 &operator-=(vec4 &v, const f32 n);
vec4 &operator*=(vec4 &v, const f32 n);
vec4 &operator/=(vec4 &v, const f32 n);
bool operator==(const vec4 v0, const vec4 v1);
bool operator!=(const vec4 v0, const vec4 v1);

P_INLINE vec4 vec_lerp(vec4 v0, vec4 v1, f32 t);

/*------------------*/
/*      mat4x4      */
/*------------------*/
mat4x4 operator*(const mat4x4 m0, const mat4x4 m1);
mat4x4 &operator*=(mat4x4 &m0, const mat4x4 m1);

vec3 operator*(const mat4x4 mat, const vec3 vec);
vec4 operator*(const mat4x4 mat, const vec4 vec);

P_INLINE mat4x4 mat4x4_identity(void);
P_INLINE mat4x4 mat4x4_mult(mat4x4 m0, mat4x4 m1);
P_INLINE mat4x4 mat4x4_flip(mat4x4 mat);
P_INLINE mat4x4 mat4x4_translate(f32 x, f32 y, f32 z);
P_INLINE mat4x4 mat4x4_scale(f32 x, f32 y, f32 z);
P_INLINE mat4x4 mat4x4_xaxis_rotate(f32 t);
P_INLINE mat4x4 mat4x4_yaxis_rotate(f32 t);
P_INLINE mat4x4 mat4x4_zaxis_rotate(f32 t);
P_INLINE mat4x4 mat4x4_rotate(f32 x_rot, f32 y_rot, f32 z_rot);
P_INLINE mat4x4 mat4x4_model(vec3 position, vec3 rotation, vec3 scale);
P_INLINE mat4x4 mat4x4_orthographic(f32 _left, f32 _bottom, f32 _right, f32 _top, f32 _near, f32 _far);
P_INLINE mat4x4 mat4x4_perspective(f32 _fov, f32 _aspect, f32 _near, f32 _far);
P_INLINE mat4x4 mat4x4_look_at(vec3 _from, vec3 _to, vec3 _up);

/*------------------*/
/*    Collision     */
/*------------------*/
bool collision_point_rect(vec2 point, vec2 p, vec2 size);
bool collision_aabb(vec2 p0, vec2 size0, vec2 p1, vec2 size1);
bool collision_O_vs_point(f32 cr, vec2 cp, vec2 point);
bool collision_O_vs_O(f32 cr0, vec2 p0, f32 cr1, vec2 p1);
bool collision_sat(vec2 *poly0_verts, i32 poly0_vert_count, vec2 poly0_pos, vec2 *poly1_verts, i32 poly1_vert_count, vec2 poly1_pos);
bool collision_sat_O(vec2 *poly_verts, i32 poly_vert_count, vec2 poly_pos, f32 cr, vec2 cp);
bool collision_sat_point(vec2 *poly_verts, i32 poly_vert_count, vec2 poly_pos, vec2 point);

/*--------------*/
/*    Random    */
/*--------------*/
typedef u64 random_seed;

// NOTE, TODO: temp
P_INLINE u64 rand_u64(random_seed *rand);
P_INLINE u32 rand_u32(random_seed *rand);
P_INLINE i32 rand_i32(random_seed *rand);
P_INLINE f32 rand_f32(random_seed *rand);
P_INLINE u64 rand_u64_in_range(random_seed *seed, u64 min, u64 max);
P_INLINE u32 rand_u32_in_range(random_seed *seed, u32 min, u32 max);
P_INLINE i32 rand_i32_in_range(random_seed *seed, i32 min, i32 max);
P_INLINE f32 rand_f32_in_range(random_seed *seed, f32 min, f32 max);

#ifdef P_MATH_IMPLEMENTATION

#include <assert.h>
#define _p_assert(assertion) assert(assertion);
#define _p_assert_range(min,max) _p_assert((min)<(max))
#define _p_size_array(array) (sizeof(array)/sizeof(array[0]))

/*------------------*/
/*                  */
/*------------------*/
P_INLINE f32 prec(f32 v, u32 p) {
    f32 m = power(10.0f, (f32)p);
    f32 result = m * v;
    result = roundf32(result) / m;
    return result;
}

P_INLINE f32 rad_to_deg(f32 rad) {
    f32 deg = rad * (180.0f / PI32);
    return deg;
}

P_INLINE f32 deg_to_rad(f32 deg) {
    f32 rad = deg * (PI32 / 180.0f);
    return rad;
}

P_INLINE f32 square_root(f32 v) {
    f32 result = sqrtf(v);
    return result;
}

P_INLINE f32 power(f32 v, f32 p) {
    f32 result = powf(v, p);
    return result;
}

P_INLINE f32 roundf32(f32 v) {
    f32 result = (f32)roundf32_to_i32(v);
    return result;
}

P_INLINE i32 roundf32_to_i32(f32 v) {
    i32 result = (i32)(v + 0.5f);
    return result;
}

P_INLINE f32 lerp(f32 v0, f32 v1, f32 t) {
    f32 result = ((1.0f - t) * v0) + (t * v1);
    return result;
}

P_INLINE f32 absolute(f32 v) {
    f32 result = fabsf(v);
    return result;
}

P_INLINE i32 absolute(i32 v) {
    i32 result = (i32)absolute((f32)v);
    return result;
}

/*------------------*/
/*       vec2       */
/*------------------*/
P_INLINE 
vec2 make_vec2(f32 e0, f32 e1) {
    vec2 result = {e0, e1};
    return result;
}

P_INLINE 
vec2 make_vec2(f32 e) {
    vec2 result = {e, e};
    return result;
}

vec2 operator-(vec2 v) {
    vec2 result = {-v.x, -v.y};
    return result;
}

vec2 operator+(const vec2 v0, const vec2 v1) {
    vec2 result = {v0.x + v1.x, v0.y + v1.y};
    return result;
}

vec2 operator-(const vec2 v0, const vec2 v1) {
    vec2 result = {v0.x - v1.x, v0.y - v1.y};
    return result;
}

vec2 operator*(const vec2 v0, const vec2 v1) {
    vec2 result = {v0.x * v1.x, v0.y * v1.y};
    return result;
}

vec2 operator/(const vec2 v0, const vec2 v1) {
    vec2 result = {v0.x / v1.x, v0.y / v1.y};
    return result;
}

vec2 operator+(const vec2 v, const f32 n) {
    vec2 result = {v.x + n, v.y + n};
    return result;
}

vec2 operator-(const vec2 v, const f32 n) {
    vec2 result = {v.x - n, v.y - n};
    return result;
}

vec2 operator*(const vec2 v, const f32 n) {
    vec2 result = {v.x * n, v.y * n};
    return result;
}

vec2 operator/(const vec2 v, const f32 n) {
    vec2 result = {v.x / n, v.y / n};
    return result;
}

vec2 &operator+=(vec2 &v0, const vec2 v1) {
    v0 = v0 + v1;
    return v0;
}

vec2 &operator-=(vec2 &v0, const vec2 v1) {
    v0 = v0 - v1;
    return v0;
}

vec2 &operator*=(vec2 &v0, const vec2 v1) {
    v0 = v0 * v1;
    return v0;
}

vec2 &operator/=(vec2 &v0, const vec2 v1) {
    v0 = v0 / v1;
    return v0;
}

vec2 &operator+=(vec2 &v, const f32 n) {
    v = v + n;
    return v;
}

vec2 &operator-=(vec2 &v, const f32 n) {
    v = v - n;
    return v;
}

vec2 &operator*=(vec2 &v, const f32 n) {
    v = v * n;
    return v;
}

vec2 &operator/=(vec2 &v, const f32 n) {
    v = v / n;
    return v;
}

bool operator==(const vec2 v0, const vec2 v1) {
    bool result = v0.x == v1.x && v0.y == v1.y;
    return result;
}

bool operator!=(const vec2 v0, const vec2 v1) {
    bool result = !(v0 == v1);
    return result;
}

P_INLINE 
vec2 vec_prec(vec2 v, u32 p) {
    vec2 result = {
        prec(v.x, p),
        prec(v.y, p)
    };
    return result;
}

P_INLINE 
vec2 vec_lerp(vec2 v0, vec2 v1, f32 t) {
    vec2 result;
    result.x = lerp(v0.x, v1.x, t);
    result.y = lerp(v0.y, v1.y, t);
    return result;
}

P_INLINE
f32 vec_length(vec2 v) {
    f32 dot = vec_dot(v, v);
    f32 result = sqrtf(dot);
    return result;
}

P_INLINE
f32 vec_atan2(vec2 v) {
    f32 result = atan2f(v.y, v.x);
    return result;
}

P_INLINE
vec2 vec_vector(vec2 v0, vec2 v1) {
    vec2 result = {v1.x - v0.x, v1.y - v0.y};
    return result;
}

P_INLINE
vec2 vec_from_angle(f32 a) {
    vec2 result;
    result.x = cosf(a);
    result.y = sinf(a);
    return result;
}

P_INLINE // TODO: 
vec2 vec_normalize(vec2 v) {
    f32 a = vec_atan2(v);
    vec2 result = vec_from_angle(a);
    return result;
}

P_INLINE
f32 vec_dot(vec2 v0, vec2 v1) {
    f32 result = (v0.x * v1.x) + (v0.y * v1.y);
    return result;
}

P_INLINE
f32 vec_distance(vec2 v0, vec2 v1) {
    f32 result = square_root(square(v1.x - v0.x) + square(v1.y - v0.y));
    return result;
}

P_INLINE
vec2 vec_line_middle(vec2 v0, vec2 v1) {
    vec2 result;
    result.x = (v0.x + v1.x) * 0.5f;
    result.y = (v0.y + v1.y) * 0.5f;
    return result;
}

P_INLINE 
vec2 vec_rotate(vec2 v, f32 t) {
    vec2 result = {
        cosf(t) * v.x - sinf(t) * v.y,
        cosf(t) * v.y + sinf(t) * v.x,
    };
    return result;
}

P_INLINE 
vec2 vec_rotate_around_p(vec2 v, f32 t, vec2 o) {
    vec2 result = vec_rotate(v - o, t) + o;
    return result;
}

// NOTE,TODO: stupid ig
P_INLINE
vec2 vec_line_normal(vec2 v0, vec2 v1, bool opposite) {
    vec2 line_dir = vec_normalize(v1 - v0);
    f32  line_angle = vec_atan2(line_dir);
    vec2 result = vec_from_angle(line_angle - (PI32 / 2.0f));
    if(opposite)
        result = -result;
    return result;
}

P_INLINE
vec2 vec_normal(vec2 v, b32 opposite) {
    f32 a = vec_atan2(v);
    vec2 result = vec_from_angle(a + (PI32 * 0.5f));
    if(opposite)
        result = -result;
    return result;
}

/*------------------*/
/*      vec2i       */
/*------------------*/
P_INLINE
vec2i make_vec2i(i32 e0, i32 e1) {
    vec2i result = {e0, e1};
    return result;
}

P_INLINE
vec2i make_vec2i(i32 e) {
    vec2i result = {e, e};
    return result;
}

vec2i operator-(vec2i v) {
    vec2i result = {-v.x, -v.y};
    return result;
}

vec2i operator+(const vec2i v0, const vec2i v1) {
    vec2i result = {v0.x + v1.x, v0.y + v1.y};
    return result;
}

vec2i operator-(const vec2i v0, const vec2i v1) {
    vec2i result = {v0.x - v1.x, v0.y - v1.y};
    return result;
}

vec2i operator*(const vec2i v0, const vec2i v1) {
    vec2i result = {v0.x * v1.x, v0.y * v1.y};
    return result;
}

vec2i operator/(const vec2i v0, const vec2i v1) {
    vec2i result = {v0.x / v1.x, v0.y / v1.y};
    return result;
}

vec2i operator+(const vec2i v, const i32 n)
{
    vec2i result = {v.x + n, v.y + n};
    return result;
}

vec2i operator-(const vec2i v, const i32 n) {
    vec2i result = {v.x - n, v.y - n};
    return result;
}

vec2i operator*(const vec2i v, const i32 n) {
    vec2i result = {v.x * n, v.y * n};
    return result;
}

vec2i operator/(const vec2i v, const i32 n) {
    vec2i result = {v.x / n, v.y / n};
    return result;
}

vec2i &operator+=(vec2i &v0, const vec2i v1) {
    v0 = v0 + v1;
    return v0;
}

vec2i &operator-=(vec2i &v0, const vec2i v1) {
    v0 = v0 - v1;
    return v0;
}

vec2i &operator*=(vec2i &v0, const vec2i v1) {
    v0 = v0 * v1;
    return v0;
}

vec2i &operator/=(vec2i &v0, const vec2i v1) {
    v0 = v0 / v1;
    return v0;
}

vec2i &operator+=(vec2i &v, const i32 n) {
    v = v + n;
    return v;
}

vec2i &operator-=(vec2i &v, const i32 n) {
    v = v - n;
    return v;
}

vec2i &operator*=(vec2i &v, const i32 n) {
    v = v * n;
    return v;
}

vec2i &operator/=(vec2i &v, const i32 n) {
    v = v / n;
    return v;
}

bool operator==(const vec2i v0, const vec2i v1) {
    bool result = v0.x == v1.x && v0.y == v1.y;
    return result;
}

bool operator!=(const vec2i v0, const vec2i v1) {
    bool result = !(v0 == v1);
    return result;
}

/*------------------*/
/*       vec3       */
/*------------------*/
P_INLINE 
vec3 make_vec3(f32 e0, f32 e1, f32 e2) {
    vec3 result = {e0, e1, e2};
    return result;
}

P_INLINE 
vec3 make_vec3(f32 e) {
    vec3 result = {e, e, e};
    return result;
}

P_INLINE 
vec3 make_vec3(vec2 v, f32 n) {
    vec3 result = {v.x, v.y, n};
    return result;
}

vec3 operator-(vec3 v) {
    vec3 result = {-v.x, -v.y, -v.z};
    return result;
}

vec3 operator+(const vec3 v0, const vec3 v1) {
    vec3 result = {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
    return result;
}

vec3 operator-(const vec3 v0, const vec3 v1) {
    vec3 result = {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
    return result;
}

vec3 operator*(const vec3 v0, const vec3 v1) {
    vec3 result = {v0.x * v1.x, v0.y * v1.y, v0.z * v1.z};
    return result;
}

vec3 operator/(const vec3 v0, const vec3 v1) {
    vec3 result = {v0.x / v1.x, v0.y / v1.y, v0.z / v1.z};
    return result;
}

vec3 operator+(const vec3 v, const f32 n) {
    vec3 result = {v.x + n, v.y + n, v.z + n};
    return result;
}

vec3 operator-(const vec3 v, const f32 n) {
    vec3 result = {v.x - n, v.y - n, v.z - n};
    return result;
}

vec3 operator*(const vec3 v, const f32 n) {
    vec3 result = {v.x * n, v.y * n, v.z * n};
    return result;
}

vec3 operator/(const vec3 v, const f32 n) {
    vec3 result = {v.x / n, v.y / n, v.z / n};
    return result;
}

vec3 &operator+=(vec3 &v0, const vec3 v1) {
    v0 = v0 + v1;
    return v0;
}

vec3 &operator-=(vec3 &v0, const vec3 v1) {
    v0 = v0 - v1;
    return v0;
}

vec3 &operator*=(vec3 &v0, const vec3 v1) {
    v0 = v0 * v1;
    return v0;
}

vec3 &operator/=(vec3 &v0, const vec3 v1) {
    v0 = v0 / v1;
    return v0;
}

vec3 &operator+=(vec3 &v, const f32 n) {
    v = v + n;
    return v;
}

vec3 &operator-=(vec3 &v, const f32 n) {
    v = v - n;
    return v;
}

vec3 &operator*=(vec3 &v, const f32 n) {
    v = v * n;
    return v;
}

vec3 &operator/=(vec3 &v, const f32 n) {
    v = v / n;
    return v;
}

bool operator==(const vec3 v0, const vec3 v1) {
    bool result = v0.x == v1.x && v0.y == v1.y && v0.z == v1.z;
    return result;
}

bool operator!=(const vec3 v0, const vec3 v1) {
    bool result = !(v0 == v1);
    return result;
}

P_INLINE 
vec3 vec_lerp(vec3 v0, vec3 v1, f32 t) {
    vec3 result;
    result.x = lerp(v0.x, v1.x, t);
    result.y = lerp(v0.y, v1.y, t);
    result.z = lerp(v0.z, v1.z, t);
    return result;
}

P_INLINE 
f32 vec_distance(vec3 v0, vec3 v1) {
    f32 result = square_root(square(v1.x - v0.x) + 
                             square(v1.y - v0.y) + 
                             square(v1.z - v0.z));
    return result;
}

P_INLINE
f32 vec_length(vec3 v) {
    f32 dot = vec_dot(v, v);
    f32 v_len = sqrtf(dot);
    return v_len;
}

P_INLINE
f32 vec_dot(vec3 v0, vec3 v1) {
    f32 result = (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z);
    return result;
}

P_INLINE
vec3 vec_normalize(vec3 v) {
    f32 v_len = vec_length(v);
    if(v_len == 0) 
        return make_vec3();
    
    vec3 result;
    result.x = v.x / v_len;
    result.y = v.y / v_len;
    result.z = v.z / v_len;
    return result;
}

P_INLINE
vec3 vec_cross(vec3 v0, vec3 v1) {
    vec3 result;
    result.x = (v0.y * v1.z) - (v0.z * v1.y);
    result.y = (v0.z * v1.x) - (v0.x * v1.z);
    result.z = (v0.x * v1.y) - (v0.y * v1.x);
    return result;
}

P_INLINE
vec3 vec_vector(vec3 v0, vec3 v1) {
    vec3 result = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
    return result;
}

P_INLINE
vec3 vec_spherical(vec3 v) {
    vec3 result;
    result.p = vec_length(v);
    
    // TODO: idk idk
    result.theta = (f32)atan2(v.z, v.x);
    if(result.theta < -PI32)
        result.theta = PI32 - (result.theta + PI32);
    else if(result.theta > PI32)
        result.theta = -PI32 + (result.theta - PI32);
    
    result.phi = acosf(v.y / result.p);
    return result;
}

P_INLINE
vec2 vec_spherical_angles(vec3 v) { 
    v = vec_normalize(v);
    vec3 spherical = vec_spherical(v);
    vec2 result = make_vec2(spherical.theta, spherical.phi);
    return result;
}

P_INLINE
vec3 vec_spherical_from_p_to_p(vec3 p0, vec3 p1) {
    vec3 vector = vec_vector(p0, p1);
    vec3 result = vec_spherical(vector);
    return result;
}

P_INLINE
vec3 vec_from_spherical(f32 p, f32 angle_theta, f32 angle_phi) {
    vec3 result;
    result.x = p * cosf(angle_theta) * sinf(angle_phi);
    result.y = p * cosf(angle_phi);
    result.z = p * sinf(angle_theta) * sinf(angle_phi);
    return result;
}

P_INLINE
vec3 vec_from_spherical(vec3 spherical) {
    vec3 result = vec_from_spherical(spherical.p,
                                     spherical.theta, 
                                     spherical.phi);
    return result;
}

P_INLINE
vec3 vec_from_spherical(f32 angle_theta, f32 angle_phi) {
    vec3 result = vec_from_spherical(1.0f, angle_theta, angle_phi);
    return result;
}


P_INLINE
vec3 vec3_from_spherical(vec2 spherical_angles) {
    vec3 result = vec_from_spherical(1.0f, spherical_angles.theta, 
                                     spherical_angles.phi);
    return result;
}

/*------------------*/
/*      vec3i       */
/*------------------*/
P_INLINE 
vec3i make_vec3i(i32 e0, i32 e1, i32 e2) {
    vec3i result = {e0, e1, e2};
    return result;
}

P_INLINE 
vec3i make_vec3i(i32 e) {
    vec3i result = {e, e, e};
    return result;
}

vec3i operator-(vec3i v) {
    vec3i result = {-v.x, -v.y, -v.z};
    return result;
}

vec3i operator+(const vec3i v0, const vec3i v1) {
    vec3i result = {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
    return result;
}

vec3i operator-(const vec3i v0, const vec3i v1) {
    vec3i result = {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
    return result;
}

vec3i operator*(const vec3i v0, const vec3i v1) {
    vec3i result = {v0.x * v1.x, v0.y * v1.y, v0.z * v1.z};
    return result;
}

vec3i operator/(const vec3i v0, const vec3i v1) {
    vec3i result = {v0.x / v1.x, v0.y / v1.y, v0.z / v1.z};
    return result;
}

vec3i operator+(const vec3i v, const i32 n) {
    vec3i result = {v.x + n, v.y + n, v.z + n};
    return result;
}

vec3i operator-(const vec3i v, const i32 n) {
    vec3i result = {v.x - n, v.y - n, v.z - n};
    return result;
}

vec3i operator*(const vec3i v, const i32 n) {
    vec3i result = {v.x * n, v.y * n, v.z * n};
    return result;
}

vec3i operator/(const vec3i v, const i32 n) {
    vec3i result = {v.x / n, v.y / n, v.z / n};
    return result;
}

vec3i &operator+=(vec3i &v0, const vec3i v1) {
    v0 = v0 + v1;
    return v0;
}

vec3i &operator-=(vec3i &v0, const vec3i v1) {
    v0 = v0 - v1;
    return v0;
}

vec3i &operator*=(vec3i &v0, const vec3i v1) {
    v0 = v0 * v1;
    return v0;
}

vec3i &operator/=(vec3i &v0, const vec3i v1) {
    v0 = v0 / v1;
    return v0;
}

vec3i &operator+=(vec3i &v, const i32 n) {
    v = v + n;
    return v;
}

vec3i &operator-=(vec3i &v, const i32 n) {
    v = v - n;
    return v;
}

vec3i &operator*=(vec3i &v, const i32 n) {
    v = v * n;
    return v;
}

vec3i &operator/=(vec3i &v, const i32 n) {
    v = v / n;
    return v;
}

bool operator==(const vec3i v0, const vec3i v1) {
    bool result = v0.x == v1.x && v0.y == v1.y && v0.z == v1.z;
    return result;
}

bool operator!=(const vec3i v0, const vec3i v1) {
    bool result = !(v0 == v1);
    return result;
}

/*------------------*/
/*       vec4       */
/*------------------*/
P_INLINE 
vec4 make_vec4(f32 e0, f32 e1, f32 e2, f32 e3) {
    vec4 result = {e0, e1, e2, e3};
    return result;
}

P_INLINE 
vec4 make_vec4(f32 e) {
    vec4 result = {e, e, e, e};
    return result;
}

P_INLINE 
vec4 make_vec4(vec3 v, f32 e) {
    vec4 result = {v.x, v.y, v.z, e};
    return result;
}

vec4 operator-(vec4 v) {
    vec4 result = {-v.x, -v.y, -v.z, -v.w};
    return result;
}

vec4 operator+(const vec4 v0, const vec4 v1) {
    vec4 result = {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w};
    return result;
}

vec4 operator-(const vec4 v0, const vec4 v1) {
    vec4 result = {v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w};
    return result;
}

vec4 operator*(const vec4 v0, const vec4 v1) {
    vec4 result = {v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w};
    return result;
}

vec4 operator/(const vec4 v0, const vec4 v1) {
    vec4 result = {v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w};
    return result;
}

vec4 operator+(const vec4 v, const f32 n) {
    vec4 result = {v.x + n, v.y + n, v.z + n, v.w + n};
    return result;
}

vec4 operator-(const vec4 v, const f32 n) {
    vec4 result = {v.x - n, v.y - n, v.z - n, v.w - n};
    return result;
}

vec4 operator*(const vec4 v, const f32 n) {
    vec4 result = {v.x * n, v.y * n, v.z * n, v.w * n};
    return result;
}

vec4 operator/(const vec4 v, const f32 n) {
    vec4 result = {v.x / n, v.y / n, v.z / n, v.w / n};
    return result;
}

vec4 &operator+=(vec4 &v0, const vec4 v1) {
    v0 = v0 + v1;
    return v0;
}

vec4 &operator-=(vec4 &v0, const vec4 v1) {
    v0 = v0 - v1;
    return v0;
}

vec4 &operator*=(vec4 &v0, const vec4 v1) {
    v0 = v0 * v1;
    return v0;
}

vec4 &operator/=(vec4 &v0, const vec4 v1) {
    v0 = v0 / v1;
    return v0;
}

vec4 &operator+=(vec4 &v, const f32 n) {
    v = v + n;
    return v;
}

vec4 &operator-=(vec4 &v, const f32 n) {
    v = v - n;
    return v;
}

vec4 &operator*=(vec4 &v, const f32 n) {
    v = v * n;
    return v;
}

vec4 &operator/=(vec4 &v, const f32 n) {
    v = v / n;
    return v;
}

bool operator==(const vec4 v0, const vec4 v1) {
    bool result = v0.x == v1.x && v0.y == v1.y && v0.z == v1.z && v0.w == v1.w;
    return result;
}

bool operator!=(const vec4 v0, const vec4 v1) {
    bool result = !(v0 == v1);
    return result;
}

P_INLINE 
vec4 vec_lerp(vec4 v0, vec4 v1, f32 t) {
    vec4 result = {
        lerp(v0.e[0], v1.e[0], t),
        lerp(v0.e[1], v1.e[1], t),
        lerp(v0.e[2], v1.e[2], t),
        lerp(v0.e[3], v1.e[3], t),
    };
    return result;
}

/*------------------*/
/*      mat4x4      */
/*------------------*/
mat4x4 operator*(const mat4x4 m0, const mat4x4 m1) {
    mat4x4 result;
    for(i32 i = 0; i < 4; ++i) {
        for(i32 j = 0; j < 4; ++j) {
            result.m[i][j]
                = (m0.m[i][0] * m1.m[0][j])
                + (m0.m[i][1] * m1.m[1][j])
                + (m0.m[i][2] * m1.m[2][j])
                + (m0.m[i][3] * m1.m[3][j]);
        }
    }
    return result;
}

mat4x4 &operator*=(mat4x4 &m0, const mat4x4 m1) {
    m0 = m0 * m1;
    return m0;
}

vec3 operator*(const mat4x4 mat, const vec3 vec) {
    vec3 result = (mat * make_vec4(vec.x, vec.y, vec.z, 1.0f)).xyz;
    return result;
}

vec4 operator*(const mat4x4 mat, const vec4 vec)
{
    vec4 result = {};
    for(i32 i = 0; i < 4; ++i) {
        for(i32 j = 0; j < 1; ++j) {
            result.e[i]
                = (mat.m[0][i] * vec.e[0])
                + (mat.m[1][i] * vec.e[1])
                + (mat.m[2][i] * vec.e[2])
                + (mat.m[3][i] * vec.e[3]);
        }
    }
    return result;
}

P_INLINE
mat4x4 mat4x4_identity(void) {
    mat4x4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_mult(mat4x4 m0, mat4x4 m1) {
    mat4x4 result = m0 * m1;
    return result;
}

P_INLINE
mat4x4 mat4x4_flip(mat4x4 mat) {
    mat4x4 result = {
        mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
        mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
        mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
        mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3],
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_translate(f32 x, f32 y, f32 z) {
    mat4x4 result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1,
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_scale(f32 x, f32 y, f32 z) {
    mat4x4 result = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1,
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_xaxis_rotate(f32 t) {
    f32 a =  cosf(t);
    f32 b = -sinf(t);
    f32 c =  sinf(t);
    f32 d =  cosf(t);
    mat4x4 result = {
        1, 0, 0, 0,
        0, a, b, 0,
        0, c, d, 0,
        0, 0, 0, 1,
    };
    return result; 
}

P_INLINE
mat4x4 mat4x4_yaxis_rotate(f32 t) {
    f32 a =  cosf(t);
    f32 b =  sinf(t);
    f32 c = -sinf(t);
    f32 d =  cosf(t);
    mat4x4 result = {
        a, 0, b, 0,
        0, 1, 0, 0,
        c, 0, d, 0,
        0, 0, 0, 1,
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_zaxis_rotate(f32 t) {
    f32 a =  cosf(t);
    f32 b = -sinf(t);
    f32 c =  sinf(t);
    f32 d =  cosf(t);
    mat4x4 result = {
        a, b, 0, 0,
        c, d, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    return result;
}

P_INLINE
mat4x4 mat4x4_rotate(f32 x_rot, f32 y_rot, f32 z_rot) {
    mat4x4 xaxis = mat4x4_identity();
    mat4x4 yaxis = mat4x4_identity();
    mat4x4 zaxis = mat4x4_identity();
    
    if(x_rot != 0.0f) { xaxis = mat4x4_xaxis_rotate(x_rot); }
    if(y_rot != 0.0f) { yaxis = mat4x4_yaxis_rotate(y_rot); }
    if(z_rot != 0.0f) { zaxis = mat4x4_zaxis_rotate(z_rot); }
    
    mat4x4 result;
    result = mat4x4_mult(xaxis,  yaxis);
    result = mat4x4_mult(result, zaxis);
    return result;
}

P_INLINE
mat4x4 mat4x4_model(vec3 position, vec3 rotation, vec3 scale) {
    mat4x4 rotate_m    = mat4x4_rotate(rotation.x, rotation.y, rotation.z);
    mat4x4 translate_m = mat4x4_translate(position.x, position.y, position.z);
    mat4x4 scale_m     = mat4x4_scale(scale.x, scale.y, scale.z);
    
    mat4x4 model;
    model = mat4x4_mult(rotate_m, scale_m);
    model = mat4x4_mult(model, translate_m);
    return model;
}

P_INLINE
mat4x4 mat4x4_orthographic(f32 _left, f32 _bottom, f32 _right, f32 _top, f32 _near, f32 _far) {
    f32 m00 = 2.0f / (_right - _left);
    f32 m11 = 2.0f / (_top - _bottom); 
    f32 m22 = 2.0f / (_far - _near);
    f32 m03 = -((_right + _left) / (_right - _left));
    f32 m13 = -((_top + _bottom) / (_top - _bottom));
    f32 m23 = -((_far + _near) / (_far - _near));
    mat4x4 projection = {
        m00,  0.0f, 0.0f, 0.0f,
        0.0f, m11,  0.0f, 0.0f,
        0.0f, 0.0f, m22,  0.0f,
        m03,  m13,  m23,  1.0f,
    };
    return projection;
}

P_INLINE // TODO: check if near and far are correct
mat4x4 mat4x4_perspective(f32 _fov, f32 _aspect, f32 _near, f32 _far) {
    f32 tan = tanf(deg_to_rad(_fov) * 0.5f);
    f32 m00 = 1.0f / (_aspect * tan);
    f32 m11 = 1.0f / tan;
    f32 m22 = -(_far + _near) / (_far - _near);
    f32 m23 = (-2.0f * _far * _near) / (_far - _near);
    mat4x4 projection = {
        m00,  0.0f, 0.0f, 0.0f,
        0.0f, m11,  0.0f, 0.0f,
        0.0f, 0.0f, m22, -1.0f,
        0.0f, 0.0f, m23,  0.0f,
    };
    return projection;
}

P_INLINE
mat4x4 mat4x4_look_at(vec3 _from, vec3 _to, vec3 _up) {
    vec3 forward = vec_normalize(_to - _from);
    vec3 side = vec_normalize(vec_cross(forward, _up));
    vec3 up =  vec_cross(side, forward);
    f32 m03 = -vec_dot(side, _from);
    f32 m13 = -vec_dot(up, _from);
    f32 m23 =  vec_dot(forward, _from);
    mat4x4 view = {
        side.e[0], up.e[0], -forward.e[0], 0.0f,
        side.e[1], up.e[1], -forward.e[1], 0.0f,
        side.e[2], up.e[2], -forward.e[2], 0.0f,
        m03,       m13,      m23,          1.0f,
    };
    return view;
}

/*------------------*/
/*    Collision     */
/*------------------*/
// TODO, NOTE: 

#define P_SAT_AXIS_PREC 4
#define P_SAT_MIN_VERT 3
#define P_SAT_MAX_VERT 128 

struct _Poly {
    vec2 *verts;
    i32 vert_count;
};

bool collision_point_rect(vec2 point, vec2 p, vec2 size) {
    bool result = (is_in_bounds(point.x, p.x, p.x + size.x) &&
                   is_in_bounds(point.y, p.y, p.y + size.y));
    return result;
}

// NOTE: center based
bool collision_aabb(vec2 p0, vec2 size0, vec2 p1, vec2 size1) {
    vec2 p = p0;
    vec2 p_mink = p1;
    vec2 half_size_mink = (size0 + size1) * make_vec2(0.5f);
    bool result = (is_in_bounds(p0.x, p_mink.x - half_size_mink.x, p_mink.x + half_size_mink.x) &&
                   is_in_bounds(p0.y, p_mink.y - half_size_mink.y, p_mink.y + half_size_mink.y));
    return result;
}

bool collision_O_vs_point(f32 cr, vec2 cp, vec2 point) {
    f32 dist = vec_distance(cp, point);
    b32 result = (dist <= cr);
    return result;
}

bool collision_O_vs_O(f32 cr0, vec2 p0, f32 cr1, vec2 p1) {
    f32 dist = vec_distance(p0, p1);
    f32 rads = cr0 + cr1;
    bool result = (dist <= rads);
    return result;
}

vec2 _get_farthest_vert(vec2 *verts, i32 vert_count) {
    vec2 result = make_vec2();
    f32 largest_len = -F32_MAX;
    for(i32 i = 0; i < vert_count; ++i) {
        f32 tested_len = vec_length(verts[i]);
        if(tested_len > largest_len) {
            largest_len = tested_len;
            result = verts[i];
        }
    }
    return result;
}

bool collision_sat(vec2 *poly0_verts, i32 poly0_vert_count, vec2 poly0_pos, 
                   vec2 *poly1_verts, i32 poly1_vert_count, vec2 poly1_pos) {
    _p_assert(poly0_vert_count >= P_SAT_MIN_VERT);
    _p_assert(poly1_vert_count >= P_SAT_MIN_VERT);
    
    _Poly poly0 = {poly0_verts, poly0_vert_count};
    _Poly poly1 = {poly1_verts, poly1_vert_count};
    
    // NOTE: Check if there's is chance for polys to collide
    f32 p0_radius = vec_length(_get_farthest_vert(poly0_verts, poly0_vert_count));
    f32 p1_radius = vec_length(_get_farthest_vert(poly1_verts, poly1_vert_count));
    if(!collision_O_vs_O(p0_radius, poly0_pos, p1_radius, poly1_pos)) {
        return false;
    }
    
    i32  tested_count = 0;
    vec2 tested_axises[P_SAT_MAX_VERT * 2];
    
    vec2 poly0_p = poly0_pos;
    vec2 poly1_p = poly1_pos;
    _Poly *poly_check0 = &poly0;
    _Poly *poly_check1 = &poly1;
    for(bool first = true; true;) {
        for(i32 v0 = 0; v0 < poly_check0->vert_count; ++v0) {
            u32 v1 = (v0 + 1) % poly_check0->vert_count;
            
            vec2 line0 = poly_check0->verts[v0] + poly0_p;
            vec2 line1 = poly_check0->verts[v1] + poly0_p;
            vec2 axis_proj = vec_line_normal(line0, line1, false);
            
            // NOTE: Check for tested axises
            {
                bool found = false;
                for(i32 i = 0; i < tested_count; ++i) {
                    if(vec_prec(axis_proj, P_SAT_AXIS_PREC) == 
                       vec_prec(tested_axises[i], P_SAT_AXIS_PREC)) {
                        found = true;
                    }
                }
                
                if(found) {
                    continue;
                }
                else {
                    tested_axises[tested_count++] = axis_proj;
                }
            }
            
            f32 min0 = F32_MAX;
            f32 max0 = F32_MIN;
            for(i32 _vert = 0; _vert < poly_check0->vert_count; ++_vert) {
                vec2 vert = poly_check0->verts[_vert] + poly0_p;
                f32 dot = vec_dot(vert, axis_proj);
                if(dot < min0) {
                    min0 = dot;
                }
                if(dot > max0) {
                    max0 = dot;
                }
            }
            
            f32 min1 = F32_MAX;
            f32 max1 = F32_MIN;
            for(i32 _vert = 0; _vert < poly_check1->vert_count; ++_vert) {
                vec2 vert = poly_check1->verts[_vert] + poly1_p;
                f32 dot = vec_dot(vert, axis_proj);
                if(dot < min1) {
                    min1 = dot;
                }
                if(dot > max1) {
                    max1 = dot;
                }
            }
            
            if(!(max1 >= min0 && max0 >= min1)) {
                return false;
            }
        }
        
        if(first) {
            poly_check0 = &poly1;
            poly_check1 = &poly0;
            poly0_p = poly1_pos;
            poly1_p = poly0_pos;
            first = false;
        }
        else { 
            break; 
        }
    }
    return true;
}

bool collision_sat_O(vec2 *poly_verts, i32 poly_vert_count, vec2 poly_pos, 
                     f32 cr, vec2 cp) {
    _p_assert(poly_vert_count >= P_SAT_MIN_VERT);
    
    // NOTE: Check if there's is chance for a collision
    f32 poly_radius = vec_length(_get_farthest_vert(poly_verts, poly_vert_count));
    if(!collision_O_vs_O(poly_radius, poly_pos, cr, cp)) {
        return false;
    }
    
    i32  tested_count = 0;
    vec2 tested_axises[P_SAT_MAX_VERT * 2];
    
    // NOTE: Get closest vertice to the circle
    vec2 c_vert = make_vec2(F32_MAX, F32_MAX);
    for(i32 v = 0; v < poly_vert_count; ++v) {
        f32 last_dist = vec_distance(cp, c_vert);
        f32 test_dist = vec_distance(cp, poly_verts[v] + poly_pos);
        if(test_dist < last_dist) {
            c_vert = poly_verts[v] + poly_pos;
        }
    }
    
    u32 loops = poly_vert_count + 1;
    for(u32 v0 = 0; v0 < loops; ++v0) {
        // NOTE: Get line normal
        vec2 line0;
        vec2 line1;
        if(v0 == loops - 1) {
            // NOTE: Circle
            line0 = cp;
            line1 = c_vert;
        }
        else {
            // NOTE: Poly
            u32 v1 = (v0 + 1) % poly_vert_count;
            line0 = poly_verts[v0] + poly_pos;
            line1 = poly_verts[v1] + poly_pos;
        }
        vec2 axis_proj = vec_normalize(vec_line_normal(line0, line1));
        
        // NOTE: Check for tested axises
        {
            bool found = false;
            for(i32 i = 0; i < tested_count; ++i) {
                if(vec_prec(axis_proj, P_SAT_AXIS_PREC) ==
                   vec_prec(tested_axises[i], P_SAT_AXIS_PREC)) {
                    found = true;
                }
            }
            
            if(found) {
                continue;
            }
            else {
                tested_axises[tested_count++] = axis_proj;
            }
        }
        
        f32 min0 = F32_MAX;
        f32 max0 = F32_MIN;
        for(i32 _vert = 0; _vert < poly_vert_count; ++_vert) {
            vec2 vert = poly_verts[_vert] + poly_pos;
            f32 dot = vec_dot(vert, axis_proj);
            if(dot < min0) {
                min0 = dot;
            }
            if(dot > max0) {
                max0 = dot;
            }
        }
        
        f32 min1 = F32_MAX;
        f32 max1 = F32_MIN;
        vec2 cvs[2] = {
            (axis_proj * +cr) + cp,
            (axis_proj * -cr) + cp
        };
        
        for(i32 i = 0; i < _p_size_array(cvs); ++i) {
            f32 dot = vec_dot(cvs[i], axis_proj);
            if(dot < min1) {
                min1 = dot;
            }
            if(dot > max1) {
                max1 = dot;
            }
        }
        
        if(!(max1 >= min0 && max0 >= min1)) {
            return false;
        }
    }
    return true;
}

bool collision_sat_point(vec2 *poly_verts, i32 poly_vert_count, vec2 poly_pos, vec2 point) {
    _p_assert(poly_vert_count >= P_SAT_MIN_VERT);
    
    i32  tested_count = 0;
    vec2 tested_axises[P_SAT_MAX_VERT * 2];
    
    // NOTE: poly's axises
    for(i32 v0 = 0; v0 < poly_vert_count; ++v0) {
        u32 v1 = (v0 + 1) % poly_vert_count;
        
        // NOTE: Get line normal
        vec2 line0 = poly_verts[v0] + poly_pos;
        vec2 line1 = poly_verts[v1] + poly_pos;
        vec2 axis_proj = vec_normalize(vec_vector(line0, line1));
        
        // NOTE: Check for tested axises
        {
            bool found = false;
            for(i32 i = 0; i < tested_count; ++i) {
                if(vec_prec(axis_proj, P_SAT_AXIS_PREC) == 
                   vec_prec(tested_axises[i], P_SAT_AXIS_PREC)) {
                    found = true;
                }
            }
            
            if(found) {
                continue;
            }
            else {
                tested_axises[tested_count++] = axis_proj;
            }
        }
        
        f32 min0 = F32_MAX;
        f32 max0 = F32_MIN;
        for(i32 _vert = 0; _vert < poly_vert_count; ++_vert) {
            vec2 vert = poly_verts[_vert] + poly_pos;
            f32 dot = vec_dot(vert, axis_proj);
            if(dot < min0) {
                min0 = dot;
            }
            if(dot > max0) {
                max0 = dot;
            }
        }
        
        f32 min1 = F32_MAX;
        f32 max1 = F32_MIN;
        {
            vec2 vert = point;
            f32 dot = vec_dot(vert, axis_proj);
            if(dot < min1) {
                min1 = dot;
            }
            if(dot > max1) {
                max1 = dot;
            }
        }
        
        if(!(max1 >= min0 && max0 >= min1)) {
            return false;
        }
    }
    return true;
}

/*--------------*/
/*    Random    */
/*--------------*/
// NOTE, TODO: temp

P_INLINE
u64 rand_u64(random_seed *rand) {
    u64 r = *rand;
    r ^= r << 13;
    r ^= r >> 17;
    r ^= r << 5;
    return (*rand = r);
}

P_INLINE // TODO:
u32 rand_u32(random_seed *rand) {
    u64 random_u64 = rand_u64(rand);
    u32 r = (u32)random_u64;
    return r;
}

P_INLINE // NOTE, TODO: no idea what im doing but seems to work
i32 rand_i32(random_seed *rand) {
    i32 r = 0;
    u32 random_u32 = rand_u32(rand);
    if(random_u32 > ((u32)I32_MAX + 1)) {
        r = (i32)(random_u32 - ((u32)I32_MAX + 1));
    }
    else {
        r = -(i32)(U32_MAX - random_u32 - ((u32)I32_MAX + 1));
    }
    return r;
}

P_INLINE // TODO:
f32 rand_f32(random_seed *rand) {
    i32 random_i32 = rand_i32(rand);
    f32 r = (f32)random_i32;
    return r;
}

P_INLINE
u64 rand_u64_in_range(random_seed *rand, u64 min, u64 max) {
    _p_assert_range(min, max);
    u64 range = max - min + 1;
    u64 random_u64 = rand_u64(rand);
    u64 r = min + (random_u64 % range);
    return r;
}

P_INLINE
u32 rand_u32_in_range(random_seed *rand, u32 min, u32 max) {
    _p_assert_range(min, max);
    u32 range = max - min + 1;
    u32 random_u32 = rand_u32(rand);
    u32 r = min + (random_u32 % range);
    return r;
}

P_INLINE // NOTE: temp range gotta be lower than I32_MAX
i32 rand_i32_in_range(random_seed *rand, i32 min, i32 max) {
    _p_assert_range(min, max);
    u32 range = (u32)(max - min);
    _p_assert(range <= I32_MAX);
    u32 random_u32 = rand_u32_in_range(rand, 0, range);
    i32 r = random_u32 + min;
    return r;
}

P_INLINE
f32 rand_f32_in_range(random_seed *rand, f32 min, f32 max) {
    _p_assert_range(min, max);
    f32 range = max - min;
    _p_assert(roundf32_to_i32(range) <= I32_MAX);
    i32 random_i32 = rand_i32_in_range(rand, 0, I32_MAX);
    f32 r = ((f32)random_i32 / (f32)(I32_MAX / range)) + min;
    return r;
}

#endif /* P_MATH_IMPLEMENTATION */ 
#endif /* P_MATH_H */
