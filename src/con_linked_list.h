//
// Created by Joe on 4/4/2024.
//

#ifndef COP4520_PA3_CPP_CON_LINKED_LIST_H
#define COP4520_PA3_CPP_CON_LINKED_LIST_H
#include <iostream>
#include <mutex>
#include <memory>
#include <limits>

struct ListNode {
    int value;
    std::shared_ptr<ListNode> next;
    std::shared_ptr<ListNode> prev;
    bool marked; // marked as logically deleted

    explicit ListNode(int value) : value(value), prev(nullptr), next(nullptr), marked(false) {}
    friend std::ostream& operator<<(std::ostream& os, const ListNode* node) {
        return os << (node == nullptr ? "null" : std::to_string(node->value));
    }
};


class ConcurrentLinkedList {
private:
    std::shared_ptr<ListNode> head;
    std::shared_ptr<ListNode> tail;
    size_t length;
    mutable std::mutex list_mutex;

public:
    ConcurrentLinkedList();
    ~ConcurrentLinkedList();

    void insert(int value);
    bool contains(int value) const;
    bool remove(int value);
    int cleanup();
    int pop_front();
    std::size_t size() const;
    bool empty() const;

    friend std::ostream& operator<<(std::ostream& os, const ConcurrentLinkedList list) {
        std::lock_guard<std::mutex> guard(list.list_mutex);
        auto current = list.head;
        while (current) {
            os << current->value;
            os << (current->marked ? "(X) " : " ");
            current = current->next;
        }
        return os;
    }
};


#endif //COP4520_PA3_CPP_CON_LINKED_LIST_H
