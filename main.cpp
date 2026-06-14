//#include <algorithm>
#include <iostream>
#include <map>
#include <memory>    // для std::allocator_traits
#include "slist.hpp"

// Управляющая структура, которая владеет зарезервированным блоком памяти
// Это наш STATE (состояние аллокатора, его личная "память")
struct FixedBuffer {
    std::unique_ptr<std::byte[]> raw_memory = nullptr; // сырая память
    std::size_t max_elements;                          // лимит элементов
    std::size_t bytes_per_element = 0;                 // размер одного узла контейнера
    std::size_t allocated_count = 0;                   // текущее количество выделенных элементов

    FixedBuffer( std::size_t count ) { max_elements = count; }

    ~FixedBuffer() {
        std::cout << "[FixedBuffer] Освобождение всей зарезервированной памяти.\n";
    }

    // Запрет копирования, памятью управляет только этот объект, т.к. НЕ stateless
    FixedBuffer( const FixedBuffer& ) = delete;
    FixedBuffer& operator=( const FixedBuffer& ) = delete;
};

// Класс аллокатора
template <typename T>
class FixedBlockAllocator {
public:
    using value_type = T;

    // Шаблонный rebind для совместимости со структурами std::map
    template <typename U>
    struct rebind {
        using other = FixedBlockAllocator<U>;
    };

    // Конструктор принимает точное количество элементов для резервирования
    explicit FixedBlockAllocator( std::size_t elements_count ) {
        buffer_ = std::make_shared<FixedBuffer>( elements_count );
    }

    // Конструктор копирования для связи между rebind-копиями аллокатора
    template <typename U>
    FixedBlockAllocator( const FixedBlockAllocator<U>& other ) noexcept : buffer_(other.buffer_) {}

    T* allocate( std::size_t n ) {
        if( n == 0 ) return nullptr;

        // Выполняем резервирование при самом первом запросе памяти
        if( buffer_->bytes_per_element == 0 ) {
            buffer_->bytes_per_element = sizeof( T );
            std::size_t total_bytes = buffer_->max_elements * buffer_->bytes_per_element;

            std::cout << "[Allocator] Выполняется резервирование памяти под "
                      << buffer_->max_elements << " элементов (" << total_bytes << " байт).\n";

            buffer_->raw_memory = std::make_unique<std::byte[]>( total_bytes );
        }

        // Проверяем жесткий лимит элементов
        if (buffer_->allocated_count + n > buffer_->max_elements) {
            std::cout << "[Allocator] Ошибка: Запрошено элементов больше, чем зарезервировано!\n";
            throw std::bad_alloc();
        }

        // Вычисляем смещение указателя в зарезервированном блоке
        std::size_t offset = buffer_->allocated_count * buffer_->bytes_per_element;
        void* result_ptr = buffer_->raw_memory.get() + offset;

        buffer_->allocated_count += n;
        return static_cast<T*>(result_ptr);
    }

    // Поэлементное освобождение отсутствует по условию задачи
    void deallocate(T* p, std::size_t n) noexcept {
        // Ничего не делаем. Память вернется операционной системе в деструкторе FixedBuffer.
        p = p; // :-)
        n = n; // обход ошибки unused variables;
    }

    // Операторы сравнения для STL-контейнеров
    template <typename U>
    bool operator==(const FixedBlockAllocator<U>& other) const noexcept {
        return buffer_ == other.buffer_;
    }

    template <typename U>
    bool operator!=(const FixedBlockAllocator<U>& other) const noexcept {
        return buffer_ != other.buffer_;
    }

    // Разрешаем доступ к private-полям для других инстанций шаблона (при rebind)
    template <typename U> friend class FixedBlockAllocator;

private:
    std::shared_ptr<FixedBuffer> buffer_;
};

constexpr int fact( int i )
{
  return ( i == 0 || i == 1 ) ? 1 : i * fact( i - 1);
}

#define LIMIT_ELEMENTS (10)

int main()
{

/**Прикладной код должен содержать следующие вызовы:**/

//- создание экземпляра std::map<int, int>
    std::map<int, int> map1;

//- заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(int k=0; k<LIMIT_ELEMENTS; ++k)
        map1[k]=fact(k);

//- создание экземпляра std::map<int, int> с новым аллокатором, ограниченным 10 элементами
    FixedBlockAllocator<std::pair<const int, int>> my_alloc1(LIMIT_ELEMENTS);
    std::map<int,
             int,
             std::less<int>,
             FixedBlockAllocator<std::pair<const int, int>>> map2(my_alloc1);

//- заполнение 10 элементами, где ключ - это число от 0 до 9, а значение - факториал ключа
    for(int k=0; k<LIMIT_ELEMENTS; ++k)
        map2[k]=fact(k);

//- вывод на экран всех значений (ключ и значение разделены пробелом) хранящихся в контейнере
    for( const auto& [ key , value ] : map1 )
        std::cout << "map1 " << key << " : " << value << std::endl;
    for( const auto& [ key , value ] : map2 )
        std::cout << "map2 " << key << " : " << value << std::endl;

//- создание экземпляра своего контейнера для хранения значений типа int
    SingleList<double> list1;

//- заполнение 10 элементами от 0 до 9
    for(int k=0; k<LIMIT_ELEMENTS; ++k)
        list1.push_front(k);

//- создание экземпляра своего контейнера для хранения значений типа int с новым аллокатором, ограниченным 10 элементами
    FixedBlockAllocator<int> my_alloc2(LIMIT_ELEMENTS);
    SingleList<int, FixedBlockAllocator<int>> list2(my_alloc2);

//- заполнение 10 элементами от 0 до 9
    for(int k=0; k<LIMIT_ELEMENTS; ++k)
        list2.push_front(k);

//- вывод на экран всех значений, хранящихся в контейнере
    std::cout << "Элементы контейнера list1: ";
    for (const auto& item : list1) {
        std::cout << item << " ";
    }
    std::cout << "\n";

    std::cout << "Элементы контейнера list2: ";
    for (const auto& item : list2) {
        std::cout << item << " ";
    }
    std::cout << "\n";

//  Проверка контейнера на аллокаторе с ограничениями
    try {
        // пробуем добавить "лишний" 11-ый элемент в контейнер с ограничением в 10 элементов
        map2[10]=fact(10);
        // list2.push_front(10);
    } catch (const std::bad_alloc& e) {
        std::cout << "\nПерехвачено исключение: Превышен лимит выделения элементов.\n";
    }

    return 0;
}
