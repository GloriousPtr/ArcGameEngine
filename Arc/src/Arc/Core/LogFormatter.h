#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "UUID.h"

namespace std
{
	template<>
	struct formatter<ArcEngine::UUID>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const ArcEngine::UUID& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", static_cast<uint64_t>(input));
		}
	};

	template<>
	struct formatter<eastl::string>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const eastl::string& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", input.c_str());
		}
	};

	template<>
	struct formatter<eastl::string_view>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const eastl::string_view& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", input.data());
		}
	};

	template<>
	struct formatter<filesystem::path>
	{
		constexpr auto parse(format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const filesystem::path& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "{}", input.string());
		}
	};

	template<>
	struct formatter<glm::vec2>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const glm::vec2& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "[{}, {}]", input.x, input.y);
		}
	};

	template<>
	struct formatter<glm::vec3>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const glm::vec3& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "[{}, {}, {}]", input.x, input.y, input.z);
		}
	};

	template<>
	struct formatter<glm::vec4>
	{
		constexpr auto parse(const format_parse_context& ctx) const -> decltype(ctx.begin())
		{
			return ctx.end();
		}

		template <typename FormatContext>
		auto format(const glm::vec4& input, FormatContext& ctx) -> decltype(ctx.out())
		{
			return format_to(ctx.out(), "[{}, {}, {}, {}]", input.x, input.y, input.z, input.w);
		}
	};
}
