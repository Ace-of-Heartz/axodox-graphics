#include "pch.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_uwp.h"
#include "dxr_include.h"
#include <wrl/client.h>

using namespace Microsoft::WRL;
using namespace std;
using namespace winrt;

using namespace winrt::Windows;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Composition;

using namespace Axodox::Graphics::D3D12;
using namespace Axodox::Infrastructure;
using namespace Axodox::Storage;
using namespace DirectX;



using namespace AceOfHearts::Graphics::DXR;


struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{

	struct FrameResources
	{
		CommandAllocator Allocator;
		CommandFence Fence;
		CommandFenceMarker Marker;
		DynamicBufferManager DynamicBuffer;

		MutableTexture DepthBuffer;
		MutableTexture PostProcessingBuffer;
		descriptor_ptr<ShaderResourceView> ScreenResourceView;

		FrameResources(const ResourceAllocationContext& context) :
			Allocator(*context.Device),
			Fence(*context.Device),
			Marker(),
			DynamicBuffer(*context.Device),
			DepthBuffer(context),
			PostProcessingBuffer(context)
		{ }
	};

	struct SimpleRootDescription : public RootSignatureMask
	{
		RootDescriptor<RootDescriptorType::ConstantBuffer> ConstantBuffer;
		RootDescriptorTable<1> Texture;
		StaticSampler Sampler;

		SimpleRootDescription(const RootSignatureContext& context) :
			RootSignatureMask(context),
			ConstantBuffer(this, { 0 }, ShaderVisibility::Vertex),
			Texture(this, { DescriptorRangeType::ShaderResource }, ShaderVisibility::Pixel),
			Sampler(this, { 0 }, Filter::Linear, TextureAddressMode::Clamp, ShaderVisibility::Pixel)
		{
			Flags = RootSignatureFlags::AllowInputAssemblerInputLayout;
		}
	};

	struct PostProcessingRootDescription : public RootSignatureMask
	{
		RootDescriptor<RootDescriptorType::ConstantBuffer> ConstantBuffer;
		RootDescriptorTable<1> InputTexture;
		RootDescriptorTable<1> OutputTexture;
		StaticSampler Sampler;

		PostProcessingRootDescription(const RootSignatureContext& context) :
			RootSignatureMask(context),
			ConstantBuffer(this, { 0 }),
			InputTexture(this, { DescriptorRangeType::ShaderResource }),
			OutputTexture(this, { DescriptorRangeType::UnorderedAccess }),
			Sampler(this, { 0 }, Filter::Linear, TextureAddressMode::Clamp)
		{
			Flags = RootSignatureFlags::AllowInputAssemblerInputLayout;
		}
	};


	IFrameworkView CreateView()
	{
		return *this;
	}



	void Initialize(CoreApplicationView const&)
	{
	}

	void Load(hstring const&)
	{
	}

	void Uninitialize()
	{
	}

	struct Constants
	{
		XMFLOAT4X4 WorldViewProjection;
	};

	void Run()
	{
		bool rasterMode = false;

		

		CoreWindow window = CoreWindow::GetForCurrentThread();
		window.Activate();

		CoreDispatcher dispatcher = window.Dispatcher();

		GraphicsDevice device{};
		
		try {
			CheckRayTracingSupport(&device);
		} catch (std::exception e) {
			rasterMode = true;
		}

		XMFLOAT4 clearColor;

		if (rasterMode) {
			clearColor = { 1.f, 0.f, 0.78f, 0.f };
		} else {
			clearColor = { 0.88f, 0.78f, 0.f, 0.f };
		}


		CommandQueue directQueue{ device };
		CoreSwapChain swapChain{ directQueue, window, SwapChainFlags::IsShaderResource };

		PipelineStateProvider pipelineStateProvider{ device };

		RootSignature<SimpleRootDescription> simpleRootSignature{ device };

		ResourcesDXR resources
			{
			&device,
			nullptr,
			&directQueue,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		};

		//CreateAccelerationStructures();


		RootSignature<PostProcessingRootDescription> postProcessingRootSignature{ device };
		ComputeShader postProcessingComputeShader{ app_folder() / L"PostProcessingComputeShader.cso" };
		ComputePipelineStateDefinition postProcessingStateDefinition{
		  .RootSignature = &postProcessingRootSignature,
		  .ComputeShader = &postProcessingComputeShader
		};
		auto postProcessingPipelineState = pipelineStateProvider.CreatePipelineStateAsync(postProcessingStateDefinition).get();

		GroupedResourceAllocator groupedResourceAllocator{ device };
		ResourceUploader resourceUploader{ device };
		CommonDescriptorHeap commonDescriptorHeap{ device, 2 };
		DepthStencilDescriptorHeap depthStencilDescriptorHeap{ device };
		ResourceAllocationContext immutableAllocationContext{
		  .Device = &device,
		  .ResourceAllocator = &groupedResourceAllocator,
		  .ResourceUploader = &resourceUploader,
		  .CommonDescriptorHeap = &commonDescriptorHeap,
		  .DepthStencilDescriptorHeap = &depthStencilDescriptorHeap
		};

		ImmutableMesh cubeMesh{ immutableAllocationContext, CreateCube() };
		ImmutableTexture texture{ immutableAllocationContext, app_folder() / L"image.jpeg" };

		groupedResourceAllocator.Build();

		auto mutableAllocationContext = immutableAllocationContext;
		CommittedResourceAllocator committedResourceAllocator{ device };
		mutableAllocationContext.ResourceAllocator = &committedResourceAllocator;

		array<FrameResources, 2> frames{ mutableAllocationContext, mutableAllocationContext };

		swapChain.Resizing(no_revoke, [&](SwapChain*) {
			for (auto& frame : frames) frame.ScreenResourceView.reset();
			commonDescriptorHeap.Clean();
			});


		//InitImGui();

		auto i = 0u;
		while (true)
		{
			//Process user input
			dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			//Get frame resources
			auto& resources = frames[i++ & 0x1u];
			auto renderTargetView = swapChain.RenderTargetView();

			//Wait until buffers can be reused
			if (resources.Marker) resources.Fence.Await(resources.Marker);

			//Update constants
			Constants constants{};
			auto resolution = swapChain.Resolution();
			{
				auto projection = XMMatrixPerspectiveFovRH(90.f, float(resolution.x) / float(resolution.y), 0.01f, 10.f);
				auto view = XMMatrixLookAtRH(XMVectorSet(1.5f * cos(i * 0.002f), 1.5f * sin(i * 0.002f), 1.5f * cos(i * 0.0005f), 1.f), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 1.f, 1.f));
				auto world = XMMatrixIdentity();
				auto worldViewProjection = XMMatrixTranspose(world * view * projection);

				XMStoreFloat4x4(&constants.WorldViewProjection, worldViewProjection);
			}

			//Ensure depth buffer
			if (!resources.DepthBuffer || !TextureDefinition::AreSizeCompatible(*resources.DepthBuffer.Definition(), renderTargetView->Definition()))
			{
				auto depthDefinition = renderTargetView->Definition().MakeSizeCompatible(Format::D32_Float, TextureFlags::DepthStencil);
				resources.DepthBuffer.Allocate(depthDefinition);
			}

			//Ensure screen shader resource view
			if (!resources.ScreenResourceView || resources.ScreenResourceView->Resource() != renderTargetView->Resource())
			{
				Texture screenTexture{ renderTargetView->Resource() };
				resources.ScreenResourceView = commonDescriptorHeap.CreateShaderResourceView(&screenTexture);
			}

			//Ensure post processing buffer
			if (!resources.PostProcessingBuffer || !TextureDefinition::AreSizeCompatible(*resources.PostProcessingBuffer.Definition(), renderTargetView->Definition()))
			{
				auto postProcessingDefinition = renderTargetView->Definition().MakeSizeCompatible(Format::B8G8R8A8_UNorm, TextureFlags::UnorderedAccess);
				resources.PostProcessingBuffer.Allocate(postProcessingDefinition);
			}

			//Begin frame command list
			auto& allocator = resources.Allocator;
			{
				allocator.Reset();
				allocator.BeginList();
				allocator.TransitionResource(*renderTargetView, ResourceStates::Present, ResourceStates::RenderTarget);

				committedResourceAllocator.Build();
				depthStencilDescriptorHeap.Build();

				commonDescriptorHeap.Build();
				commonDescriptorHeap.Set(allocator);

				renderTargetView->Clear(allocator, clearColor);
				resources.DepthBuffer.DepthStencil()->Clear(allocator);
			}




			//Draw objects
			{
				/*auto mask = simpleRootSignature.Set(allocator, RootSignatureUsage::Graphics);
				mask.ConstantBuffer = resources.DynamicBuffer.AddBuffer(constants);
				mask.Texture = texture;

				allocator.SetRenderTargets({ renderTargetView }, resources.DepthBuffer.DepthStencil());
				simplePipelineState.Apply(allocator);
				cubeMesh.Draw(allocator);*/
			}

			//Post processing
			{
				/*allocator.TransitionResource(*renderTargetView, ResourceStates::RenderTarget, ResourceStates::NonPixelShaderResource);

				auto mask = postProcessingRootSignature.Set(allocator, RootSignatureUsage::Compute);
				mask.ConstantBuffer = resources.DynamicBuffer.AddBuffer(i * 0.02f);
				mask.InputTexture = *resources.ScreenResourceView;
				mask.OutputTexture = *resources.PostProcessingBuffer.UnorderedAccess();
				postProcessingPipelineState.Apply(allocator);

				auto definition = resources.PostProcessingBuffer.Definition();
				allocator.Dispatch(definition->Width / 16 + 1, definition->Height / 16 + 1);

				allocator.TransitionResources({
				  { resources.PostProcessingBuffer, ResourceStates::UnorderedAccess, ResourceStates::CopySource },
				  { *renderTargetView, ResourceStates::NonPixelShaderResource, ResourceStates::CopyDest }
					});

				allocator.CopyResource(resources.PostProcessingBuffer, *renderTargetView);

				allocator.TransitionResources({
				  { resources.PostProcessingBuffer, ResourceStates::CopySource, ResourceStates::UnorderedAccess },
				  { *renderTargetView, ResourceStates::CopyDest, ResourceStates::RenderTarget }
					});*/
			}


			//End frame command list
			{
				allocator.TransitionResource(*renderTargetView, ResourceStates::RenderTarget, ResourceStates::Present);
				auto drawCommandList = allocator.EndList();

				allocator.BeginList();
				resources.DynamicBuffer.UploadResources(allocator);
				resourceUploader.UploadResourcesAsync(allocator);
				auto initCommandList = allocator.EndList();

				directQueue.Execute(initCommandList);
				directQueue.Execute(drawCommandList);
				resources.Marker = resources.Fence.EnqueueSignal(directQueue);
			}

			//Present frame
			swapChain.Present();
		}

		//CleanImGui();
	}

	void InitImGui() {
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;


		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui_ImplUwp_InitForCurrentView();

		// TODO: Figure out what heck this needs to work
		// How do I even start with that bruh
		/*
		D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle;
		gpuSrvHandle = immutableAllocationContext.CommonDescriptorHeap->get()->GetGPUDescriptorHandleForHeapStart();

		D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle;
		cpuSrvHandle = immutableAllocationContext.CommonDescriptorHeap->get()->GetCPUDescriptorHandleForHeapStart();

		ImGui_ImplDX12_Init(device.get(), 2,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			nullptr,
			cpuSrvHandle,
			gpuSrvHandle);

		*/

	}

	void RunImGui(CommandAllocator allocator) {

		//ImGui begin
		ImGui_ImplUwp_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow(); // Show demo window! :)
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), allocator.getCommand());
	}

	void CleanImGui() {
		//ImGui_ImplDX12_Shutdown();
		ImGui_ImplUwp_Shutdown();
		ImGui::DestroyContext();
	}


	void SetWindow(CoreWindow const& /*window*/)
	{

	}
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	CoreApplication::Run(make<App>());
}
