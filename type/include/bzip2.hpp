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
            {"ran", 0},
            {"spec_init", 1},
            {"spec_random_load", 2},
            {"spec_load", 3},
            {"spec_read", 4},
            {"spec_fread", 5},
            {"spec_getc", 6},
            {"spec_ungetc", 7},
            {"spec_rewind", 8},
            {"spec_reset", 9},
            {"spec_write", 10},
            {"spec_fwrite", 11},
            {"spec_putc", 12},
            {"main", 13},
            {"debug_time", 14},
            {"spec_initbufs", 15},
            {"spec_compress", 16},
            {"spec_uncompress", 17},
            {"BZ2_blockSort", 18},
            {"fallbackSort", 19},
            {"mainSort", 20},
            {"mainQSort3", 21},
            {"mainSimpleSort", 22},
            {"mmed3", 23},
            {"mainGtU", 24},
            {"fallbackQSort3", 25},
            {"fallbackSimpleSort", 26},
            {"compressStream", 27},
            {"myfeof", 28},
            {"uInt64_from_UInt32s", 29},
            {"uInt64_to_double", 30},
            {"uInt64_toAscii", 31},
            {"configError", 32},
            {"outOfMemory", 33},
            {"ioError", 34},
            {"panic", 35},
            {"showFileNames", 36},
            {"cleanUpAndFail", 37},
            {"setExit", 38},
            {"uInt64_qrm10", 39},
            {"uInt64_isZero", 40},
            {"uncompressStream", 41},
            {"crcError", 42},
            {"compressedStreamEOF", 43},
            {"cadvise", 44},
            {"BZ2_bz__AssertH__fail", 45},
            {"BZ2_bzlibVersion", 46},
            {"BZ2_bzCompressInit", 47},
            {"bz_config_ok", 48},
            {"default_bzalloc", 49},
            {"default_bzfree", 50},
            {"init_RL", 51},
            {"prepare_new_block", 52},
            {"BZ2_bzCompress", 53},
            {"handle_compress", 54},
            {"isempty_RL", 55},
            {"copy_output_until_stop", 56},
            {"copy_input_until_stop", 57},
            {"flush_RL", 58},
            {"add_pair_to_block", 59},
            {"BZ2_bzCompressEnd", 60},
            {"BZ2_bzDecompressInit", 61},
            {"BZ2_indexIntoF", 62},
            {"BZ2_bzDecompress", 63},
            {"unRLE_obuf_to_output_SMALL", 64},
            {"unRLE_obuf_to_output_FAST", 65},
            {"BZ2_bzDecompressEnd", 66},
            {"BZ2_bzWriteOpen", 67},
            {"BZ2_bzWrite", 68},
            {"BZ2_bzWriteClose", 69},
            {"BZ2_bzWriteClose64", 70},
            {"BZ2_bzReadOpen", 71},
            {"BZ2_bzReadClose", 72},
            {"BZ2_bzRead", 73},
            {"myfeof.93", 74},
            {"BZ2_bzReadGetUnused", 75},
            {"BZ2_bzBuffToBuffCompress", 76},
            {"BZ2_bzBuffToBuffDecompress", 77},
            {"BZ2_bzopen", 78},
            {"bzopen_or_bzdopen", 79},
            {"BZ2_bzdopen", 80},
            {"BZ2_bzread", 81},
            {"BZ2_bzwrite", 82},
            {"BZ2_bzflush", 83},
            {"BZ2_bzclose", 84},
            {"BZ2_bzerror", 85},
            {"BZ2_bsInitWrite", 86},
            {"BZ2_compressBlock", 87},
            {"bsPutUChar", 88},
            {"bsPutUInt32", 89},
            {"bsW", 90},
            {"generateMTFValues", 91},
            {"sendMTFValues", 92},
            {"bsFinishWrite", 93},
            {"makeMaps_e", 94},
            {"BZ2_decompress", 95},
            {"makeMaps_d", 96},
            {"BZ2_hbMakeCodeLengths", 97},
            {"BZ2_hbAssignCodes", 98},
            {"BZ2_hbCreateDecodeTables", 99},
            {"__ctype_b_loc", 100},
            {"__errno_location", 101},
            {"llvm.dbg.value", 102},
            {"printf", 103},
            {"malloc", 104},
            {"exit", 105},
            {"llvm.dbg.declare", 106},
            {"open", 107},
            {"strerror", 108},
            {"fprintf", 109},
            {"read", 110},
            {"close", 111},
            {"atoi", 112},
            {"llvm.dbg.label", 113},
            {"perror", 114},
            {"llvm.fmuladd.f64", 115},
            {"free", 116},
            {"strcat", 117},
            {"strcmp", 118},
            {"llvm.memset.p0.i64", 119},
            {"llvm.memcpy.p0.p0.i64", 120}
        };

        // Graph Representation: Vector of (caller, callee) pairs
        const std::vector<EdgeT> graph = {
            {1, 104},
            {1, 105},
            {1, 119},
            {1, 103},
            {2, 0},
            {2, 120},
            {2, 103},
            {3, 101},
            {3, 103},
            {3, 105},
            {3, 107},
            {3, 108},
            {3, 109},
            {3, 110},
            {3, 111},
            {3, 120},
            {4, 120},
            {4, 105},
            {4, 109},
            {4, 103},
            {5, 120},
            {5, 105},
            {5, 109},
            {5, 103},
            {6, 105},
            {6, 109},
            {6, 103},
            {7, 105},
            {7, 109},
            {7, 103},
            {9, 119},
            {10, 120},
            {10, 105},
            {10, 109},
            {10, 103},
            {11, 120},
            {11, 105},
            {11, 109},
            {11, 103},
            {12, 105},
            {12, 109},
            {12, 103},
            {13, 1},
            {13, 3},
            {13, 103},
            {13, 8},
            {13, 105},
            {13, 9},
            {13, 104},
            {13, 14},
            {13, 15},
            {13, 112},
            {13, 16},
            {13, 17},
            {16, 27},
            {17, 41},
            {18, 109},
            {18, 19},
            {18, 20},
            {18, 45},
            {19, 25},
            {19, 109},
            {19, 45},
            {20, 21},
            {20, 45},
            {20, 109},
            {21, 45},
            {21, 22},
            {21, 23},
            {22, 24},
            {25, 26},
            {25, 45},
            {27, 32},
            {27, 33},
            {27, 34},
            {27, 67},
            {27, 35},
            {27, 5},
            {27, 70},
            {27, 68},
            {27, 109},
            {27, 28},
            {27, 29},
            {27, 30},
            {27, 31},
            {28, 6},
            {28, 7},
            {30, 115},
            {31, 120},
            {31, 40},
            {31, 39},
            {32, 105},
            {32, 109},
            {32, 38},
            {33, 37},
            {33, 36},
            {33, 109},
            {34, 114},
            {34, 109},
            {34, 36},
            {34, 37},
            {35, 37},
            {35, 36},
            {35, 109},
            {36, 109},
            {37, 105},
            {37, 38},
            {41, 32},
            {41, 33},
            {41, 34},
            {41, 35},
            {41, 5},
            {41, 71},
            {41, 72},
            {41, 73},
            {41, 8},
            {41, 11},
            {41, 75},
            {41, 109},
            {41, 42},
            {41, 43},
            {41, 28},
            {42, 109},
            {42, 44},
            {42, 37},
            {42, 36},
            {43, 36},
            {43, 37},
            {43, 44},
            {43, 109},
            {43, 114},
            {44, 109},
            {45, 105},
            {45, 109},
            {45, 46},
            {47, 48},
            {47, 51},
            {47, 52},
            {49, 104},
            {50, 116},
            {53, 54},
            {53, 55},
            {54, 52},
            {54, 87},
            {54, 55},
            {54, 56},
            {54, 57},
            {54, 58},
            {57, 59},
            {58, 59},
            {58, 51},
            {61, 48},
            {63, 64},
            {63, 65},
            {63, 109},
            {63, 95},
            {64, 62},
            {67, 104},
            {67, 116},
            {67, 47},
            {68, 11},
            {68, 53},
            {69, 70},
            {70, 116},
            {70, 11},
            {70, 60},
            {70, 53},
            {71, 104},
            {71, 116},
            {71, 61},
            {72, 66},
            {72, 116},
            {73, 74},
            {73, 5},
            {73, 63},
            {74, 6},
            {74, 7},
            {76, 60},
            {76, 53},
            {76, 47},
            {77, 66},
            {77, 61},
            {77, 63},
            {78, 79},
            {79, 67},
            {79, 100},
            {79, 71},
            {79, 117},
            {79, 118},
            {79, 119},
            {80, 79},
            {81, 73},
            {82, 68},
            {84, 72},
            {84, 69},
            {87, 109},
            {87, 18},
            {87, 86},
            {87, 88},
            {87, 89},
            {87, 90},
            {87, 91},
            {87, 92},
            {87, 93},
            {88, 90},
            {89, 90},
            {91, 94},
            {92, 97},
            {92, 98},
            {92, 109},
            {92, 45},
            {92, 90},
            {95, 96},
            {95, 99},
            {95, 45},
            {95, 109},
            {95, 62},
            {97, 45},
            {100, 104},
            {101, 104}
        };

        const unsigned int graph_size = 220;
    
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
