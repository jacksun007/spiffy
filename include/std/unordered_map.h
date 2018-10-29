#ifndef STD_UNORDERED_MAP_H
#define STD_UNORDERED_MAP_H

#ifndef __KERNEL__
#error "This is a kernel port of std::unordered_map"
#endif

#include "move.h"

namespace std
{
    template<typename K>
    struct hash;
    
    template<>
    struct hash<unsigned>
    {
        size_t operator()(unsigned value)
        {
            // TODO: currently does nothing... (lol)
            return value;
        }
    };
    
    template<typename A, typename B>
    struct pair
    {
        A first;
        B second;
        
        pair(const A & a, B && b) : first(a), second(move(b)) {}  
    };

    template<typename K, typename T, class H = hash<K> >
    class unordered_map
    {
        struct hlist_node
        {
            pair<K, T>   item;
            hlist_node * next;
            
            hlist_node(const K & k, T && t) : item(k, move(t)), next(nullptr) {}
        };
        
        hlist_node ** bucket;
        unsigned      num_buckets;
        size_t        num_elements;        
        
        void clear_internal()
        {
            for ( unsigned i = 0; i < num_buckets; i++ )
            {
                hlist_node * head = bucket[i];
                
                while (head != nullptr)
                {
                    hlist_node * temp = head;
                    head = head->next;
                    delete temp;
                }
            }
        }
        
    public:
        class iterator
        {
            friend class unordered_map;
        
            struct hlist_node ** bucket;
            struct hlist_node *  current;
            unsigned             bucket_number;
            unsigned             num_buckets;
            
            iterator(struct hlist_node ** start, unsigned nb) : bucket(start), 
                current(nullptr), bucket_number(0), num_buckets(nb)
            {
                // used by begin
                ++(*this);
            }
            
            iterator(struct hlist_node ** start, unsigned nb, 
                     struct hlist_node * curr, unsigned bnum) 
                : bucket(start), current(curr), bucket_number(bnum), num_buckets(nb)
            {
                // used by find
            }
          
        public:           
            iterator() : bucket(nullptr), current(nullptr), bucket_number(0),
                         num_buckets(0) {}
            
            iterator & operator++()     // prefix increment operator
            {
                while ( bucket_number < num_buckets )
                {
                    if ( current == nullptr )
                        current = bucket[bucket_number];
                    else
                        current = current->next;
                
                    if ( current != nullptr )
                        return *this;
                        
                    ++bucket_number;
                }
                    
                return *this;
            }
            
            bool operator!=(const iterator & rhs) const
            {
                return ( current != rhs.current );
            }
            
            pair<K, T> & operator*()
            {
                return current->item;
            }
            
            pair<K, T> * operator->() const
            {
                return &current->item;
            }
        };
    
        unordered_map(unsigned nb)
            : bucket(new hlist_node *[nb])
            , num_buckets(nb)
            , num_elements(0)  
        {
            memset(bucket, 0, sizeof(hlist_node *)*nb);
        }
        
        ~unordered_map()
        {
            clear_internal();
            delete [] bucket;
        }
        
        void clear()
        {
            clear_internal();
            num_elements = 0;
            memset(bucket, 0, sizeof(hlist_node *)*num_buckets);
        }
        
        size_t size() const
        {
            return num_elements;
        }
    
        iterator begin() const
        {
            return iterator(bucket, num_buckets);
        }    
    
        iterator end() const
        {
            return iterator();
        }
        
        iterator find(K k) const
        {
            H hasher;
            unsigned bucket_number = hasher(k) % num_buckets;
            hlist_node * head = bucket[bucket_number];
            
            while ( head != nullptr )
            {
                if ( head->item.first == k )
                {
                    return iterator(bucket, num_buckets, head, bucket_number);
                }
                
                head = head->next;
            }
            
            return iterator();
        }
        
        iterator erase(const iterator & it)
        {
            hlist_node * victim = bucket[it.bucket_number];
            iterator ret = it;
            
            // advance the return value first before we modify it
            ++ret;
        
            if ( bucket[it.bucket_number] == it.current )
            {
                bucket[it.bucket_number] = bucket[it.bucket_number]->next;
            }
            else
            {
                hlist_node * prev;
                
                do {
                    prev = victim;
                    victim = victim->next;
                } while ( victim != it.current );
                
                if ( victim == nullptr )
                {
                    printk("ERROR: std::unordered_map::erase failed!\n");
                    return it;
                }
                
                // re-link linked list, excluding victim
                prev->next = victim->next;
            }
            
            delete victim;
            --num_elements;
            return ret;
        }
        
        // TODO: non-standard return value
        void emplace(const K & k, T && t)
        {
            H hasher;
            unsigned bucket_number = hasher(k) % num_buckets;
            hlist_node * node = new hlist_node(k, move(t));
            
            if ( bucket[bucket_number] == nullptr )
            {
                bucket[bucket_number] = node;
            }
            else
            {
                node->next = bucket[bucket_number];  
                bucket[bucket_number] = node;
            }
            
            ++num_elements;
        }
    };
}

#endif /* STD_UNORDERED_MAP_H */

