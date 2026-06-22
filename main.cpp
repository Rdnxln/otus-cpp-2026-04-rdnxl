/*!
 * \file print_ip.cpp
 * @brief Программа для условного вывода IP-адресов из различных типов данных.
 * @details Демонстрирует использование SFINAE (std::enable_if), метапрограммирования
 * и C++20 лямбда-шаблонов для перегрузки одной функции print_ip под разные структуры данных.
 * \author Александр Демин
 * \date 2026-06-19
 */

#include <iostream>
#include <vector>
#include <type_traits>
#include <list>
#include <set>
#include <deque>
#include <array>
#include <string>
#include <tuple>

/**
 * @brief Вспомогательный псевдоним типа для реализации SFINAE-проверок.
 * @tparam Args Произвольный набор типов.
 */
template <typename... Args>
using void_t_override = void;

/**
 * @brief Базовый шаблон проверки: является ли тип итерируемым контейнером.
 * @details По умолчанию наследуется от std::false_type.
 * @tparam T Тестируемый тип.
 * @tparam Enable Вспомогательный параметр для SFINAE (по умолчанию void).
 */
template <typename T, typename = void>
struct is_iterable_container : std::false_type {};

/**
 * @brief Специализация шаблона is_iterable_container для валидных контейнеров.
 * @details Проверяет наличие вложенного типа value_type, а также методов begin() и end().
 * Наследуется от std::true_type, если подстановка успешна.
 * @tparam T Тестируемый тип контейнера.
 */
template <typename T>
struct is_iterable_container<T, void_t_override<
    typename T::value_type,
    decltype( std::declval<const T>().begin() ),
    decltype( std::declval<const T>().end() )
>> : std::true_type {};

/**
 * @brief Базовый шаблон проверки: является ли тип строкой.
 * @tparam T Тестируемый тип.
 */
template <typename T>
struct is_string : std::false_type {};

/**
 * @brief Специализация is_string для типа std::string.
 */
template <>
struct is_string<std::string> : std::true_type {};

/**
 * @brief Специализация is_string для типа std::wstring.
 */
template <>
struct is_string<std::wstring> : std::true_type {};

/**
 * @brief Вспомогательная переменная-шаблон (Value Template) для проверки на строку.
 * @tparam T Тестируемый тип.
 */
template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

/**
 * @brief Базовый шаблон проверки: является ли тип кортежем std::tuple.
 * @tparam T Тестируемый тип.
 */
template <typename T>
struct is_tuple : std::false_type {};

/**
 * @brief Специализация is_tuple для любых вариаций std::tuple.
 * @tparam Args Типы элементов кортежа.
 */
template <typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

/**
 * @brief Вспомогательная переменная-шаблон (Value Template) для проверки на кортеж.
 * @tparam T Тестируемый тип.
 */
template <typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;


// Перегрузки функции print_ip

/**
 * @brief Выводит элементы итерируемых контейнеров через точку.
 * @details Исключает строки из обработки с помощью маски `!is_string_v<T>`.
 * @tparam T Тип контейнера (должен удовлетворять условиям итерируемости).
 * @param[in] container Ссылка на константный контейнер с данными.
 */
template <typename T>
typename std::enable_if<is_iterable_container<T>::value && !is_string_v<T>, void>::type
print_ip( const T& container ) {
    bool first = true;
    for( const auto& elem : container ) {
        if( !first ) {
            std::cout << '.';
        }
        first = false;
        std::cout << elem;
    }
    std::cout << "\n";
}

/**
 * @brief Выводит целочисленные типы в побайтовом представлении.
 * @details Разбирает переданное число побайтово,
 * начиная со старшего байта (Big-Endian/Network-Order стиль).
 * @tparam T Целочисленный тип данных (проверяется через std::is_integral).
 * @param[in] val Значение целочисленного типа для побайтового вывода.
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
print_ip( T val ) {
    bool first = true;
    for( int s = sizeof(T) - 1 ; s >= 0 ; --s ) {
        if( !first ) std::cout << '.';
        std::cout << ( ( val >> ( s * 8 ) ) & 0xFF);
        first = false;
    }
    std::cout << std::endl;
}

/**
 * @brief Выводит строковые типы данных в поток как есть.
 * @tparam T Строковый тип данных (std::string или std::wstring).
 * @param[in] str Ссылка на константную строку.
 */
