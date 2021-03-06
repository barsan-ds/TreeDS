#pragma once

#include <cassert>     // assert
#include <cstddef>     // std::size_t
#include <functional>  // std::invoke()
#include <tuple>       // std::std::make_from_tuple
#include <type_traits> // std::decay_t, std::std::enable_if_t, std::is_invocable_v, std::void_t
#include <utility>     // std::declval(), std::make_index_sequence

namespace md {

/*   ---   FORWARD DECLARATIONS   ---   */
template <typename, typename, typename>
class struct_node;

template <typename>
class binary_node;

template <typename>
class nary_node;

template <typename>
class node_navigator;

template <typename, typename, typename, typename>
class generative_navigator;

template <typename, typename...>
class multiple_node_pointer;

/*   ---   TYPE TRAITS   ---   */
template <typename T, typename = void>
constexpr bool is_equality_comparable = false;

template <typename T>
constexpr bool is_equality_comparable<
    T,
    std::enable_if_t<
        std::is_same_v<
            bool,
            decltype(std::declval<const T>() == std::declval<const T>())>>> = true;

template <
    typename T,
    typename Tuple,
    typename = void>
constexpr bool is_constructible_from_tuple = false;

template <
    typename T,
    typename Tuple>
constexpr bool is_constructible_from_tuple<
    T,
    Tuple,
    std::enable_if_t<
        std::is_invocable_v<
            decltype(std::make_from_tuple<T>),
            const Tuple&>>> = true;

template <typename Policy, typename = void>
constexpr bool is_tag_of_policy = false;

template <typename Policy>
constexpr bool is_tag_of_policy<
    Policy,
    std::void_t<
        decltype(
            std::declval<Policy>()
                .template get_instance<
                    binary_node<int>*,
                    node_navigator<binary_node<int>*>,
                    std::allocator<binary_node<int>>>(
                    std::declval<binary_node<int>*>(),
                    std::declval<node_navigator<binary_node<int>*>>(),
                    std::declval<std::allocator<int>>())),
        Policy>> = true;

// Check method Type::get_resources() exists
template <typename Type, typename = void>
constexpr bool holds_resources = false;

template <typename Type>
constexpr bool holds_resources<
    Type,
    std::void_t<decltype(std::declval<Type>().get_resources())>> = true;

// Check if two types are instantiation of the same template
template <typename T, typename U>
constexpr bool is_same_template = std::is_same_v<T, U>;

template <
    template <typename...> class T,
    typename... A,
    typename... B>
constexpr bool is_same_template<T<A...>, T<B...>> = true;

template <
    template <auto...> class T,
    auto... A,
    auto... B>
constexpr bool is_same_template<T<A...>, T<B...>> = true;

template <typename A, typename B>
constexpr bool is_const_cast_equivalent
    = std::is_same_v<A, B> || std::conjunction_v<
          // If both are either pointers or not
          std::bool_constant<std::is_pointer_v<A> == std::is_pointer_v<B>>,
          // They point to the same type (ignoring const/volatile)
          std::is_same<std::decay_t<std::remove_pointer_t<A>>, std::decay_t<std::remove_pointer_t<B>>>>;

template <
    typename T,
    typename = void>
constexpr bool is_binary_node_pointer = false;

template <typename T>
constexpr bool is_binary_node_pointer<
    T,
    std::enable_if_t<
        is_same_template<
            std::decay_t<std::remove_pointer_t<T>>,
            binary_node<void>>>> = true;

template <typename T, typename... Others>
constexpr bool is_binary_node_pointer<
    multiple_node_pointer<T, Others...>,
    std::enable_if_t<is_binary_node_pointer<T>>> = true;

template <std::size_t Index>
struct const_index {};

template <char... Name>
struct const_name {};

template <typename T>
struct type_value {
    using type = T;
};

namespace detail {
    struct empty_t {};
} // namespace detail

template <typename X>
inline constexpr bool is_empty = std::is_same_v<std::decay_t<X>, detail::empty_t>;

template <typename X>
inline constexpr bool is_empty_node = false;

template <typename FirstChild, typename NextSibling>
inline constexpr bool is_empty_node<struct_node<detail::empty_t, FirstChild, NextSibling>> = true;

template <typename Node, typename Call, typename Test, typename Result>
Node keep_calling(Node from, Call&& call, Test&& test, Result&& result) {
    Node prev = from;
    Node next = call(prev);
    while (next) {
        if (test(prev, next)) {
            return result(prev, next);
        }
        prev = next;
        next = call(prev);
    }
    return prev; // Returns something only if test() succeeds
}

/**
 * This function can be used to keep calling a specific lambda {@link Callable}. The passed type must be convertible to
 * a function that take a reference to constant node and returns a pointer to constant node. The best is to pass a
 * lambda so that it can be inlined.
 *
 * The case from == nullptr is correctly managed.
 */
template <typename Node, typename Call>
Node keep_calling(Node from, Call&& call) {
    Node prev = from;
    Node next = call(prev);
    while (next) {
        prev = next;
        next = call(prev);
    }
    return prev;
}

template <typename T>
std::size_t calculate_size(const binary_node<T>& node) {
    return 1
        + (node.get_left_child() != nullptr
               ? calculate_size(*node.get_left_child())
               : 0)
        + (node.get_right_child() != nullptr
               ? calculate_size(*node.get_right_child())
               : 0);
}

template <typename T>
std::size_t calculate_size(const nary_node<T>& node) {
    std::size_t size          = 1;
    const nary_node<T>* child = node.get_first_child();
    while (child != nullptr) {
        size += calculate_size(*child);
        child = child->get_next_sibling();
    }
    return size;
}

template <typename Node>
std::size_t calculate_arity(const Node& node, std::size_t max_expected_arity) {
    const Node* child = node.get_first_child();
    std::size_t arity = child
        ? child->following_siblings() + 1
        : 0u;
    while (child) {
        if (arity == max_expected_arity) {
            return arity;
        }
        arity = std::max(arity, calculate_arity(*child, max_expected_arity));
        child = child->get_next_sibling();
    }
    return arity;
}

namespace detail {
    /***
     * @brief Retrieve unsing a runtime index the element of a tuple.
     */
    template <std::size_t Target> // target is index + 1, Target = 0 is invalid
    struct element_apply_construction {
        template <typename F, typename Tuple>
        static constexpr decltype(auto) single_apply(F&& f, Tuple&& tuple, std::size_t index) {
            static_assert(Target > 0, "Requested index does not exist in the tuple.");
            if (index == Target - 1) {
                return f(std::get<Target - 1>(tuple));
            } else {
                return element_apply_construction<Target - 1>::single_apply(
                    std::forward<F>(f),
                    std::forward<Tuple>(tuple),
                    index);
            }
        }
    };
    template <>
    struct element_apply_construction<0> {
        template <typename F, typename Tuple>
        static constexpr decltype(auto) single_apply(F&& f, Tuple&& tuple, std::size_t) {
            assert(false);
            // This return is just for return type deduction
            return f(std::get<0>(tuple));
        }
    };
} // namespace detail

/**
 * @brief Invoke the Callable object with an argument taken from the tuple at position {@link index}.
 */
template <typename F, typename Tuple>
decltype(auto) apply_at_index(F&& f, Tuple&& tuple, std::size_t index) {
    static_assert(is_same_template<std::decay_t<Tuple>, std::tuple<>>, "Expected the second argument to be a tuple.");
    constexpr std::size_t tuple_size = std::tuple_size_v<std::decay_t<Tuple>>;
    return detail::element_apply_construction<tuple_size>::single_apply(
        std::forward<F>(f),
        std::forward<Tuple>(tuple),
        index);
}

template <typename F, typename Initial>
constexpr auto foldl(F&&, Initial&& initial) {
    return initial;
}

template <typename F, typename Initial, typename First, typename... Remainning>
constexpr auto foldl(F&& f, Initial&& initial, First&& x, Remainning&&... xs) {
    return foldl(f, f(initial, x), xs...);
}

template <typename F, typename Initial>
constexpr auto foldr(F&&, Initial&& initial) {
    return initial;
}

template <typename F, typename Initial, typename First, typename... Remainning>
constexpr auto foldr(F&& f, Initial&& initial, First&& x, Remainning&&... xs) {
    return f(x, foldr(f, initial, xs...));
}

} // namespace md

