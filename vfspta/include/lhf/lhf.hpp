#ifndef LHF_HPP
#define LHF_HPP
#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <string>

#include "lhf_config.hpp"
#include "profiling.hpp"

namespace lhf {

#ifdef LHF_ENABLE_DEBUG
#define LHF_DEBUG(x) { x };
#else
#define LHF_DEBUG(x)
#endif

template<typename T>
using UniquePointer = std::unique_ptr<T>;

template<typename T>
using Vector = std::vector<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename K, typename V>
using OrderedMap = std::map<K, V>;

template<typename T>
using HashSet = std::unordered_set<T>;

template<typename T>
using OrderedSet = std::set<T>;

using String = std::string;

using Index = std::size_t;

// The index of the empty set. The first set that will ever be inserted
// in the property set value storage is the empty set.
static const constexpr Index EMPTY_SET = 0;

/**
 * @brief      Converts an iterable container to a string.
 */
template<typename SetT>
String container_to_string(const SetT &k) {
	std::stringstream s;

	s << "{ ";

	for (auto &i : k) {
		s << i << " ";
	}

	s << "}";

	return s.str();
}

/**
 * @brief      Converts an iterable container (storing smart pointers) to a
 *             string.
 */
template<typename PtrSetT>
String ptr_container_to_string(const PtrSetT &k) {
	std::stringstream s;

	s << "{ ";

	for (auto &i : *k.get()) {
		s << i << " ";
	}

	s << "}";

	return s.str();
}

/**
 * @brief      This struct contains the information about the operands of an
 *             operation (union, intersection, etc.)
 */
struct OperationNode {
	Index left;
	Index right;

	std::string to_string() const {
		std::stringstream s;
		s << "(" << left << "," << right << ")";
		return s.str();
	}

	bool operator<(const OperationNode &op) const {
		return (left < op.left) || (right < op.right);
	}

	bool operator==(const OperationNode &op) const {
		return (left == op.left) && (right == op.right);
	}
};

inline std::ostream &operator<<(std::ostream &os, const OperationNode &op) {
	return os << op.to_string();
}

};

/************************** START GLOBAL NAMESPACE ****************************/

template <>
struct std::hash<lhf::OperationNode> {
	std::size_t operator()(const lhf::OperationNode& k) const {
		return
			std::hash<lhf::Index>()(k.left) ^
			(std::hash<lhf::Index>()(k.right) << 1);
	}
};

/************************** END GLOBAL NAMESPACE ******************************/

