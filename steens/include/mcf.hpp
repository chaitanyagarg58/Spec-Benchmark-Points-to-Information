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
            {"global_opt", 0},
            {"main", 1},
            {"refresh_neighbour_lists", 2},
            {"refresh_potential", 3},
            {"flow_cost", 4},
            {"flow_org_cost", 5},
            {"primal_feasible", 6},
            {"dual_feasible", 7},
            {"getfree", 8},
            {"read_min", 9},
            {"resize_prob", 10},
            {"insert_new_arc", 11},
            {"replace_weaker_arc", 12},
            {"price_out_impl", 13},
            {"suspend_impl", 14},
            {"primal_start_artificial", 15},
            {"write_circulations", 16},
            {"update_tree", 17},
            {"primal_iminus", 18},
            {"primal_update_flow", 19},
            {"primal_net_simplex", 20},
            {"bea_is_dual_infeasible", 21},
            {"sort_basket", 22},
            {"primal_bea_mpp", 23},
            {"fgets", 24},
            {"llvm.dbg.declare", 25},
            {"printf", 26},
            {"exit", 27},
            {"strcpy", 28},
            {"llvm.fmuladd.f64", 29},
            {"llvm.dbg.label", 30},
            {"fprintf", 31},
            {"free", 32},
            {"fopen", 33},
            {"__isoc99_sscanf", 34},
            {"calloc", 35},
            {"fclose", 36},
            {"realloc", 37},
            {"fflush", 38},
            {"llvm.dbg.value", 39},
            {"llvm.memset.p0.i64", 40},
            {"llvm.memcpy.p0.p0.i64", 41}
        };

        // Graph Representation: Vector of (caller, callee) pairs
        const std::vector<EdgeT> graph = {
            {0, 4},
            {0, 13},
            {0, 14},
            {0, 20},
            {0, 26},
            {0, 27},
            {1, 0},
            {1, 8},
            {1, 40},
            {1, 9},
            {1, 15},
            {1, 16},
            {1, 26},
            {1, 28},
            {4, 29},
            {5, 29},
            {6, 26},
            {7, 31},
            {8, 32},
            {9, 33},
            {9, 34},
            {9, 35},
            {9, 36},
            {9, 8},
            {9, 24},
            {9, 26},
            {10, 26},
            {10, 37},
            {10, 38},
            {13, 10},
            {13, 2},
            {13, 11},
            {13, 12},
            {14, 41},
            {14, 2},
            {16, 33},
            {16, 2},
            {16, 36},
            {16, 31},
            {20, 3},
            {20, 6},
            {20, 7},
            {20, 17},
            {20, 18},
            {20, 19},
            {20, 23},
            {22, 22},
            {23, 21},
            {23, 22}
        };

        const unsigned int graph_size = 49;
    
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
            {{"net", "mcf.c", "28"}, 18},
            {{"perm", "pbeampp.c", "60"}, 57},
            {{"basket", "pbeampp.c", "59"}, 61},
            {{"call5", "mcf.c", "139"}, 186},
            {{"net", "mcfutil.c", "28"}, 224},
            {{"nodes", "mcfutil.c", "39"}, 226},
            {{"i", "mcfutil.c", "39"}, 228},
            {{"stop_nodes", "mcfutil.c", "40"}, 230},
            {{"i1", "mcfutil.c", "40"}, 232},
            {{"i2", "mcfutil.c", "40"}, 233},
            {{"node.0", "mcfutil.c", "0"}, 237},
            {{"incdec.ptr", "mcfutil.c", "40"}, 238},
            {{"i3", "mcfutil.c", "40"}, 240},
            {{"firstout", "mcfutil.c", "43"}, 246},
            {{"arcs", "mcfutil.c", "46"}, 253},
            {{"i4", "mcfutil.c", "46"}, 255},
            {{"stop_arcs", "mcfutil.c", "47"}, 257},
            {{"i5", "mcfutil.c", "47"}, 259},
            {{"i6", "mcfutil.c", "47"}, 260},
            {{"arc.0", "mcfutil.c", "0"}, 264},
            {{"incdec.ptr11", "mcfutil.c", "47"}, 265},
            {{"i7", "mcfutil.c", "47"}, 267},
            {{"head8", "mcfutil.c", "52"}, 287},
            {{"i13", "mcfutil.c", "52"}, 288},
            {{"firstin9", "mcfutil.c", "52"}, 289},
            {{"net", "mcfutil.c", "68"}, 299},
            {{"nodes", "mcfutil.c", "75"}, 301},
            {{"net", "mcfutil.c", "122"}, 403},
            {{"stop_arcs", "mcfutil.c", "136"}, 407},
            {{"i", "mcfutil.c", "136"}, 408},
            {{"i1", "mcfutil.c", "136"}, 409},
            {{"arcs", "mcfutil.c", "137"}, 411},
            {{"i2", "mcfutil.c", "137"}, 412},
            {{"arc.0", "mcfutil.c", "0"}, 416},
            {{"incdec.ptr", "mcfutil.c", "137"}, 417},
            {{"i3", "mcfutil.c", "137"}, 419},
            {{"flow2", "mcfutil.c", "142"}, 430},
            {{"stop_nodes", "mcfutil.c", "145"}, 437},
            {{"i5", "mcfutil.c", "145"}, 438},
            {{"i6", "mcfutil.c", "145"}, 439},
            {{"nodes", "mcfutil.c", "146"}, 441},
            {{"i7", "mcfutil.c", "146"}, 442},
            {{"incdec.ptr3", "mcfutil.c", "146"}, 444},
            {{"node.0", "mcfutil.c", "0"}, 448},
            {{"incdec.ptr10", "mcfutil.c", "146"}, 449},
            {{"i8", "mcfutil.c", "146"}, 451},
            {{"basic_arc", "mcfutil.c", "147"}, 457},
            {{"i10", "mcfutil.c", "147"}, 458},
            {{"flow8", "mcfutil.c", "147"}, 459},
            {{"stop_arcs12", "mcfutil.c", "149"}, 465},
            {{"i11", "mcfutil.c", "149"}, 466},
            {{"i12", "mcfutil.c", "149"}, 467},
            {{"arcs13", "mcfutil.c", "150"}, 469},
            {{"i13", "mcfutil.c", "150"}, 470},
            {{"arc.1", "mcfutil.c", "0"}, 476},
            {{"incdec.ptr34", "mcfutil.c", "150"}, 477},
            {{"i14", "mcfutil.c", "150"}, 483},
            {{"bigM", "mcfutil.c", "158"}, 511},
            {{"cost28", "mcfutil.c", "162"}, 520},
            {{"bigM36", "mcfutil.c", "168"}, 544},
            {{"net", "mcfutil.c", "239"}, 707},
            {{"dummy_arcs", "mcfutil.c", "247"}, 709},
            {{"stop_dummy1", "mcfutil.c", "248"}, 713},
            {{"nodes", "mcfutil.c", "253"}, 717},
            {{"stop_nodes", "mcfutil.c", "254"}, 720},
            {{"feas_tol", "mcfutil.c", "262"}, 751},
            {{"feas_tol9", "mcfutil.c", "271"}, 764},
            {{"feas_tol13", "mcfutil.c", "272"}, 770},
            {{"feasible", "mcfutil.c", "276"}, 776},
            {{"feasible21", "mcfutil.c", "282"}, 786},
            {{"net", "mcfutil.c", "297"}, 794},
            {{"stop_arcs", "mcfutil.c", "304"}, 796},
            {{"arcs", "mcfutil.c", "309"}, 799},
            {{"feas_tol", "mcfutil.c", "329"}, 825},
            {{"feas_tol5", "mcfutil.c", "339"}, 834},
            {{"net", "mcfutil.c", "369"}, 868},
            {{"nodes", "mcfutil.c", "377"}, 870},
            {{"i", "mcfutil.c", "377"}, 871},
            {{"nodes1", "mcfutil.c", "377"}, 874},
            {{"i1", "mcfutil.c", "377"}, 875},
            {{"i2", "mcfutil.c", "377"}, 876},
            {{"arcs", "mcfutil.c", "378"}, 882},
            {{"i3", "mcfutil.c", "378"}, 883},
            {{"arcs4", "mcfutil.c", "378"}, 886},
            {{"i4", "mcfutil.c", "378"}, 887},
            {{"i5", "mcfutil.c", "378"}, 888},
            {{"dummy_arcs", "mcfutil.c", "379"}, 892},
            {{"i6", "mcfutil.c", "379"}, 893},
            {{"dummy_arcs8", "mcfutil.c", "379"}, 896},
            {{"i7", "mcfutil.c", "379"}, 897},
            {{"i8", "mcfutil.c", "379"}, 898},
            {{"stop_nodes", "mcfutil.c", "380"}, 902},
            {{"nodes10", "mcfutil.c", "380"}, 904},
            {{"stop_arcs", "mcfutil.c", "381"}, 906},
            {{"arcs11", "mcfutil.c", "381"}, 908},
            {{"stop_dummy", "mcfutil.c", "382"}, 910},
            {{"dummy_arcs12", "mcfutil.c", "382"}, 912},
            {{"net", "readmin.c", "29"}, 918},
            {{"instring", "readmin.c", "36"}, 920},
            {{"inputfile", "readmin.c", "43"}, 933},
            {{"arraydecay", "readmin.c", "43"}, 934},
            {{"call", "readmin.c", "43"}, 936},
            {{"arraydecay1", "readmin.c", "46"}, 944},
            {{"call2", "readmin.c", "46"}, 945},
            {{"arraydecay3", "readmin.c", "47"}, 949},
            {{"n_trips", "readmin.c", "51"}, 957},
            {{"m_org", "readmin.c", "52"}, 960},
            {{"n", "readmin.c", "53"}, 966},
            {{"m", "readmin.c", "54"}, 975},
            {{"n_trips12", "readmin.c", "56"}, 977},
            {{"m15", "readmin.c", "58"}, 981},
            {{"max_m", "readmin.c", "58"}, 983},
            {{"max_new_m", "readmin.c", "59"}, 985},
            {{"max_m16", "readmin.c", "67"}, 991},
            {{"max_new_m17", "readmin.c", "71"}, 994},
            {{"max_m19", "readmin.c", "74"}, 998},
            {{"m20", "readmin.c", "74"}, 1000},
            {{"max_residual_new_m", "readmin.c", "74"}, 1003},
            {{"n21", "readmin.c", "80"}, 1005},
            {{"call23", "readmin.c", "80"}, 1008},
            {{"call23", "readmin.c", "80"}, 1009},
            {{"i13", "readmin.c", "80"}, 1013},
            {{"nodes", "readmin.c", "80"}, 1014},
            {{"n24", "readmin.c", "81"}, 1016},
            {{"call25", "readmin.c", "81"}, 1018},
            {{"call25", "readmin.c", "81"}, 1019},
            {{"i15", "readmin.c", "81"}, 1021},
            {{"dummy_arcs", "readmin.c", "81"}, 1022},
            {{"max_m26", "readmin.c", "82"}, 1024},
            {{"call27", "readmin.c", "82"}, 1026},
            {{"call27", "readmin.c", "82"}, 1027},
            {{"i17", "readmin.c", "82"}, 1028},
            {{"arcs", "readmin.c", "82"}, 1029},
            {{"nodes28", "readmin.c", "84"}, 1031},
            {{"i18", "readmin.c", "84"}, 1032},
            {{"arcs29", "readmin.c", "84"}, 1035},
            {{"i19", "readmin.c", "84"}, 1036},
            {{"dummy_arcs32", "readmin.c", "84"}, 1039},
            {{"i20", "readmin.c", "84"}, 1040},
            {{"nodes38", "readmin.c", "108"}, 1046},
            {{"i21", "readmin.c", "108"}, 1047},
            {{"n39", "readmin.c", "108"}, 1048},
            {{"add.ptr", "readmin.c", "108"}, 1050},
            {{"add.ptr40", "readmin.c", "108"}, 1051},
            {{"stop_nodes", "readmin.c", "108"}, 1052},
            {{"arcs41", "readmin.c", "109"}, 1054},
            {{"i23", "readmin.c", "109"}, 1055},
            {{"m42", "readmin.c", "109"}, 1056},
            {{"add.ptr43", "readmin.c", "109"}, 1058},
            {{"stop_arcs", "readmin.c", "109"}, 1059},
            {{"dummy_arcs44", "readmin.c", "110"}, 1061},
            {{"i25", "readmin.c", "110"}, 1062},
            {{"n45", "readmin.c", "110"}, 1063},
            {{"add.ptr46", "readmin.c", "110"}, 1065},
            {{"stop_dummy", "readmin.c", "110"}, 1066},
            {{"nodes47", "readmin.c", "113"}, 1068},
            {{"i27", "readmin.c", "113"}, 1069},
            {{"arcs48", "readmin.c", "114"}, 1071},
            {{"i28", "readmin.c", "114"}, 1072},
            {{"arc.0", "readmin.c", "0"}, 1079},
            {{"n_trips49", "readmin.c", "116"}, 1083},
            {{"arraydecay51", "readmin.c", "118"}, 1087},
            {{"arraydecay53", "readmin.c", "120"}, 1089},
            {{"arrayidx", "readmin.c", "123"}, 1100},
            {{"number", "readmin.c", "123"}, 1101},
            {{"n_trips62", "readmin.c", "126"}, 1107},
            {{"n_trips66", "readmin.c", "127"}, 1113},
            {{"n_trips73", "readmin.c", "130"}, 1126},
            {{"n77", "readmin.c", "132"}, 1132},
            {{"bigM", "readmin.c", "134"}, 1140},
            {{"n_trips87", "readmin.c", "141"}, 1170},
            {{"n91", "readmin.c", "142"}, 1176},
            {{"n_trips109", "readmin.c", "151"}, 1210},
            {{"bigM113", "readmin.c", "152"}, 1216},
            {{"bigM116", "readmin.c", "152"}, 1220},
            {{"n_trips130", "readmin.c", "161"}, 1257},
            {{"arc.1", "readmin.c", "0"}, 1268},
            {{"incdec.ptr169", "readmin.c", "165"}, 1269},
            {{"m_org137", "readmin.c", "165"}, 1272},
            {{"arraydecay141", "readmin.c", "167"}, 1276},
            {{"arraydecay143", "readmin.c", "169"}, 1278},
            {{"n_trips149", "readmin.c", "172"}, 1284},
            {{"stop_arcs171", "readmin.c", "183"}, 1325},
            {{"i75", "readmin.c", "183"}, 1326},
            {{"stop_arcs175", "readmin.c", "185"}, 1329},
            {{"arcs176", "readmin.c", "186"}, 1331},
            {{"i76", "readmin.c", "186"}, 1332},
            {{"m177", "readmin.c", "187"}, 1334},
            {{"arc.2", "readmin.c", "0"}, 1338},
            {{"incdec.ptr186", "readmin.c", "187"}, 1339},
            {{"stop_arcs179", "readmin.c", "187"}, 1341},
            {{"i77", "readmin.c", "187"}, 1342},
            {{"m183", "readmin.c", "188"}, 1345},
            {{"m188", "readmin.c", "189"}, 1353},
            {{"m_org189", "readmin.c", "189"}, 1355},
            {{"clustfile", "readmin.c", "195"}, 1362},
            {{"arrayidx192", "readmin.c", "195"}, 1363},
            {{"n_trips194", "readmin.c", "197"}, 1371},
            {{"bigM198", "readmin.c", "200"}, 1375},
            {{"bigM202", "readmin.c", "200"}, 1379},
            {{"arcs207", "readmin.c", "199"}, 1387},
            {{"i83", "readmin.c", "199"}, 1388},
            {{"arrayidx210", "readmin.c", "199"}, 1392},
            {{"cost211", "readmin.c", "199"}, 1393},
            {{"bigM212", "readmin.c", "202"}, 1395},
            {{"bigM216", "readmin.c", "202"}, 1399},
            {{"arcs221", "readmin.c", "201"}, 1406},
            {{"i86", "readmin.c", "201"}, 1407},
            {{"arrayidx224", "readmin.c", "201"}, 1410},
            {{"org_cost225", "readmin.c", "201"}, 1411},
            {{"net", "implicit.c", "28"}, 1435},
            {{"max_new_m", "implicit.c", "42"}, 1437},
            {{"max_m", "implicit.c", "42"}, 1439},
            {{"max_new_m1", "implicit.c", "43"}, 1443},
            {{"max_residual_new_m", "implicit.c", "43"}, 1445},
            {{"arcs", "implicit.c", "55"}, 1449},
            {{"i4", "implicit.c", "55"}, 1450},
            {{"i5", "implicit.c", "55"}, 1451},
            {{"max_m3", "implicit.c", "55"}, 1452},
            {{"call", "implicit.c", "55"}, 1456},
            {{"i7", "implicit.c", "55"}, 1459},
            {{"inputfile", "implicit.c", "58"}, 1463},
            {{"arraydecay", "implicit.c", "58"}, 1464},
            {{"arcs6", "implicit.c", "63"}, 1473},
            {{"i10", "implicit.c", "63"}, 1474},
            {{"arcs7", "implicit.c", "65"}, 1478},
            {{"m", "implicit.c", "66"}, 1480},
            {{"add.ptr", "implicit.c", "66"}, 1482},
            {{"stop_arcs", "implicit.c", "66"}, 1483},
            {{"nodes", "implicit.c", "68"}, 1485},
            {{"i13", "implicit.c", "68"}, 1486},
            {{"incdec.ptr", "implicit.c", "69"}, 1489},
            {{"stop_nodes", "implicit.c", "69"}, 1491},
            {{"node.0", "implicit.c", "0"}, 1498},
            {{"incdec.ptr13", "implicit.c", "69"}, 1499},
            {{"i20", "implicit.c", "71"}, 1511},
            {{"basic_arc11", "implicit.c", "71"}, 1512},
            {{"net", "implicit.c", "129"}, 1649},
            {{"max_residual_new_m", "implicit.c", "152"}, 1695},
            {{"max_residual_new_m64", "implicit.c", "167"}, 1772},
            {{"net", "implicit.c", "194"}, 1797},
            {{"bigM1", "implicit.c", "207"}, 1802},
            {{"n_trips", "implicit.c", "227"}, 1808},
            {{"m", "implicit.c", "229"}, 1812},
            {{"max_new_m", "implicit.c", "229"}, 1814},
            {{"max_m", "implicit.c", "229"}, 1817},
            {{"n_trips3", "implicit.c", "231"}, 1821},
            {{"n_trips4", "implicit.c", "231"}, 1823},
            {{"m5", "implicit.c", "231"}, 1827},
            {{"max_m7", "implicit.c", "231"}, 1830},
            {{"stop_arcs", "implicit.c", "259"}, 1849},
            {{"n_trips13", "implicit.c", "260"}, 1852},
            {{"arcs", "implicit.c", "262"}, 1855},
            {{"max_residual_new_m", "implicit.c", "297"}, 1977},
            {{"stop_arcs72", "implicit.c", "314"}, 2010},
            {{"stop_arcs73", "implicit.c", "315"}, 2013},
            {{"stop_arcs75", "implicit.c", "316"}, 2017},
            {{"m104", "implicit.c", "338"}, 2077},
            {{"m_impl", "implicit.c", "339"}, 2081},
            {{"max_residual_new_m107", "implicit.c", "340"}, 2085},
            {{"net", "implicit.c", "359"}, 2095},
            {{"m_impl", "implicit.c", "376"}, 2103},
            {{"stop_arcs", "implicit.c", "379"}, 2108},
            {{"arcs", "implicit.c", "380"}, 2112},
            {{"m", "implicit.c", "380"}, 2114},
            {{"m_impl1", "implicit.c", "380"}, 2116},
            {{"m28", "implicit.c", "417"}, 2208},
            {{"m_impl30", "implicit.c", "418"}, 2212},
            {{"stop_arcs32", "implicit.c", "419"}, 2216},
            {{"max_residual_new_m", "implicit.c", "420"}, 2221},
            {{"net", "pstart.c", "29"}, 2230},
            {{"nodes", "pstart.c", "40"}, 2232},
            {{"n", "pstart.c", "46"}, 2248},
            {{"stop_arcs", "pstart.c", "51"}, 2260},
            {{"arcs", "pstart.c", "52"}, 2264},
            {{"dummy_arcs", "pstart.c", "56"}, 2287},
            {{"stop_nodes", "pstart.c", "57"}, 2290},
            {{"net", "output.c", "30"}, 2350},
            {{"stop_arcs", "output.c", "44"}, 2354},
            {{"m_impl", "output.c", "44"}, 2356},
            {{"nodes", "output.c", "51"}, 2369},
            {{"n", "output.c", "51"}, 2371},
            {{"n_trips", "output.c", "64"}, 2410},
            {{"delta", "pbla.c", "41"}, 2804},
            {{"xchange", "pbla.c", "41"}, 2805},
            {{"w", "pbla.c", "41"}, 2808},
            {{"net", "psimplex.c", "30"}, 3004},
            {{"delta", "psimplex.c", "36"}, 3006},
            {{"xchange", "psimplex.c", "39"}, 3008},
            {{"w", "psimplex.c", "45"}, 3010},
            {{"red_cost_of_bea", "psimplex.c", "53"}, 3012},
            {{"arcs1", "psimplex.c", "48"}, 3018},
            {{"stop_arcs2", "psimplex.c", "49"}, 3021},
            {{"m3", "psimplex.c", "51"}, 3024},
            {{"iterations4", "psimplex.c", "54"}, 3028},
            {{"bound_exchanges5", "psimplex.c", "55"}, 3030},
            {{"checksum6", "psimplex.c", "56"}, 3033},
            {{"feas_tol", "psimplex.c", "129"}, 3165},
            {{"arrayidx", "pbeampp.c", "77"}, 3236},
            {{"i", "pbeampp.c", "77"}, 3237},
            {{"abs_cost", "pbeampp.c", "77"}, 3238},
            {{"arrayidx1", "pbeampp.c", "81"}, 3254},
            {{"i2", "pbeampp.c", "81"}, 3255},
            {{"abs_cost2", "pbeampp.c", "81"}, 3256},
            {{"arrayidx4", "pbeampp.c", "83"}, 3267},
            {{"i4", "pbeampp.c", "83"}, 3268},
            {{"abs_cost5", "pbeampp.c", "83"}, 3269},
            {{"arrayidx10", "pbeampp.c", "88"}, 3277},
            {{"i6", "pbeampp.c", "88"}, 3278},
            {{"arrayidx11", "pbeampp.c", "89"}, 3280},
            {{"i7", "pbeampp.c", "89"}, 3281},
            {{"arrayidx12", "pbeampp.c", "89"}, 3282},
            {{"arrayidx13", "pbeampp.c", "90"}, 3284},
            {{"red_cost_of_bea", "pbeampp.c", "119"}, 3320},
            {{"arrayidx", "pbeampp.c", "136"}, 3337},
            {{"arrayidx1", "pbeampp.c", "136"}, 3338},
            {{"arrayidx6", "pbeampp.c", "146"}, 3372},
            {{"i3", "pbeampp.c", "146"}, 3373},
            {{"a", "pbeampp.c", "146"}, 3374},
            {{"arrayidx18", "pbeampp.c", "152"}, 3404},
            {{"arrayidx20", "pbeampp.c", "153"}, 3408},
            {{"arrayidx24", "pbeampp.c", "154"}, 3419},
            {{"arrayidx45", "pbeampp.c", "174"}, 3471},
            {{"arrayidx47", "pbeampp.c", "175"}, 3476},
            {{"arrayidx55", "pbeampp.c", "176"}, 3488},
            {{"str", "None", "0th arg fgets"}, 3555},
            {{"stream", "None", "2nd arg fgets"}, 3557},
            {{"str.addr", "None", ""}, 3559},
            {{"stream.addr", "None", ""}, 3563}
        };
        
        // Graph Representation: Vector of (pointer, pointee) pairs
        const std::vector<EdgeT> graph = {
            {18, 1027},
            {18, 1009},
            {18, 1019},
            {18, 1456},
            {57, 61},
            {186, 18},
            {224, 18},
            {226, 18},
            {228, 1456},
            {230, 18},
            {232, 1456},
            {233, 1456},
            {237, 1456},
            {238, 1456},
            {240, 1456},
            {246, 1456},
            {253, 18},
            {255, 1456},
            {257, 18},
            {259, 1456},
            {260, 1456},
            {264, 1027},
            {264, 1009},
            {264, 1019},
            {264, 1456},
            {265, 1456},
            {267, 1456},
            {287, 1456},
            {288, 1027},
            {288, 1009},
            {288, 1019},
            {288, 1456},
            {289, 1027},
            {289, 1009},
            {289, 1019},
            {289, 1456},
            {299, 18},
            {301, 18},
            {403, 18},
            {407, 18},
            {408, 1027},
            {408, 1009},
            {408, 1019},
            {408, 1456},
            {409, 1027},
            {409, 1009},
            {409, 1019},
            {409, 1456},
            {411, 18},
            {412, 1027},
            {412, 1009},
            {412, 1019},
            {412, 1456},
            {416, 1027},
            {416, 1009},
            {416, 1019},
            {416, 1456},
            {417, 1027},
            {417, 1009},
            {417, 1019},
            {417, 1456},
            {419, 1027},
            {419, 1009},
            {419, 1019},
            {419, 1456},
            {430, 1027},
            {430, 1009},
            {430, 1019},
            {430, 1456},
            {437, 18},
            {438, 1027},
            {438, 1009},
            {438, 1019},
            {438, 1456},
            {439, 1027},
            {439, 1009},
            {439, 1019},
            {439, 1456},
            {441, 18},
            {442, 1027},
            {442, 1009},
            {442, 1019},
            {442, 1456},
            {444, 1027},
            {444, 1009},
            {444, 1019},
            {444, 1456},
            {448, 1027},
            {448, 1009},
            {448, 1019},
            {448, 1456},
            {449, 1027},
            {449, 1009},
            {449, 1019},
            {449, 1456},
            {451, 1027},
            {451, 1009},
            {451, 1019},
            {451, 1456},
            {457, 1027},
            {457, 1009},
            {457, 1019},
            {457, 1456},
            {458, 1027},
            {458, 1009},
            {458, 1019},
            {458, 1456},
            {459, 1027},
            {459, 1009},
            {459, 1019},
            {459, 1456},
            {465, 18},
            {466, 1027},
            {466, 1009},
            {466, 1019},
            {466, 1456},
            {467, 1027},
            {467, 1009},
            {467, 1019},
            {467, 1456},
            {469, 18},
            {470, 1027},
            {470, 1009},
            {470, 1019},
            {470, 1456},
            {476, 1027},
            {476, 1009},
            {476, 1019},
            {476, 1456},
            {477, 1027},
            {477, 1009},
            {477, 1019},
            {477, 1456},
            {483, 1027},
            {483, 1009},
            {483, 1019},
            {483, 1456},
            {511, 18},
            {520, 1027},
            {520, 1009},
            {520, 1019},
            {520, 1456},
            {544, 18},
            {707, 18},
            {709, 18},
            {713, 18},
            {717, 18},
            {720, 18},
            {751, 18},
            {764, 18},
            {770, 18},
            {776, 18},
            {786, 18},
            {794, 18},
            {796, 18},
            {799, 18},
            {825, 18},
            {834, 18},
            {868, 18},
            {870, 18},
            {871, 1027},
            {871, 1009},
            {871, 1019},
            {871, 1456},
            {874, 18},
            {875, 1027},
            {875, 1009},
            {875, 1019},
            {875, 1456},
            {876, 1027},
            {876, 1009},
            {876, 1019},
            {876, 1456},
            {882, 18},
            {883, 1027},
            {883, 1009},
            {883, 1019},
            {883, 1456},
            {886, 18},
            {887, 1027},
            {887, 1009},
            {887, 1019},
            {887, 1456},
            {888, 1027},
            {888, 1009},
            {888, 1019},
            {888, 1456},
            {892, 18},
            {893, 1027},
            {893, 1009},
            {893, 1019},
            {893, 1456},
            {896, 18},
            {897, 1027},
            {897, 1009},
            {897, 1019},
            {897, 1456},
            {898, 1027},
            {898, 1009},
            {898, 1019},
            {898, 1456},
            {902, 18},
            {904, 18},
            {906, 18},
            {908, 18},
            {910, 18},
            {912, 18},
            {918, 18},
            {933, 18},
            {934, 18},
            {944, 920},
            {945, 920},
            {949, 920},
            {957, 18},
            {960, 18},
            {966, 18},
            {975, 18},
            {977, 18},
            {981, 18},
            {983, 18},
            {985, 18},
            {991, 18},
            {994, 18},
            {998, 18},
            {1000, 18},
            {1003, 18},
            {1005, 18},
            {1008, 1027},
            {1008, 1009},
            {1008, 1019},
            {1008, 1456},
            {1009, 1027},
            {1009, 1009},
            {1009, 1019},
            {1009, 1456},
            {1013, 1027},
            {1013, 1009},
            {1013, 1019},
            {1013, 1456},
            {1014, 18},
            {1016, 18},
            {1018, 1027},
            {1018, 1009},
            {1018, 1019},
            {1018, 1456},
            {1019, 1027},
            {1019, 1009},
            {1019, 1019},
            {1019, 1456},
            {1021, 1027},
            {1021, 1009},
            {1021, 1019},
            {1021, 1456},
            {1022, 18},
            {1024, 18},
            {1026, 1027},
            {1026, 1009},
            {1026, 1019},
            {1026, 1456},
            {1027, 1027},
            {1027, 1009},
            {1027, 1019},
            {1027, 1456},
            {1028, 1027},
            {1028, 1009},
            {1028, 1019},
            {1028, 1456},
            {1029, 18},
            {1031, 18},
            {1032, 1027},
            {1032, 1009},
            {1032, 1019},
            {1032, 1456},
            {1035, 18},
            {1036, 1456},
            {1039, 18},
            {1040, 1456},
            {1046, 18},
            {1047, 1456},
            {1048, 18},
            {1050, 1456},
            {1051, 1456},
            {1052, 18},
            {1054, 18},
            {1055, 1456},
            {1056, 18},
            {1058, 1456},
            {1059, 18},
            {1061, 18},
            {1062, 1456},
            {1063, 18},
            {1065, 1456},
            {1066, 18},
            {1068, 18},
            {1069, 1456},
            {1071, 18},
            {1072, 1456},
            {1079, 1456},
            {1083, 18},
            {1087, 920},
            {1089, 920},
            {1100, 1456},
            {1101, 1456},
            {1107, 18},
            {1113, 18},
            {1126, 18},
            {1132, 18},
            {1140, 18},
            {1170, 18},
            {1176, 18},
            {1210, 18},
            {1216, 18},
            {1220, 18},
            {1257, 18},
            {1268, 1456},
            {1269, 1456},
            {1272, 18},
            {1276, 920},
            {1278, 920},
            {1284, 18},
            {1325, 18},
            {1326, 1456},
            {1329, 18},
            {1331, 18},
            {1332, 1456},
            {1334, 18},
            {1338, 1456},
            {1339, 1456},
            {1341, 18},
            {1342, 1456},
            {1345, 18},
            {1353, 18},
            {1355, 18},
            {1362, 18},
            {1363, 18},
            {1371, 18},
            {1375, 18},
            {1379, 18},
            {1387, 18},
            {1388, 1456},
            {1392, 1456},
            {1393, 1456},
            {1395, 18},
            {1399, 18},
            {1406, 18},
            {1407, 1456},
            {1410, 1456},
            {1411, 1456},
            {1435, 18},
            {1437, 18},
            {1439, 18},
            {1443, 18},
            {1445, 18},
            {1449, 18},
            {1450, 1456},
            {1451, 1456},
            {1452, 18},
            {1456, 1027},
            {1456, 1009},
            {1456, 1019},
            {1456, 1456},
            {1459, 1456},
            {1463, 18},
            {1464, 18},
            {1473, 18},
            {1474, 1456},
            {1478, 18},
            {1480, 18},
            {1482, 1456},
            {1483, 18},
            {1485, 18},
            {1486, 1456},
            {1489, 1456},
            {1491, 18},
            {1498, 1456},
            {1499, 1456},
            {1511, 1027},
            {1511, 1009},
            {1511, 1019},
            {1511, 1456},
            {1512, 1456},
            {1649, 18},
            {1695, 18},
            {1772, 18},
            {1797, 18},
            {1802, 18},
            {1808, 18},
            {1812, 18},
            {1814, 18},
            {1817, 18},
            {1821, 18},
            {1823, 18},
            {1827, 18},
            {1830, 18},
            {1849, 18},
            {1852, 18},
            {1855, 18},
            {1977, 18},
            {2010, 18},
            {2013, 18},
            {2017, 18},
            {2077, 18},
            {2081, 18},
            {2085, 18},
            {2095, 18},
            {2103, 18},
            {2108, 18},
            {2112, 18},
            {2114, 18},
            {2116, 18},
            {2208, 18},
            {2212, 18},
            {2216, 18},
            {2221, 18},
            {2230, 18},
            {2232, 18},
            {2248, 18},
            {2260, 18},
            {2264, 18},
            {2287, 18},
            {2290, 18},
            {2350, 18},
            {2354, 18},
            {2356, 18},
            {2369, 18},
            {2371, 18},
            {2410, 18},
            {2804, 3006},
            {2805, 3008},
            {2808, 3010},
            {3004, 18},
            {3018, 18},
            {3021, 18},
            {3024, 18},
            {3028, 18},
            {3030, 18},
            {3033, 18},
            {3165, 18},
            {3236, 57},
            {3237, 61},
            {3238, 61},
            {3254, 57},
            {3255, 61},
            {3256, 61},
            {3267, 57},
            {3268, 61},
            {3269, 61},
            {3277, 57},
            {3278, 61},
            {3280, 57},
            {3281, 61},
            {3282, 57},
            {3284, 57},
            {3320, 3012},
            {3337, 61},
            {3338, 57},
            {3372, 57},
            {3373, 61},
            {3374, 61},
            {3404, 57},
            {3408, 57},
            {3419, 57},
            {3471, 57},
            {3476, 57},
            {3488, 57},
            {3555, 920},
            {3557, 936},
            {3559, 920},
            {3563, 936}
        };

        const unsigned int graph_size = 469;
    
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
