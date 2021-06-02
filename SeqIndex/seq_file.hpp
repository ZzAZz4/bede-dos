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
    using Node = Node<Record>;
    using NodePtr = Pointer<Node>;
    using Name = fixed_string<32>;
    using Mode = Pointer<>::Mode;
//    using NodeIter = typename Node::Iterator;
    Name header_name;

    struct Header
    {
        Pointer<Node> begin;
        Pointer<Node> aux_begin;
        int main_alloc_pos = 0;
        int aux_alloc_pos = 0;
    };
    Header header;

    explicit SeqIndex (
        Name name, Mode mode = Pointer<>::WTE_FILE) : header_name(
        "seq_files/index")
    {
        constexpr char n[] = "seq_files/index";
        constexpr char aux[] = "seq_files/aux_file";
        Name na(n);
        Name a(aux);


        if (mode == Pointer<>::CTE_FILE)
        {
            create_index_file(na, a);
        }
        else
            load_from_memory(na);
    }

    static constexpr auto compare = [] (const auto& it, const auto& a)
    {
        return !it.erased && get_key(it.data) < get_key(a);
    };


    bool push (const Record& elem)
    {
        bool success = false;
        const auto it = std::lower_bound(vec_begin(), vec_end(), elem, compare);
        if (it == vec_end())
        {
            push_back(elem);
            success = true;
        }
            // there should be a case when the element is erased
        else if (get_key((*it).data) == get_key(elem)) success = false;
        else if (it == vec_begin())
        {
            push_front(elem);
            success = true;
        }
        else success = try_insert_before(elem, it);


        if (success)
        {
            Pointer<Header> header_ptr(header_name, 0);
            header_ptr.set(header);
        }

        return success;

    }


    bool pop (const Key& elem)
    {
        //locate the node to delete
        auto it = std::lower_bound(vec_begin(), vec_end(), elem, compare);

        if (it == vec_end()) return false;

        auto it_node = (*it);
        if (get_key(it_node.data) == get_key(elem))
        {
            if (it == vec_begin()) pop_front();
            else if (it == vec_end() - 1) pop_back();
            else remove_from_main(it);
            return true;
        }
        else return try_remove_from_aux(it, elem);

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
    std::optional <Record> find (const Key& key) const
    {
        auto ret_val = find(key, key);
        if (ret_val.empty()) return std::nullopt;
        assert(ret_val.size() == 1);
        return ret_val[0];
    }

    [[nodiscard]] auto buffer_begin () const
    { return NodePtr(header.begin.filePath, sizeof(Header)); }

    [[nodiscard]] auto vec_begin () const
    { return header.begin; }

    [[nodiscard]] auto vec_end () const
    { return vec_begin() + header.main_alloc_pos; }

    [[nodiscard]] auto aux_size () const
    { return header.aux_alloc_pos; }


    void print () const
    {
        auto it = vec_begin();
        auto end_it = vec_end();
        for (; it != end_it; it = (*it).next)
        {
            auto it_node = (*it);
            if (it_node.erased)
                continue;
            std::cout << it_node.data << '\n';
        }
    }

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

        // Including hardcoded popped case bcz yes

        auto begin = vec_begin();
        if (std::distance(buffer_begin(), begin) > 0)
        {
            header.begin--;
            header.main_alloc_pos++;
            auto new_begin = vec_begin();
            new_begin.set(Node(elem, begin));
            return;
        }

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
        if ((*next).erased)
        {
            return begin + 1;
        }
        header.aux_alloc_pos++;
        return NodePtr(header.aux_begin.filePath, 0) + aux_size();
    }

    bool try_insert_before (const Record& elem, NodePtr it)
    {
        // case for when the previous is deleted goes somewhere over here, probs

        // or maybe inside begin_unsafe_get_leq_ptr, who knows.
        const auto prev_ptr = begin_unsafe_get_leq_ptr(elem, it);
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
    auto begin_unsafe_get_leq_ptr (const Record& elem, NodePtr it) const
    {
        NodePtr prev_it = it - 1;

        for (;;)
        {
            Node prev_node = *prev_it;
            if (get_key((*(prev_node.next)).data) > get_key(elem))
                return prev_it;
            prev_it = prev_node.next;
        }
    }

    [[nodiscard]]
    auto begin_unsafe_get_less_ptr (const Key& elem, NodePtr it) const
    {
        NodePtr prev_it = it - 1;

        for (;;)
        {
            Node prev_node = *prev_it;
            if (!(get_key((*(prev_node.next)).data) < get_key(elem)))
                return prev_it;
            prev_it = prev_node.next;
        }
    }

    bool try_remove_from_aux (NodePtr& it, const Key& elem) const
    {
        if (it == vec_begin()) return false;
        auto prev = begin_unsafe_get_less_ptr(elem, it);
        auto prev_node = *prev;
        auto next = prev_node.next;
        auto next_node = *next;
        if (get_key(next_node.data) == get_key(elem))
        {
            prev_node.next = next_node.next;
            next_node.erased = true;
            prev.set(prev_node);
            next.set(next_node);
            return true;
        }
        else return false;
    }

    void remove_from_main (NodePtr& it) const
    {
        auto it_node = (*it);
        auto next_ptr = it_node.next;
        auto next_node = *next_ptr;
        it.set(next_node);
        next_node.erased = true;
        next_ptr.set(next_node);
    }

    void pop_back ()
    {
        auto it = vec_end() - 1;
        auto prev = it - 1;
        auto prev_node = *prev;
        if (prev_node.next == it)
        {
            --header.main_alloc_pos;
            Pointer<Header>(header.begin.filePath, 0).set(header);
        }
        else
        {
            flatten_from_prev(it);
        }
    }

    void flatten_from_prev (NodePtr& it) const
    {
        auto prev = it - 1;
        auto prev_node = *prev;
        auto it_node = *it;
        auto repl = prev_node.next;
        auto repl_node = *repl;
        repl_node.next = it_node.next;
        it.set(repl_node);
        prev_node.next = it;
        prev.set(prev_node);
    }

    void pop_front ()
    {
        auto begin = vec_begin();
        auto begin_node = *begin;

        if (begin_node.next == vec_end()) reset_index();
        else if (begin_node.next != begin + 1) flatten_into_front();
        else mark_begin_as_erased();
    }

    void mark_begin_as_erased ()
    {
        auto begin = vec_begin();
        auto begin_node = *begin;
        begin_node.erased = true;
        begin.set(begin_node);
        ++header.begin;
        --header.main_alloc_pos;
        Pointer<Header>(header.begin.filePath, 0).set(header);
    }

    void flatten_into_front () const
    {
        auto begin = vec_begin();
        auto begin_node = *begin;
        begin.set(*begin_node.next);
    }

    void reset_index ()
    {
        header.begin.position = sizeof(Header);
        header.main_alloc_pos = 0;
        Pointer<Header>(header.begin.filePath, 0).set(header);
    }
};

#endif //MEMORYMAP_SEQ_FILE_HPP