#ifndef NDEBUG
#include <iomanip> // std::setfill, std::setw
#include <ios>     // std::hex
#include <iostream>
#include <string>
#include <string_view>

namespace md {

template <typename T, typename = void>
constexpr bool is_printable = false;

template <typename T>
constexpr bool is_printable<T, std::void_t<decltype(std::declval<decltype(std::cout)>() << std::declval<T>())>> = true;

void code_like_print(std::ostream& stream) {
    stream << "n()";
}

void code_like_print(std::ostream& stream, char c) {
    stream << '\'' << c << '\'';
}

void code_like_print(std::ostream& stream, const char* c) {
    stream << '\"' << c << '\"';
}

void code_like_print(std::ostream& stream, const std::string& c) {
    stream << '\"' << c << '\"';
}

void code_like_print(std::ostream& stream, std::string_view c) {
    stream << '\"' << c << '\"';
}

template <typename T>
void code_like_print(std::ostream& stream, const T& c) {
    if constexpr (is_printable<T>) {
        stream << c;
    } else if constexpr (std::is_convertible_v<T, std::string>) {
        stream << static_cast<std::string>(c);
    } else {
        stream << "_";
    }
}

#ifndef MD_PRINT_TREE_MAX_NODES
#define MD_PRINT_TREE_MAX_NODES 10
#endif

#ifndef MD_PRINT_TREE_INDENTATION
#define MD_PRINT_TREE_INDENTATION 8
#endif

#ifndef MD_PRINT_TREE_ADDRESS_DIGITS
#define MD_PRINT_TREE_ADDRESS_DIGITS 0
#endif

struct print_preferences {
    unsigned indentation_increment = MD_PRINT_TREE_INDENTATION;
    int limit                      = MD_PRINT_TREE_MAX_NODES;
    unsigned address_digits        = MD_PRINT_TREE_ADDRESS_DIGITS;
};

template <typename Node>
void print_node(
    std::ostream& os,
    const Node& node,
    unsigned indentation,
    print_preferences&& preferences = {}) {
    if (preferences.limit <= 0) {
        return;
    }
    os << std::string(indentation, ' ') << "n(";
    code_like_print(os, node.get_value());
    if (preferences.address_digits > 0) {
        os << " @"
           << std::setfill('0')
           << std::setw(preferences.address_digits)
           << std::hex
           << (reinterpret_cast<std::size_t>(&node) & ((std::size_t(1u) << (4u * preferences.address_digits)) - 1));
    }
    os << ')';
    const Node* current = node.get_first_child();
    if (current) {
        os << "(\n";
        if constexpr (is_same_template<Node, binary_node<void>>) {
            if (current->is_right_child()) {
                os << std::string(indentation + preferences.indentation_increment, ' ') << "n(),\n";
            }
        }
        if (--preferences.limit > 0) {
            print_node(os, *current, indentation + preferences.indentation_increment, std::move(preferences));
            for (
                current = current->get_next_sibling();
                current != nullptr;
                current = current->get_next_sibling()) {
                os << ",\n";
                if (--preferences.limit > 0) {
                    print_node(os, *current, indentation + preferences.indentation_increment, std::move(preferences));
                } else {
                    os << std::string(indentation + preferences.indentation_increment, ' ') << "...";
                    break;
                }
            }
        } else {
            os << std::string(indentation + 4, ' ') << "...";
        }
        os << ")";
    }
}

template <typename Tree>
void print_tree(
    std::ostream& os,
    const Tree& tree,
    print_preferences&& preferences = {}) {
    if (!tree.empty()) {
        print_node(os, *tree.raw_root_node(), 0u, std::move(preferences));
    } else {
        code_like_print(os);
    }
}

} // namespace md

#endif
