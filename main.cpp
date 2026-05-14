#include "lib.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <map>

// ("",  '.') -> [""]
// ("11", '.') -> ["11"]
// ("..", '.') -> ["", "", ""]
// ("11.", '.') -> ["11", ""]
// (".11", '.') -> ["", "11"]
// ("11.22", '.') -> ["11", "22"]
std::vector<std::string> split(const std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop = str.find_first_of(d);
    while(stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

std::tuple<unsigned char,
            unsigned char,
            unsigned char,
            unsigned char> convertIP( const std::string &str, char d )
{
  unsigned char c[4] = {0, 0, 0, 0};
  int count = 0;
  int value = 0;

  std::string::size_type start = 0;
  std::string::size_type stop = str.find_first_of(d);

  while(stop != std::string::npos)
  {
    value = 0;
    try {
        value = std::stoi(str.substr(start, stop - start));
    } catch (const std::invalid_argument& e) {
        // Строка не содержит чисел
      return {};
    } catch (const std::out_of_range& e) {
        // Число не влезает в int
      return {};
    }

    if( count > 3) break;
    c[count++] = static_cast<unsigned char>(value);

    start = stop + 1;
    stop = str.find_first_of(d, start);
  }

  try {
    value = std::stoi(str.substr(start, stop - start));
  } catch (const std::invalid_argument& e) {
    // Строка не содержит чисел
    return {};
  } catch (const std::out_of_range& e) {
    // Число не влезает в int
    return {};
  }

  if( count == 3)
    c[count] = static_cast<unsigned char>(value);

  return { c[0], c[1], c[2], c[3] };
}

void print_ip(const std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>& ip) {
    std::cout << (int)std::get<0>(ip) << "." 
              << (int)std::get<1>(ip) << "." 
              << (int)std::get<2>(ip) << "." 
              << (int)std::get<3>(ip) << "\n";
}

struct IP_cmp_Greater {
    bool operator()(const std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>& lhs,
                    const std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>& rhs) const {
//        if (std::get<0>(lhs) != std::get<0>(rhs)) return std::get<0>(lhs) > std::get<0>(rhs);
//        if (std::get<1>(lhs) != std::get<1>(rhs)) return std::get<1>(lhs) > std::get<1>(rhs);
//        if (std::get<2>(lhs) != std::get<2>(rhs)) return std::get<2>(lhs) > std::get<2>(rhs);
//        return std::get<3>(lhs) > std::get<3>(rhs);

        return std::tie(lhs) > std::tie(rhs);
      };
};

int main( /* int argc, char const *argv[] */
        )
{
    try
    {
        std::map<std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>, // IP-адрес, разложенный в кортеж
                            int,             // число повторений данного адреса (т.к. у нас сортировка, сходные адреса идут подряд)
                            IP_cmp_Greater>  // сравнение кортежей
                            ip_pool;

        for(std::string line; std::getline(std::cin, line);)
        {
          std::vector<std::string>v = split(line, '\t');
          auto new_ip = convertIP(v.at(0), '.');

          auto it = ip_pool.find(new_ip);
          if (it != ip_pool.end()) {     // если такой IP есть
            int curr_value = it->second; // увеличиваем ключ (число повторений)
            it->second = curr_value+1; 
          } else {                       // иначе IP новый
            ip_pool.insert({new_ip, 1}); // встретился пока 1 раз
          }
        }

        // TODO reverse lexicographically sort
        // ^^^- уже отсортировано "by-design" при вставке в map с использованием функции сравнения ключей

/*
        for(std::vector<std::vector<std::string> >::const_iterator ip = ip_pool.cbegin(); ip != ip_pool.cend(); ++ip)
        {
            for(std::vector<std::string>::const_iterator ip_part = ip->cbegin(); ip_part != ip->cend(); ++ip_part)
            {
                if (ip_part != ip->cbegin())
                {
                    std::cout << ".";

                }
                std::cout << *ip_part;
            }
            std::cout << std::endl;
        }
 */

        for (const auto& pair : ip_pool) {
          const auto& ip     = pair.first;  // key
          const  int& repeat = pair.second; // val

          for( int j=0; j<repeat; j++)
            print_ip(ip);
        }
 
 
        // 222.173.235.246
        // 222.130.177.64
        // 222.82.198.61
        // ...
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first byte and output
        // ip = filter(1)

        /*
        auto filter1  = [](const auto& ip, unsigned char c1) { return std::get<0>(ip) == c1; };
        auto filter12 = [](const auto& ip, unsigned char c1,
                                           unsigned char c2) { return std::get<0>(ip) == c1
                                                                   && std::get<1>(ip) == c2; };
        ...
          */

        // если c1..c4 = -1, то они не участвуют в фильтрации
        auto filter = [](const auto& ip, int c1, int c2, int c3, int c4)
        {
          return
          (
            c1 == -1 ||
              ( c1 != -1 && c1 == (int)std::get<0>(ip) )
          )
          &&
          (
            c2 == -1 ||
              ( c2 != -1 && c2 == (int)std::get<1>(ip) )
          )
          &&
          (
            c3 == -1 ||
              ( c3 != -1 && c3 == (int)std::get<2>(ip) )
          )
          &&
          (
            c4 == -1 ||
              ( c4 != -1 && c4 == (int)std::get<3>(ip) )
          )
          ;
        };

        for (const auto& pair : ip_pool) {
          const auto& ip     = pair.first;  // key
          const  int& repeat = pair.second; // val

          if(filter(ip, 1, -1, -1, -1))
            for( int j=0; j<repeat; j++)
              print_ip(ip);
        }

        // 1.231.69.33
        // 1.87.203.225
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first and second bytes and output
        // ip = filter(46, 70)

        for (const auto& pair : ip_pool) {
          const auto& ip     = pair.first;  // key
          const  int& repeat = pair.second; // val

          // Выводим ключ и значение
          if(filter(ip, 46, 70, -1, -1))
            for( int j=0; j<repeat; j++)
              print_ip(ip);
        }

        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76

        // TODO filter by any byte and output
        // ip = filter_any(46)

        auto filter_any = [](const auto& ip, unsigned char c) {
          //constexpr std::size_t num_c = std::tuple_size<decltype(ip)>::value;
          return    std::get<0>(ip) == c
                 || std::get<1>(ip) == c
                 || std::get<2>(ip) == c
                 || std::get<3>(ip) == c ;
        };


        for (const auto& pair : ip_pool) {
          const auto& ip     = pair.first;  // key
          const  int& repeat = pair.second; // val

          // Выводим ключ и значение
          if(filter_any(ip, 46))
            for( int j=0; j<repeat; j++)
              print_ip(ip);
        }

        // 186.204.34.46
        // 186.46.222.194
        // 185.46.87.231
        // 185.46.86.132
        // 185.46.86.131
        // 185.46.86.131
        // 185.46.86.22
        // 185.46.85.204
        // 185.46.85.78
        // 68.46.218.208
        // 46.251.197.23
        // 46.223.254.56
        // 46.223.254.56
        // 46.182.19.219
        // 46.161.63.66
        // 46.161.61.51
        // 46.161.60.92
        // 46.161.60.35
        // 46.161.58.202
        // 46.161.56.241
        // 46.161.56.203
        // 46.161.56.174
        // 46.161.56.106
        // 46.161.56.106
        // 46.101.163.119
        // 46.101.127.145
        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76
        // 46.55.46.98
        // 46.49.43.85
        // 39.46.86.85
        // 5.189.203.46
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
