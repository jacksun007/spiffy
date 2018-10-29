#ifndef STD_LIST_H
#define STD_LIST_H

#ifndef __KERNEL__
#error "This is a kernel port of std::list"
#endif

namespace std
{
// NOTE: this is a partial implementation of the whole C++ standard
    // for list, AND we are using singly-linked list 
    template <typename T>
    class list
    {
        struct node
        {
            T value;
            struct node * next;
            
            template<typename... Args>
            node(Args && ... args) : value(args...), next(nullptr) {}
        };        
        
        struct node * head;
        struct node * tail;
        size_t        num_elements;
        
        void push_back(struct node * n)
        {
            if ( head == nullptr )
            {
                head = n;
            }
            
            if ( tail != nullptr )
            {
                tail->next = n; 
            }
            
            tail = n;
            ++num_elements; 
        }
         
    public:
        list() : head(nullptr), tail(nullptr), num_elements(0) {}
        ~list()
        {
            clear();
        }
    
        class iterator
        {
            friend class list;
            struct node * curr;
            
            iterator(struct node * n) : curr(n) {}
        public:
            iterator() : curr(nullptr) {}
            
            // prefix
            iterator & operator++()
            {
                curr = curr->next;
                return *this;
            }
            
            bool operator!=(const iterator & rhs) const
            {
                return ( curr != rhs.curr );
            }
            
            T & operator*()
            {
                return curr->value;
            }
            
            T * operator->() const
            {
                return &curr->value;
            }
        };
        
        size_t size() const
        {
            return num_elements;
        }
        
        void clear()
        {
            struct node * n = head;
            
            while ( n != nullptr )
            {
                struct node * temp = n;
                n = n->next;
                delete temp;
            }
            
            head = nullptr;
            tail = nullptr;
            num_elements = 0;
        }
        
        iterator begin() const
        {
            return iterator(head);
        }
        
        iterator end() const
        {
            return iterator();
        }
        
        bool empty() const
        {
            return (head == nullptr);
        }
        
        const T & front() const
        {
            return head->value;
        }
        
        T & front()
        {
            return head->value;
        }
        
        const T & back() const
        {
            return tail->value;
        }
        
        T & back()
        {
            return tail->value;
        }
    
        template<typename... Args>
        void emplace_back(Args && ... args)
        {
            struct node * n = new struct node(args...);
            push_back(n);
        }
        
        void pop_front()
        {
            struct node * temp = head;
            head = head->next;
            if ( head == nullptr )
                tail = nullptr;
            --num_elements;
            delete temp;
        }
    };
} /* namespace std */

#endif /* STD_LIST_H */

