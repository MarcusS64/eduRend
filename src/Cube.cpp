#include "Cube.h"

Cube::Cube(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context) : Model(dxdevice, dxdevice_context)
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

	// Backside of Cube
	Vertex v8, v9, v10, v11;
	v8.Pos = { -0.5, -0.5f, -1.0f };
	v8.Normal = { 0, 0, -1 };
	v8.TexCoord = { 0, 0 };
	v9.Pos = { 0.5, -0.5f, -1.0f };
	v9.Normal = { 0, 0, -1 };
	v9.TexCoord = { 0, 1 };
	v10.Pos = { 0.5, 0.5f, -1.0f };
	v10.Normal = { 0, 0, -1 };
	v10.TexCoord = { 1, 1 };
	v11.Pos = { -0.5, 0.5f, -1.0f };
	v11.Normal = { 0, 0, -1 };
	v11.TexCoord = { 1, 0 };
	vertices.push_back(v8);
	vertices.push_back(v9);
	vertices.push_back(v10);
	vertices.push_back(v11);

	//My first Vertex
	Vertex v4, v5, v6, v7;
	v4.Pos = { -0.5, 0.0f, -0.5f };
	v4.Normal = { 1, 0, 0 };
	v4.TexCoord = { 0, 0 };
	v5.Pos = { 0.5, -0.5f, 0.0f };
	v5.Normal = { 1, 0, 0 };
	v5.TexCoord = { 0, 1 };
	v6.Pos = { 0.5, 0.5f, 0.0f };
	v6.Normal = { 1, 0, 0 };
	v6.TexCoord = { 1, 1 };
	v7.Pos = { -0.5, 0.0f, -0.5f };
	v7.Normal = { 1, 0, 0 };
	v7.TexCoord = { 1, 0 };
	vertices.push_back(v4);
	vertices.push_back(v5);
	vertices.push_back(v6);
	vertices.push_back(v7);

	// Populate the index array with two triangles
	// Triangle #1
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);
	// Triangle #2
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);

	// Triangle #3
	indices.push_back(4);
	indices.push_back(6);
	indices.push_back(7);
	// Triangle #4
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);

	// Triangle Backside
	indices.push_back(8);
	indices.push_back(9);
	indices.push_back(11);
	// Triangle Backside
	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(11);

	// Vertex array descriptor
	D3D11_BUFFER_DESC vbufferDesc = { 0 };
	vbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferDesc.CPUAccessFlags = 0;
	vbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vbufferDesc.MiscFlags = 0;
	vbufferDesc.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
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
	ibufferDesc.ByteWidth = (UINT)(indices.size() * sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA idata;
	idata.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	const HRESULT ihr = dxdevice->CreateBuffer(&ibufferDesc, &idata, &index_buffer);
	SETNAME(index_buffer, "IndexBuffer");

	nbr_indices = (unsigned int)indices.size();

	
}

void Cube::Render() const
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