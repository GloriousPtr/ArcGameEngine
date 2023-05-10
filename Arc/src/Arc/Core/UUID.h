#pragma once

namespace ArcEngine
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

namespace eastl
{
	template<>
	struct hash<ArcEngine::UUID>
	{
		std::size_t operator()(const ArcEngine::UUID& uuid) const noexcept
		{
			return hash<uint64_t>()(uuid);
		}
	};
}
