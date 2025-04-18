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
            {"quantum_ipow", 0},
            {"quantum_gcd", 1},
            {"quantum_frac_approx", 2},
            {"quantum_getwidth", 3},
            {"quantum_inverse_mod", 4},
            {"quantum_conj", 5},
            {"quantum_real", 6},
            {"quantum_imag", 7},
            {"quantum_prob", 8},
            {"quantum_prob_inline", 9},
            {"quantum_cexp", 10},
            {"quantum_get_decoherence", 11},
            {"quantum_set_decoherence", 12},
            {"quantum_decohere", 13},
            {"quantum_exp_mod_n", 14},
            {"quantum_cnot", 15},
            {"quantum_toffoli", 16},
            {"quantum_unbounded_toffoli", 17},
            {"quantum_sigma_x", 18},
            {"quantum_sigma_y", 19},
            {"quantum_sigma_z", 20},
            {"quantum_swaptheleads", 21},
            {"quantum_swaptheleads_omuln_controlled", 22},
            {"quantum_gate1", 23},
            {"quantum_add_hash", 24},
            {"quantum_get_state", 25},
            {"quantum_prob_inline.5", 26},
            {"quantum_real.6", 27},
            {"quantum_imag.7", 28},
            {"quantum_hash64", 29},
            {"quantum_gate2", 30},
            {"quantum_hadamard", 31},
            {"quantum_walsh", 32},
            {"quantum_r_x", 33},
            {"quantum_r_y", 34},
            {"quantum_r_z", 35},
            {"quantum_phase_scale", 36},
            {"quantum_phase_kick", 37},
            {"quantum_cond_phase", 38},
            {"quantum_cond_phase_inv", 39},
            {"quantum_cond_phase_kick", 40},
            {"quantum_gate_counter", 41},
            {"quantum_memman", 42},
            {"quantum_new_matrix", 43},
            {"quantum_delete_matrix", 44},
            {"quantum_print_matrix", 45},
            {"quantum_real.17", 46},
            {"quantum_imag.18", 47},
            {"quantum_frand", 48},
            {"quantum_measure", 49},
            {"quantum_prob_inline.23", 50},
            {"quantum_real.24", 51},
            {"quantum_imag.25", 52},
            {"quantum_bmeasure", 53},
            {"quantum_bmeasure_bitpreserve", 54},
            {"test_sum", 55},
            {"muxfa", 56},
            {"muxfa_inv", 57},
            {"muxha", 58},
            {"muxha_inv", 59},
            {"madd", 60},
            {"madd_inv", 61},
            {"addn", 62},
            {"addn_inv", 63},
            {"add_mod_n", 64},
            {"quantum_mu2char", 65},
            {"quantum_int2char", 66},
            {"quantum_double2char", 67},
            {"quantum_char2mu", 68},
            {"quantum_char2int", 69},
            {"quantum_char2double", 70},
            {"quantum_objcode_start", 71},
            {"quantum_objcode_stop", 72},
            {"quantum_objcode_put", 73},
            {"quantum_objcode_write", 74},
            {"quantum_objcode_file", 75},
            {"quantum_objcode_exit", 76},
            {"quantum_objcode_run", 77},
            {"emul", 78},
            {"muln", 79},
            {"muln_inv", 80},
            {"mul_mod_n", 81},
            {"quantum_qec_set_status", 82},
            {"quantum_qec_get_status", 83},
            {"quantum_qec_encode", 84},
            {"quantum_qec_decode", 85},
            {"quantum_qec_counter", 86},
            {"quantum_sigma_x_ft", 87},
            {"quantum_cnot_ft", 88},
            {"quantum_toffoli_ft", 89},
            {"quantum_qft", 90},
            {"quantum_qft_inv", 91},
            {"quantum_matrix2qureg", 92},
            {"quantum_new_qureg", 93},
            {"quantum_qureg2matrix", 94},
            {"quantum_destroy_hash", 95},
            {"quantum_delete_qureg", 96},
            {"quantum_delete_qureg_hashpreserve", 97},
            {"quantum_print_qureg", 98},
            {"quantum_real.52", 99},
            {"quantum_imag.53", 100},
            {"quantum_prob_inline.54", 101},
            {"quantum_print_expn", 102},
            {"quantum_addscratch", 103},
            {"quantum_print_hash", 104},
            {"quantum_kronecker", 105},
            {"quantum_state_collapse", 106},
            {"quantum_dot_product", 107},
            {"quantum_add_hash.61", 108},
            {"quantum_get_state.62", 109},
            {"quantum_hash64.63", 110},
            {"main", 111},
            {"quantum_get_version", 112},
            {"spec_srand", 113},
            {"spec_rand", 114},
            {"llvm.dbg.declare", 115},
            {"llvm.fabs.f32", 116},
            {"llvm.fmuladd.f32", 117},
            {"cos", 118},
            {"sin", 119},
            {"calloc", 120},
            {"printf", 121},
            {"exit", 122},
            {"llvm.fmuladd.f64", 123},
            {"log", 124},
            {"sqrt", 125},
            {"__mulsc3", 126},
            {"free", 127},
            {"malloc", 128},
            {"llvm.va_start", 129},
            {"llvm.va_end", 130},
            {"realloc", 131},
            {"__divsc3", 132},
            {"fprintf", 133},
            {"fopen", 134},
            {"fwrite", 135},
            {"fclose", 136},
            {"perror", 137},
            {"feof", 138},
            {"fgetc", 139},
            {"fread", 140},
            {"getenv", 141},
            {"atexit", 142},
            {"atoi", 143},
            {"llvm.dbg.value", 144},
            {"llvm.memcpy.p0.p0.i64", 145}
        };

        // Graph Representation: Vector of (caller, callee) pairs
        const std::vector<EdgeT> graph = {
            {2, 116},
            {5, 6},
            {5, 7},
            {8, 9},
            {9, 117},
            {9, 6},
            {9, 7},
            {10, 118},
            {10, 119},
            {13, 41},
            {13, 42},
            {13, 10},
            {13, 48},
            {13, 117},
            {13, 120},
            {13, 121},
            {13, 122},
            {13, 123},
            {13, 124},
            {13, 125},
            {13, 126},
            {13, 127},
            {14, 81},
            {14, 18},
            {15, 88},
            {15, 73},
            {15, 83},
            {15, 13},
            {16, 89},
            {16, 73},
            {16, 83},
            {16, 13},
            {17, 128},
            {17, 129},
            {17, 130},
            {17, 42},
            {17, 13},
            {17, 121},
            {17, 122},
            {17, 127},
            {18, 73},
            {18, 83},
            {18, 13},
            {18, 87},
            {19, 73},
            {19, 13},
            {19, 126},
            {20, 73},
            {20, 13},
            {20, 126},
            {21, 73},
            {21, 83},
            {21, 15},
            {22, 16},
            {23, 26},
            {23, 131},
            {23, 42},
            {23, 13},
            {23, 25},
            {23, 24},
            {23, 121},
            {23, 122},
            {23, 120},
            {23, 126},
            {23, 127},
            {24, 29},
            {25, 29},
            {26, 27},
            {26, 28},
            {26, 117},
            {30, 26},
            {30, 131},
            {30, 42},
            {30, 13},
            {30, 24},
            {30, 25},
            {30, 120},
            {30, 121},
            {30, 122},
            {30, 126},
            {30, 127},
            {31, 73},
            {31, 43},
            {31, 44},
            {31, 145},
            {31, 23},
            {31, 125},
            {32, 31},
            {33, 73},
            {33, 43},
            {33, 44},
            {33, 145},
            {33, 119},
            {33, 118},
            {33, 23},
            {34, 73},
            {34, 43},
            {34, 44},
            {34, 145},
            {34, 23},
            {34, 118},
            {34, 119},
            {35, 132},
            {35, 73},
            {35, 10},
            {35, 13},
            {35, 126},
            {36, 73},
            {36, 10},
            {36, 13},
            {36, 126},
            {37, 73},
            {37, 10},
            {37, 13},
            {37, 126},
            {38, 73},
            {38, 10},
            {38, 13},
            {38, 126},
            {39, 10},
            {39, 13},
            {39, 126},
            {40, 73},
            {40, 10},
            {40, 13},
            {40, 126},
            {43, 120},
            {43, 121},
            {43, 122},
            {43, 42},
            {44, 42},
            {44, 127},
            {45, 121},
            {45, 46},
            {45, 47},
            {48, 114},
            {49, 48},
            {49, 73},
            {49, 50},
            {50, 51},
            {50, 52},
            {50, 117},
            {53, 97},
            {53, 73},
            {53, 106},
            {53, 48},
            {53, 145},
            {53, 50},
            {54, 97},
            {54, 73},
            {54, 42},
            {54, 48},
            {54, 145},
            {54, 50},
            {54, 120},
            {54, 121},
            {54, 122},
            {54, 125},
            {54, 126},
            {55, 16},
            {55, 18},
            {55, 15},
            {56, 16},
            {56, 18},
            {56, 15},
            {57, 16},
            {57, 18},
            {57, 15},
            {58, 16},
            {58, 18},
            {58, 15},
            {59, 16},
            {59, 18},
            {59, 15},
            {60, 56},
            {60, 58},
            {61, 57},
            {61, 59},
            {62, 60},
            {62, 55},
            {63, 21},
            {63, 15},
            {63, 61},
            {63, 55},
            {64, 62},
            {64, 63},
            {71, 128},
            {71, 121},
            {71, 42},
            {71, 122},
            {72, 42},
            {72, 127},
            {73, 65},
            {73, 66},
            {73, 67},
            {73, 129},
            {73, 131},
            {73, 42},
            {73, 121},
            {73, 122},
            {74, 136},
            {74, 133},
            {74, 134},
            {74, 135},
            {76, 72},
            {76, 74},
            {77, 133},
            {77, 134},
            {77, 136},
            {77, 137},
            {77, 138},
            {77, 139},
            {77, 140},
            {77, 15},
            {77, 16},
            {77, 145},
            {77, 18},
            {77, 19},
            {77, 20},
            {77, 21},
            {77, 31},
            {77, 33},
            {77, 34},
            {77, 35},
            {77, 36},
            {77, 37},
            {77, 38},
            {77, 40},
            {77, 49},
            {77, 53},
            {77, 54},
            {77, 68},
            {77, 69},
            {77, 70},
            {77, 93},
            {78, 16},
            {79, 16},
            {79, 64},
            {79, 78},
            {80, 64},
            {80, 16},
            {80, 4},
            {80, 78},
            {81, 80},
            {81, 22},
            {81, 79},
            {84, 11},
            {84, 12},
            {84, 15},
            {84, 82},
            {84, 31},
            {85, 11},
            {85, 12},
            {85, 15},
            {85, 82},
            {85, 20},
            {85, 53},
            {85, 31},
            {86, 84},
            {86, 85},
            {87, 18},
            {87, 11},
            {87, 12},
            {87, 86},
            {88, 11},
            {88, 12},
            {88, 86},
            {88, 15},
            {89, 13},
            {89, 86},
            {90, 38},
            {90, 31},
            {91, 31},
            {91, 39},
            {92, 120},
            {92, 121},
            {92, 42},
            {92, 122},
            {93, 71},
            {93, 73},
            {93, 42},
            {93, 75},
            {93, 141},
            {93, 142},
            {93, 120},
            {93, 121},
            {93, 122},
            {94, 145},
            {94, 43},
            {95, 42},
            {95, 127},
            {96, 42},
            {96, 127},
            {96, 95},
            {97, 42},
            {97, 127},
            {98, 121},
            {98, 99},
            {98, 100},
            {98, 101},
            {101, 99},
            {101, 100},
            {101, 117},
            {102, 121},
            {104, 121},
            {105, 42},
            {105, 120},
            {105, 121},
            {105, 122},
            {105, 126},
            {106, 101},
            {106, 42},
            {106, 120},
            {106, 121},
            {106, 122},
            {106, 125},
            {106, 126},
            {107, 5},
            {107, 108},
            {107, 109},
            {107, 126},
            {108, 110},
            {109, 110},
            {111, 0},
            {111, 1},
            {111, 2},
            {111, 3},
            {111, 14},
            {111, 15},
            {111, 143},
            {111, 145},
            {111, 31},
            {111, 49},
            {111, 53},
            {111, 90},
            {111, 93},
            {111, 96},
            {111, 103},
            {111, 113},
            {111, 114},
            {111, 121},
            {111, 122}
        };

        const unsigned int graph_size = 342;
    
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
            
        };
        
        // Graph Representation: Vector of (pointer, pointee) pairs
        const std::vector<EdgeT> graph = {
            
        };

        const unsigned int graph_size = 0;
    
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
