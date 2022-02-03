
#pragma once
#ifndef MATRIXBUFFERS_H
#define MATRIXBUFFERS_H

#include "vec\vec.h"
#include "vec\mat.h"

using namespace linalg;

struct MatrixBuffer_t //Replace with TransformationBuffer
{
	mat4f ModelToWorldMatrix;
	mat4f WorldToViewMatrix;
	mat4f ProjectionMatrix;
};

struct LightCamBuffer
{
	vec4f camPos;
	vec4f lightPos;
};

struct PhongShinyBuffer
{
	vec4f Ambient;
	vec4f Diffuse;
	vec4f Speculair;

	float shinyness;
};
#endif