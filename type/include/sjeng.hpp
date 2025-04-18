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
            {"calc_attackers", 0},
            {"is_attacked", 1},
            {"nk_attacked", 2},
            {"init_book", 3},
            {"choose_book_move", 4},
            {"ProcessHoldings", 5},
            {"text_to_piece", 6},
            {"SwitchColor", 7},
            {"SwitchPromoted", 8},
            {"addHolding", 9},
            {"removeHolding", 10},
            {"DropaddHolding", 11},
            {"DropremoveHolding", 12},
            {"printHolding", 13},
            {"is_draw", 14},
            {"storeECache", 15},
            {"checkECache", 16},
            {"reset_ecache", 17},
            {"alloc_ecache", 18},
            {"free_ecache", 19},
            {"setup_epd_line", 20},
            {"check_solution", 21},
            {"run_epd_testsuite", 22},
            {"run_autotest", 23},
            {"initialize_eval", 24},
            {"eval", 25},
            {"losers_eval", 26},
            {"l_pawn_mobility", 27},
            {"l_rook_mobility", 28},
            {"l_bishop_mobility", 29},
            {"l_knight_mobility", 30},
            {"l_king_mobility", 31},
            {"check_legal", 32},
            {"gen", 33},
            {"push_pawn", 34},
            {"push_pawn_simple", 35},
            {"push_knighT", 36},
            {"push_slidE", 37},
            {"push_king", 38},
            {"push_king_castle", 39},
            {"try_drop", 40},
            {"add_move", 41},
            {"add_capture", 42},
            {"in_check", 43},
            {"f_in_check", 44},
            {"extended_in_check", 45},
            {"make", 46},
            {"unmake", 47},
            {"check_phase", 48},
            {"King", 49},
            {"Queen", 50},
            {"rook_mobility", 51},
            {"bishop_mobility", 52},
            {"Rook", 53},
            {"Bishop", 54},
            {"Knight", 55},
            {"Pawn", 56},
            {"ErrorIt", 57},
            {"std_eval", 58},
            {"ResetHandValue", 59},
            {"BegForPartner", 60},
            {"GreetPartner", 61},
            {"HandlePartner", 62},
            {"HandlePtell", 63},
            {"CheckBadFlow", 64},
            {"Xmalloc", 65},
            {"Xfree", 66},
            {"freenodes", 67},
            {"pn_eval", 68},
            {"suicide_pn_eval", 69},
            {"losers_pn_eval", 70},
            {"std_pn_eval", 71},
            {"select_most_proving", 72},
            {"set_proof_and_disproof_numbers", 73},
            {"develop_node", 74},
            {"update_ancestors", 75},
            {"pn2_eval", 76},
            {"proofnumberscan", 77},
            {"proofnumbersearch", 78},
            {"proofnumbercheck", 79},
            {"read_rcfile", 80},
            {"order_moves", 81},
            {"perft", 82},
            {"qsearch", 83},
            {"remove_one", 84},
            {"search", 85},
            {"search_root", 86},
            {"think", 87},
            {"tree", 88},
            {"setup_attackers", 89},
            {"findlowest", 90},
            {"see", 91},
            {"suicide_eval", 92},
            {"suicide_mid_eval", 93},
            {"s_pawn_mobility", 94},
            {"black_saccers", 95},
            {"white_saccers", 96},
            {"s_rook_mobility", 97},
            {"s_bishop_mobility", 98},
            {"s_knight_mobility", 99},
            {"s_king_mobility", 100},
            {"main", 101},
            {"clear_tt", 102},
            {"clear_dp_tt", 103},
            {"initialize_zobrist", 104},
            {"initialize_hash", 105},
            {"QStoreTT", 106},
            {"StoreTT", 107},
            {"LearnStoreTT", 108},
            {"ProbeTT", 109},
            {"QProbeTT", 110},
            {"alloc_hash", 111},
            {"free_hash", 112},
            {"allocate_time", 113},
            {"comp_to_san", 114},
            {"comp_to_coord", 115},
            {"display_board", 116},
            {"init_game", 117},
            {"reset_piece_square", 118},
            {"is_move", 119},
            {"perft_debug", 120},
            {"rinput", 121},
            {"verify_coord", 122},
            {"hash_extract_pv", 123},
            {"stringize_pv", 124},
            {"post_thinking", 125},
            {"rtime", 126},
            {"rdifftime", 127},
            {"post_fail_thinking", 128},
            {"post_fh_thinking", 129},
            {"post_fl_thinking", 130},
            {"post_stat_thinking", 131},
            {"print_move", 132},
            {"rdelay", 133},
            {"check_piece_square", 134},
            {"start_up", 135},
            {"toggle_bool", 136},
            {"tree_debug", 137},
            {"interrupt", 138},
            {"PutPiece", 139},
            {"reset_board", 140},
            {"seedMT", 141},
            {"reloadMT", 142},
            {"randomMT", 143},
            {"__ctype_b_loc", 144},
            {"strstr", 145},
            {"fgets", 146},
            {"llvm.dbg.declare", 147},
            {"strncpy", 148},
            {"printf", 149},
            {"malloc", 150},
            {"exit", 151},
            {"free", 152},
            {"atoi", 153},
            {"atol", 154},
            {"fopen", 155},
            {"fclose", 156},
            {"abs", 157},
            {"llvm.dbg.label", 158},
            {"__isoc99_sscanf", 159},
            {"strncmp", 160},
            {"strcpy", 161},
            {"calloc", 162},
            {"strcat", 163},
            {"sprintf", 164},
            {"fputc", 165},
            {"fprintf", 166},
            {"setbuf", 167},
            {"tolower", 168},
            {"strcmp", 169},
            {"signal", 170},
            {"llvm.fmuladd.f64", 171},
            {"getc", 172},
            {"difftime", 173},
            {"llvm.dbg.value", 174},
            {"llvm.memcpy.p0.p0.i64", 175},
            {"llvm.memset.p0.i64", 176}
        };

        // Graph Representation: Vector of (caller, callee) pairs
        const std::vector<EdgeT> graph = {
            {4, 175},
            {5, 176},
            {5, 17},
            {5, 148},
            {7, 175},
            {8, 175},
            {13, 149},
            {17, 176},
            {18, 149},
            {18, 150},
            {18, 151},
            {19, 152},
            {20, 105},
            {20, 144},
            {20, 176},
            {20, 118},
            {20, 153},
            {21, 145},
            {21, 114},
            {21, 149},
            {22, 121},
            {22, 102},
            {22, 105},
            {22, 138},
            {22, 175},
            {22, 146},
            {22, 116},
            {22, 149},
            {22, 21},
            {22, 87},
            {22, 20},
            {22, 25},
            {22, 154},
            {22, 155},
            {23, 102},
            {23, 105},
            {23, 175},
            {23, 146},
            {23, 20},
            {23, 116},
            {23, 149},
            {23, 87},
            {23, 151},
            {23, 153},
            {23, 155},
            {23, 156},
            {23, 126},
            {24, 157},
            {25, 1},
            {25, 2},
            {25, 26},
            {25, 15},
            {25, 16},
            {25, 58},
            {25, 92},
            {26, 15},
            {26, 16},
            {26, 176},
            {26, 157},
            {26, 27},
            {26, 28},
            {26, 29},
            {26, 30},
            {26, 31},
            {32, 1},
            {33, 34},
            {33, 35},
            {33, 36},
            {33, 37},
            {33, 38},
            {33, 39},
            {33, 40},
            {34, 42},
            {35, 41},
            {36, 41},
            {36, 42},
            {37, 41},
            {37, 42},
            {38, 41},
            {38, 42},
            {43, 1},
            {44, 1},
            {45, 1},
            {46, 8},
            {46, 9},
            {46, 12},
            {46, 7},
            {47, 8},
            {47, 10},
            {47, 11},
            {47, 7},
            {50, 51},
            {50, 52},
            {53, 51},
            {54, 52},
            {57, 149},
            {58, 16},
            {58, 157},
            {58, 176},
            {58, 15},
            {59, 175},
            {61, 149},
            {62, 176},
            {62, 149},
            {62, 60},
            {62, 61},
            {62, 159},
            {63, 160},
            {63, 161},
            {63, 175},
            {63, 145},
            {63, 149},
            {63, 59},
            {64, 32},
            {64, 33},
            {64, 43},
            {64, 11},
            {64, 12},
            {64, 46},
            {64, 47},
            {64, 149},
            {67, 152},
            {67, 67},
            {68, 69},
            {68, 70},
            {68, 71},
            {70, 32},
            {70, 1},
            {70, 33},
            {70, 46},
            {70, 47},
            {71, 32},
            {71, 33},
            {71, 1},
            {71, 46},
            {71, 47},
            {72, 46},
            {73, 32},
            {73, 33},
            {73, 107},
            {73, 43},
            {73, 46},
            {73, 47},
            {73, 14},
            {74, 32},
            {74, 65},
            {74, 33},
            {74, 68},
            {74, 73},
            {74, 43},
            {74, 46},
            {74, 47},
            {74, 175},
            {75, 73},
            {75, 46},
            {75, 47},
            {76, 68},
            {76, 72},
            {76, 73},
            {76, 74},
            {76, 75},
            {77, 138},
            {77, 149},
            {77, 152},
            {77, 32},
            {77, 33},
            {77, 162},
            {77, 43},
            {77, 46},
            {77, 175},
            {77, 47},
            {77, 176},
            {77, 66},
            {77, 68},
            {77, 72},
            {77, 73},
            {77, 74},
            {77, 75},
            {77, 114},
            {77, 126},
            {77, 127},
            {78, 66},
            {78, 163},
            {78, 162},
            {78, 68},
            {78, 72},
            {78, 73},
            {78, 138},
            {78, 74},
            {78, 75},
            {78, 46},
            {78, 175},
            {78, 176},
            {78, 47},
            {78, 115},
            {78, 149},
            {78, 152},
            {78, 126},
            {78, 127},
            {79, 162},
            {79, 66},
            {79, 68},
            {79, 72},
            {79, 73},
            {79, 74},
            {79, 75},
            {79, 46},
            {79, 47},
            {79, 175},
            {79, 149},
            {79, 152},
            {79, 126},
            {79, 127},
            {80, 24},
            {80, 18},
            {80, 164},
            {80, 111},
            {81, 91},
            {81, 157},
            {82, 32},
            {82, 33},
            {82, 43},
            {82, 46},
            {82, 47},
            {82, 82},
            {83, 33},
            {83, 106},
            {83, 138},
            {83, 110},
            {83, 46},
            {83, 47},
            {83, 113},
            {83, 81},
            {83, 83},
            {83, 84},
            {83, 149},
            {83, 175},
            {83, 25},
            {83, 126},
            {83, 127},
            {85, 0},
            {85, 138},
            {85, 14},
            {85, 149},
            {85, 26},
            {85, 157},
            {85, 32},
            {85, 33},
            {85, 43},
            {85, 44},
            {85, 46},
            {85, 175},
            {85, 47},
            {85, 81},
            {85, 83},
            {85, 84},
            {85, 85},
            {85, 92},
            {85, 107},
            {85, 109},
            {85, 113},
            {85, 126},
            {85, 127},
            {86, 32},
            {86, 33},
            {86, 129},
            {86, 128},
            {86, 130},
            {86, 43},
            {86, 44},
            {86, 46},
            {86, 175},
            {86, 47},
            {86, 14},
            {86, 114},
            {86, 81},
            {86, 84},
            {86, 85},
            {86, 125},
            {87, 5},
            {87, 138},
            {87, 17},
            {87, 149},
            {87, 32},
            {87, 33},
            {87, 43},
            {87, 46},
            {87, 175},
            {87, 176},
            {87, 47},
            {87, 48},
            {87, 77},
            {87, 79},
            {87, 86},
            {87, 102},
            {87, 113},
            {87, 114},
            {87, 124},
            {87, 125},
            {87, 126},
            {87, 127},
            {88, 32},
            {88, 33},
            {88, 132},
            {88, 165},
            {88, 166},
            {88, 43},
            {88, 46},
            {88, 47},
            {88, 116},
            {88, 88},
            {90, 157},
            {90, 175},
            {91, 89},
            {91, 90},
            {91, 157},
            {92, 93},
            {93, 96},
            {93, 97},
            {93, 98},
            {93, 99},
            {93, 100},
            {93, 15},
            {93, 16},
            {93, 176},
            {93, 94},
            {93, 95},
            {95, 0},
            {95, 1},
            {96, 0},
            {96, 1},
            {101, 5},
            {101, 135},
            {101, 136},
            {101, 139},
            {101, 140},
            {101, 14},
            {101, 144},
            {101, 17},
            {101, 145},
            {101, 19},
            {101, 20},
            {101, 149},
            {101, 22},
            {101, 23},
            {101, 151},
            {101, 25},
            {101, 154},
            {101, 159},
            {101, 160},
            {101, 161},
            {101, 163},
            {101, 167},
            {101, 168},
            {101, 169},
            {101, 170},
            {101, 46},
            {101, 175},
            {101, 47},
            {101, 176},
            {101, 48},
            {101, 59},
            {101, 60},
            {101, 62},
            {101, 63},
            {101, 64},
            {101, 78},
            {101, 80},
            {101, 82},
            {101, 87},
            {101, 102},
            {101, 104},
            {101, 105},
            {101, 112},
            {101, 115},
            {101, 116},
            {101, 117},
            {101, 118},
            {101, 119},
            {101, 121},
            {101, 122},
            {101, 126},
            {101, 127},
            {102, 176},
            {103, 176},
            {104, 141},
            {104, 143},
            {111, 149},
            {111, 150},
            {111, 151},
            {112, 152},
            {113, 171},
            {114, 32},
            {114, 33},
            {114, 161},
            {114, 163},
            {114, 164},
            {114, 43},
            {114, 46},
            {114, 47},
            {114, 175},
            {115, 164},
            {115, 175},
            {116, 166},
            {116, 175},
            {117, 176},
            {117, 118},
            {117, 175},
            {118, 176},
            {119, 144},
            {120, 153},
            {120, 168},
            {120, 169},
            {120, 46},
            {120, 82},
            {120, 116},
            {120, 149},
            {120, 117},
            {120, 151},
            {120, 121},
            {120, 122},
            {121, 172},
            {122, 32},
            {122, 33},
            {122, 169},
            {122, 46},
            {122, 175},
            {122, 47},
            {122, 115},
            {123, 32},
            {123, 33},
            {123, 163},
            {123, 109},
            {123, 46},
            {123, 47},
            {123, 114},
            {123, 123},
            {124, 163},
            {124, 46},
            {124, 47},
            {124, 176},
            {124, 114},
            {124, 123},
            {125, 46},
            {125, 47},
            {125, 176},
            {125, 114},
            {125, 149},
            {125, 123},
            {125, 157},
            {125, 126},
            {125, 127},
            {127, 173},
            {128, 46},
            {128, 47},
            {128, 175},
            {128, 114},
            {128, 149},
            {128, 157},
            {128, 126},
            {128, 127},
            {129, 46},
            {129, 175},
            {129, 47},
            {129, 114},
            {129, 149},
            {129, 157},
            {129, 126},
            {129, 127},
            {130, 46},
            {130, 175},
            {130, 47},
            {130, 114},
            {130, 149},
            {130, 157},
            {130, 126},
            {130, 127},
            {131, 149},
            {131, 126},
            {131, 127},
            {132, 114},
            {132, 166},
            {132, 175},
            {133, 126},
            {133, 127},
            {134, 116},
            {134, 149},
            {135, 149},
            {137, 153},
            {137, 166},
            {137, 82},
            {137, 149},
            {137, 117},
            {137, 88},
            {137, 121},
            {137, 155},
            {140, 176},
            {140, 118},
            {140, 175},
            {142, 141},
            {143, 142},
            {144, 150}
        };

        const unsigned int graph_size = 502;
    
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