namespace lhf {

/**
 * @brief      Generic Less-than comparator for set types.
 *
 * @tparam     SetT          The set type (like std::set or std::unordered_set)
 * @tparam     ElementT      The element type of the set (the first template
 *                           param of SetT)
 * @tparam     PropertyLess  Comparator for properties
 */
template<typename OrderedSetT, typename ElementT, typename PropertyLess>
struct SetLess {
	bool operator()(const OrderedSetT *a, const OrderedSetT *b) const {
		PropertyLess less;

		auto cursor_1 = a->begin();
		const auto &cursor_end_1 = a->end();
		auto cursor_2 = b->begin();
		const auto &cursor_end_2 = b->end();

		while (cursor_1 != cursor_end_1 && cursor_2 != cursor_end_2) {
			if (less(*cursor_1, *cursor_2)) {
				return true;
			}

			if (less(*cursor_2, *cursor_1)) {
				return false;
			}

			cursor_1++;
			cursor_2++;
		}

		return a->size() < b->size();
	}
};

/**
 * @brief      Generic Equality comparator for set types.
 *
 * @tparam     SetT      The set type (like std::set or std::unordered_set)
 * @tparam     ElementT  The element type of the set (the first template param
 *                       of SetT)
 */
template<typename SetT, typename ElementT, typename PropertyEqual>
struct SetEqual {
	inline bool operator()(const SetT *a, const SetT *b) const {
		PropertyEqual eq;
		if (a->size() != b->size()) {
			return false;
		}

		if (a->size() == 0) {
			return true;
		}

#ifdef LHF_USE_ORDERED_CONTAINER_FOR_PROPERTY_SETS
		auto cursor_1 = a->begin();
		const auto &cursor_end_1 = a->end();
		auto cursor_2 = b->begin();

		while (cursor_1 != cursor_end_1) {
			if (!eq(*cursor_1, *cursor_2)) {
				return false;
			}

			cursor_1++;
			cursor_2++;
		}
#else
		std::vector<ElementT> idlist_a, idlist_b;

		for (const auto value : *a) {
			idlist_a.push_back(value);
		}

		for (const auto value : *b) {
			idlist_b.push_back(value);
		}

		std::sort(idlist_a.begin(), idlist_a.end());
		std::sort(idlist_b.begin(), idlist_b.end());

		for (std::size_t i = 0; i < idlist_a.size(); i++) {
			if (!eq(idlist_a[i] != idlist_b[i])) {
				return false;
			}
		}
#endif

		return true;
	}
};

/**
 * @brief      Hasher for set types.
 *
 * @tparam     SetT      The set type (like std::set or std::unordered_set)
 * @tparam     ElementT  The element type of the set (the first template param
 *                       of SetT)
 */
template<typename SetT, typename ElementT>
struct SetHash {
	std::size_t operator()(const SetT *k) const {
		std::vector<ElementT> idlist_a;

		for (const auto value : *k) {
			idlist_a.push_back(value);
		}

		std::sort(idlist_a.begin(), idlist_a.end());

		// Adapted from boost::hash_combine
		size_t hash_value = 0;
		for (const auto value : idlist_a) {
			hash_value ^=
				std::hash<ElementT>()(value) +
				0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
		}

		return hash_value;
	}
};

/**
 * @brief      Struct that is thrown on an assertion failure.
 */
struct AssertError : public std::invalid_argument {
	AssertError(const std::string &message):
		std::invalid_argument(message.c_str()) {}
};

#define __STR(x) #x
#define STR(x) __STR(x)
#define EXCEPT(x) AssertError(x " [At: " __FILE__ ":" STR(__LINE__) "]")

#ifdef LHF_ENABLE_DEBUG

/// Check whether the index is a valid index within the property set.
/// @note Signed/unsigned is made to be ignored here for the sanity checking.
#define LHF_PROPERTY_SET_INDEX_VALID(index) { \
	_Pragma("GCC diagnostic push"); \
	_Pragma("GCC diagnostic ignored \"-Wtype-limits\"") \
	if ((index) < 0 || ((index) > property_sets.size() - 1)) { \
		throw EXCEPT("Invalid index supplied"); \
	} \
	_Pragma("GCC diagnostic pop") \
}


/// Check whether the pair of indexes is a valid within the property set.
#define LHF_PROPERTY_SET_PAIR_VALID(index1, index2) \
	LHF_PROPERTY_SET_INDEX_VALID(index1); \
	LHF_PROPERTY_SET_INDEX_VALID(index2);

/// Check whether the pair of indexes are unequal. Used for sanity checking.
#define LHF_PROPERTY_SET_PAIR_UNEQUAL(index1, index2) \
	if ((index1) == (index2)) { \
		throw EXCEPT("Equal set condition not handled by caller"); \
	}

#else

#define LHF_PROPERTY_SET_INDEX_VALID(index)
#define LHF_PROPERTY_SET_PAIR_VALID(index1, index2)
#define LHF_PROPERTY_SET_PAIR_UNEQUAL(index1, index2)

#endif


#if defined(LHF_USE_SORTED_VECTOR_FOR_PROPERTY_SETS)

/**
 * \def LHF_PUSH_ONE(__cont, __val)
 * @brief      Pushes one element to a PropertySet. Use this when implementing
 *             operations.
 *
 * @param      __cont  The container object
 * @param      __val   The value
 *
 */
#define LHF_PUSH_ONE(__cont, __val) (__cont).push_back((__val))

/**
 * \def LHF_PUSH_RANGE(__cont, __start, __end)
 * @brief      Pushes a range of elements to a PropertySet using an iterator.
 *             Use this when implementing operations.
 *
 * @param      __cont  The container object
 * @param      __start The start of the range (e.g. input.begin())
 * @param      __end   The end of the range (e.g. input.end())
 *
 */
#define LHF_PUSH_RANGE(__cont, __start, __end) (__cont).insert((__cont).end(), (__start), (__end))

