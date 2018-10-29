#ifndef STD_VECTOR_H
#define STD_VECTOR_H

#ifndef __KERNEL__
#error "This is a kernel port of std::vector"
#endif

#include "move.h"

namespace std
{
    template<typename T>
    class vector
    {
        T *    array;
        size_t len;
        size_t cap;
    
    public:
        class iterator
        {
            T * element;
        public:
            iterator(T * ptr) : element(ptr) {}
            iterator(const iterator & rhs) : element(rhs.element) {}
        
            bool operator==(const iterator & rhs) const
            {
                return element == rhs.element;
            }
            
            bool operator!=(const iterator & rhs) const
            {
                return element != rhs.element;
            }
            
            iterator & operator=(const iterator & rhs)
            {
                if ( this != &rhs )
                {
                    element = rhs.element;
                }
                
                return *this;
            }
            
            iterator operator+(int val) const
            {
                return iterator(element + 1);
            }
            
            // we only implement the prefix increment operator
            iterator & operator++()
            {
                element += 1;
                return *this;
            }
                
            // dereference operator
            T& operator*() const
            {
               return *element;
            }
        };       
        
        vector() : array(nullptr), len(0), cap(0) {}  
        ~vector()
        {
            for ( unsigned i = 0; i < len; ++i )
            {
                
            }
        }
    
        void reserve(size_t sz)
        {
            if ( sz > cap )
            {
                array = realloc(array, sz*sizeof(T));
                cap = sz;
            }
        }
    
        bool empty() const
        {
            return len == 0;
        }
        
        template< class... Args >
        void emplace_back( Args&&... args )
        {
            if ( cap <= len )
            {
                if ( cap == 0 )
                    reserve(2);
                else
                    reserve(cap << 1);
            }
            
            new(&array[len]) T(args...);
            ++len;
        }
        
        T & back()
        {
            return array[len - 1];
        }
        
        T & front()
        {
            return array[0];
        }
    };
    
    // TODO: this is does not conform... 
    template<typename T>
    void sort(vector<T>::iterator start, vector<T>::iterator end)
    {
        // selection sort is preferred to minimize swapping
        while ( vector<T>::iterator i = start; i != end; ++i )
        {
            vector<T>::iterator min = i;
            
            while ( vector<T>::iterator j = i + 1; j != end; ++j )
            {
                if ( *j < *min )
                    min = j;
            }
        }
        
        if ( i != min )
        {
            T temp = std::move(*i);
            *i = *min;
            *min = temp;
        }
    }
}

#endif /* STD_VECTOR_H */

