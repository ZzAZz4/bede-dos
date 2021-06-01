#ifndef MEMORYMAP_SEQ_FILE_HPP
#define MEMORYMAP_SEQ_FILE_HPP

#include "../headers/utils.hpp"
#include "../headers/pointer.tcc"

template<class T>
struct Node
{
    T data;
    Pointer<Node> next;
    bool erased = false;

    explicit Node (T data, Pointer<Node> next = Pointer<Node>())
        : data(data), next(next)
    {
    }

    void insert_next (Pointer<Node> new_next)
    {
        auto prev_val = new_next.get();
        prev_val.next = this->next;
        this->next = new_next;
        new_next.set(prev_val);
    }
};

template<class Record>
struct SeqIndex
{
    using Key = decltype(get_key(std::declval<const Record&>()));
    using NodePtr = Pointer<Node<Record>>;
    using Name = fixed_string<32>;
    using Mode = Pointer<>::Mode;
//    using NodeIter = typename Node::Iterator;

    struct Header
    {
        Pointer<Node<Record>> begin;
        Pointer<Node<Record>> aux_begin;
        int main_alloc_pos = 0;
        int aux_alloc_pos = 0;
    };
    Header header;

    explicit SeqIndex (
        Name name, Name aux_name, Mode mode = Pointer<>::WTE_FILE)
    {
        if (mode == Pointer<>::CTE_FILE)
            create_index_file(name, aux_name);
        else
            load_from_memory(name);
    }

    static constexpr auto compare = [] (const auto& it, const auto& a)
    {
        return !it.erased && get_key(it.data) < get_key(a);
    };

    bool push (const Record& elem)
    {
        const auto it = std::lower_bound(vec_begin(), vec_end(), elem, compare);
        if (it == vec_end())
        {
            push_back(elem);
            return true;
        }
        // there should be a case when the element is erased
        if (get_key((*it).data) == get_key(elem)) return false;
        // erase previous return false; and add the following instead(?):
        // {if ((*it).erased) == true) {(*it).erased == true; return true;} return false;}
        if (it == vec_begin())
        {
            push_front(elem);
            return true;
        }
        else return try_insert_before(elem, it);

    }

    bool pop (const Key& elem)
    {
        //locate the node to delete
        const auto it = lower_bound(vec_begin(), vec_end(), elem, compare);
        //if found at the beginning simply mark as deleted
        if (it == vec_begin())
        {
            //probably should get rid of dependencies too
            auto a = *it;
            a.erased = false;
            *it.set(a);
            header.main_alloc_pos--;
            return true;
        }
         //si es end, vete a end - 1 y sigue la chain, pq solo puede estar ahi
        if (it == vec_end())
        {
            //get previous, set next as null
            /*
            //some operation to get the previous one it_prev
            auto a_prev = *it_prev;
            a_prev.next = nullptr;
            *it_prev.set(a_prev);
            */
            auto a = *it;
            a.erased = false;
            *it.set(a);
            header.main_alloc_pos--;
            return true;
        }
        //if found in the middle, change pointers
        /*
        something something. change pointer from prev to node-to-delete's next
        */
        //if found in aux, flattening
        //same as previous but know that you're moving things from one array to the other
        //do the header.aux_alloc_pos--; thing

        //if it points to other registers in aux, handle those connections

    }

    [[nodiscard]]
    std::vector <Record> find (const Key& first, const Key& last) const
    {
        std::vector <Record> ret_val;
        auto it = std::lower_bound(vec_begin(), vec_end(), first, compare);

        auto cur_it = find_prev_main(it);
        for (; cur_it != vec_end();)
        {
            const auto cur_node = *cur_it;
            const auto cur_record = cur_node.data;
            if (get_key(cur_record) > get_key(last)) break;
            if (get_key(cur_record) >= get_key(first))
                ret_val.emplace_back(cur_record);
            cur_it = (*cur_it).next;
        }
        return ret_val;
    }


