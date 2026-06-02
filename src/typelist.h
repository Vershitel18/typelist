#pragma once
#include <utility>

namespace ct::tl {

template <typename...>
struct TypeList {};

// contains<Type, List>

template <typename, typename>
struct is_same {
  static constexpr bool value = false;
};

template <typename Type>
struct is_same<Type, Type> {
  static constexpr bool value = true;
};

template <typename Type1, typename Type2>
inline constexpr bool is_same_v = is_same<Type1, Type2>::value;

template <typename Type, typename List>
struct contains_impl;

template <typename Type, template <typename...> typename List, typename... Types>
struct contains_impl<Type, List<Types...>> {
  static constexpr bool value = (is_same_v<Type, Types> || ...);
};

template <typename Type, typename List>
inline constexpr bool contains = contains_impl<Type, List>::value;

// flip_all<List>

template <typename Pair>
struct flip_pair;

template <template <typename...> typename List, typename First, typename Second>
struct flip_pair<List<First, Second>> {
  using type = List<Second, First>;
};

template <typename List>
struct flip_all_impl;

template <template <typename...> typename List, typename... Pairs>
struct flip_all_impl<List<Pairs...>> {
  using type = List<typename flip_pair<Pairs>::type...>;
};

template <typename List>
using flip_all = flip_all_impl<List>::type;

// Concat many (fast)

template <template <typename...> typename List, typename... Types1, typename... Types2>
List<Types1..., Types2...> operator+(List<Types1...>, List<Types2...>);

template <typename... Lists>
using concat = decltype((Lists{} + ...));

template <std::size_t VALUE>
struct index {
  static constexpr std::size_t value = VALUE;
};

template <typename List>
struct count_impl;

template <template <typename...> typename List, typename... Types>
struct count_impl<List<Types...>> {
  static constexpr std::size_t value = sizeof...(Types);
};

template <typename List>
inline constexpr std::size_t count = count_impl<List>::value;

template <typename List, typename Seq>
struct enumerate_impl;

template <template <typename...> typename List, typename... Types, std::size_t... INDICES>
struct enumerate_impl<List<Types...>, std::index_sequence<INDICES...>> {
  using type = List<List<index<INDICES>, Types>...>;
};

template <typename List>
using enumerate = enumerate_impl<List, std::make_index_sequence<count<List>>>::type;

template <typename... Types>
struct inherit : Types... {};

template <typename Map, typename Key>
struct map_find_impl;

template <template <typename...> typename List, typename... Lists, typename Key>
struct map_find_impl<List<Lists...>, Key> {
  template <typename Index>
  static List<Index, Key> f(List<Index, Key>*);

  using derived = inherit<Lists...>;

