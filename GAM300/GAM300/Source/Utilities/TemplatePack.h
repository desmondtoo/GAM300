#ifndef TEMPLATE_PACK_H
#define TEMPLATE_PACK_H

template<typename T, typename... Ts>
static constexpr bool contains()
{
	return std::disjunction_v<std::is_same<T, Ts>...>;
}

struct None{};

template <auto N>
struct TemplateContainer
{};

template <typename T, typename... Ts>
struct TemplatePack
{
	template<typename T1>
	static constexpr bool Has()
	{
		return contains<T1,T, Ts...>();
	}

	template <typename... T1s>
	constexpr TemplatePack<T,Ts..., T1s...> Concatenate(TemplatePack<T1s...> pack)
	{
		return TemplatePack<T,Ts..., T1s...>();
	}

	//constexpr auto Pop()
	//{
	//	return Pop<>();
	//}

	//template <typename... T1s>
	//constexpr auto Pop()
	//{
	//	if constexpr (sizeof...(Ts) + 1 > 1)
	//	{
	//		return TemplatePack<Ts...>().Pop<T1s..., T>();
	//	}
	//	else
	//	{
	//		return TemplatePack<T1s...>();
	//	}
	//}
};

#endif // !TEMPLATE_PACK_H