    [[nodiscard]]
    std::optional<Record> find (const Key& key) const
    {
        auto ret_val = find(key, key);
        if (ret_val.empty()) return std::nullopt;
        assert(ret_val.size() == 1);
        return ret_val[0];
    }



    [[nodiscard]] auto vec_begin () const
    { return NodePtr(header.begin.filePath, sizeof(Header)); }

    [[nodiscard]] auto vec_end () const
    { return vec_begin() + header.main_alloc_pos; }

    [[nodiscard]] auto aux_size () const
    { return header.aux_alloc_pos; }

private:
    NodePtr find_prev_main (const NodePtr& it) const
    {
        if (it == vec_begin()) return it;
        auto prev_it = it - 1;
        while ((*prev_it).erased)
        {
            prev_it--;
        }
        return prev_it;
    }
    void create_index_file (Name& name, Name& aux_name)
    {
        Pointer<Header> header_ptr(name, 0);
        {
            volatile std::fstream tmp(name.data(), Pointer<>::CTE_FILE);
            volatile std::fstream tmp2(aux_name.data(), Pointer<>::CTE_FILE);
        }
        header = Header{ .begin = NodePtr(name, sizeof(Header)),
            .aux_begin = NodePtr(aux_name, 0)
        };

        header_ptr.set(header);
    }

    void load_from_memory (Name& name)
    { header = *Pointer<Header>(name, 0); }

    void push_back (const Record& elem)
    {
        const auto new_ptr = header.begin + header.main_alloc_pos;
        new_ptr.set(Node(elem, new_ptr + 1));
        header.main_alloc_pos++;
    }

    void push_front (const Record& elem)
    {
        // i'm now sure there is a lot more cases to this. Here is one:
        // main: [3], push 1 should place 1 in front and 3 would normally go to aux.
        // But a chain can only exist between two non-erased contiguous nodes,
        // so it would inmediately promote back. Idk about promotion and that,
        // so here is the hardcoded version.

        auto begin = vec_begin();
        auto new_ptr = allocate_after_begin(begin);

        auto begin_node = *begin;
        new_ptr.set(begin_node);
        begin.set(Node(elem, new_ptr));
    }

    NodePtr allocate_after_begin (NodePtr& begin)
    {
        auto next = begin + 1;
        if (next == vec_end())
        {
            header.main_alloc_pos++;
            auto data = *begin;
            data.next = vec_end();
            begin.set(data);
            return next;
        }
        if ((*next).erased) {
            return begin + 1;
        }
        header.aux_alloc_pos++;
        return NodePtr(header.aux_begin.filePath, 0) + aux_size();
    }

    bool try_insert_before (const Record& elem, NodePtr it)
    {
        // case for when the previous is deleted goes somewhere over here, probs

        // or maybe inside begin_unsafe_get_prev_ptr, who knows.
        const auto prev_ptr = begin_unsafe_get_prev_ptr(elem, it);
        if (get_key((*prev_ptr).data) == get_key(elem)) return false;

        const auto new_ptr = NodePtr(header.aux_begin.filePath, 0) + aux_size();
        header.aux_alloc_pos++;

        auto prev_node = *prev_ptr;
        auto new_node = *new_ptr;

        new_node.data = elem;
        new_node.next = prev_node.next;
        prev_node.next = new_ptr;

        prev_ptr.set(prev_node);
        new_ptr.set(new_node);
        return true;
    }

    [[nodiscard]]
    auto begin_unsafe_get_prev_ptr (const Record& elem, NodePtr it) const
    {
        NodePtr prev_it = it - 1;

        for (;;)
        {
            Node<Record> prev_node = *prev_it;
            if (get_key((*(prev_node.next)).data) > get_key(elem))
                return prev_it;
            prev_it = prev_node.next;
        }
    }
};

#endif //MEMORYMAP_SEQ_FILE_HPP
