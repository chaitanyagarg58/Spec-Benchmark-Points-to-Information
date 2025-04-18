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
            {"LBM_allocateGrid", 0},
            {"LBM_freeGrid", 1},
            {"LBM_initializeGrid", 2},
            {"LBM_swapGrids", 3},
            {"LBM_loadObstacleFile", 4},
            {"LBM_initializeSpecialCellsForLDC", 5},
            {"LBM_initializeSpecialCellsForChannel", 6},
            {"LBM_performStreamCollide", 7},
            {"LBM_handleInOutFlow", 8},
            {"LBM_showGridStatistics", 9},
            {"LBM_storeVelocityField", 10},
            {"storeValue", 11},
            {"LBM_compareVelocityField", 12},
            {"loadValue", 13},
            {"main", 14},
            {"MAIN_parseCommandLine", 15},
            {"MAIN_printInfo", 16},
            {"MAIN_initialize", 17},
            {"MAIN_finalize", 18},
            {"llvm.dbg.declare", 19},
            {"malloc", 20},
            {"printf", 21},
            {"exit", 22},
            {"free", 23},
            {"fopen", 24},
            {"fgetc", 25},
            {"fclose", 26},
            {"llvm.fmuladd.f64", 27},
            {"sqrt", 28},
            {"fprintf", 29},
            {"fwrite", 30},
            {"__isoc99_fscanf", 31},
            {"llvm.fmuladd.f32", 32},
            {"fread", 33},
            {"atoi", 34},
            {"stat", 35},
            {"llvm.dbg.value", 36},
            {"llvm.memcpy.p0.p0.i64", 37}
        };

        // Graph Representation: Vector of (caller, callee) pairs
        const std::vector<EdgeT> graph = {
            {0, 20},
            {0, 21},
            {0, 22},
            {1, 23},
            {4, 24},
            {4, 25},
            {4, 26},
            {7, 27},
            {8, 27},
            {9, 27},
            {9, 28},
            {9, 21},
            {10, 24},
            {10, 26},
            {10, 11},
            {10, 29},
            {11, 30},
            {12, 32},
            {12, 13},
            {12, 21},
            {12, 24},
            {12, 26},
            {12, 28},
            {12, 31},
            {13, 33},
            {14, 3},
            {14, 7},
            {14, 8},
            {14, 9},
            {14, 15},
            {14, 16},
            {14, 17},
            {14, 18},
            {14, 21},
            {15, 34},
            {15, 35},
            {15, 21},
            {15, 22},
            {16, 21},
            {16, 37},
            {17, 0},
            {17, 2},
            {17, 4},
            {17, 5},
            {17, 6},
            {17, 9},
            {18, 1},
            {18, 10},
            {18, 12},
            {18, 9}
        };

        const unsigned int graph_size = 50;
    
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
            {{"srcGrid", "main.c", "21"}, 13},
            {{"srcGrid", "main.c", "21"}, 14},
            {{"dstGrid", "main.c", "21"}, 16},
            {{"ptr", "lbm.c", "25"}, 29},
            {{"call", "lbm.c", "29"}, 34},
            {{"i", "lbm.c", "29"}, 38},
            {{"i1", "lbm.c", "30"}, 40},
            {{"i2", "lbm.c", "39"}, 52},
            {{"add.ptr", "lbm.c", "39"}, 53},
            {{"ptr", "lbm.c", "44"}, 73},
            {{"i", "lbm.c", "47"}, 76},
            {{"add.ptr", "lbm.c", "47"}, 77},
            {{"i1", "lbm.c", "47"}, 79},
            {{"grid1", "lbm.c", "89"}, 216},
            {{"grid2", "lbm.c", "89"}, 217},
            {{"ux", "lbm.c", "557"}, 2953},
            {{"ux", "lbm.c", "557"}, 2954},
            {{"uy", "lbm.c", "557"}, 2955},
            {{"uy", "lbm.c", "557"}, 2956},
            {{"uz", "lbm.c", "557"}, 2957},
            {{"uz", "lbm.c", "557"}, 2958},
            {{"call", "lbm.c", "559"}, 2969},
            {{"file", "lbm.c", "516"}, 3581},
            {{"v", "lbm.c", "516"}, 3582},
            {{"litteBigEndianTest", "lbm.c", "517"}, 3584},
            {{"buffer", "lbm.c", "520"}, 3586},
            {{"i", "lbm.c", "518"}, 3591},
            {{"i2", "lbm.c", "519"}, 3596},
            {{"arrayidx", "lbm.c", "524"}, 3613},
            {{"arrayidx7", "lbm.c", "524"}, 3616},
            {{"arraydecay", "lbm.c", "526"}, 3623},
            {{"i4", "lbm.c", "529"}, 3629},
            {{"fileUx", "lbm.c", "618"}, 3648},
            {{"fileUx", "lbm.c", "618"}, 3649},
            {{"fileUy", "lbm.c", "618"}, 3650},
            {{"fileUy", "lbm.c", "618"}, 3651},
            {{"fileUz", "lbm.c", "618"}, 3652},
            {{"fileUz", "lbm.c", "618"}, 3653},
            {{"call", "lbm.c", "622"}, 3665},
            {{"file", "lbm.c", "535"}, 4302},
            {{"v", "lbm.c", "535"}, 4303},
            {{"litteBigEndianTest", "lbm.c", "536"}, 4305},
            {{"buffer", "lbm.c", "539"}, 4307},
            {{"i", "lbm.c", "537"}, 4312},
            {{"i2", "lbm.c", "538"}, 4317},
            {{"arraydecay", "lbm.c", "542"}, 4320},
            {{"arrayidx", "lbm.c", "545"}, 4336},
            {{"arrayidx7", "lbm.c", "545"}, 4339},
            {{"i4", "lbm.c", "548"}, 4347},
            {{"param", "main.c", "26"}, 4370},
            {{"nTimeSteps", "main.c", "39"}, 4389},
            {{"simType", "main.c", "40"}, 4393},
            {{"param", "main.c", "63"}, 4429},
            {{"fileStat", "main.c", "64"}, 4431},
            {{"nTimeSteps", "main.c", "71"}, 4449},
            {{"resultFilename", "main.c", "72"}, 4454},
            {{"action", "main.c", "73"}, 4460},
            {{"simType", "main.c", "74"}, 4465},
            {{"obstacleFilename", "main.c", "77"}, 4472},
            {{"obstacleFilename11", "main.c", "79"}, 4474},
            {{"obstacleFilename15", "main.c", "81"}, 4481},
            {{"st_size", "main.c", "84"}, 4486},
            {{"obstacleFilename20", "main.c", "88"}, 4491},
            {{"st_size21", "main.c", "88"}, 4493},
            {{"obstacleFilename24", "main.c", "93"}, 4502},
            {{"action26", "main.c", "95"}, 4505},
            {{"resultFilename29", "main.c", "96"}, 4509},
            {{"resultFilename34", "main.c", "98"}, 4514},
            {{"param", "main.c", "105"}, 4522},
            {{"actionString", "main.c", "106"}, 4524},
            {{"simTypeString", "main.c", "107"}, 4526},
            {{"i", "main.c", "106"}, 4529},
            {{"i1", "main.c", "107"}, 4536},
            {{"nTimeSteps", "main.c", "116"}, 4538},
            {{"resultFilename", "main.c", "116"}, 4540},
            {{"action", "main.c", "117"}, 4542},
            {{"arrayidx", "main.c", "117"}, 4545},
            {{"arraydecay", "main.c", "117"}, 4546},
            {{"simType", "main.c", "117"}, 4547},
            {{"arrayidx2", "main.c", "117"}, 4550},
            {{"arraydecay3", "main.c", "117"}, 4551},
            {{"obstacleFilename", "main.c", "118"}, 4552},
            {{"obstacleFilename4", "main.c", "119"}, 4558},
            {{"param", "main.c", "124"}, 4566},
            {{"obstacleFilename", "main.c", "131"}, 4576},
            {{"obstacleFilename3", "main.c", "132"}, 4582},
            {{"obstacleFilename5", "main.c", "133"}, 4587},
            {{"simType", "main.c", "136"}, 4592},
            {{"param", "main.c", "150"}, 4616},
            {{"action", "main.c", "153"}, 4621},
            {{"resultFilename", "main.c", "154"}, 4627},
            {{"action2", "main.c", "155"}, 4633},
            {{"resultFilename6", "main.c", "156"}, 4639},
            {{"i", "main.c", "106"}, 4663},
            {{"i1", "main.c", "107"}, 4666}
        };
        
        // Graph Representation: Vector of (pointer, pointee) pairs
        const std::vector<EdgeT> graph = {
            {13, 16},
            {13, 14},
            {14, 34},
            {16, 34},
            {29, 16},
            {29, 14},
            {38, 34},
            {40, 34},
            {52, 34},
            {53, 34},
            {73, 16},
            {73, 14},
            {76, 34},
            {77, 34},
            {79, 34},
            {216, 16},
            {216, 14},
            {217, 16},
            {2953, 2954},
            {2953, 2956},
            {2953, 2958},
            {2955, 2954},
            {2955, 2956},
            {2955, 2958},
            {2957, 2954},
            {2957, 2956},
            {2957, 2958},
            {3581, 2969},
            {3582, 2954},
            {3582, 2956},
            {3582, 2958},
            {3591, 3584},
            {3596, 2954},
            {3596, 2956},
            {3596, 2958},
            {3613, 2954},
            {3613, 2956},
            {3613, 2958},
            {3616, 3586},
            {3623, 3586},
            {3629, 2954},
            {3629, 2956},
            {3629, 2958},
            {3648, 3649},
            {3648, 3651},
            {3648, 3653},
            {3650, 3649},
            {3650, 3651},
            {3650, 3653},
            {3652, 3649},
            {3652, 3651},
            {3652, 3653},
            {4302, 3665},
            {4303, 3649},
            {4303, 3651},
            {4303, 3653},
            {4312, 4305},
            {4317, 3649},
            {4317, 3651},
            {4317, 3653},
            {4320, 4307},
            {4336, 4307},
            {4339, 3649},
            {4339, 3651},
            {4339, 3653},
            {4347, 3649},
            {4347, 3651},
            {4347, 3653},
            {4389, 4370},
            {4393, 4370},
            {4429, 4370},
            {4449, 4370},
            {4454, 4370},
            {4460, 4370},
            {4465, 4370},
            {4472, 4370},
            {4474, 4370},
            {4481, 4370},
            {4486, 4431},
            {4491, 4370},
            {4493, 4431},
            {4502, 4370},
            {4505, 4370},
            {4509, 4370},
            {4514, 4370},
            {4522, 4370},
            {4529, 4524},
            {4536, 4526},
            {4538, 4370},
            {4540, 4370},
            {4542, 4370},
            {4545, 4524},
            {4546, 4524},
            {4547, 4370},
            {4550, 4526},
            {4551, 4526},
            {4552, 4370},
            {4558, 4370},
            {4566, 4370},
            {4576, 4370},
            {4582, 4370},
            {4587, 4370},
            {4592, 4370},
            {4616, 4370},
            {4621, 4370},
            {4627, 4370},
            {4633, 4370},
            {4639, 4370},
            {4663, 4524},
            {4666, 4526}
        };

        const unsigned int graph_size = 110;
    
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
