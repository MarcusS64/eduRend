#pragma once
#ifndef MATRIXBUFFERS_H
#define MATRIXBUFFERS_H

#include "vec\vec.h"
#include "vec\mat.h"
#include <minwinbase.h>

using namespace linalg;

struct DirectionalLight {
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	mat4f Ambient;
	mat4f Diffuse;
	mat4f Speculair;
	mat3f Direction;
	float Pad;
};

struct PointLight {
	PointLight() { ZeroMemory(this, sizeof(this)); }

	mat4f Ambient;
	mat4f Diffuse;
	mat4f Speculair;

	mat3f position;
	float Range;

	mat3f Att;
};