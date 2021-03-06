#pragma once

#include <cstddef> // std::size_t

#include <TreeDS/utility.hpp>

namespace md {

/*   ---   FORWARD DECLARATIONS   ---   */
template <typename, typename, typename, typename>
class matcher;

template <typename, typename, typename>
class capture_node;

/*   ---   TYPES DEINITIONS   ---   */
struct matcher_info_t {
    bool matches_null;
    bool shallow_matches_null;
    bool prefers_null;
    bool possessive;
    constexpr matcher_info_t(bool matches_null, bool shallow_matches_null, bool prefers_null, bool possessive) :
            matches_null(matches_null),
            shallow_matches_null(shallow_matches_null),
            prefers_null(prefers_null),
            possessive(possessive) {
    }
};

enum class quantifier {
    DEFAULT,
    RELUCTANT,
    GREEDY,
    POSSESSIVE
};

namespace detail {

    template <typename T>
    struct matcher_traits {};

    template <typename Derived, typename ValueMatcher>
    struct matcher_traits<matcher<Derived, ValueMatcher, detail::empty_t, detail::empty_t>> {
        using children_captures = std::tuple<>;
        using siblings_captures = std::tuple<>;
    };

    template <typename Derived, typename ValueMatcher, typename NextSibling>
    struct matcher_traits<matcher<Derived, ValueMatcher, detail::empty_t, NextSibling>> {
        using children_captures = std::tuple<>;
        using siblings_captures = typename NextSibling::captures_t;
    };

    template <typename Derived, typename ValueMatcher, typename FirstChild>
    struct matcher_traits<matcher<Derived, ValueMatcher, FirstChild, detail::empty_t>> {
        using children_captures = std::tuple<>;
        using siblings_captures = typename FirstChild::captures_t;
    };

    template <typename Derived, typename ValueMatcher, typename FirstChild, typename NextSibling>
    struct matcher_traits<matcher<Derived, ValueMatcher, FirstChild, NextSibling>> {
        using children_captures = typename FirstChild::captures_t;
        using siblings_captures = typename NextSibling::captures_t;
    };

    /*   ---   METAPROGRAMMING VARIABLES   ---   */
    template <typename T>
    constexpr bool is_const_name = is_same_template<T, const_name<>> && !std::is_same_v<T, const_name<>>;

    /*
     * This following variables are used to get the index of the matcher having a specific const_name<> from captures
     * tuple. The correct way to use it is to always refer to index_of_capture<const_name<...>, captures_t>. Let's see
     * it in action using an example.
     *
     * Examples (types are unrelated, just to keep notation simple and explain the logic behind):
     * Exists:
     * Name = int,
     * Tuple = std::tuple<std::vector<char>, std::list<int>, std::set<int>, std::deque<bool>>
     * We want to get the type set<int>
     * index_of_capture<int, std::tuple<std::vector<char>, std::list<int>, std::set<int>, std::deque<bool>>>
     *     = 1 + index_of_capture<int, std::tuple<std::list<int>, std::set<int>, std::deque<bool>>> // #3
     *     = 1 + 1 + index_of_capture<int, std::tuple<std::set<int>, std::deque<bool>>>             // #2
     *                                                ^^^^^^^^^^^^^ // Remember we are looking for this guy
     *     = 1 + 1 + 0
     *     = 2 <- std::set<int> is Tuple's element 2.
     *
     * Doesn't exist:
     * Name = int,
     * index_of_capture<int, std::tuple<std::vector<char>, std::list<int>, std::set<int>, std::deque<bool>>>
     * We want to get the type deque<int>
     * index_of_capture<int, std::tuple<std::vector<char>, std::list<int>, std::set<int>, std::deque<bool>>>
     *     = 1 + index_of_capture<int, std::tuple<std::list<int>, std::set<int>, std::deque<bool>>> // #3
     *     = 1 + 1 + index_of_capture<int, std::tuple<std::set<int>, std::deque<bool>>>             // #3
     *     = 1 + 1 + 1 + index_of_capture<int, std::tuple<std::deque<bool>>>                        // #3
     *     = 1 + 1 + 1 + 1 + index_of_capture<int, std::tuple<>>                                    // #1
     *     = 1 + 1 + 1 + 1 + 0
     *     = 4 <- Larger than the maximum allowed index.
     */
    // 1
    template <
        typename Name,  // Name to look for
        typename Tuple> // Tuple with the rest of the elements
    constexpr std::size_t index_of_capture = 1;

    // 2
    template <typename CaptureName, typename FirstChild, typename NextSibling, typename... Types>
    constexpr std::size_t index_of_capture<
        CaptureName,
        std::tuple<
            matcher<
                capture_node<CaptureName, FirstChild, NextSibling>,
                CaptureName,
                FirstChild,
                NextSibling>&,
            Types...>> = 0;
    //                                  ^^^^^^^^^^^          ^^^^^^^^^^^ We are looking for this

    // 3
    template <class T, class U, typename... Types>
    // Unpacking happens here: we discard the first element of the tuple (U) and recur using the remaining elements
    constexpr std::size_t index_of_capture<
        T,
        std::tuple<U, Types...>> = 1 + index_of_capture<T, std::tuple<Types...>>;

    template <typename Name, typename Tuple>
    constexpr bool is_valid_name = index_of_capture<Name, Tuple> < std::tuple_size_v<Tuple>;
} // namespace detail

} // namespace md
