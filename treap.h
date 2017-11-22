#ifndef TREAP_H
#define TREAP_H

#include <algorithm>
#include <iostream>
#include <functional>

namespace Treap {
    template <typename DataType, class Compare = std::less <DataType>>
    struct Node {
        Node <DataType, Compare> *left, *right;

        DataType value;

        DataType toAssign;

        uint64_t size;
        int32_t priority;

        bool reverse;
        bool assign;

        explicit Node(const DataType &from) :
                left(nullptr),
                right(nullptr),
                value(from),
                size(1),
                priority(rand()),
                reverse(false),
                assign(false) {
        }

        explicit Node() :
                left(nullptr),
                right(nullptr),
                size(0),
                priority(0),
                reverse(false),
                assign(false) {
        }
    };

    template <typename DataType, class Compare = std::less <DataType>>
    uint64_t size(Node <DataType, Compare> *root) {
        return root != nullptr ? root->size : 0;
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void update(Node <DataType, Compare> *&root) {
        if (root != nullptr) {
            root->size = size(root->left) + size(root->right) + 1;
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void assign(Node <DataType, Compare> *&root, const DataType &toAssign) {
        if (root != nullptr) {
            root->value = toAssign;

            root->assign = true;
            root->toAssign = toAssign;
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void reverse(Node <DataType, Compare> *&root) {
        if (root != nullptr) {
            root->reverse ^= true;

            std::swap(root->left, root->right);
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void push(Node <DataType, Compare> *&root) {
        if (root->assign) {
            assign(root->left, root->toAssign);
            assign(root->right, root->toAssign);

            root->assign = false;
        }
        if (root->reverse) {
            reverse(root->left);
            reverse(root->right);

            root->reverse = false;
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void split(Node <DataType, Compare> *root, Node <DataType, Compare> *&left, Node <DataType, Compare> *&right, size_t index) {
        if (root == nullptr)
            left = right = nullptr;
        else {
            push(root);
            size_t rootIndex = size(root->left);
            if (index <= rootIndex) {
                split(root->left, left, root->left, index);
                right = root;
                update(right);
            } else {
                split(root->right, root->right, right, index - rootIndex - 1);
                left = root;
                update(left);
            }
        }
    }

    void split(Node <Vector> *root, Node <Vector> *&left, Node <Vector> *&right, const Point &point, Action action) {
        if (root == nullptr)
            left = right = nullptr;
        else {
            push(root);
            if (!root->value.compare(point, action)) {
                split(root->left, left, root->left, point, action);
                right = root;
                update(right);
            } else {
                split(root->right, root->right, right, point, action);
                left = root;
                update(left);
            }
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    Node <DataType, Compare> *merge(Node <DataType, Compare> *left, Node <DataType, Compare> *right) {
        if (left == nullptr || right == nullptr)
            return left == nullptr ? right : left;

        push(left);
        push(right);

        if (left->priority < right->priority) {
            left->right = merge(left->right, right);
            update(left);
            return left;
        } else {
            right->left = merge(left, right->left);
            update(right);
            return right;
        }
    }

    template <typename DataType, class Compare = std::less <DataType>, class Lambda>
    auto apply(Node <DataType, Compare> *&root, size_t leftIndex, size_t rightIndex, Lambda lambda)
    -> decltype(lambda(new Node <DataType, Compare> (), new Node <DataType, Compare> ())) {
        if (leftIndex >= size(root) || rightIndex >= size(root))
            throw std::out_of_range("No such element.");

        Node <DataType, Compare> *first, *second, *third;

        split(root, first, second, leftIndex);
        split(second, second, third, rightIndex - leftIndex + 1);

        auto res = lambda(root, second);

        root = merge(first, second);
        root = merge(root, third);

        return res;
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void insert(Node <DataType, Compare> *&root, const DataType &value, size_t index) {
        if (root->size == 0) {
            delete root;
            root = new Node <DataType, Compare> (value);
            return;
        }
        auto *temp = new Node <DataType, Compare> (value);
        Node <DataType, Compare> *left, *right;
        left = right = nullptr;

        split(root, left, right, index);
        left = merge(left, temp);
        root = merge(left, right);
    }

    void insert(Node <Vector> *&root, Node <Vector> *&temp, const Point &point) {
        if (root == nullptr) {
            root = temp;
            return;
        }

        if (root->size == 0) {
            delete root;
            root = temp;
            return;
        }

        Node <Vector> *first, *second;
        split(root, first, second, point, Action::Higher);
        root = merge(first, merge(temp, second));
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void destroy(Node <DataType, Compare> *&root) {
        if (root == nullptr)
            return;

        if (root->left != nullptr)
            destroy(root->left);

        if (root->right != nullptr)
            destroy(root->right);

        delete root;
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void erase(Node <DataType, Compare> *&root, size_t index) {
        Node <DataType, Compare> *left = nullptr;
        Node <DataType, Compare> *middle = nullptr;
        Node <DataType, Compare> *right = nullptr;

        split(root, left, middle, index);
        split(middle, middle, right, 1);

        root = merge(left, right);
    }

    void erase(Node <Vector> *&root, const Point &point) {
        Node <Vector> *first, *second, *third;
        split(root, first, second, point, Action::Higher);
        split(second, second, third, point, Action::HigherOrEqual);

        destroy(second);

        root = merge(first, third);
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void replace(Node <DataType, Compare> *&root, const DataType &to, size_t index) {
        if (root == nullptr)
            return;

        push(root);

        size_t rootIndex = size(root->left);

        if (rootIndex == index) {
            root->value = to;
            update(root);
        } else {
            if (index <= rootIndex)
                replace(root->left, to, index);
            else
                replace(root->right, to, index - rootIndex - 1);

            update(root);
        }
    }

    template <typename DataType, class Compare = std::less <DataType>>
    const DataType at(Node <DataType, Compare> *root, size_t index) {
        if (index >= size(root))
            throw std::out_of_range("No such element");

        push(root);

        size_t rootIndex = size(root->left);

        if (rootIndex == index)
            return root->value;

        if (index <= rootIndex)
            return at(root->left, index);
        else
            return at(root->right, index - rootIndex - 1);
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void reverse(Node <DataType, Compare> *&root, size_t leftIndex, size_t rightIndex) {
        apply(root, leftIndex, rightIndex,
              [&] (Node <DataType, Compare> *_root, Node <DataType, Compare> *second) -> int {reverse(_root);
                  return EXIT_SUCCESS;});
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void assignRange(Node <DataType, Compare> *&root, const DataType &value, size_t leftIndex, size_t rightIndex) {
        apply(root, leftIndex, rightIndex,
              [&] (Node <DataType, Compare> *_root, Node <DataType, Compare> *second) -> int {assign(second, value);
                  return EXIT_SUCCESS;});
    }

    template <typename DataType, class Compare = std::less <DataType>>
    void print(Node <DataType, Compare> *root) {
        if (root == nullptr)
            return;

        push(root);

        print(root->left);
        std::cout << root->value << ' ';
        print(root->right);
    }
}

#endif
