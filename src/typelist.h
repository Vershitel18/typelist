#pragma once
#include <type_traits>
#include <utility>

namespace ct::tl {

template <typename... Types>
struct TypeList {};

// contains<Type, List> — возвращает true, если List содержит Type, иначе false;
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

// flip_all<List> — принимает список пар типов (пара — тайплист размера 2),
// возвращает список, в котором у каждой пары элементы переставлены местами;

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

// template <template <typename> typename F, typename List>
// struct transform_impl;
//
// template <template <typename> typename F, typename... Types>
// struct transform_impl<F, TypeList<Types...>>
// {
//     using type = TypeList<F<Types>...>;
// };
//
// template <template <typename> typename F, typename List>
// using transform = transform_impl<F, List>::type;

// Apply

template <template <typename...> typename F, typename List>
struct apply_impl;

template <template <typename...> typename F, template <typename...> typename List, typename... Types>
struct apply_impl<F, List<Types...>> {
  using type = F<Types...>;
};

template <template <typename...> typename F, typename List>
using apply = apply_impl<F, List>::type;

// Concat of two

// template <typename List1, typename List2>
// struct concat2_impl;
//
// template <template<typename...> typename List, typename... Types1, typename... Types2>
// struct concat2_impl<List<Types1...>, List<Types2...>>
// {
//     using type = List<Types1..., Types2...>;
// };
//
// template <typename List1, typename List2>
// using concat2 = concat2_impl<List1, List2>::type;

// Push front/back
//
// template <typename Type, typename List>
// using push_back = concat2<List, TypeList<Type>>;

// Pop front

// template <typename List>
// struct pop_front_impl;
//
// template <typename Head, typename... Rest>
// struct pop_front_impl<TypeList<Head, Rest...>>
// {
//     using type = TypeList<Rest...>;
// };
//
// template <typename List>
// using pop_front = pop_front_impl<List>::type;

// Replace front

// template <typename Type, typename List>
// using replace_front = push_front<Type, pop_front<List>>;

// Pop back

// template <typename List>
// struct pop_back_impl;
//
// template <typename List>
// using pop_back = pop_back_impl<List>::type;
//
// template <typename Type>
// struct pop_back_impl<TypeList<Type>>
// {
//     using type = TypeList<>;
// };
//
// template <typename Head, typename... Rest>
// struct pop_back_impl<TypeList<Head, Rest...>>
// {
//     using type = push_front<Head, pop_back<TypeList<Rest...>>>;
// };

// template<typename List>
// using pop_back = reverse<pop_front<reverse<List>>>;

// Concat many (slow)

// template <typename... Lists>
// struct concat_impl;
//
// template <typename... Lists>
// using concat = concat_impl<Lists...>::type;
//
// template <typename List>
// struct concat_impl<List>
// {
//     using type = List;
// };
//
// template <typename Head, typename... Rest>
// struct concat_impl<Head, Rest...>
// {
//     using type = concat2<Head, concat<Rest...>>;
// };

// Concat many (fast)

template <template <typename...> typename List, typename... Types1, typename... Types2>
List<Types1..., Types2...> operator+(List<Types1...>, List<Types2...>);

template <typename... Lists>
using concat_fast = decltype((Lists{} + ...));

// Lifting values/functions to types

// template <auto VALUE>
// struct lift_value
// {
//     static constexpr auto value = VALUE;
// };
// template <template <typename...> typename F>
// struct lift_func
// {
//     template <typename... Types>
//     using type = F<Types...>;
// };

// Filter

// template <template <typename> typename Pred, typename List>
// struct filter_impl;
//
// template <template <typename> typename Pred, typename... Types>
// struct filter_impl<Pred, TypeList<Types...>>
// {
//     template <typename T>
//     using single = std::conditional_t<Pred<T>::value, TypeList<T>, TypeList<>>;
//
//     using type = concat_fast<single<Types>...>;
// };
//
// template <template <typename> typename Pred, typename List>
// using filter = filter_impl<Pred, List>::type;
//
// template <typename T>
// using my_predicate = std::bool_constant<(sizeof(T) > 4)>;

// index_of_unique<Type, List> — возвращает индекс единственного вхождения Type в
// List (если вхождение не единственное, вызов должен приводить к ошибке компиляции);

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
struct index_in;

template <template <typename...> typename List, typename First, typename Second>
struct index_in<List<First, Second>> {
  static constexpr std::size_t index = First::value;
};

template <typename Type, typename List>
struct index_of_unique_impl;

template <typename Type, template <typename...> typename List, typename... Types>
struct index_of_unique_impl<Type, List<Types...>> {
  static constexpr std::size_t index = index_in<map_find<enumerate<List<Types...>>, Type>>::index;
};

template <typename Type, typename List>
inline constexpr std::size_t index_of_unique = index_of_unique_impl<Type, List>::index;

template <typename List>
struct flatten_impl;

// template <template <typename...> typename List, typename Head, typename... Rest>
// struct flatten_impl<List<Head, Rest...>>
// {
//   using type = concat_fast<List<Head>, typename flatten_impl<List<Rest...>>::type>;
// };
template <typename T>
struct flutten_one {
  using type = TypeList<T>;
};

template <template <typename...> typename List, typename... Types>
struct flutten_one<List<Types...>> {
  using type = flatten_impl<TypeList<Types...>>::type;
};

template <template <typename...> typename List, typename... ChildTypes>
struct flatten_impl<List<ChildTypes...>> {
  using type = concat_fast<typename flutten_one<ChildTypes>::type...>;
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

// merge_sort<Compare, List> — возвращает копию List, но с элементами,
// отсортированными компаратором Compare (он принимает два типа, и возвращает true,
// если первый должен идти до второго).

template <std::size_t N, typename List>
struct take_impl;

template <std::size_t N, template <typename...> typename List, typename Head, typename... Tail>
  requires (N > 0)
struct take_impl<N, List<Head, Tail...>> {
  using type = concat_fast<List<Head>, typename take_impl<N - 1, List<Tail...>>::type>;
};

template <template <typename...> typename List, typename... Types>
struct take_impl<0, List<Types...>> {
  using type = List<>;
};

template <std::size_t N, typename List>
using take = take_impl<N, List>::type;

template <std::size_t N, typename List>
struct dropchik_impl;

template <std::size_t N, template <typename...> typename List, typename Head, typename... Tail>
  requires (N > 0)
struct dropchik_impl<N, List<Head, Tail...>> {
  using type = dropchik_impl<N - 1, List<Tail...>>::type;
};

template <template <typename...> typename List, typename... Types>
struct dropchik_impl<0, List<Types...>> {
  using type = List<Types...>;
};

template <std::size_t N, typename List>
using dropchik = dropchik_impl<N, List>::type;

// template <typename Type, typename List>
// using push_front = concat_fast<TypeList<Type>, List>;

// template <bool B>
// struct conditional_impl;
//
// template <>
// struct conditional_impl<true>
// {
//   template <typename T, typename F>
//   using value = T;
// };
//
// template <>
// struct conditional_impl<false>
// {
//   template <typename T, typename F>
//   using value = F;
// };
//
// template <bool B, typename T, typename F>
// using conditional = conditional_impl<B>::template value<T, F>;

template <template <typename, typename> typename Compare, typename List1, typename List2>
struct merge;

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List1,
    typename... Types,
    template <typename...> typename List2>
struct merge<Compare, List1<Types...>, List2<>> {
  using type = List1<Types...>;
};

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List1,
    template <typename...> typename List2,
    typename... Types>
struct merge<Compare, List1<>, List2<Types...>> {
  using type = List2<Types...>;
};

// true
template <bool Cond, template <typename, typename> typename Compare, typename List1, typename List2>
struct merge_choise;

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List1,
    typename Head1,
    typename... Tail1,
    template <typename...> typename List2,
    typename Head2,
    typename... Tail2>
struct merge_choise<false, Compare, List1<Head1, Tail1...>, List2<Head2, Tail2...>> {
  using type = concat_fast<List1<Head1>, typename merge<Compare, List1<Tail1...>, List2<Head2, Tail2...>>::type>;
};

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List1,
    typename Head1,
    typename... Tail1,
    template <typename...> typename List2,
    typename Head2,
    typename... Tail2>
struct merge_choise<true, Compare, List1<Head1, Tail1...>, List2<Head2, Tail2...>> {
  using type = concat_fast<List2<Head2>, typename merge<Compare, List1<Head1, Tail1...>, List2<Tail2...>>::type>;
};

template <
    template <typename, typename> typename Compare,
    template <typename...> typename List1,
    typename Head1,
    typename... Tail1,
    template <typename...> typename List2,
    typename Head2,
    typename... Tail2>
struct merge<Compare, List1<Head1, Tail1...>, List2<Head2, Tail2...>> {
  using type =
      typename merge_choise<Compare<Head2, Head1>::value, Compare, List1<Head1, Tail1...>, List2<Head2, Tail2...>>::
          type;
};

template <template <typename, typename> typename Compare, typename List>
struct merge_sort_impl;

// base recursion
template <template <typename, typename> typename Compare, template <typename...> typename List, typename Type>
struct merge_sort_impl<Compare, List<Type>> {
  using type = List<Type>;
};

template <template <typename, typename> typename Compare, template <typename...> typename List>
struct merge_sort_impl<Compare, List<>> {
  using type = List<>;
};

template <template <typename, typename> typename Compare, template <typename...> typename List, typename... Types>
  requires (count<List<Types...>> > 1)
struct merge_sort_impl<Compare, List<Types...>> {
  using left = typename merge_sort_impl<Compare, take<count<List<Types...>> / 2, List<Types...>>>::type;
  using right = typename merge_sort_impl<Compare, dropchik<count<List<Types...>> / 2, List<Types...>>>::type;
  using type = typename merge<Compare, left, right>::type;
};

template <template <typename, typename> typename Compare, typename List>
using merge_sort = typename merge_sort_impl<Compare, List>::type;

} // namespace ct::tl