#else

#define LHF_PUSH_ONE(__cont, __val) (__cont).insert((__val))
#define LHF_PUSH_RANGE(__cont, __start, __end) (__cont).insert((__start), (__end))

#endif

struct HintNone {
	static const constexpr bool recursive = false;
	
};

struct HintRecursive {
	static const constexpr bool recursive = true;

};

/**
 * @brief      The main LatticeHashForest structure.
 *             This class can be used as-is with a type or derived for
 *             additional functionality as needed
 *
 *
 * @tparam     PropertyT      The type of the property.
 *                            The property type must satisfy the following:
 *                            * It must be hashable with std::hash
 *                            * It must be less-than comparable
 *                            * It can be checked for equality
 *
 * @tparam     PropertyLess   Custom less-than comparator (if required)
 * @tparam     PropertyEqual  Custom equality comaparator (if required)
 */
template <
	typename PropertyT,
	typename HintType = HintNone,
	typename PropertyLess = std::less<PropertyT>,
	typename PropertyEqual = std::equal_to<PropertyT>>
struct LatticeHashForest {

	/**
	 * @brief      Used to store a subset relation between two set indices.
	 *             Because the index pair must be in sorted order to prevent
	 *             duplicates, it necessitates this enum.
	 */
	enum SubsetRelation {
		UNKNOWN  = 0,
		SUBSET   = 1,
		SUPERSET = 2
	};

#if defined(LHF_USE_ORDERED_SET_FOR_PROPERTY_SETS)
	using PropertySet = std::set<PropertyT>;
#elif defined(LHF_USE_SORTED_VECTOR_FOR_PROPERTY_SETS)
	using PropertySet = std::vector<PropertyT>;
#else
	using PropertySet = std::unordered_set<PropertyT>;
#endif

#ifdef LHF_USE_ORDERED_MAP_FOR_PROPERTY_SET_MAP
	using PropertySetMap =
		std::map<
			PropertySet *, Index,
			SetLess<PropertySet, PropertyT, PropertyLess>>;
#else
	using PropertySetMap =
		std::unordered_map<
			PropertySet *, Index,
			SetHash<PropertySet, PropertyT>,
			SetEqual<PropertySet, PropertyT, PropertyEqual>>;
#endif

	using UnaryOperationMap = HashMap<Index, Index>;
	using BinaryOperationMap = HashMap<OperationNode, Index>;


#ifdef LHF_ENABLE_PERFORMANCE_METRICS
	/**
	 * @brief      Operation performance Statistics.
	 */
	struct OperationPerf {
		// Number of direct hits (operation pair in map)
		size_t hits = 0;

		// Number of equal hits (both arguments consist of the same set)
		size_t equal_hits = 0;

		// Number of subset hits (operation pair not in but resolvable using
		// subset relation)
		size_t subset_hits = 0;

		// Number of empty hits (operation is optimised because at least one of
		// the sets is empty)
		size_t empty_hits = 0;

		// Number of cold misses (operation pair not in map, and neither
		// resultant set in map. Neither node in lattice exists, nor the edges)
		size_t cold_misses = 0;

		// Number of edge misses (operation pair not in map, but resultant set
		// in map. Node in lattice exists, but not the edges)
		size_t edge_misses = 0;

		String to_string() {
			std::stringstream s;
			s << "      " << "Hits       : " << hits << "\n"
			  << "      " << "Equal Hits : " << equal_hits << "\n"
			  << "      " << "Subset Hits: " << subset_hits << "\n"
			  << "      " << "Empty Hits : " << empty_hits << "\n"
			  << "      " << "Cold Misses: " << cold_misses << "\n"
			  << "      " << "Edge Misses: " << edge_misses << "\n";
			return s.str();
		}
	};

	HashMap<String, OperationPerf> perf;

#define LHF_PERF_INC(__oper, __category) (perf[STR(__oper)] . __category ++)

#else

#define LHF_PERF_INC(__oper, __category)

#endif

	// The property set storage array.
	Vector<UniquePointer<PropertySet>> property_sets = {};

	// The property set -> Index in storage array mapping.
	PropertySetMap property_set_map = {};

