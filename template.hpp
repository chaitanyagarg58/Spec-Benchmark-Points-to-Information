#ifndef PTA_GRAPHS_HPP
#define PTA_GRAPHS_HPP

#include "lhf/lhf.hpp"
#include <map>
#include <vector>
#include <string>

namespace PTA{

	using FuncDataT = std::string;

	struct PtrDataT {
		std::string var;
		std::string scope;
		std::string line;

		bool operator<(const PtrDataT& rhs) const {
			return std::tie(var, scope, line) < std::tie(rhs.var, rhs.scope, rhs.line);
		}
	};
	using NodeIdT = unsigned int;
	using EdgeT = std::pair<NodeIdT, NodeIdT>;

	enum IndexType {
		NODE_FOREST = 1,
		EDGE_FOREST = 2,
	};


	template<typename NodeDataT, typename DerivedGraphT>
	class BaseGraphAPI {
	protected:
		using NodePropertyT = unsigned int;
		using EdgePropertyT = long long;

		using NodeHashForest = lhf::LatticeHashForest<NodePropertyT>;
		using EdgeHashForest = lhf::LatticeHashForest<EdgePropertyT>;
	
		EdgeHashForest edgeForest;
		NodeHashForest nodeForest;
	
		inline const std::vector<EdgeT>& getGraph() const {
			return static_cast<const DerivedGraphT*>(this)->getGraph();
		}

		inline const std::map<NodeDataT, NodeIdT>& getNodeMap() const {
			return static_cast<const DerivedGraphT*>(this)->getNodeMap();
		}
	
		EdgePropertyT getEdgeProperty(const EdgeT& edge) const {
			const std::vector<EdgeT>& graph = getGraph();
			if (std::find(graph.begin(), graph.end(), edge) == graph.end()) {
				throw std::out_of_range("Invalid Edge");
			}
			EdgePropertyT property = (static_cast<EdgePropertyT>(edge.first) << 32) | static_cast<EdgePropertyT>(edge.second);
			return property;
		}
	
		EdgeT getEdge(const EdgePropertyT& edge_property) const {
			NodeIdT src = static_cast<NodeIdT>(edge_property >> 32);
			NodeIdT dst = static_cast<NodeIdT>(edge_property & 0xFFFFFFFF);
			return {src, dst};
		}
	
	public:
		inline NodeIdT getNodeId(const NodeDataT& node) const {
			auto it = getNodeMap().find(node);
			return (it != getNodeMap().end()) ? it->second : -1;
		}

		inline const NodeDataT& getNodeDetails(NodeIdT node_id) const {
			for (const auto& [node, nodeId] : getNodeMap()) {
				if (nodeId == node_id) {
					return node;
				}
			}
			throw std::out_of_range("NodeID not found in node_map");
		}

		inline lhf::Index getEmptySetIndex() const {
			return lhf::EMPTY_SET;
		}
	
		inline bool is_empty(lhf::Index i) {
			return edgeForest.is_empty(i);
		}
	
		inline bool is_subset(IndexType indexType, lhf::Index a, lhf::Index b) {
			if (indexType == EDGE_FOREST) {
				return edgeForest.is_subset(a, b) == EdgeHashForest::SubsetRelation::SUBSET;
			} else if (indexType == NODE_FOREST) {
				return nodeForest.is_subset(a, b) == NodeHashForest::SubsetRelation::SUBSET;
			}
			throw std::invalid_argument("Invalid index type");
		}

		inline bool is_superset(IndexType indexType, lhf::Index a, lhf::Index b) {
			if (indexType == EDGE_FOREST) {
				return edgeForest.is_subset(a, b) == EdgeHashForest::SubsetRelation::SUPERSET;
			} else if (indexType == NODE_FOREST) {
				return nodeForest.is_subset(a, b) == NodeHashForest::SubsetRelation::SUPERSET;
			}
			throw std::invalid_argument("Invalid index type");
		}
	
		inline lhf::Index getEdgeIndex(const EdgeT& edge) {
			return edgeForest.register_set_single(getEdgeProperty(edge));
		}
	
		inline lhf::Index getEdgeIndex(const EdgeT& edge, bool &cold) {
			return edgeForest.register_set_single(getEdgeProperty(edge), cold);
		}
	
		inline const EdgeHashForest::PropertySet &get_value_edge(lhf::Index idx) const {
			return edgeForest.get_value(idx);
		}

		inline const NodeHashForest::PropertySet &get_value_node(lhf::Index idx) const {
			return nodeForest.get_value(idx);
		}
	
		inline std::size_t size_of(IndexType indexType, lhf::Index idx) const {
			if (indexType == EDGE_FOREST) {
				return edgeForest.size_of(idx);
			} else if (indexType == NODE_FOREST) {
				return nodeForest.size_of(idx);
			}
			throw std::invalid_argument("Invalid index type");
		}
	
		inline bool contains(lhf::Index idx, const EdgeT& edge) const {
			return edgeForest.contains(idx, getEdgeProperty(edge));
		}

		inline bool contains(lhf::Index idx, const NodeIdT& node) const {
			return nodeForest.contains(idx, node);
		}
	
