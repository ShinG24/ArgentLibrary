#include "Core/SubsystemLocator.h"

#include "Subsystem/Platform/Platform.h"

#include "Subsystem/Graphics/API/DXGI/SwapChain.h"
#include "Subsystem/Graphics/API/DXGI/DxgiFactory.h"

#include "Subsystem/Graphics/API/D3D12/GraphicsDevice.h"
#include "Subsystem/Graphics/API/D3D12/CommandQueue.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsCommandList.h"
#include "Subsystem/Graphics/API/D3D12/DescriptorHeap.h"
#include "Subsystem/Graphics/API/D3D12/BottomLevelAccelerationStructure.h"
#include "Subsystem/Graphics/API/D3D12/TopLevelAccelerationStructure.h"
#include "Subsystem/Graphics/API/D3D12/ConstantBuffer.h"
#include "Subsystem/Graphics/API/D3D12/RootSignature.h"
#include "Subsystem/Graphics/API/D3D12/GraphicsPipelineState.h"

#include "Subsystem/Graphics/Wrapper/DXR/AccelerationStructureManager.h"
#include "Subsystem/Graphics/Wrapper/ImGuiWrapper.h"
#include "Subsystem/Graphics/Wrapper/FrameResource.h"

#include "Subsystem/Graphics/GraphicsLibrary.h"

#include "Subsystem/Graphics/Raytracer.h"

#include "Subsystem/Graphics/RenderingManager.h"
#include "Subsystem/Input/InputManager.h"
#include "Subsystem/Timer/Timer.h"

#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibrary.h"
#include "Subsystem/Graphics/Wrapper/DXR/ShaderLibraryManager.h"

#include "Subsystem/Scene/BaseScene.h"
#include "Subsystem/Scene/SceneManager.h"


namespace argent
{
	void SubsystemLocator::Awake()
	{
		AddSubsystem<platform::Platform>();
		AddSubsystem<graphics::GraphicsLibrary>();
		AddSubsystem<graphics::RenderingManager>();
		AddSubsystem<input::InputManager>();
		AddSubsystem<Timer>();
		AddSubsystem<scene::SceneManager>();
	}

	void SubsystemLocator::Shutdown()
	{
		GetSubsystem<scene::SceneManager>()->Shutdown();
		GetSubsystem<Timer>()->Shutdown();
		GetSubsystem<input::InputManager>()->Shutdown();
		GetSubsystem<graphics::RenderingManager>();
		GetSubsystem<graphics::GraphicsLibrary>()->Shutdown();
		GetSubsystem<platform::Platform>()->Shutdown();
	}
}
