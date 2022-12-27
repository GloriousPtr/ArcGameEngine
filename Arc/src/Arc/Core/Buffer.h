#pragma once

namespace ArcEngine
{
	struct Buffer
	{
		uint8_t* Data = nullptr;
		uint64_t Size = 0;

		Buffer() = default;
		explicit Buffer(uint64_t size) { Allocate(size); }

		[[nodiscard]] static Buffer Copy(Buffer other)
		{
			auto result = Buffer(other.Size);
			std::memcpy(result.Data, other.Data, result.Size);
			return result;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = new uint8_t[size];
			Size = size;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		template<typename T>
		[[nodiscard]] T* As() { return reinterpret_cast<T*>(Data); }

		operator bool() const {	return Data; }
	};

	struct ScopedBuffer
	{
		ScopedBuffer(Buffer buffer)
			: m_Buffer(buffer)
		{
		}

		explicit ScopedBuffer(uint64_t size)
			: m_Buffer(size)
		{
		}

		~ScopedBuffer() { m_Buffer.Release(); }

		[[nodiscard]] uint8_t* Data() const { return m_Buffer.Data; }
		[[nodiscard]] uint64_t Size() const { return m_Buffer.Size; }

		template<typename T>
		[[nodiscard]] T* As() { return m_Buffer.As<T>(); }

		operator bool() const { return m_Buffer; }

	private:
		Buffer m_Buffer;
	};
}
