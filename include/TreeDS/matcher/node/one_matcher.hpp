#pragma once

#include <functional> // std::reference_wrapper
#include <optional>

#include <TreeDS/allocator_utility.hpp>
#include <TreeDS/matcher/node/matcher.hpp>
#include <TreeDS/matcher/pattern.hpp>
#include <TreeDS/matcher/value/true_matcher.hpp>
#include <TreeDS/policy/siblings.hpp>

namespace md {

template <typename ValueMatcher, typename FirstChild, typename NextSibling>
class one_matcher : public matcher<
                        one_matcher<ValueMatcher, FirstChild, NextSibling>,
                        ValueMatcher,
                        FirstChild,
                        NextSibling> {

    /*   ---   FRIENDS   ---   */
    template <typename, typename, typename, typename>
    friend class matcher;

    /*   ---   ATTRIBUTES   ---   */
    public:
    static constexpr matcher_info_t info {
        // Matches null
        false,
        // Shallow matches null
        false,
        // Reluctant
        false,
        // Possessive
        true};

    /*   ---   CONSTRUCTOR   ---   */
    using matcher<one_matcher, ValueMatcher, FirstChild, NextSibling>::matcher;

    /*   ---   METHODS   ---   */
    template <typename NodeAllocator>
    bool search_node_impl(allocator_value_type<NodeAllocator>& node, NodeAllocator& allocator) {
        if (!this->match_value(node.get_value())) {
            return false;
        }
        auto target = policy::siblings().get_instance(
            node.get_first_child(),
            node_navigator<allocator_value_type<NodeAllocator>*>(),
            allocator);
        // Pattern search children of the pattern
        auto do_search_child = [&](auto& it, auto& child) -> bool {
            return child.search_node(it.get_current_node(), allocator);
        };
        return this->search_children(allocator, std::move(target), do_search_child);
    }

    template <typename NodeAllocator>
    unique_node_ptr<NodeAllocator> result_impl(NodeAllocator& allocator) {
        unique_node_ptr<NodeAllocator> result = this->clone_node(allocator);
        auto attach_child                     = [&](auto& child) {
            if (!child.empty()) {
                result->assign_child_like(
                    child.result(allocator),
                    *child.get_node(allocator));
            }
        };
        std::apply(
            [&](auto&... children) {
                (..., attach_child(children));
            },
            this->get_children());
        return std::move(result);
    }

    template <typename... Nodes>
    constexpr one_matcher<ValueMatcher, Nodes...> replace_children(Nodes&... nodes) const {
        return {this->value, nodes...};
    }

    template <typename Child>
    constexpr one_matcher<ValueMatcher, std::remove_reference_t<Child>, NextSibling>
    with_first_child(Child&& child) const {
        return {this->value, child, this->next_sibling};
    }

    template <typename Sibling>
    constexpr one_matcher<ValueMatcher, FirstChild, std::remove_reference_t<Sibling>>
    with_next_sibling(Sibling&& sibling) const {
        return {this->value, this->first_child, sibling};
    }
};

template <typename ValueMatcher>
one_matcher<ValueMatcher, detail::empty_t, detail::empty_t>
one(const ValueMatcher& value_matcher) {
    return {value_matcher, detail::empty_t(), detail::empty_t()};
}

one_matcher<true_matcher, detail::empty_t, detail::empty_t> one() {
    return {true_matcher(), detail::empty_t(), detail::empty_t()};
}

} // namespace md
