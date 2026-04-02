#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <vector>
#include <cstdint>
#include <limits>

/**
 * @brief Represents a single triangle in the navigation mesh.
 *
 * Each triangle stores its three vertices (in world space) and up to three
 * neighbor indices — one per edge.  An index of -1 means the edge is a
 * boundary (no adjacent triangle).
 *
 * Edge convention:
 *   edge 0 = vertices[0] → vertices[1]
 *   edge 1 = vertices[1] → vertices[2]
 *   edge 2 = vertices[2] → vertices[0]
 */
struct NavMeshTriangle
{
    glm::vec3 vertices[3]{}; ///< The three corners of the triangle.
    int32_t neighbors[3]{ -1, -1, -1 }; ///< Adjacent triangle index per edge (-1 = none).

    /**
     * @brief Returns the centroid of the triangle.
     * @return glm::vec3  Average of the three vertices.
     */
    glm::vec3 GetCenter() const;
};


/**
 * @brief Classification of a boundary edge.
 */
enum class BoundaryType
{
    None,      ///< Not a boundary (internal to the mesh graph).
    External,  ///< Part of the outer contour of the walkable area.
    Internal   ///< Surrounds an internal hole / obstacle cut-out.
};

/**
 * @brief Describes a single boundary edge of the navigation mesh.
 */
struct NavMeshEdge
{
    glm::vec3 v0{};            ///< First endpoint of the edge.
    glm::vec3 v1{};            ///< Second endpoint of the edge.
    size_t triangleIndex{}; ///< Index of the owning triangle.
    int edgeIndex{};     ///< Edge slot (0\u20132) within that triangle.
    BoundaryType type = BoundaryType::External; ///< Boundary classification.
};


/**
 * @brief A navigation mesh used for AI pathfinding on walkable surfaces.
 *
 * The mesh is a collection of NavMeshTriangle.  After all triangles are
 * added, call BuildAdjacency() to auto-detect shared edges and fill the
 * neighbor arrays.
 *
 * Pathfinding is done in two steps:
 *   1. A* over the triangle adjacency graph  (FindPath)
 *   2. Simple funnel / string-pulling to produce a smooth waypoint list
 *
 * Point-location queries project positions onto the XZ plane (Y is up),
 * which is the standard ground plane used by the engine's coordinate system.
 */
class NavMesh
{
public:
    NavMesh() = default;

    // -------------------------------------------------------------------------
    // CONSTRUCTION
    // -------------------------------------------------------------------------

    /**
     * @brief Adds a single triangle to the mesh.
     *
     * Neighbors are NOT filled automatically — call BuildAdjacency() after
     * all triangles have been added.
     *
     * @param tri Triangle to append.
     */
    void AddTriangle(const NavMeshTriangle& tri);

    /**
     * @brief Builds the entire mesh from a flat list of vertices and indices.
     *
     * Every three consecutive indices form one triangle.
     * Previous content is cleared.  BuildAdjacency() is called internally.
     *
     * @param vertices World-space vertex positions.
     * @param indices  Index buffer (size must be a multiple of 3).
     */
    void BuildFromMesh(const std::vector<glm::vec3>& vertices,const std::vector<uint32_t>& indices);

    /**
     * @brief Detects shared edges between triangles and fills neighbor data.
     *
     * Two triangles are neighbors if they share exactly two vertex positions
     * (compared with a small epsilon).  This is an O(n²) brute-force pass
     * — perfectly fine for typical game-level meshes.
     */
    void BuildAdjacency();

    // -------------------------------------------------------------------------
    // QUERIES
    // -------------------------------------------------------------------------

    /**
     * @brief Returns the number of triangles in the mesh.
     * @return size_t
     */
    size_t GetTriangleCount() const;

    /**
     * @brief Returns a reference to a triangle by index.
     *
     * @param index  Triangle index (must be < GetTriangleCount()).
     * @return const NavMeshTriangle&
     */
    const NavMeshTriangle& GetTriangle(size_t index) const;

    /**
     * @brief Finds which triangle contains a world-space point (XZ projection).
     *
     * @param point World-space position.
     * @return Index of the containing triangle, or -1 if none found.
     */
    int32_t FindTriangle(const glm::vec3& point) const;

    /**
     * @brief Tests whether a point lies inside a given triangle (XZ projection).
     *
     * Uses the cross-product (sign-of-area) method on the XZ plane.
     *
     * @param point         World-space position to test.
     * @param triangleIndex Index of the triangle.
     * @return true if the point is inside or on the edge.
     */
    bool IsPointInTriangle(const glm::vec3& point, size_t triangleIndex) const;

    // -------------------------------------------------------------------------
    // BOUNDARY QUERIES
    // -------------------------------------------------------------------------

