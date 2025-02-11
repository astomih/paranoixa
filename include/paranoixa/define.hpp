#ifndef PARANOIXA_DEFINE_HPP
#define PARANOIXA_DEFINE_HPP
#include "allocator.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
namespace paranoixa {

// Macro to define the build type
#ifdef _DEBUG
#define PARANOIXA_BUILD_DEBUG
#elif NDEBUG
#define PARANOIXA_BUILD_RELEASE
#endif

// Macro to define the platform
#ifdef _WIN32
#define PARANOIXA_PLATFORM_WINDOWS
#elif __linux__
#define PARANOIXA_PLATFORM_LINUX
#elif __APPLE__
#define PARANOIXA_PLATFORM_MACOS
#elif __EMSCRIPTEN__
#define PARANOIXA_PLATFORM_EMSCRIPTEN
#endif

// Shared pointer and reference type aliases
template <class T> using Ptr = std::shared_ptr<T>;
template <class T> using Ref = std::weak_ptr<T>;

// Allocation wrapper functions
template <class T, class... Args>
Ptr<T> MakePtr(AllocatorPtr allocator, Args &&...args) {
  STLAllocator<T> stdAllocator{allocator};
  return std::allocate_shared<T>(stdAllocator, std::forward<Args>(args)...);
}
template <class T, class U> Ptr<T> DownCast(Ptr<U> ptr) {
#ifdef PARANOIXA_BUILD_DEBUG
  return std::dynamic_pointer_cast<T>(ptr);
#else
  return std::static_pointer_cast<T>(ptr);
#endif
}

// Type aliases
using uint32 = std::uint32_t;
using uint8 = std::uint8_t;
using int32 = std::int32_t;
using int8 = std::int8_t;
using float32 = std::float_t;
using float64 = std::double_t;

// Array class
template <typename T> class Array : public std::vector<T, STLAllocator<T>> {
public:
  Array(AllocatorPtr allocator) : std::vector<T, STLAllocator<T>>(allocator) {}
};

using String =
    std::basic_string<char, std::char_traits<char>, STLAllocator<char>>;

// Hash map class
template <typename K, typename V, typename Hash = std::hash<K>,
          typename Equal = std::equal_to<K>>
class HashMap : public std::unordered_map<K, V, Hash, Equal,
                                          STLAllocator<std::pair<const K, V>>> {
public:
  HashMap(AllocatorPtr allocator)
      : std::unordered_map<K, V, Hash, Equal,
                           STLAllocator<std::pair<const K, V>>>(allocator) {}
};

} // namespace paranoixa
#endif // PARANOIXA_DEFINE_HPP