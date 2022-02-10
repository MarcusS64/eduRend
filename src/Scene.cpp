
#include "Scene.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	dxdevice(dxdevice),
	dxdevice_context(dxdevice_context),
	window_width(window_width),
	window_height(window_height)
{ }

void Scene::WindowResize(
	int window_width,
	int window_height)
{
	this->window_width = window_width;
	this->window_height = window_height;
}

OurTestScene::OurTestScene(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context, int window_width, int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();	
	// + init other CBuffers
	InitLightCamBuffer();
}

//
// Called once at initialization
//
void OurTestScene::Init()
{
	camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)window_width / window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	camera->moveTo({ 0, 0, 5 });

	// Create objects
	quad = new QuadModel(dxdevice, dxdevice_context);
	cube = new Cube(dxdevice, dxdevice_context); 
	sponza = new OBJModel("assets/crytek-sponza/sponza.obj", dxdevice, dxdevice_context);
	sun = new OBJModel("assets/sphere/sphere.obj", dxdevice, dxdevice_context);
	earth = new OBJModel("assets/sphere/sphere.obj", dxdevice, dxdevice_context);
	moon = new OBJModel("assets/sphere/sphere.obj", dxdevice, dxdevice_context);
	plane = new OBJModel("assets/Maya/Plane.obj", dxdevice, dxdevice_context);
	lightSource = new OBJModel("assets/sphere/sphere.obj", dxdevice, dxdevice_context);

	LightPos = { 0.0f, 3.0f, 0.0f};
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(float dt, InputHandler* input_handler)
{
	// Basic camera control
	if (input_handler->IsKeyPressed(Keys::Up) || input_handler->IsKeyPressed(Keys::W))
		camera->move({ 0.0f, 0.0f, -camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Down) || input_handler->IsKeyPressed(Keys::S))
		camera->move({ 0.0f, 0.0f, camera_vel * dt });
	if (input_handler->IsKeyPressed(Keys::Right) || input_handler->IsKeyPressed(Keys::D))
		camera->move({ camera_vel * dt, 0.0f, 0.0f });
	if (input_handler->IsKeyPressed(Keys::Left) || input_handler->IsKeyPressed(Keys::A))
		camera->move({ -camera_vel * dt, 0.0f, 0.0f });
	if ((GetKeyState(VK_LBUTTON) & 0x100) != 0) {
		camera->rotate(input_handler->GetMouseDeltaY() / 300.0, input_handler->GetMouseDeltaX() / 300.0);
	}

	LightPos.x = 10.0f * cos(angle/2.0f);
	LightPos.z = -20.0f + 10.0f * sin(angle/2.0f);

	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	Mquad = mat4f::translation(0, 0, 0) *			// No translation
		mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%

	// Sponza model-to-world transformation
	Msponza = mat4f::translation(0, -5, 0) * mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.05f);						 
	Msun = mat4f::translation(0, 2, 0) * mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.5f);
	Mearth = Msun * mat4f::translation(3, 0, 0) * mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.3f);
	Mmoon = Mearth * mat4f::translation(2, 0, 0) * mat4f::rotation(-angle, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.2f);
	Mplane = mat4f::translation(0, 0, -9.0f) * mat4f::rotation(0.0f, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.5f);
	MlightSource = mat4f::translation(LightPos.x, LightPos.y, LightPos.z) * mat4f::rotation(0.0f, 0.0f, 1.0f, 0.0f) * mat4f::scaling(0.2f);
	// Increment the rotation angle.
	angle += angle_vel * dt;

	// Print fps
	fps_cooldown -= dt;
	if (fps_cooldown < 0.0)
	{
		//std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		fps_cooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	// Bind transformation_buffer to slot b0 of the VS
	dxdevice_context->VSSetConstantBuffers(0, 1, &transformation_buffer);
	dxdevice_context->PSSetConstantBuffers(0, 1, &lightCam_buffer);
	
	//dxdevice_context->PSSetSamplers(0, 1, &tex_sampler[filterIndex]); //Comment in to bind the sampler
	// Obtain the matrices needed for rendering from the camera
	Mview = camera->get_WorldToViewMatrix();
	Mproj = camera->get_ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	UpdateTransformationBuffer(Mquad, Mview, Mproj);
	UpdateLightCamBuffer(camera->position, LightPos);
	//quad->Render();
	cube->Render();	

	UpdateTransformationBuffer(Msponza, Mview, Mproj);	
	//UpdateLightCamBuffer(camera->position, LightPos);
	sponza->Render();

	UpdateTransformationBuffer(Msun, Mview, Mproj);
	//UpdateLightCamBuffer(camera->position, LightPos);
	sun->Render();

	UpdateTransformationBuffer(Mearth, Mview, Mproj);
	earth->Render();

	UpdateTransformationBuffer(Mmoon, Mview, Mproj);
	moon->Render();

	UpdateTransformationBuffer(Mplane, Mview, Mproj);
	plane->Render();

	UpdateTransformationBuffer(MlightSource, Mview, Mproj);
	lightSource->Render();
}

