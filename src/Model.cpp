//
//  Model.cpp
//
//  CJ Gribel 2016, cjgribel@gmail.com
//

#include "Model.h"

QuadModel::QuadModel(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context) : Model(dxdevice, dxdevice_context)
{
	// Vertex and index arrays
	// Once their data is loaded to GPU buffers, they are not needed anymore
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	// Populate the vertex array with 4 vertices
	Vertex v0, v1, v2, v3;
	v0.Pos = { -0.5, -0.5f, 0.0f };
	v0.Normal = { 0, 0, 1 };
	v0.TexCoord = { 0, 0 };
	v1.Pos = { 0.5, -0.5f, 0.0f };
	v1.Normal = { 0, 0, 1 };
	v1.TexCoord = { 0, 1 };
	v2.Pos = { 0.5, 0.5f, 0.0f };
	v2.Normal = { 0, 0, 1 };
	v2.TexCoord = { 1, 1 };
	v3.Pos = { -0.5, 0.5f, 0.0f };
	v3.Normal = { 0, 0, 1 };
	v3.TexCoord = { 1, 0 };
	vertices.push_back(v0);
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	// Populate the index array with two triangles
	// Triangle #1
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);
	// Triangle #2
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(vertices.size()*sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vdata;
	vdata.pSysMem = &vertices[0];
	// Create vertex buffer on device using descriptor & data
	const HRESULT vhr = dxdevice->CreateBuffer(&vbufferDesc, &vdata, &vertex_buffer);
	SETNAME(vertex_buffer, "VertexBuffer");
    
	//  Index array descriptor
	D3D11_BUFFER_DESC ibufferDesc = { 0 };
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = 0;
	ibufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufferDesc.MiscFlags = 0;
	ibufferDesc.ByteWidth = (UINT)(indices.size()*sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	const HRESULT ihr = dxdevice->CreateBuffer(&ibufferDesc, &idata, &index_buffer);
	SETNAME(index_buffer, "IndexBuffer");
    
	nbr_indices = (unsigned int)indices.size();
}


void QuadModel::Render() const
{
	// Bind our vertex buffer
	const UINT32 stride = sizeof(Vertex); //  sizeof(float) * 8;
	const UINT32 offset = 0;
	dxdevice_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	// Bind our index buffer
	dxdevice_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);

	// Make the drawcall
	dxdevice_context->DrawIndexed(nbr_indices, 0, 0);
}