		inline lhf::Index set_union(IndexType indexType, lhf::Index a, lhf::Index b) {
			if (indexType == EDGE_FOREST) {
				return edgeForest.set_union(a, b);
			} else if (indexType == NODE_FOREST) {
				return nodeForest.set_union(a, b);
			}
			throw std::invalid_argument("Invalid index type");
		}
	
		inline lhf::Index set_insert_single(lhf::Index a, const EdgeT& edge) {
			return edgeForest.set_insert_single(a, getEdgeProperty(edge));
		}

		inline lhf::Index set_insert_single(lhf::Index a, const NodeIdT& node) {
			return nodeForest.set_insert_single(a, node);
		}
	
		inline lhf::Index set_remove_single(lhf::Index a, const EdgeT& edge) {
			return edgeForest.set_remove_single(a, getEdgeProperty(edge));
		}

		inline lhf::Index set_remove_single(lhf::Index a, const NodeIdT& node) {
			return nodeForest.set_remove_single(a, node);
		}
	
		inline lhf::Index set_difference(IndexType indexType, lhf::Index a, lhf::Index b) {
			if (indexType == EDGE_FOREST) {
				return edgeForest.set_difference(a, b);
			} else if (indexType == NODE_FOREST) {
				return nodeForest.set_difference(a, b);
			}
			throw std::invalid_argument("Invalid index type");
		}
	
		inline lhf::Index set_intersection(IndexType indexType, lhf::Index a, lhf::Index b) {
			if (indexType == EDGE_FOREST) {
				return edgeForest.set_intersection(a, b);
			} else if (indexType == NODE_FOREST) {
				return nodeForest.set_intersection(a, b);
			}
			throw std::invalid_argument("Invalid index type");
		}
	
		inline const NodeHashForest::PropertySet& get_value_points_to(lhf::Index idx) const {
			return nodeForest.get_value(idx);
		}
	
		lhf::Index get_points_to_set(lhf::Index a, NodeIdT node_id) {
			EdgeHashForest::PropertySet property_set = edgeForest.get_value(a);
			std::vector<NodeIdT> result;
			for (EdgePropertyT property : property_set) {
				EdgeT edge = getEdge(property);
				if (edge.first == node_id) {
					result.push_back(edge.second);
				}
			}
			return nodeForest.register_set(result);
		}
	
		lhf::Index get_points_to_set(lhf::Index a, const std::vector<NodeIdT>& node_ids) {
			lhf::Index result = getEmptySetIndex();
			for (NodeIdT node_id : node_ids) {
				lhf::Index index = get_points_to_set(a, node_id);
				result = nodeForest.set_union(result, index);
			}
			return result;
		}
	
		lhf::Index get_points_to_set(lhf::Index a, lhf::Index idx) {
			return get_points_to_set(a, get_value_points_to(idx));
		}
	
		lhf::Index get_points_to_set(lhf::Index a, NodeIdT node_id, unsigned int recursion_depth) {
			if (recursion_depth == 0) return {};
			lhf::Index result = get_points_to_set(a, node_id);
			for (int i = 1; i < recursion_depth; i++) {
				result = get_points_to_set(a, result);
				if (nodeForest.is_empty(result)) break;
			}
			return result;
		}
	
		lhf::Index get_points_to_set(lhf::Index a, const std::vector<NodeIdT>& node_ids, unsigned int recursion_depth) {
			if (recursion_depth == 0) return {};
			lhf::Index result = get_points_to_set(a, node_ids);
			for (int i = 1; i < recursion_depth; i++) {
				result = get_points_to_set(a, result);
				if (nodeForest.is_empty(result)) break;
			}
			return result;
		}
	
		void print_points_to_set(lhf::Index idx) const {
			const auto& set = nodeForest.get_value(idx);
			std::cout << "Points-to set (Index = " << idx << "): (";
			for (const auto& v : set) std::cout << v << ", ";
			std::cout << ")" << std::endl;
		}
	};


	class CallGraph : public BaseGraphAPI<FuncDataT, CallGraph> {
	private:
		// Mapping from NodeData to ID
		const std::map<FuncDataT, NodeIdT> node_map = {
			${CallGraph_nodes}
		};

		// Graph Representation: Vector of (caller, callee) pairs
		const std::vector<EdgeT> graph = {
			${CallGraph_edges}
		};

		const unsigned int graph_size = ${CallGraph_size};
	
	public:
		inline const std::vector<EdgeT>& getGraph() const {
			return graph;
		}

		inline const std::map<FuncDataT, NodeIdT>& getNodeMap() const {
			return node_map;
		}

		inline unsigned int getGraphSize() const {
			return graph_size;
		}
	};


	class PointerGraph : public BaseGraphAPI<PtrDataT, PointerGraph> {
	private:
		// Mapping from NodeData to ID
		const std::map<PtrDataT, NodeIdT> node_map = {
			${PointerGraph_nodes}
		};
		
		// Graph Representation: Vector of (pointer, pointee) pairs
		const std::vector<EdgeT> graph = {
			${PointerGraph_edges}
		};

		const unsigned int graph_size = ${PointerGraph_size};
	
	public:
		inline const std::vector<EdgeT>& getGraph() const {
			return graph;
		}

		inline const std::map<PtrDataT, NodeIdT>& getNodeMap() const {
			return node_map;
		}

		inline unsigned int getGraphSize() const {
			return graph_size;
		}
	};
} // namespace PointerGraph

#endif // PTA_GRAPHS_HPP
