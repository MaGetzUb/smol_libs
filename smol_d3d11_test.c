#define SMOL_FRAME_IMPLEMENTATION
#include "smol_frame.h"

#define SMOL_CANVAS_IMPLEMENTATION
#include "smol_canvas.h"

#define SMOL_UTILS_IMPLEMENTATION
#include "smol_utils.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define INITGUID
#define COBJMACROS
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>


#define DXAssert( hres ) if(FAILED(hres)) SMOL_BREAKPOINT()

#define SMOL_SAFE_COM_RELEASE(ptr) if(ptr) ptr->lpVtbl->Release(ptr), ptr = NULL

const char* vsh =
"struct vs_out {\n"
"	float4 color	: COLOR0;\n"
"	float2 texcoord	: TEXCOORD0;\n"
"	float4 position	: SV_POSITION;\n"
"};\n"
"struct vs_in {\n"
"	float3 position : POSITION;\n"
"	float4 color	: COLOR0;\n"
"	float2 texcoord : TEXCOORD0;\n"
"};\n"
"\n"
"vs_out vs_main(vs_in input) {\n"
"	vs_out res;"
"	res.position = float4(input.position, 0.5);\n"	
"	res.color = input.color;\n"
"	res.texcoord = input.texcoord;\n"	
"	return res;\n"
"}";

const char* psh =
"struct vs_out {\n"
"	float4 color	: COLOR0;\n"
"	float2 texcoord	: TEXCOORD0;\n"
"};\n"
"\n"
"SamplerState Sampler0 : register(s0);\n"
"Texture2D Texture0 : register(t0);\n"
"float4 ps_main(vs_out vs_res): SV_TARGET {\n"
"	return Texture0.Sample(Sampler0, vs_res.texcoord) * vs_res.color;\n"
"}";

typedef struct vertex_t {
	float pos[3];
	UINT32 col;
	float texcoord[2];
} vertex_t;


int main() {

	smol_frame_t* frame = smol_frame_create(800, 600, "D3D11 Test");
	
	smol_image_t image = smol_load_image_qoi("res/gfx/test.qoi");

	HRESULT result = 0;
	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
	

	
	IDXGISwapChain* dxgi_swap_chain = NULL;			  //This handles buffer swapping
	ID3D11Device* d3d11_device = NULL;				  //This handles resource creation and deletion
	ID3D11DeviceContext* d3d11_device_context = NULL; //This handles the GPU state (setting shaders, rendering etc)

	D3D_FEATURE_LEVEL feature_level;

	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 };
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.OutputWindow = smol_frame_get_win32_window_handle(frame);
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.Flags = 0;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	

	result = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		flags, 
		NULL, 
		0, 
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		&dxgi_swap_chain,
		&d3d11_device,
		&feature_level,
		&d3d11_device_context
	);

	DXAssert(result);