OBJModel::OBJModel(const std::string& objfile, ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context) : Model(dxdevice, dxdevice_context)
{
	// Load the OBJ
	OBJLoader* mesh = new OBJLoader();
	mesh->Load(objfile);

	// Load and organize indices in ranges per drawcall (material)

	std::vector<unsigned> indices;
	unsigned int i_ofs = 0;

	for (auto& dc : mesh->drawcalls)
	{
		// Append the drawcall indices
		for (auto& tri : dc.tris)
			indices.insert(indices.end(), tri.vi, tri.vi + 3);

		// Create a range
		unsigned int i_size = (unsigned int)dc.tris.size() * 3;
		int mtl_index = dc.mtl_index > -1 ? dc.mtl_index : -1;
		index_ranges.push_back({ i_ofs, i_size, 0, mtl_index });

		i_ofs = (unsigned int)indices.size();
	}

	for (int i = 0; i < indices.size(); i += 3) { //New
		compute_TB(
			mesh->vertices[indices[i+0]],
			mesh->vertices[indices[i+1]],
			mesh->vertices[indices[i+2]]);
	}

	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(mesh->vertices.size()*sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vdata;
	vdata.pSysMem = &(mesh->vertices)[0];
	// Create vertex buffer on device using descriptor & data
	HRESULT vhr = dxdevice->CreateBuffer(&vbufferDesc, &vdata, &vertex_buffer);
	SETNAME(vertex_buffer, "VertexBuffer");
    
	// Index array descriptor
	D3D11_BUFFER_DESC ibufferDesc = { 0 };
	ibufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferDesc.CPUAccessFlags = 0;
	ibufferDesc.Usage = D3D11_USAGE_DEFAULT;
	ibufferDesc.MiscFlags = 0;
	ibufferDesc.ByteWidth = (UINT)(indices.size()*sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	HRESULT ihr = dxdevice->CreateBuffer(&ibufferDesc, &idata, &index_buffer);
	SETNAME(index_buffer, "IndexBuffer");
    
	// Copy materials from mesh
	append_materials(mesh->materials);

	// Go through materials and load textures (if any) to device
	std::cout << "Loading textures..." << std::endl;
	for (auto& mtl : materials)
	{
		HRESULT hr;

		// Load Diffuse texture
		//
		if (mtl.Kd_texture_filename.size()) {

			hr = LoadTextureFromFile( 
				dxdevice, 
				dxdevice_context, //For mipmapping
				mtl.Kd_texture_filename.c_str(), 
				&mtl.diffuse_texture);
			std::cout << "\t" << mtl.Kd_texture_filename 
				<< (SUCCEEDED(hr) ? " - OK" : "- FAILED") << std::endl;
		}

		// + other texture types here - see Material class
		// ...
		if (mtl.normal_texture_filename.size()) {

			hr = LoadTextureFromFile(
				dxdevice,
				dxdevice_context, //For mipmapping
				mtl.normal_texture_filename.c_str(),
				&mtl.normal_texture);
			std::cout << "\t" << mtl.normal_texture_filename
				<< (SUCCEEDED(hr) ? " - OK" : "- FAILED") << std::endl;
		}
	}
	std::cout << "Done." << std::endl;

	SAFE_DELETE(mesh);
}

void Model::compute_TB(Vertex& v0, Vertex& v1, Vertex& v2) {
	vec3f tangent, binormal;
	vec3f D, E; 
	vec2f F, G;

	D = v1.Pos - v0.Pos;
	E = v2.Pos - v0.Pos;
	F = v1.TexCoord - v0.TexCoord;
	G = v2.TexCoord - v0.TexCoord;

	float determinant = 1.0f / (F.x * G.y - F.y * G.x);
	tangent.x = determinant * (G.y * D.x - F.y * E.x);
	tangent.y = determinant * (G.y * D.y - F.y * E.y);
	tangent.z = determinant * (G.y * D.z - F.y * E.z);

	binormal.x = determinant * (-G.x * D.x + F.x * E.x);
	binormal.y = determinant * (-G.x * D.y + F.x * E.y);
	binormal.z = determinant * (-G.x * D.z + F.x * E.z);

	v0.Tangent = v1.Tangent = v2.Tangent = tangent;
	v0.Binormal = v1.Binormal = v2.Binormal = binormal;
}

void Model::InitPhongShinyBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(PhongShinyBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &phongShiny_buffer));
}

void Model::UpdatePhongShinyBuffer(vec3f Ambient, vec3f Diffuse, vec3f Speculair, float shinyness) const
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(phongShiny_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	PhongShinyBuffer* matrix_buffer_ = (PhongShinyBuffer*)resource.pData;
	//Padding with one extra float value added to make a vec4f to make it 16-bit
	matrix_buffer_->Ambient = vec4f(Ambient, 0);
	matrix_buffer_->Diffuse = vec4f(Diffuse, 0);
	matrix_buffer_->Speculair = vec4f(Speculair, shinyness);
	//matrix_buffer_->shinyness = shinyness;
	dxdevice_context->Unmap(phongShiny_buffer, 0);
}

void OBJModel::Render() const
{
	// Bind vertex buffer
	const UINT32 stride = sizeof(Vertex);
	const UINT32 offset = 0;
	dxdevice_context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);

	// Bind index buffer
	dxdevice_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
	dxdevice_context->PSSetConstantBuffers(1, 1, &phongShiny_buffer);
	// Iterate drawcalls
	for (auto& irange : index_ranges)
	{
		// Fetch material
		const Material& mtl = materials[irange.mtl_index];

		// Bind diffuse texture to slot t0 of the PS
		dxdevice_context->PSSetShaderResources(0, 1, &mtl.diffuse_texture.texture_SRV);
		// + bind other textures here, e.g. a normal map, to appropriate slots
		dxdevice_context->PSSetShaderResources(1, 1, &mtl.normal_texture.texture_SRV); //New
		UpdatePhongShinyBuffer(mtl.Ka, mtl.Kd, mtl.Ks, mtl.shinyness);
		// Make the drawcall
		dxdevice_context->DrawIndexed(irange.size, irange.start, 0);
	}

}

OBJModel::~OBJModel()
{
	for (auto& material : materials)
	{
		SAFE_RELEASE(material.diffuse_texture.texture_SRV);

		// Release other used textures ...
		SAFE_RELEASE(material.normal_texture.texture_SRV); //New
	}
}