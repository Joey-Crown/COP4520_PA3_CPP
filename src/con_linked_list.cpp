//
// Created by Joe on 4/4/2024.
//

#include "con_linked_list.h"

ConcurrentLinkedList::ConcurrentLinkedList(): head(nullptr), tail(nullptr), length(0) {}

ConcurrentLinkedList::~ConcurrentLinkedList() = default;

void ConcurrentLinkedList::insert(int value) {
    std::lock_guard<std::mutex> guard(list_mutex);

    auto newNode = std::make_shared<ListNode>(value);
    if (!head) {
        head = newNode;
        tail = newNode;
    } else if (head->value >= newNode->value) {
        newNode->prev = tail;
        tail->next = newNode;
        tail = newNode;
    } else {
        auto current = head;

        while (current->next != nullptr && current->next->value < newNode->value) {
            current = current->next;
        }

        newNode->next = current->next;

        if (current->next != nullptr) {
            newNode->next->prev = newNode;
        }

        current->next = newNode;
        newNode->prev = current;

        if (newNode->next == nullptr) {
            tail = newNode;
        }
    }

    length++;
}

bool ConcurrentLinkedList::contains(int value) const {
    std::lock_guard<std::mutex> guard(list_mutex);

    auto current = head;
    while (current) {
        if (!current->marked && current->value == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

bool ConcurrentLinkedList::remove(int value) {
    std::lock_guard<std::mutex> guard(list_mutex);

    auto current = head;
    while (current) {
        if (current->value == value) {
            current->marked = true;
            return true;
        }
        current = current->next;
    }
    return false;
}

int ConcurrentLinkedList::cleanup() {
    std::lock_guard<std::mutex> guard(list_mutex);
    int count = 0;
    auto current = head;
    while (current && current->next) {
        if (current->next->marked) {
            auto to_delete = current->next;
            current->next = to_delete->next;
            if (to_delete->next) {
                to_delete->next->prev = current;
            }
            count++;
            // to_delete node is now isolated and will be deleted when out of scope
        } else {
            current = current->next;
        }
    }
    return count;
}

int ConcurrentLinkedList::pop_front() {
    std::lock_guard<std::mutex> guard(list_mutex);

    if (!head) {
        return std::numeric_limits<int>::min();
    }

    int value = head->value;
    head->marked = true;
    head = head->next;
    return value;
}

std::size_t ConcurrentLinkedList::size() const {
    return length;
}

bool ConcurrentLinkedList::empty() const {
    return head == nullptr;
}
