#pragma once

#define DECLARE_TYPES(VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T)                                                           \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::value_type;                                      \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::reference;                                       \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::const_reference;                                 \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::node_type;                                       \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::size_type;                                       \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::difference_type;                                 \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::pointer;                                         \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::const_pointer;                                   \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::policy_type;                                     \
    using typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::allocator_type;                                  \
    template <typename P>                                                                                               \
    using iterator = typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::template iterator<P>;                 \
    template <typename P>                                                                                               \
    using const_iterator = typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::template const_iterator<P>;     \
    template <typename P>                                                                                               \
    using reverse_iterator = typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::template reverse_iterator<P>; \
    template <typename P>                                                                                               \
    using const_reverse_iterator = typename basic_tree<VALUE_T, NODE_T, POLICY_T, ALLOCATOR_T>::template const_reverse_iterator<P>;

#include <cstddef>
#include <iterator> // std::make_reverse_iterator
#include <limits>   // std::numeric_limits()
#include <memory>   // std::unique_ptr, std::allocator_traits

#include <TreeDS/policy/breadth_first.hpp>
#include <TreeDS/tree_iterator.hpp>

namespace md {

using default_policy = breadth_first;

template <typename, typename, typename, typename>
class tree;

template <typename T, typename Node, typename Policy, typename Allocator>
class basic_tree {

    template <typename, typename, typename, typename>
    friend class basic_tree;

    template <typename, typename, typename, typename>
    friend class tree;

    //   ---   TYPES   ---
    public:
    // General
    using value_type      = T;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using node_type       = Node;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer         = T*;
    using const_pointer   = const T*;
    using policy_type     = Policy;
    using allocator_type  = typename std::allocator_traits<Allocator>::
        template rebind_alloc<std::decay_t<node_type>>;

    // Iterators
    template <typename P>
    using iterator = tree_iterator<basic_tree, P, false>;
    template <typename P>
    using const_iterator = tree_iterator<basic_tree, P, true>;
    template <typename P>
    using reverse_iterator = std::reverse_iterator<iterator<P>>;
    template <typename P>
    using const_reverse_iterator = std::reverse_iterator<const_iterator<P>>;

    protected:
    //   ---   ATTRIBUTES   ---
    /// @brief Allocator object used to allocate the nodes.
    allocator_type allocator {};
    /// @brief Owning pointer to the root node.
    node_type* root = nullptr;
    /// @brief The number of nodes in the tree.
    size_type size_value = 0u;
    /// @brief Maximum number of children a node can have.
    size_type arity_value = 0u;

    protected:
    basic_tree() {
    }

    basic_tree(node_type* root, size_type size, size_type arity) :
            root(root),
            size_value(size),
            arity_value(arity) {
    }

    template <typename Alloc>
    basic_tree(node_type* root, size_type size, size_type arity, Alloc&& allocator) :
            allocator(allocator),
            root(root),
            size_value(size),
            arity_value(arity) {
    }

    basic_tree(const allocator_type& allocator) :
            allocator(allocator) {
    }

    ~basic_tree() {
    }

    //   ---   ITERATORS   ---
    public:
    /**
     * @brief Returns a constant iterator to the beginning.
     * @details The iterator will point to the first element of the container. Which one is the first depends on the
     * algorithm used to traverse the tree. Using this iterator you can't modify the container.
     * @return constant iterator to the first element
     */
    template <typename P = Policy>
    const_iterator<P> begin(P policy = P()) const {
        return this->cbegin(policy);
    }

    template <typename P = Policy>
    const_iterator<P> cbegin(P = P()) const {
        // Incremented to shift it to the first element (initially it's end-equivalent)
        return ++const_iterator<P>(*this);
    }

    template <typename P = Policy>
    const_iterator<P> end(P policy = P()) const {
        return this->cend(policy);
    }

    template <typename P = Policy>
    const_iterator<P> cend(P = P()) const {
        return const_iterator<P>(*this);
    }

    template <typename P = Policy>
    const_reverse_iterator<P> rbegin(P policy = P()) const {
        return this->crbegin(policy);
    }

    template <typename P = Policy>
    const_reverse_iterator<P> crbegin(P policy = P()) const {
        return std::make_reverse_iterator(this->cend(policy));
    }

    template <typename P = Policy>
    const_reverse_iterator<P> rend(P policy = P()) const {
        return this->crend(policy);
    }

