#ifndef STD_MOVE_H
#define STD_MOVE_H

#ifndef __KERNEL__
#error "This is a kernel port of std::move"
#endif

namespace std
{
    // for port of std::move
    template<class T>
    struct remove_reference
    {  
        typedef T type;
    };

    template<class T>
    struct remove_reference<T&>
    {   // remove reference
        typedef T type;
    };

    template<class T>
    struct remove_reference<T&&>
    {   // remove rvalue reference
        typedef T type;
    };

    template <typename T>
    inline typename remove_reference<T>::type&& move(T&& arg) noexcept
    {
      return static_cast<typename remove_reference<T>::type&&>(arg);
    }
}

#endif /* STD_MOVE_H */

