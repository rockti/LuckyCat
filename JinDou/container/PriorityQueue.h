#pragma once

#include <vector>
#include <functional>
// CLASS TEMPLATE priority_queue
template <class _Ty, 
	class _Pr = std::less<_Ty> >
	class MutablePriorityQueue { // priority queue implemented with a _Container
	public:
		using _Container = std::vector<_Ty>;
		using container_type  = _Container;
		using value_compare   = _Pr;
		using value_type      = typename _Container::value_type;
		using size_type       = typename _Container::size_type;
		using reference       = typename _Container::reference;
		using const_reference = typename _Container::const_reference;
		using const_iterator = typename _Container::const_iterator;

// 		static_assert(is_same_v<_Ty, value_type>, "container adaptors require consistent types");

		MutablePriorityQueue()
			: c(), comp() { // construct with empty container, default comparator
		}

		explicit MutablePriorityQueue(const _Pr& _Pred)
			: c(), comp(_Pred) { // construct with empty container, specified comparator
		}

		MutablePriorityQueue(const _Pr& _Pred, const _Container& _Cont)
			: c(_Cont), comp(_Pred) { // construct by copying specified container, comparator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _InIt>
		MutablePriorityQueue(_InIt _First, _InIt _Last)
			: c(_First, _Last), comp() { // construct by copying [_First, _Last), default comparator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _InIt>
		MutablePriorityQueue(_InIt _First, _InIt _Last, const _Pr& _Pred)
			: c(_First, _Last), comp(_Pred) { // construct by copying [_First, _Last), specified comparator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _InIt>
		MutablePriorityQueue(_InIt _First, _InIt _Last, const _Pr& _Pred, const _Container& _Cont)
			: c(_Cont), comp(_Pred) { // construct by copying [_First, _Last), container, and comparator
			c.insert(c.end(), _First, _Last);
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		explicit MutablePriorityQueue(const _Alloc& _Al)
			: c(_Al), comp() { // construct with empty container, allocator
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		MutablePriorityQueue(const _Pr& _Pred, const _Alloc& _Al)
			: c(_Al), comp(_Pred) { // construct with empty container, comparator, allocator
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		MutablePriorityQueue(const _Pr& _Pred, const _Container& _Cont, const _Alloc& _Al)
			: c(_Cont, _Al), comp(_Pred) { // construct by copying specified container, comparator, allocator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		MutablePriorityQueue(const MutablePriorityQueue& _Right, const _Alloc& _Al)
			: c(_Right.c, _Al), comp(_Right.comp) { // construct by copying _Right, allocator
		}

		MutablePriorityQueue(const _Pr& _Pred, _Container&& _Cont)
			: c(_STD move(_Cont)), comp(_Pred) { // construct by moving specified container, comparator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _InIt>
		MutablePriorityQueue(_InIt _First, _InIt _Last, const _Pr& _Pred, _Container&& _Cont)
			: c(_STD move(_Cont)), comp(_Pred) { // construct by copying [_First, _Last), moving container
			c.insert(c.end(), _First, _Last);
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		MutablePriorityQueue(const _Pr& _Pred, _Container&& _Cont, const _Alloc& _Al)
			: c(_STD move(_Cont), _Al), comp(_Pred) { // construct by moving specified container, comparator, allocator
			_STD make_heap(c.begin(), c.end(), comp);
		}

		template <class _Alloc, class = enable_if_t<uses_allocator_v<_Container, _Alloc>>>
		MutablePriorityQueue(MutablePriorityQueue&& _Right, const _Alloc& _Al)
			: c(_STD move(_Right.c), _Al), comp(_STD move(_Right.comp)) { // construct by moving _Right, allocator
		}

		void push(value_type&& _Val) { // insert element at beginning
			c.push_back(_STD move(_Val));
			_STD push_heap(c.begin(), c.end(), comp);
		}

		template <class... _Valty>
		void emplace(_Valty&& ... _Val) { // insert element at beginning
			c.emplace_back(_STD forward<_Valty>(_Val)...);
			_STD push_heap(c.begin(), c.end(), comp);
		}

		_NODISCARD bool empty() const { // test if queue is empty
			return c.empty();
		}

		_NODISCARD size_type size() const { // return length of queue
			return c.size();
		}

		_NODISCARD reference top() { // return highest-priority element
			return c.front();
		}
		_NODISCARD const_reference top() const { // return highest-priority element
			return c.front();
		}

		const_iterator begin() const {
			return c.begin();
		}

		const_iterator end() const {
			return c.end();
		}
		
		

		void push(const value_type& _Val) { // insert value in priority order
			c.push_back(_Val);
			_STD push_heap(c.begin(), c.end(), comp);
		}

		void pop() { // erase highest-priority element
			_STD pop_heap(c.begin(), c.end(), comp);
			c.pop_back();
		}

		void swap(MutablePriorityQueue& _Right) _NOEXCEPT_COND(
			_Is_nothrow_swappable<_Container>::value&& _Is_nothrow_swappable<_Pr>::value) { // exchange contents with _Right
			_Swap_adl(c, _Right.c);
			_Swap_adl(comp, _Right.comp);
		}

	protected:
		_Container c; // the underlying container
		_Pr comp; // the comparator functor
};