#if _DEBUG
	ID3D11Debug* d3d11_debug = NULL;
	result = ID3D11Device_QueryInterface(d3d11_device, &IID_ID3D11Debug, &d3d11_debug);
	DXAssert(result);

	ID3D11InfoQueue* d3d11_info_queue = NULL;
	result = ID3D11Debug_QueryInterface(d3d11_debug, &IID_ID3D11InfoQueue, &d3d11_info_queue);
	DXAssert(result);

	result = ID3D11InfoQueue_SetBreakOnSeverity(d3d11_info_queue, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	DXAssert(result);
	
	result = ID3D11InfoQueue_SetBreakOnSeverity(d3d11_info_queue, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
	DXAssert(result);

	D3D11_MESSAGE_ID hide[] = {
		D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
	};

	D3D11_INFO_QUEUE_FILTER filter = { 0 };
	filter.DenyList.NumIDs = sizeof(hide)/sizeof(*hide);
	filter.DenyList.pIDList = hide;

	result = ID3D11InfoQueue_AddStorageFilterEntries(d3d11_info_queue, &filter);
	DXAssert(result);
#endif 

	//Get the back buffer texture
	ID3D11Texture2D* d3d11_back_buffer = NULL;
	ID3D11RenderTargetView* d3d11_backbuffer_rendertarget = NULL;
	
	result = IDXGISwapChain_GetBuffer(dxgi_swap_chain, 0, &IID_ID3D11Texture2D, &d3d11_back_buffer);
	DXAssert(result);

	result = ID3D11Device_CreateRenderTargetView(d3d11_device, d3d11_back_buffer, NULL, &d3d11_backbuffer_rendertarget);
	DXAssert(result);

	ID3D11VertexShader* d3d11_vs = NULL;
	ID3D11PixelShader* d3d11_ps = NULL;
	ID3DBlob* vs_bin = NULL;
	{
		ID3DBlob* binary = NULL;
		ID3DBlob* messages = NULL;

		UINT nFlags1 = 0;
		D3DCompile(vsh, strlen(vsh), "vertex_shader", NULL, NULL, "vs_main", "vs_4_0", nFlags1, NULL, &binary, &messages);

		if(messages) {
			UINT errors = ID3D10Blob_GetBufferSize(messages);
			if(errors) {
				char* str = (char*)ID3D10Blob_GetBufferPointer(messages);
				printf("VERTEX SHADER COMPILATION ERROR:\n %s\n", str);
			}
		}

		BYTE* byte_code = ID3D10Blob_GetBufferPointer(binary);
		UINT byte_code_len = ID3D10Blob_GetBufferSize(binary);
		result = ID3D11Device_CreateVertexShader(d3d11_device, byte_code, byte_code_len, NULL, &d3d11_vs);
		DXAssert(result);

		//SMOL_SAFE_COM_RELEASE(binary);
		SMOL_SAFE_COM_RELEASE(messages);

		vs_bin = binary;
	}

	{
		ID3DBlob* binary = NULL;
		ID3DBlob* messages = NULL;

		UINT nFlags1 = 0;
		D3DCompile(psh, strlen(psh), "pixel_shader", NULL, NULL, "ps_main", "ps_4_0", nFlags1, NULL, &binary, &messages);

		if(messages) {
			UINT errors = ID3D10Blob_GetBufferSize(messages);
			if(errors) {
				char* str = (char*)ID3D10Blob_GetBufferPointer(messages);
				printf("PIXEL SHADER COMPILATION ERROR:\n %s\n", str);
			}
		}

		BYTE* byte_code = ID3D10Blob_GetBufferPointer(binary);
		UINT byte_code_len = ID3D10Blob_GetBufferSize(binary);
		result = ID3D11Device_CreatePixelShader(d3d11_device, byte_code, byte_code_len, NULL, &d3d11_ps);
		DXAssert(result);

		SMOL_SAFE_COM_RELEASE(binary);
		SMOL_SAFE_COM_RELEASE(messages);
	}


	ID3D11InputLayout* d3d1_input_layout = NULL;
	{
		D3D11_INPUT_ELEMENT_DESC input_desc[] = {
			{ .SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0, .AlignedByteOffset =  0, D3D11_INPUT_PER_VERTEX_DATA },
			{ .SemanticName = "COLOR",    .SemanticIndex = 0, .Format = DXGI_FORMAT_R8G8B8A8_UNORM,  .InputSlot = 0, .AlignedByteOffset = 12, D3D11_INPUT_PER_VERTEX_DATA },
			{ .SemanticName = "TEXCOORD", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32_FLOAT,    .InputSlot = 0, .AlignedByteOffset = 16, D3D11_INPUT_PER_VERTEX_DATA },
		};

		void* bin = ID3D10Blob_GetBufferPointer(vs_bin);
		UINT len = ID3D10Blob_GetBufferSize(vs_bin);
		result = ID3D11Device_CreateInputLayout(d3d11_device, input_desc, 3, bin, len, &d3d1_input_layout);
		DXAssert(result);
	}


	ID3D11Buffer* d3d1__vertex_buffer = NULL;
	{
		vertex_t vdata[3] = {
			{ {-.25f, -.25f,  .00f}, 0xFF0000FF, {0.f, 1.f} },
			{ { .00f,  .25f,  .00f}, 0xFF00FF00, {0.5, 0.f} },
			{ { .25f, -.25f,  .00f}, 0xFFFF0000, {1.f, 1.f} },
		};

		D3D11_BUFFER_DESC buffer_desc = { 0 };
		buffer_desc.ByteWidth = sizeof(vdata);
		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER; 
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = sizeof(vertex_t);

		D3D11_SUBRESOURCE_DATA data = {
			.pSysMem = vdata,
			.SysMemPitch = sizeof(vdata),
			.SysMemSlicePitch = sizeof(vdata[0])
		};

		result = ID3D11Device_CreateBuffer(d3d11_device, &buffer_desc, &data, &d3d1__vertex_buffer);
		DXAssert(result);
	}

	ID3D11SamplerState* d3d11_sampler_state = NULL;
	ID3D11Texture2D* texture = NULL;
	ID3D11ShaderResourceView* d3d1__texture_srv = NULL;

	{
		D3D11_SAMPLER_DESC sampler_desc = {
			.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
			.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
			.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
			.MipLODBias = 0.f,
			.MaxAnisotropy = 0,
			.ComparisonFunc = D3D11_COMPARISON_NEVER,
			.BorderColor = {0.f, 0.f, 0.f, 0.f},
			.MinLOD = 0.f,
			.MaxLOD = 0.f
		};

		ID3D11Device_CreateSamplerState(d3d11_device, &sampler_desc, &d3d11_sampler_state);
	}

	{
		
		D3D11_TEXTURE2D_DESC desc = {
			.Width = image.width,
			.Height = image.height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.SampleDesc = { 1, 0 },
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_SHADER_RESOURCE,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
		};

		D3D11_SUBRESOURCE_DATA data = {
			.pSysMem = image.pixel_data,
			.SysMemPitch = image.width * sizeof(smol_pixel_t),
			.SysMemSlicePitch = 0
		};

		ID3D11Device_CreateTexture2D(d3d11_device, &desc, &data, &texture);


	}

	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = { 0 };
		srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = 1;
				

		ID3D11Device_CreateShaderResourceView(d3d11_device, texture, &srv_desc, &d3d1__texture_srv);
	}


	//Set the viewport
	D3D11_VIEWPORT viewport = {
		.TopLeftX = 0.f,
		.TopLeftY = 0.f,
		.Width = 800.f,
		.Height = 600.f,
		.MinDepth = 0.f,
		.MaxDepth = 1.f
	};
	
	//Set these states just once.

	//Set viewport
	ID3D11DeviceContext_RSSetViewports(d3d11_device_context, 1, &viewport);

	//Set sampler for the texture
	ID3D11SamplerState* samplers[] = { d3d11_sampler_state };
	ID3D11DeviceContext_PSSetSamplers(d3d11_device_context, 0, 1, samplers);

	//Set the texture
	ID3D11ShaderResourceView* srvs[] = { d3d1__texture_srv };
	ID3D11DeviceContext_PSSetShaderResources(d3d11_device_context, 0, 1, srvs);

	//Set the topology
	ID3D11DeviceContext_IASetPrimitiveTopology(d3d11_device_context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Set vertex layout
	ID3D11DeviceContext_IASetInputLayout(d3d11_device_context, d3d1_input_layout);

	//Set the vertex buffer data
	ID3D11Buffer* buffers[] = { d3d1__vertex_buffer };
	UINT strides[] = { sizeof(vertex_t) };
	UINT offsets[] = { 0 };
	ID3D11DeviceContext_IASetVertexBuffers(d3d11_device_context, 0, 1, buffers, strides, offsets);

	//Set the vertex and pixel sahaders
	ID3D11DeviceContext_VSSetShader(d3d11_device_context, d3d11_vs, NULL, NULL);
	ID3D11DeviceContext_PSSetShader(d3d11_device_context, d3d11_ps, NULL, NULL);


	while(!smol_frame_is_closed(frame)) {

		smol_frame_update(frame);
		//Clear the rendertarget
				
		//Set rendertarget to back buffer:
		ID3D11DeviceContext_OMSetRenderTargets(d3d11_device_context, 1, &d3d11_backbuffer_rendertarget, NULL);

		//Clear the back buffer
		const float color[] = {0.f, 0.f, 0.25f, 1.f};
		ID3D11DeviceContext_ClearRenderTargetView(d3d11_device_context, d3d11_backbuffer_rendertarget, color);

		//Draw the triangle
		ID3D11DeviceContext_Draw(d3d11_device_context, 3, 0);

		//Present the frame
		result = IDXGISwapChain_Present(dxgi_swap_chain, 1, 0);
		DXAssert(result);

	}
	
	SMOL_SAFE_COM_RELEASE(d3d11_sampler_state);
	SMOL_SAFE_COM_RELEASE(d3d1__texture_srv);
	SMOL_SAFE_COM_RELEASE(d3d11_vs);
	SMOL_SAFE_COM_RELEASE(d3d11_ps);
	SMOL_SAFE_COM_RELEASE(d3d1__vertex_buffer);
	SMOL_SAFE_COM_RELEASE(d3d1_input_layout);

	SMOL_SAFE_COM_RELEASE(d3d11_backbuffer_rendertarget);
	SMOL_SAFE_COM_RELEASE(d3d11_device_context);
	SMOL_SAFE_COM_RELEASE(d3d11_device);
	SMOL_SAFE_COM_RELEASE(d3d11_info_queue);
	SMOL_SAFE_COM_RELEASE(d3d11_debug);
	SMOL_SAFE_COM_RELEASE(dxgi_swap_chain);

	smol_image_destroy(&image);
	smol_frame_destroy(frame);

	return 0;
}
