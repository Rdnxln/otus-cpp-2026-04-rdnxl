// Single Linked List
#include <memory>
#include <iterator>
//#include <iostream>
//#include <cstddef>

template <typename T, typename Allocator = std::allocator<T>>
class SingleList {
private:
    // Узел списка
    struct Elem {
        T value;
        Elem* next;

        template <typename... Args>
        Elem(Elem* nextElem, Args&&... args)
            : value(std::forward<Args>(args)...), next(nextElem) {}
    };

    // Перепривязка аллокатора для выделения памяти под узлы, а не под T
    using ElemAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Elem>;
    using AllocTraits = std::allocator_traits<ElemAllocator>;

    Elem* head = nullptr;
    std::size_t list_size = 0;
    [[no_unique_address]] ElemAllocator allocator; // Оптимизация пустого базового класса (C++20)

public:
    // Обязательные типы для STL-совместимости
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    // Конструкторы и деструктор
    SingleList() : head(nullptr), list_size(0), allocator(ElemAllocator()) {}
    explicit SingleList(const Allocator& alloc) : head(nullptr), list_size(0), allocator(alloc) {}

    ~SingleList() {
        clear();
    }

    // Реализация итератора для обхода в одном направлении
    template <typename ValueType>
    class SLIter {
    private:
        Elem* current;
        friend class SingleList;
        explicit SLIter(Elem* node) : current(node) {}

    public:
        // Тэги и типы итератора для совместимости со std::iterator_traits
        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        SLIter() : current(nullptr) {}

        reference operator*() const { return current->value; }
        pointer operator->() const { return &(current->value); }

        // Префиксный инкремент (++it)
        SLIter& operator++() {
            if (current) current = current->next;
            return *this;
        }

        // Постфиксный инкремент (it++)
        SLIter operator++(int) {
            SLIter prev = *this;
            if (current) current = current->next;
            return prev;
        }

        bool operator==(const SLIter& other) const { return current == other.current; }
        bool operator!=(const SLIter& other) const { return current != other.current; }
    };

    using iterator = SLIter<T>;
    using const_iterator = SLIter<const T>;

    // Методы навигации (Итераторы)
    iterator begin() noexcept { return iterator(head); }
    iterator end() noexcept { return iterator(nullptr); }
    const_iterator begin() const noexcept { return const_iterator(head); }
    const_iterator end() const noexcept { return const_iterator(nullptr); }
    const_iterator cbegin() const noexcept { return const_iterator(head); }
    const_iterator cend() const noexcept { return const_iterator(nullptr); }

    // Основной метод: добавление элемента в начало (O(1))
    template <typename... Args>
    void emplace_front(Args&&... args) {
        // Выделение памяти под узел
        Elem* newElem = AllocTraits::allocate(allocator, 1);
        try {
            // Конструирование узла на месте
            AllocTraits::construct(allocator, newElem, head, std::forward<Args>(args)...);
        } catch (...) {
            AllocTraits::deallocate(allocator, newElem, 1);
            throw;
        }
        head = newElem;
        ++list_size;
    }

    void push_front(const T& value) { emplace_front(value); }
    void push_front(T&& value) { emplace_front(std::move(value)); }

    // Опциональные методы STL-совместимости
    bool empty() const noexcept { return head == nullptr; }
    size_type size() const noexcept { return list_size; }

    void clear() noexcept {
        Elem* current = head;
        while (current != nullptr) {
            Elem* next = current->next;
            AllocTraits::destroy(allocator, current);  // Вызов деструктора узла и элемента
            AllocTraits::deallocate(allocator, current, 1); // Освобождение памяти
            current = next;
        }
        head = nullptr;
        list_size = 0;
    }
};