  using type = decltype(f(static_cast<derived*>(nullptr)));
};

template <typename Map, typename Key>
using map_find = map_find_impl<Map, Key>::type;

template <typename Pair>
struct index_in_list;

template <template <typename...> typename List, typename First, typename Second>
struct index_in_list<List<First, Second>> {
  static constexpr std::size_t index = First::value;
};

template <typename Type, typename List>
struct index_of_unique_impl;

template <typename Type, template <typename...> typename List, typename... Types>
struct index_of_unique_impl<Type, List<Types...>> {
  static constexpr std::size_t index = index_in_list<map_find<enumerate<List<Types...>>, Type>>::index;
};

template <typename Type, typename List>
inline constexpr std::size_t index_of_unique = index_of_unique_impl<Type, List>::index;

template <typename List>
struct flatten_impl;

template <typename T>
struct flatten_one {
  using type = TypeList<T>;
};

template <template <typename...> typename List, typename... Types>
struct flatten_one<List<Types...>> {
  using type = flatten_impl<TypeList<Types...>>::type;
};

template <template <typename...> typename List, typename... ChildTypes>
struct flatten_impl<List<ChildTypes...>> {
  using type = concat<typename flatten_one<ChildTypes>::type...>;
};

template <template <typename...> typename List>
struct flatten_impl<List<>> {
  using type = TypeList<>;
};

template <template <typename...> typename OldList, typename NewList>
struct old_type;

template <template <typename...> typename OldList, typename... Types>
struct old_type<OldList, TypeList<Types...>> {
  using type = OldList<Types...>;
};

template <template <typename...> typename OldList, typename NewList>
using old_type_n = old_type<OldList, NewList>::type;

template <typename List>
struct flatten_impl_repack;

template <template <typename...> typename List, typename... Types>
struct flatten_impl_repack<List<Types...>> {
  using type = old_type_n<List, typename flatten_impl<List<Types...>>::type>;
};

template <typename List>
using flatten = flatten_impl_repack<List>::type;

template <bool B>
struct conditional_impl;

template <>
struct conditional_impl<true> {
  template <typename T, typename F>
  using value = T;
};

template <>
struct conditional_impl<false> {
  template <typename T, typename F>
  using value = F;
};

template <bool B, typename T, typename F>
using conditional = conditional_impl<B>::template value<T, F>;

template <bool First, std::size_t N, typename List, typename Seq>
struct index_choice;

template <std::size_t N, template <typename...> typename List, typename... Types, std::size_t... Indexes>
struct index_choice<true, N, List<Types...>, std::index_sequence<Indexes...>> {
  using type = concat<conditional<(Indexes < N), List<Types>, List<>>...>;
};

template <std::size_t N, template <typename...> typename List, typename... Types, std::size_t... Indexes>
struct index_choice<false, N, List<Types...>, std::index_sequence<Indexes...>> {
  using type = concat<conditional<(Indexes >= N), List<Types>, List<>>...>;
};

template <bool First, std::size_t N, typename List>
struct select_index;

template <template <typename...> typename List, typename... Types>
struct select_index<true, 0, List<Types...>> {
  using type = List<>;
};

template <bool First, std::size_t N, template <typename...> typename List, typename... Types>
struct select_index<First, N, List<Types...>> {
  using type = index_choice<First, N, List<Types...>, std::make_index_sequence<sizeof...(Types)>>::type;
};

template <template <typename...> typename List, typename... Types>
struct select_index<false, 0, List<Types...>> {
  using type = List<Types...>;
};

template <std::size_t N, typename List>
using take = select_index<true, N, List>::type;

template <std::size_t N, typename List>
using drop = select_index<false, N, List>::type;

template <template <typename, typename> typename Compare, typename Left, typename Right>
struct merge;

template <template <typename, typename> typename Compare, template <typename...> typename List, typename... Right>
struct merge<Compare, List<>, List<Right...>> {
  using type = List<Right...>;
};

template <template <typename, typename> typename Compare, template <typename...> typename List, typename... Left>
struct merge<Compare, List<Left...>, List<>> {
  using type = List<Left...>;
};

template <bool Cond, template <typename, typename> typename Compare, typename List1, typename List2>
struct merge_choice;

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List,
    typename Head1,
    typename... Tail1,
    typename Head2,
    typename... Tail2>
struct merge_choice<false, Compare, List<Head1, Tail1...>, List<Head2, Tail2...>> {
  using type = concat<List<Head1>, typename merge<Compare, List<Tail1...>, List<Head2, Tail2...>>::type>;
};

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List,
    typename Head1,
    typename... Tail1,
    typename Head2,
    typename... Tail2>
struct merge_choice<true, Compare, List<Head1, Tail1...>, List<Head2, Tail2...>> {
  using type = concat<List<Head2>, typename merge<Compare, List<Head1, Tail1...>, List<Tail2...>>::type>;
};

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List,
    typename Head1,
    typename... Tail1,
    typename Head2,
    typename... Tail2>
struct merge<Compare, List<Head1, Tail1...>, List<Head2, Tail2...>> {
  using type = merge_choice<Compare<Head2, Head1>::value, Compare, List<Head1, Tail1...>, List<Head2, Tail2...>>::type;
};

template <template <typename, typename> typename Compare, typename List>
struct merge_sort_impl;

template <template <typename, typename> typename Compare, template <typename...> typename List, typename Type>
struct merge_sort_impl<Compare, List<Type>> {
  using type = List<Type>;
};

template <template <typename, typename> typename Compare, template <typename...> typename List>
struct merge_sort_impl<Compare, List<>> {
  using type = List<>;
};

template <template <typename, typename> typename Compare, template <typename...> typename List, typename... Types>
struct merge_sort_impl<Compare, List<Types...>> {
  using left = merge_sort_impl<Compare, take<count<List<Types...>> / 2, List<Types...>>>::type;
  using right = merge_sort_impl<Compare, drop<count<List<Types...>> / 2, List<Types...>>>::type;
  using type = merge<Compare, left, right>::type;
};

template <template <typename, typename> typename Compare, typename List>
using merge_sort = merge_sort_impl<Compare, List>::type;

} // namespace ct::tl