	BinaryOperationMap unions = {};
	BinaryOperationMap intersections = {};
	BinaryOperationMap differences = {};
	HashMap<OperationNode, SubsetRelation> subsets = {};

	inline bool is_empty(const Index i) {
		return i == EMPTY_SET;
	}

	SubsetRelation is_subset(const Index a, const Index b) {
		LHF_PROPERTY_SET_PAIR_VALID(a, b)
		__lhf_calc_functime();
		auto i = subsets.find({a, b});

		if (i == subsets.end()) {
			return UNKNOWN;
		} else {
			return i->second;
		}
	}

	/**
	 * @brief      Stores index `a` as the subset of index `b` if a < b,
	 *             else stores index `a` as the superset of index `b`
	 *
	 * @param[in]  a     The index of the first set
	 * @param[in]  b     The index of the second set.
	 */
	void store_subset(const Index a, const Index b) {
		LHF_PROPERTY_SET_PAIR_VALID(a, b)
		LHF_PROPERTY_SET_PAIR_UNEQUAL(a, b)
		__lhf_calc_functime();

		// We need to maintain the operation pair in index-order here as well.
		if (a > b) {
			subsets.insert({{b, a}, SUPERSET});
		} else {
			subsets.insert({{a, b}, SUBSET});
		}
	}

	/**
	 * @brief         Inserts a (or gets an existing) single-element set into
	 *                property set storage.
	 *
	 * @param[in]  c  The single-element property set.
	 *
	 * @return        Index of the newly created/existing set.
	 *
	 * @todo          Check whether the cache hit check can be removed.
	 */
	Index register_set_single(const PropertyT &c) {
		__lhf_calc_functime();

		UniquePointer<PropertySet> new_set =
			UniquePointer<PropertySet>(new PropertySet{c});

		auto cursor = property_set_map.find(new_set.get());

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		return cursor->second;
	}

	/**
	 * @brief      Inserts a (or gets an existing) single-element set into
	 *             the property set storage, and reports whether this set
	 *             was already  present or not.
	 *
	 * @param[in]  c     The single-element property set.
	 * @param[out] cold  Report if this was a cold miss.
	 *
	 * @return     Index of the newly created set.
	 */
	Index register_set_single(const PropertyT &c, bool &cold) {
		__lhf_calc_functime();

		std::unique_ptr<PropertySet> new_set =
			UniquePointer<PropertySet>(new PropertySet{c});

		auto cursor = property_set_map.find(new_set.get());

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			cold = true;
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		cold = false;
		return cursor->second;
	}

#ifdef LHF_USE_SORTED_VECTOR_FOR_PROPERTY_SETS
	/**
	 * Deduplicates and sorts a vector (to function equivalently to a set).
	 *
	 * The deduplicate-sort function here is based on a stackoverflow answer
	 * (chosen for speed metrics): https://stackoverflow.com/a/24477023
	 *
	 * Ideally, this shouldn't be used or require being used.
	 */
	void prepare_vector_set(PropertySet &c) {
		std::unordered_set<PropertyT> deduplicator;
		for (auto &i : c) {
			deduplicator.insert(i);
		}
		c.assign(deduplicator.begin(), deduplicator.end());
		std::sort(c.begin(), c.end());
	}
#endif

	/**
	 * @brief         Inserts a (or gets an existing) set into property set
	 *                storage.
	 *
	 * @param[in]  c  The property set.
	 *
	 * @return        Index of the newly created set.
	 *
	 * @todo          Check whether the cache hit check can be removed.
	 */
	Index register_set(PropertySet &c) {
		__lhf_calc_functime();

		auto cursor = property_set_map.find(&c);

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			UniquePointer<PropertySet> new_set =
				UniquePointer<PropertySet>(new PropertySet(c));
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		return cursor->second;
	}

	/**
	 * @brief         Inserts a (or gets an existing) set into property set
	 *                storage, and reports whether this set was already
	 *                present or not.
	 *
	 * @param[in]  c     The property set.
	 * @param[out] cold  Report if this was a cold miss.
	 *
	 * @return        Index of the newly created/existing set.
	 *
	 * @todo          Check whether the cache hit check can be removed.
	 */
	Index register_set(PropertySet &c, bool &cold) {
		__lhf_calc_functime();

		auto cursor = property_set_map.find(&c);

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			UniquePointer<PropertySet> new_set =
				UniquePointer<PropertySet>(new PropertySet(c));
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			cold = true;
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		cold = false;
		return cursor->second;
	}