    /**
     * @brief Checks whether a specific edge of a triangle is a boundary.
     *
     * @param triangleIndex Index of the triangle.
     * @param edge          Edge slot (0\u20132).
     * @return true if the edge has no adjacent neighbor.
     */
    bool IsBoundaryEdge(size_t triangleIndex, int edge) const;

    /**
     * @brief Returns all boundary edges, classified as External or Internal.
     *
     * The classification is computed during BuildAdjacency() (or when
     * ClassifyBoundaries() is called explicitly).
     *
     * @return const reference to the cached boundary-edge list.
     */
    const std::vector<NavMeshEdge>& GetBoundaryEdges() const;

    /**
     * @brief Y-aware triangle lookup.
     *
     * When several triangles overlap in the XZ projection (e.g. bridges,
     * multi-floor), this picks the one whose centroid Y is closest to the
     * query point, within an optional vertical tolerance.
     *
     * @param point     World-space position to locate.
     * @param maxYDelta Maximum allowed vertical distance (default: no limit).
     * @return Index of the best triangle, or -1 if none found.
     */
    int32_t FindClosestTriangle(const glm::vec3& point, float maxYDelta = std::numeric_limits<float>::max()) const;

    /**
     * @brief Pushes a point away from nearby boundary edges by @p agentRadius.
     *
     * Iterates over all boundary edges and, if the XZ distance from @p point
     * to an edge is less than @p agentRadius, moves the point inward.
     *
     * @param point       World-space position to constrain.
     * @param agentRadius Minimum clearance from any boundary edge.
     * @return Adjusted position.
     */
    glm::vec3 ConstrainToBounds(const glm::vec3& point, float agentRadius) const;

    // -------------------------------------------------------------------------
    // PATHFINDING
    // -------------------------------------------------------------------------

    /**
     * @brief Computes a path from start to end across the navigation mesh.
     *
     * Internally:
     *   1. Locates the start / end triangles with FindTriangle().
     *   2. Runs A* over the triangle adjacency graph.
     *   3. Applies a simple funnel algorithm to produce smooth waypoints.
     *
     * If start or end is outside the mesh, or no path exists, an empty
     * vector is returned.
     *
     * @param start World-space start position.
     * @param end   World-space goal position.
     * @return Ordered list of world-space waypoints (may be empty).
     */
    std::vector<glm::vec3> FindPath(const glm::vec3& start, const glm::vec3& end) const;

private:
    std::vector<NavMeshTriangle> m_triangles;  ///< All triangles forming the mesh.
    std::vector<NavMeshEdge> m_boundaryEdges; ///< Cached boundary edges.
    bool m_boundariesClassified = false;        ///< True after ClassifyBoundaries().

    // ---- internal helpers ---------------------------------------------------

    /**
     * @brief Collects boundary edges and classifies them as External / Internal.
     *
     * Called automatically at the end of BuildAdjacency().
     * Uses connected boundary-edge loops and the shoelace signed-area test
     * on the XZ plane to distinguish outer contours from inner holes.
     */
    void ClassifyBoundaries();

    /**
     * @brief Cross-product sign test on the XZ plane.
     *
     * Returns a positive value if C is to the left of AB, negative if to
     * the right, and zero if collinear.
     *
     * @param a First point.
     * @param b Second point.
     * @param c Point to classify.
     * @return float  Signed area × 2 on the XZ plane.
     */
    static float CrossXZ(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    /**
     * @brief Retrieves the shared-edge portal between two adjacent triangles.
     *
     * The two portal endpoints are written to left / right so that they
     * form a consistent winding when walking from triA to triB.
     *
     * @param triA  Index of the first triangle.
     * @param triB  Index of the second (neighbor) triangle.
     * @param left  [out] Left endpoint of the portal.
     * @param right [out] Right endpoint of the portal.
     */
    void GetPortal(size_t triA, size_t triB, glm::vec3& left, glm::vec3& right) const;

    /**
     * @brief Simple string-pulling (funnel) algorithm.
     *
     * Given the ordered list of triangle indices produced by A*, generates
     * a tight corridor of waypoints that avoids unnecessary detours.
     *
     * @param trianglePath Ordered triangle indices from A*.
     * @param start        World-space start position.
     * @param end          World-space goal position.
     * @return Smoothed waypoint list.
     */
    std::vector<glm::vec3> FunnelPath(const std::vector<int32_t>& trianglePath, const glm::vec3& start, const glm::vec3& end) const;
};


/**
 * @brief ECS component that gives an entity access to a navigation mesh.
 *
 * Attach this component to any entity (e.g. the level or a manager entity)
 * so that AI systems can query pathfinding through the registry.
 */
struct NavMeshComponent
{
    NavMesh* navMesh = nullptr; ///< Pointer to the shared NavMesh resource.
};
