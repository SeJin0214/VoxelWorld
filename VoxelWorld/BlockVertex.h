#pragma once

struct Vector4
{
	float x;
	float y;
	float z;
	float a;
};

struct Vector3
{
	float x;
	float y;
	float z;
};

struct Vector2
{
	float x;
	float y;
};

struct BlockVertex
{
	Vector3 position;
	Vector3 normal;
	Vector4 color;
	Vector2 uv;
};
