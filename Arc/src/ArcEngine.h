#pragma once

// For use by Arc applications

#include "Arc/Core/Base.h"

#include "Arc/Core/Application.h"
#include "Arc/Core/Layer.h"
#include "Arc/Core/Log.h"
#include "Arc/Core/Assert.h"
#include "Arc/Core/AssetManager.h"

#include "Arc/Core/Timestep.h"

#include "Arc/Core/Input.h"
#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"

#include "Arc/ImGui/ImGuiLayer.h"

#include "Arc/Scene/Scene.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"

//-----Renderer-----------------------------------
#include "Arc/Renderer/Renderer.h"
#include "Arc/Renderer/Renderer2D.h"
#include "Arc/Renderer/Renderer3D.h"
#include "Arc/Renderer/RenderCommand.h"
#include "Arc/Renderer/RenderGraphData.h"

#include "Arc/Renderer/Buffer.h"
#include "Arc/Renderer/Shader.h"
#include "Arc/Renderer/Material.h"
#include "Arc/Renderer/Framebuffer.h"
#include "Arc/Renderer/Texture.h"
#include "Arc/Renderer/VertexArray.h"

//------------------------------------------------

#include "Arc/Scripting/ScriptEngine.h"

#include "Arc/Utils/ColorUtils.h"
#include "Arc/Utils/PlatformUtils.h"
#include "Arc/Utils/StringUtils.h"
