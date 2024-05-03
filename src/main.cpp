#include <iostream>
#include <thread>
#include <unordered_set>
#include <random>
#include <chrono>
#include <algorithm>
#include "con_linked_list.h"

#define NUM_GUESTS 20
#define HOURS 30
#define NUM_THREADS 8

/*********************/
/*PROBLEM 1 FUNCTIONS*/
/*********************/
enum Tasks {
    ADD_PRESENT,
    WRITE_CARD,
    SEARCH_LIST,
    CLEANUP_LIST
};

std::mutex mutex;

void start_work(ConcurrentLinkedList *list,
                std::unordered_set<int> *bag,
                std::unordered_set<int> *cards,
                std::mt19937 *rng
) {
    // Randomly generates value between 0-3 corresponding to a task value.
    // The task with value 3 (cleanup) happens 4% (1/25) of the time, as this
    // task locks the whole list in order to dispose of marked values.
    std::discrete_distribution<int> task_dist({32, 32, 32, 4});
    std::uniform_int_distribution minotaur_dist(0, NUM_GUESTS);
    std::cout << "Thread with ID " << std::this_thread::get_id() << " Has started doing work" << std::endl;
    while(cards->size() < NUM_GUESTS) {
        int task_type = task_dist(*rng);
        switch (task_type) {
            case ADD_PRESENT: {
                mutex.lock();
                if (bag->empty()) {
                    mutex.unlock();
                    continue;
                }

                auto iter = bag->begin();
                int value = *iter;
                bag->erase(iter);
                mutex.unlock();

                list->insert(value);
                break;
            }
            case WRITE_CARD: {
                int value = list->pop_front();
                if (value < 0) {
                    continue;
                }

                cards->insert(value);
                std::cout << "Thank you Guest #" << value << "!\n" << "Cards written: " << cards->size() << std::endl;
                break;
            }
            case SEARCH_LIST: {
                int random_gift = minotaur_dist(*rng);
                int found = list->contains(random_gift);
                std::cout << "Servant checks chain for Guest #" << random_gift
                << " and " << (found ? "found" : "did not find") << " it!"
                << std::endl;
                break;
            }
            case CLEANUP_LIST: {
                int num_removed = list->cleanup();
                std::cout << "Cleaned up list! " << num_removed << " gifts removed from chain." << std::endl;
                break;
            }
        }
    }
}

std::unordered_set<int> generateGifts() {
    // generate ids for unordered set
    std::vector<int> ids;
    for (int i = 0; i < NUM_GUESTS; ++i) {
        ids.push_back(i);
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(ids.begin(), ids.end(), std::default_random_engine(seed));
    return {ids.begin(), ids.end()};
}

/*********************/
/*PROBLEM 2 FUNCTIONS*/
/*********************/

bool check_ready(int sid, std::vector<bool>& ready) {
    for (int i = 0; i < ready.size(); ++i) {
        if (!ready[i] && i != sid) {
            return false;
        }
    }
    return true;
}

void find_max_diff(std::vector<int>& readings) {
    int skip = 10;
    int start = 0;
    int max_diff = std::numeric_limits<int>::min();

    for (int i = 0; i < NUM_THREADS; ++i) {
        int offset = i * 60;

        for (int j = offset; j < 60 - skip + 1; j++) {
            auto iter = readings.begin();
            int diff = *std::max_element(iter + i, iter + i + skip)
                    - *std::min_element(iter + i, iter + i + skip);
            if (diff > max_diff) {
                max_diff = diff;
                start = i;
            }
        }
    }

    std::cout << "Maximum temp difference: " << max_diff << "F"
        << " on interval [" << start << ", " << start + 10 << "]" << std::endl;
}

void create_report(int h, std::vector<int>& readings) {
    std::cout << "Report for Hour " << h << "." << std::endl;
    find_max_diff(readings);

    std::sort(readings.begin(), readings.end());

    for (int i = 0; i < readings.size(); ++i) {
        std::cout << readings[i] << " ";
    }

    std::cout << std::endl;

    std::cout << "Highest Temps: ";
    std::vector<int> highs(readings.end() - 5, readings.end());
    for (int high : highs) {
        std::cout << high << " ";
    }
    std::cout << std::endl;

    std::cout << "Lowest Temps: ";
    std::vector<int> lows(readings.begin(), readings.begin() + 5);
    for (int low : lows) {
        std::cout << low << " ";
    }
    std::cout << std::endl << std::endl;

}

void take_readings(int sid, std::vector<int>* readings, std::vector<bool>* ready) {
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution temp_dist(-100, 70);
    for (int i = 1; i <= HOURS; ++i) {
        for (int j = 0; j < 60; ++j) {
            (*ready)[sid] = false;
            (*readings)[j + (sid * 60)] = temp_dist(rng);
            (*ready)[sid] = true;

            while (!check_ready(sid, *ready)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        if (sid == 0) {
            mutex.lock();
            create_report(i, *readings);
            mutex.unlock();
        }
    }
}

int main() {
    // PROBLEM 1
    ConcurrentLinkedList list = ConcurrentLinkedList();
    std::unordered_set<int> unordered_bag = generateGifts();
    std::cout << "Bag contains this many: " << unordered_bag.size() << std::endl;
    std::unordered_set<int> card_bag;
    std::vector<std::thread> threads(NUM_THREADS);
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

    std::cout << "Beginning problem 1: The Birthday Presents Party with " << NUM_THREADS << " threads..." << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(start_work, &list, &unordered_bag, &card_bag, &rng);
    }

    auto start1 = std::chrono::high_resolution_clock::now();

    for (std::thread& thread: threads) {
        if (thread.joinable())
            thread.join();
    }
    threads.clear();

    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration<double, std::milli>(end1 - start1);

    // PROBLEM 2
    auto readings = std::make_shared<std::vector<int>>(NUM_THREADS * 60);
    auto ready = std::make_shared<std::vector<bool>>(NUM_THREADS);

    std::cout << "Beginning problem 2: Atmospheric Temperature Reading Module with " << NUM_THREADS << " threads..." << std::endl;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(take_readings, i, readings.get(), ready.get());
    }

    auto start2 = std::chrono::high_resolution_clock::now();

    for (std::thread& thread : threads) {
        if (thread.joinable())
            thread.join();
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration<double, std::milli>(end2 - start2);

    std::cout << "Problem 1 took " << duration1.count() << "ms to finish." << std::endl;
    std::cout << "Problem 2 took " << duration2.count() << "ms to finish." << std::endl;


    return 0;
}