	Index register_set(PropertySet &&c) {
		__lhf_calc_functime();

		auto cursor = property_set_map.find(&c);

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			UniquePointer<PropertySet> new_set =
				UniquePointer<PropertySet>(new PropertySet(c));
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		return cursor->second;
	}

	Index register_set(PropertySet &&c, bool &cold) {
		__lhf_calc_functime();

		auto cursor = property_set_map.find(&c);

		if (cursor == property_set_map.end()) {
			LHF_PERF_INC(property_sets, cold_misses);
			UniquePointer<PropertySet> new_set =
				UniquePointer<PropertySet>(new PropertySet(c));
			property_sets.push_back(std::move(new_set));
			Index ret = property_sets.size() - 1;
			property_set_map.insert(std::make_pair(property_sets[ret].get(), ret));
			cold = true;
			return ret;
		}

		LHF_PERF_INC(property_sets, hits);
		cold = false;
		return cursor->second;
	}

	/**
	 * @brief      Gets the actual property set specified by index.
	 *
	 * @param[in]  index  The index
	 *
	 * @return     The property set.
	 */
	inline const PropertySet &get_value(Index index) const {
		LHF_PROPERTY_SET_INDEX_VALID(index);
		return *property_sets[index].get();
	}

	/**
	 * @brief      Returns the size of the set at `index`
	 *
	 * @param[in]  index  The index
	 *
	 * @return     size of the set.
	 */
	inline std::size_t size_of(Index index) const {
		if (index == EMPTY_SET) {
			return 0;
		} else {
			return get_value(index).size();
		}
	}

	/**
	 * @brief      Less than comparator for operations. You MUST use this
	 *             instead of directly using anything else like "<"
	 *
	 * @param[in]  a     LHS Property
	 * @param[in]  b     RHS Property
	 *
	 * @return     Result of doing a < b according to provided semantics.
	 */
	static inline bool less(const PropertyT &a, const PropertyT &b) {
		return PropertyLess()(a, b);
	}

	/**
	 * @brief      Equality comparator for operations. You MUST use this
	 *             instead of directly using anything else like "<"
	 *
	 * @param[in]  a     LHS Property
	 * @param[in]  b     RHS Property
	 *
	 * @return     Result of doing a == b according to provided semantics.
	 */
	static inline bool equal(const PropertyT &a, const PropertyT &b) {
		return PropertyEqual()(a, b);
	}

	/**
	 * @brief      Determines whether the property set at `index` contains the
	 *             element `prop` or not.
	 *
	 * @param[in]  index  The index
	 * @param[in]  prop   The property
	 *
	 * @return     `true` if `prop` is in `index`, false otherwise.
	 */
	inline bool contains(const Index index, const PropertyT prop) const {
		if (index == EMPTY_SET) {
			return false;
		}

		const PropertySet &s = get_value(index);

#ifdef LHF_USE_SORTED_VECTOR_FOR_PROPERTY_SETS
		if (s.size() <= LHF_SORTED_VECTOR_BINARY_SEARCH_THRESHOLD) {
			for (PropertyT i : s) {
				if (equal(i, prop)) {
					return true;
				}
			}
		} else {
			// Binary search implementation
			std::size_t low = 0;
			std::size_t high = s.size() - 1;

			while (low <= high) {
				std::size_t mid = low + (high - low) / 2;

				if (equal(s[mid], prop)) {
					return true;
				} else if (less(s[mid], prop)) {
					low = mid + 1;
				} else {
					high = mid - 1;
				}
			}
		}
#else
		return s.count(prop) > 0;
#endif

		return false;
	}

