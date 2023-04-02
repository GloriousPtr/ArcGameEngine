#pragma once

namespace ArcEngine
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	[[nodiscard]] static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::None:									return 0;
			case ShaderDataType::Float:									return 4;
			case ShaderDataType::Float2:								return 4 * 2;
			case ShaderDataType::Float3:								return 4 * 3;
			case ShaderDataType::Float4:								return 4 * 4;
			case ShaderDataType::Mat3:									return 4 * 3 * 3;
			case ShaderDataType::Mat4:									return 4 * 4 * 4;
			case ShaderDataType::Int:									return 4;
			case ShaderDataType::Int2:									return 4 * 2;
			case ShaderDataType::Int3:									return 4 * 3;
			case ShaderDataType::Int4:									return 4 * 4;
			case ShaderDataType::Bool:									return 1;
		}

		ARC_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}
	
	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		size_t Offset;
		bool Normalized;

		BufferElement()
			: Type(ShaderDataType::None), Size(0), Offset(0), Normalized(false)
		{
		}

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{
		}

		[[nodiscard]] uint8_t GetComponentCount() const
		{
			switch(Type)
			{
				case ShaderDataType::None:									return 0;
				case ShaderDataType::Float:									return 1;
				case ShaderDataType::Float2:								return 2;
				case ShaderDataType::Float3:								return 3;
				case ShaderDataType::Float4:								return 4;
				case ShaderDataType::Mat3:									return 3;
				case ShaderDataType::Mat4:									return 4;
				case ShaderDataType::Int:									return 1;
				case ShaderDataType::Int2:									return 2;
				case ShaderDataType::Int3:									return 3;
				case ShaderDataType::Int4:									return 4;
				case ShaderDataType::Bool:									return 1;
			}

			ARC_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};
	
	class BufferLayout
	{
	public:
		BufferLayout() = default;
		
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		[[nodiscard]] uint32_t GetStride() const { return m_Stride; }
		[[nodiscard]] const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		[[nodiscard]] std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		[[nodiscard]] std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};
	
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;
		
		virtual void Bind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		
		[[nodiscard]] static Ref<VertexBuffer> Create(uint32_t size, uint32_t stride);
		[[nodiscard]] static Ref<VertexBuffer> Create(const float* vertices, uint32_t size, uint32_t stride);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;

		[[nodiscard]] virtual uint32_t GetCount() const = 0;

		[[nodiscard]] static Ref<IndexBuffer> Create(const uint32_t* indices, size_t count);
	};

	class ConstantBuffer
	{
	public:
		virtual ~ConstantBuffer() = default;

		virtual void Bind(uint32_t index) const = 0;
		virtual void SetData(const void* data, uint32_t size, uint32_t index) = 0;

		[[nodiscard]] static Ref<ConstantBuffer> Create(uint32_t stride, uint32_t count, uint32_t registerIndex);
	};

	class StructuredBuffer
	{
	public:
		virtual ~StructuredBuffer() = default;

		virtual void Bind() const = 0;
		virtual void SetData(const void* data, uint32_t size, uint32_t index) = 0;

		[[nodiscard]] static Ref<StructuredBuffer> Create(uint32_t stride, uint32_t count, uint32_t registerIndex);
	};
}