template <typename T>
typename std::enable_if<is_string_v<T>, void>::type
print_ip( const T& str ) {
    std::cout << str << std::endl;
}

/**
 * @brief Выводит элементы кортежа через точку, если все элементы имеют одинаковый тип.
 * @details Содержит внутреннюю структуру и статические проверки компиляции (static_assert),
 * гарантирующие однородность типов внутри std::tuple.
 * @tparam T Тип кортежа std::tuple.
 * @param[in] tup Ссылка на константный кортеж.
 * @note Вызовет ошибку компиляции, если элементы кортежа относятся к разным типам.
 */
template <typename T>
typename std::enable_if<is_tuple_v<T>, void>::type
print_ip( const T& tup ) {

    /**
     * @brief Внутренняя локальная структура для инкапсуляции логики печати кортежа.
     */
    struct _InternalSpace {
        /**
         * @brief Выполняет валидацию типов и непосредственный вывод элементов кортежа.
         * @param[in] tuple Ссылка на обрабатываемый кортеж.
         */
        static void doit( const T& tuple ) {
            using TupleType = typename std::decay<T>::type;
            constexpr std::size_t Size = std::tuple_size<TupleType>::value;

            // Базовый случай для пустого кортежа
            if( Size == 0 ) {
                std::cout << "\n";
                return;
            }

            // Берем тип 0-го элемента как образец для сравнения
            using FirstType = typename std::tuple_element<0, TupleType>::type;

            // Внутренний лямбда-шаблон для распаковки индексов и проверки типов
            auto verify_types = []<std::size_t... Is>( std::index_sequence<Is...> ) {
                // fold-expression проверяет, что каждый тип Is совпадает с FirstType
                static_assert( ( std::is_same<FirstType, typename std::tuple_element<Is, TupleType>::type>::value && ... ),
                               "ОШИБКА: Случай отличающихся типов в кортеже" );
            };
            verify_types( std::make_index_sequence<Size>{} );

            // вывод кортежа
            auto out_items = [&]<std::size_t... Is>( std::index_sequence<Is...> ) {
                bool first = true;

                // лямбда вывода текущего элемента
                auto out_one_item = [&]( const auto& val ) {
                    if( !first ) std::cout << '.';
                    first = false;
                    std::cout << val;
                };

                // все элементы на печать через точки одной командой
                ( out_one_item( std::get<Is>( tuple ) ), ...);
                std::cout << "\n";
            };
            out_items( std::make_index_sequence<Size>{} );
        }
    };

    _InternalSpace::doit( tup );
}

/**
 * @brief Главная функция программы.
 * @details Демонстрирует работу функции print_ip() с разнотипными аргументами
 */
int main() {
    print_ip( int8_t { -1 } );                     // 255
    print_ip( int16_t{ 0 } );                      // 0.0
    print_ip( int32_t{ 2130706433 } );             // 127.0.0.1
    print_ip( int64_t{ 8875824491850138409 } );    // 123.45.67.89.101.112.131.41

    print_ip( std::string{"Hello, World!"} );      // Hello, World!

    print_ip( std::vector<int>{ 100, 200, 300, 400 } ); // 100.200.300.400
    print_ip( std::list<short>{ 400, 300, 200, 100 } ); // 400.300.200.100

    print_ip( std::make_tuple( 123, 456, 789, 0 ) );    // 123.456.789.0
    // Раскомментирование строки ниже вызовет ошибку компиляции (типы не совпадают):
    // print_ip( std::make_tuple(123, 456.0, "error") );

    return 0;
}