	/**
	 * @brief      Calculates, or returns a cached result of the union
	 *             of `a` and `b`
	 *
	 * @param[in]  a     The first set
	 * @param[in]  b     The second set
	 *
	 * @return     Index of the new property set.
	 */
	Index set_union(const Index _a, const Index _b) {
		LHF_PROPERTY_SET_PAIR_VALID(_a, _b);
		__lhf_calc_functime();

		if (_a == _b) {
			LHF_PERF_INC(unions, equal_hits);
			return _a;
		}

		if (is_empty(_a)) {
			LHF_PERF_INC(unions, empty_hits);
			return _b;
		} else if (is_empty(_b)) {
			LHF_PERF_INC(unions,empty_hits);
			return _a;
		}

		const Index a = std::min(_a, _b);
		const Index b = std::max(_a, _b);

		SubsetRelation r = is_subset(a, b);

		if (r == SUBSET) {
			LHF_PERF_INC(unions, subset_hits);
			return b;
		} else if (r == SUPERSET) {
			LHF_PERF_INC(unions, subset_hits);
			return a;
		}

		auto cursor = unions.find({a, b});

		if (cursor == unions.end()) {
			PropertySet new_set;
			const PropertySet &first = get_value(a);
			const PropertySet &second = get_value(b);

#ifdef LHF_USE_ORDERED_CONTAINER_FOR_PROPERTY_SETS
			// The union implementation here is adopted from the example
			// suggested implementation provided of std::set_union from
			// cppreference.com
			auto cursor_1 = first.begin();
			const auto &cursor_end_1 = first.end();
			auto cursor_2 = second.begin();
			const auto &cursor_end_2 = second.end();

			while (cursor_1 != cursor_end_1) {
				if (cursor_2 == cursor_end_2) {
					LHF_PUSH_RANGE(new_set, cursor_1, cursor_end_1);
					break;
				}

				if (less(*cursor_2, *cursor_1)) {
					LHF_PUSH_ONE(new_set, *cursor_2);
					cursor_2++;
				} else {
					LHF_PUSH_ONE(new_set, *cursor_1);
					if (!(less(*cursor_1, *cursor_2)))
						cursor_2++;
					cursor_1++;
				}
			}
			LHF_PUSH_RANGE(new_set, cursor_2, cursor_end_2);
#else
			for (auto &v : first) {
				new_set.insert(v);
			}

			for (auto &v : second) {
				new_set.insert(v);
			}
#endif
			bool cold = false;
			Index ret = register_set(new_set, cold);

			unions.insert({{a, b}, ret});


			if (ret == a) {
				store_subset(b, ret);
			} else if (ret == b) {
				store_subset(a, ret);
			} else {
				store_subset(a, ret);
				store_subset(b, ret);
			}

			if (cold) {
				LHF_PERF_INC(unions, cold_misses);
			} else {
				LHF_PERF_INC(unions, edge_misses);
			}
			return ret;
		}

		LHF_PERF_INC(unions, hits);
		return cursor->second;
	}

	/**
	 * @brief      Inserts a single element from a given set (and returns the
	 *             index of the set). This is a wrapper over the union
	 *             operation.
	 *
	 * @param[in]  a     The set to insert the element to
	 * @param[in]  b     The element to be inserted.
	 *
	 * @return     Index of the new PropertySet.
	 */
	Index set_insert_single(const Index a, const PropertyT &b) {
		return set_union(a, register_set_single(b));
	}

	/**
	 * @brief      Removes a single element from a given set (and returns the
	 *             index of the set). This is a wrapper over the diffrerence
	 *             operation.
	 *
	 * @param[in]  a     The set to remove the element from
	 * @param[in]  b     The element to be removed
	 *
	 * @return     Index of the new PropertySet.
	 */
	Index set_remove_single(const Index a, const PropertyT &b) {
		return set_difference(a, register_set_single(b));
	}

