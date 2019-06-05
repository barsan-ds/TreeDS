#pragma once

#include <cstddef> // std::size_t
#include <optional>
#include <stdexcept> // std::invalid_argument
#include <typeindex>

#include <TreeDS/tree_base.hpp>
#include <TreeDS/matcher/node/matcher.hpp>
#include <TreeDS/tree.hpp>

namespace md {

template <typename PatternTree>
class pattern {

    protected:
    PatternTree pattern_tree;
    std::optional<std::type_index> node_type = std::nullopt;
    void* matched_tree                       = nullptr;

    public:
    pattern(PatternTree&& tree) :
            pattern_tree(tree) {
    }

    private:
    template <typename Tree>
    bool do_match(Tree& tree) {
        std::type_index tree_type(typeid(tree.root));
        if (matched_tree != nullptr && matched_tree == tree.root) {
            return true;
        }
        pattern_tree.reset();
        if (this->pattern_tree.match_node(tree.root, tree.get_node_allocator())) {
            this->node_type    = tree_type;
            this->matched_tree = &tree;
            return true;
        }
        return false;
    }

    public:
    template <typename Node, typename Policy, typename Allocator>
    bool match(tree_base<Node, Policy, Allocator>& tree) {
        return this->do_match(tree);
    }

    template <typename Node, typename Policy, typename Allocator>
    bool match(const tree_base<Node, Policy, Allocator>& tree) {
        return this->do_match(tree);
    }

    template <typename Node, typename Policy, typename Allocator>
    void assign_result(tree<Node, Policy, Allocator>& tree) {
        if (this->node_type != typeid(tree.root)) {
            throw std::invalid_argument(
                "Tried to assign the matched result to a tree having a different type of nodes (binary->nary or nary->binary).");
        }
        tree = pattern_tree.result(tree.allocator);
    }

    template <std::size_t Index, typename Node, typename Policy, typename Allocator>
    void assign_mark(capture_index<Index> index, tree<Node, Policy, Allocator>& tree) {
        if (this->node_type != typeid(tree.root)) {
            throw std::invalid_argument(
                "Tried to assign the matched result to a tree having a different type of nodes (binary->nary or nary->binary).");
        }
        tree = pattern_tree.marked_result(index, tree.allocator);
    }

    template <char... Name, typename Node, typename Policy, typename Allocator>
    void assign_mark(capture_name<Name...> name, tree<Node, Policy, Allocator>& tree) {
        if (this->node_type != typeid(tree.root)) {
            throw std::invalid_argument(
                "Tried to assign the matched result to a tree having a different type of nodes (binary->nary or nary->binary).");
        }
        tree = pattern_tree.marked_result(name, tree.allocator);
    }

    std::size_t size() const {
        return this->pattern_tree.mark_count();
    }
};

} // namespace md