void OurTestScene::Release()
{
	SAFE_DELETE(quad);
	SAFE_DELETE(cube);
	SAFE_DELETE(sponza);
	SAFE_DELETE(sun);
	SAFE_DELETE(earth);
	SAFE_DELETE(moon);
	SAFE_DELETE(plane);
	SAFE_DELETE(camera);
	SAFE_DELETE(lightSource);

	SAFE_RELEASE(transformation_buffer);
	// + release other CBuffers
	SAFE_RELEASE(lightCam_buffer);
	
}

void OurTestScene::WindowResize(
	int window_width,
	int window_height)
{
	if (camera)
		camera->aspect = float(window_width) / window_height;

	Scene::WindowResize(window_width, window_height);
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(TransformationBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &transformation_buffer));
}

void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrix_buffer_ = (TransformationBuffer*)resource.pData;
	matrix_buffer_->ModelToWorldMatrix = ModelToWorldMatrix;
	matrix_buffer_->WorldToViewMatrix = WorldToViewMatrix;
	matrix_buffer_->ProjectionMatrix = ProjectionMatrix;
	dxdevice_context->Unmap(transformation_buffer, 0);
}

void OurTestScene::InitLightCamBuffer() 
{
	HRESULT hr;
	D3D11_BUFFER_DESC MatrixBuffer_desc = { 0 };
	MatrixBuffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	MatrixBuffer_desc.ByteWidth = sizeof(LightCamBuffer);
	MatrixBuffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatrixBuffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MatrixBuffer_desc.MiscFlags = 0;
	MatrixBuffer_desc.StructureByteStride = 0;
	ASSERT(hr = dxdevice->CreateBuffer(&MatrixBuffer_desc, nullptr, &lightCam_buffer));
}

void OurTestScene::UpdateLightCamBuffer(
	vec3f camPos, //Should be a vec3f?
	vec3f lightPos) 
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	dxdevice_context->Map(lightCam_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	LightCamBuffer* matrix_buffer_ = (LightCamBuffer*)resource.pData;
	matrix_buffer_->camPos = vec4f(camPos, 0);
	matrix_buffer_->lightPos = vec4f(lightPos, 0);
	dxdevice_context->Unmap(lightCam_buffer, 0);
}

void OurTestScene::InitTexSampler() { //Three samplers created. Make a method changing between them using an index

	HRESULT hr;
	D3D11_SAMPLER_DESC samplerDesc = { //Använd wrap
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_MIRROR,
		0.0f,
		16,
		D3D11_COMPARISON_NEVER,
		{1.0f, 1.0f, 1.0f, 1.0f},
		-FLT_MAX,
		FLT_MAX,
	};
	ASSERT(hr = dxdevice->CreateSamplerState(&samplerDesc, &tex_sampler[0]));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ASSERT(hr = dxdevice->CreateSamplerState(&samplerDesc, &tex_sampler[1]));

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	ASSERT(hr = dxdevice->CreateSamplerState(&samplerDesc, &tex_sampler[2]));
}

void OurTestScene::SwapFilter(InputHandler* input) {
	//if(imput == )
}