	/**
	 * @brief      Calculates, or returns a cached result of the difference
	 *             of `a` from `b`
	 *
	 * @param[in]  a     The first set (what to subtract from)
	 * @param[in]  b     The second set (what will be subtracted)
	 *
	 * @return     Index of the new PropertySet.
	 */
	Index set_difference(const Index a, const Index b) {
		LHF_PROPERTY_SET_PAIR_VALID(a, b);
		__lhf_calc_functime();

		if (a == b) {
			LHF_PERF_INC(differences, equal_hits);
			return EMPTY_SET;
		}

		if (is_empty(a)) {
			LHF_PERF_INC(differences, empty_hits);
			return EMPTY_SET;
		} else if (is_empty(b)) {
			LHF_PERF_INC(differences, empty_hits);
			return a;
		}

		auto cursor = differences.find({a, b});

		if (cursor == differences.end()) {
			PropertySet new_set;
			const PropertySet &first = get_value(a);
			const PropertySet &second = get_value(b);

#ifdef LHF_USE_ORDERED_CONTAINER_FOR_PROPERTY_SETS
			// The difference implementation here is adopted from the example
			// suggested implementation provided of std::set_difference from
			// cppreference.com
			auto cursor_1 = first.begin();
			const auto &cursor_end_1 = first.end();
			auto cursor_2 = second.begin();
			const auto &cursor_end_2 = second.end();

			while (cursor_1 != cursor_end_1) {
				if (cursor_2 == cursor_end_2) {
					LHF_PUSH_RANGE(new_set, cursor_1, cursor_end_1);
					break;
				}

				if (less(*cursor_1, *cursor_2)) {
					LHF_PUSH_ONE(new_set, *cursor_1);
					cursor_1++;
				} else {
					if (!(less(*cursor_2, *cursor_1)))
						cursor_1++;
					cursor_2++;
				}
			}
#else
			for (auto &v : first) {
				new_set.insert(v);
			}

			for (auto &v : second) {
				new_set.erase(v);
			}
#endif

			bool cold = false;
			Index ret = register_set(new_set, cold);
			differences.insert({{a, b}, ret});

			if (ret != a) {
				store_subset(ret, a);
			} else {
				intersections.insert({{ std::min(a, b) , std::max(a, b) }, EMPTY_SET});
			}

			if (cold) {
				LHF_PERF_INC(differences, cold_misses);
			} else {
				LHF_PERF_INC(differences, edge_misses);
			}

			return ret;
		}

		LHF_PERF_INC(differences, hits);
		return cursor->second;
	}

	/**
	 * @brief      Calculates, or returns a cached result of the intersection
	 *             of `a` and `b`
	 *
	 * @param[in]  a     The first set
	 * @param[in]  b     The second set
	 *
	 * @return     Index of the new property set.
	 */
	Index set_intersection(const Index _a, const Index _b) {
		LHF_PROPERTY_SET_PAIR_VALID(_a, _b);
		__lhf_calc_functime();

		if (_a == _b) {
			LHF_PERF_INC(intersections, equal_hits);
			return _a;
		}

		if (is_empty(_a) || is_empty(_b)) {
			LHF_PERF_INC(intersections, empty_hits);
			return EMPTY_SET;
		}

		const Index a = std::min(_a, _b);
		const Index b = std::max(_a, _b);

		SubsetRelation r = is_subset(a, b);

		if (r == SUBSET) {
			LHF_PERF_INC(intersections, subset_hits);
			return a;
		} else if (r == SUPERSET) {
			LHF_PERF_INC(intersections, subset_hits);
			return b;
		}

		auto cursor = intersections.find({a, b});

		if (cursor == intersections.end()) {
			PropertySet new_set;
			const PropertySet &first = get_value(a);
			const PropertySet &second = get_value(b);

#ifdef LHF_USE_ORDERED_CONTAINER_FOR_PROPERTY_SETS
			// The intersection implementation here is adopted from the example
			// suggested implementation provided for std::set_intersection from
			// cppreference.com
			auto cursor_1 = first.begin();
			const auto &cursor_end_1 = first.end();
			auto cursor_2 = second.begin();
			const auto &cursor_end_2 = second.end();
#else
			Vector<PropertyT> list_first(first.begin(), first.end());
			std::sort(list_first.begin(), list_first.end());
			Vector<PropertyT> list_second(second.begin(), second.end());
			std::sort(list_second.begin(), list_second.end());

			auto cursor_1 = list_first.begin();
			const auto &cursor_end_1 = list_first.end();
			auto cursor_2 = list_second.begin();
			const auto &cursor_end_2 = list_second.end();
#endif

			while (cursor_1 != cursor_end_1 && cursor_2 != cursor_end_2)
			{
				if (less(*cursor_1,*cursor_2)) {
					cursor_1++;
				} else {
					if (!(less(*cursor_2, *cursor_1))) {
						LHF_PUSH_ONE(new_set, *cursor_1);
						cursor_1++;
					}
					cursor_2++;
				}
			}

			bool cold = false;
			Index ret = register_set(new_set, cold);
			intersections.insert({{a, b}, ret});

			if (ret != a) {
				store_subset(ret, a);
			} else if (ret != b) {
				store_subset(ret, b);
			} else {
				store_subset(ret, a);
				store_subset(ret, b);
			}

			if (cold) {
				LHF_PERF_INC(intersections, cold_misses);
			} else {
				LHF_PERF_INC(intersections, edge_misses);
			}
			return ret;
		}

		LHF_PERF_INC(intersections, hits);
		return cursor->second;
	}