    template <typename P = Policy>
    const_reverse_iterator<P> crend(P policy = P()) const {
        // Incremented to shift it to the first element (initially it's end-equivalent)
        return std::make_reverse_iterator(this->cbegin(policy));
    }

    template <typename OtherPolicy, bool Constant>
    bool is_own_iterator(
        const tree_iterator<
            basic_tree<T, Node, Policy, Allocator>,
            OtherPolicy,
            Constant>& it) const {
        return it.pointed_tree == this;
    }

    //   ---   CAPACITY   ---
    public:
    /**
     * @brief Checks whether the container is empty.
     * @details If a tree is empty then:
     * <ul>
     *     <li>It doesn't have a root.</li>
     *     <li>It has a {@link #size()} of 0.</li>
     *     <li>{@link tree#begin() begin()} == {@link tree#end() end()}.</li>
     * </ul>
     * @return true if the tree is empty
     */
    bool empty() const {
        return this->size_value == 0;
    }

    /**
     * @brief Returns the number of the nodes in this tree
     * @return the number of nodes
     */
    size_type size() const {
        return this->size_value;
    }

    size_type arity() const {
        return this->arity_value;
    }

    /**
     * @brief Returns the maximum possible number of elements the tree can hold.
     * @details Please note that this is not the real value because the size of the tree will be more likely limited by
     * the amount of free memory. You never want to use this method, it is stupid, it exists just to say that this
     * library is stl-like.
     * @return maximum possible number of elements
     */
    size_type max_size() const {
        return std::numeric_limits<size_type>::max();
    }

    //   ---   GETTER   ---
    allocator_type get_allocator() const {
        return this->allocator;
    }

    const Node* get_root() const {
        return this->root;
    }

    //  ---   COMPARISON   ---
    public:
    template <typename OtherPolicy>
    bool operator==(const basic_tree<T, Node, OtherPolicy, Allocator>& other) const {
        // Test if different size or arity
        if (this->size_value != other.size_value
            || this->arity_value != other.arity_value) {
            return false;
        }
        // At the end is either null (both) or same as the other.
        return (this->root == nullptr && other.root == nullptr) || this->root->operator==(*other.root);
    }

    template <
        typename ConvertibleT,
        typename... Children,
        CHECK_CONVERTIBLE(ConvertibleT, value_type)>
    bool operator==(const struct_node<ConvertibleT, Children...>& other) const {
        // Test if different size or arity
        if (this->size_value != other.get_subtree_size()
            || this->arity_value != other.get_subtree_arity()) {
            return false;
        }
        // Deep test for equality
        return this->root != nullptr && this->root->operator==(other);
    }

    bool operator==(const struct_node<std::nullptr_t>&) const {
        return this->empty();
    }
};

// TODO: replace as soon as space ship operator (<=>) is available
// tree
template <
    typename T,
    typename Node,
    typename Policy1,
    typename Policy2,
    typename Allocator>
bool operator!=(const basic_tree<T, Node, Policy1, Allocator>& lhs, const basic_tree<T, Node, Policy2, Allocator>& rhs) {
    return !lhs.operator==(rhs);
}

// struct_node
template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator==(const struct_node<ConvertibleT, Children...>& lhs, const basic_tree<T, Node, Policy, Allocator>& rhs) {
    return rhs.operator==(lhs);
}
template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator!=(const basic_tree<T, Node, Policy, Allocator>& lhs, const struct_node<ConvertibleT, Children...>& rhs) {
    return !lhs.operator==(rhs);
}
template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator,
    typename ConvertibleT,
    typename... Children,
    CHECK_CONVERTIBLE(ConvertibleT, T)>
bool operator!=(const struct_node<ConvertibleT, Children...>& lhs, const basic_tree<T, Node, Policy, Allocator>& rhs) {
    return !rhs.operator==(lhs);
}

// empty struct_node
template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator>
bool operator==(const struct_node<std::nullptr_t>& lhs, const basic_tree<T, Node, Policy, Allocator>& rhs) {
    return rhs.operator==(lhs);
}

template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator>
bool operator!=(const basic_tree<T, Node, Policy, Allocator>& lhs, const struct_node<std::nullptr_t>& rhs) {
    return !lhs.operator==(rhs);
}

template <
    typename T,
    typename Node,
    typename Policy,
    typename Allocator>
bool operator!=(const struct_node<std::nullptr_t>& lhs, const basic_tree<T, Node, Policy, Allocator>& rhs) {
    return !rhs.operator==(lhs);
}

} // namespace md