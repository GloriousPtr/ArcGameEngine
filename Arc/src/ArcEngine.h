#pragma once

// For use by Arc applications

#include <glm/glm.hpp>
#include <magic_enum.hpp>

#include <EASTL/span.h>
#include <EASTL/memory.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/string.h>
#include <EASTL/string_view.h>
#include <EASTL/array.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>
#include <EASTL/hash_map.h>
#include <EASTL/map.h>
#include <EASTL/unordered_map.h>
#include <EASTL/set.h>
#include <EASTL/unordered_set.h>
#include <EASTL/stack.h>
#include <EASTL/queue.h>

#include "Arc/Core/Base.h"

#include "Arc/Core/Application.h"
#include "Arc/Core/Layer.h"
#include "Arc/Core/Log.h"
#include "Arc/Core/Assert.h"
#include "Arc/Core/AssetManager.h"
#include "Arc/Core/Window.h"

#include "Arc/Core/Timestep.h"

#include "Arc/Core/Input.h"
#include "Arc/Core/KeyCodes.h"
#include "Arc/Core/MouseCodes.h"
#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Events/KeyEvent.h"
#include "Arc/Events/MouseEvent.h"

#include "Arc/ImGui/ImGuiLayer.h"

#include "Arc/Scene/Scene.h"
#include "Arc/Scene/Entity.h"
#include "Arc/Scene/Components.h"

#include "Arc/Project/Project.h"

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
