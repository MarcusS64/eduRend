#pragma once
#include "Model.h"

class Cube : public Model {

		unsigned nbr_indices = 0;

	public:
		Material mtl;
		Cube( ID3D11Device* dx3ddevice, ID3D11DeviceContext* dx3ddevice_context);

		void Render() const;

		~Cube();
	
};