	/**
	 * @brief      Filters a set based on a criterion function.
	 *             This is supposed to be an abstract filtering mechanism that
	 *             derived classes will use to implement caching on a filter
	 *             operation rather than letting them implement their own.
	 *
	 * @param[in]  s            The set to filter
	 * @param[in]  filter_func  The filter function (can be a lambda)
	 * @param      cache        The cache to use (possibly defined by the user)
	 *
	 * @tparam     is_sort_bounded  Useful for telling the function that the
	 *                              filter criterion will have a lower and an
	 *                              upper bound in a sorted list. This can
	 *                              potentially result in a faster filtering.
	 *
	 * @todo Implement sort bound optimization
	 *
	 * @return     Index of the filtered set.
	 */
	template <typename Func, bool is_sort_bounded = false>
	Index set_filter(
		Index s,
		std::function<bool(const PropertyT &)> filter_func,
		HashMap<Index, Index> &cache) {
		LHF_PROPERTY_SET_INDEX_VALID(s);
		__lhf_calc_functime();

		if (is_empty(s)) {
			return s;
		}

		auto cursor = cache.find(s);

		if (cursor == cache.end()) {
			PropertySet new_set;
			for (PropertyT value : get_value(s)) {
				if (filter_func(value)) {
					LHF_PUSH_ONE(new_set, value);
				}
			}

			bool cold;
			Index new_index = register_set(std::move(new_set), cold);

			cache.insert({s, new_index});

			if (cold) {
				LHF_PERF_INC(filter, cold_misses);
			} else {
				LHF_PERF_INC(filter, edge_misses);
			}

			return new_index;
		}

		LHF_PERF_INC(filter, hits);
		return cursor->second;
	}

	String dump() {
		std::stringstream s;
		s << "LatticeHashForest {\n";

		s << "    " << "Unions: " << "(Count: " << unions.size() << ")\n";
		for (auto i : unions) {
			s << "      {" << i.first << " -> " << i.second << "} \n";
		}

		s << "\n";
		s << "    " << "Differences:" << "(Count: " << differences.size() << ")\n";
		for (auto i : differences) {
			s << "      {" << i.first << " -> " << i.second << "} \n";
		}

		s << "\n";
		s << "    " << "Intersections: " << "(Count: " << intersections.size() << ")\n";
		for (auto i : intersections) {
			s << "      {" << i.first << " -> " << i.second << "} \n";
		}

		s << "\n";
		s << "    " << "Subsets: " << "(Count: " << subsets.size() << ")\n";
		for (auto i : subsets) {
			s << "      " << i.first << " -> " << (i.second == SUBSET ? "sub" : "sup") << "\n";
		}

		s << "\n";
		s << "    " << "PropertySets: " << "(Count: " << property_set_map.size() << ")\n";
		for (auto i : property_set_map) {
			s << "      " << i.second << " : " << ptr_container_to_string(property_sets[i.second]) << "\n";
		}
		s << "}\n";
		return s.str();
	}

#ifdef LHF_ENABLE_PERFORMANCE_METRICS
	String dump_perf() {
		std::stringstream s;
		s << "LHF Perf: \n";
		for (auto &p : perf) {
			s << p.first << "\n"
			  << p.second.to_string() << "\n";
		}
		s << __stat.dump();
		return s.str();
	}
#endif


	LatticeHashForest() {
		// INSERT EMPTY SET AT INDEX 0
		register_set({ });
	}
};

};

#endif