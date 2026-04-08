#include "NavMesh.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>

// =============================================================================
// NavMeshTriangle
// =============================================================================

glm::vec3 NavMeshTriangle::GetCenter() const
{
    // Centroid = arithmetic mean of the three vertices.
    return (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
}

// =============================================================================
// NavMesh — construction
// =============================================================================

void NavMesh::AddTriangle(const NavMeshTriangle& tri)
{
    m_triangles.push_back(tri);
}

void NavMesh::BuildFromMesh(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>&   indices)
{
    m_triangles.clear();

    // Every three consecutive indices describe one triangle.
    size_t triCount = indices.size() / 3;

    for (size_t i = 0; i < triCount; ++i)
    {
        NavMeshTriangle tri;
        tri.vertices[0] = vertices[indices[i * 3 + 0]];
        tri.vertices[1] = vertices[indices[i * 3 + 1]];
        tri.vertices[2] = vertices[indices[i * 3 + 2]];
        m_triangles.push_back(tri);
    }

    // Once all triangles are present we can resolve neighborhood.
    BuildAdjacency();
}

// ---- adjacency detection ----------------------------------------------------

/// @brief Small epsilon used for floating-point vertex comparisons.
static constexpr float kEpsilon = 1e-5f;

/**
 * @brief Checks whether two 3D points are the same within a tolerance.
 */
static bool VecEqual(const glm::vec3& a, const glm::vec3& b)
{
    return glm::length(a - b) < kEpsilon;
}

/**
 * @brief Returns the number of vertices shared by two triangles.
 *
 * If exactly two vertices are shared the triangles share an edge.
 */
static int SharedVertexCount(const NavMeshTriangle& a, const NavMeshTriangle& b)
{
    int count = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (VecEqual(a.vertices[i], b.vertices[j]))
                ++count;
        }
    }
    return count;
}

/**
 * @brief For two neighbor triangles, returns the edge index in triA
 *        whose two endpoints are shared with triB.
 *
 * Edge convention:
 *   edge 0 = vertices[0]→vertices[1]
 *   edge 1 = vertices[1]→vertices[2]
 *   edge 2 = vertices[2]→vertices[0]
 *
 * @return Edge index (0–2), or -1 if no shared edge is found.
 */
static int FindSharedEdge(const NavMeshTriangle& triA, const NavMeshTriangle& triB)
{
    // For each edge of triA check if both endpoints exist in triB.
    for (int e = 0; e < 3; ++e)
    {
        const glm::vec3& e0 = triA.vertices[e];
        const glm::vec3& e1 = triA.vertices[(e + 1) % 3];

        bool found0 = false;
        bool found1 = false;

        for (int j = 0; j < 3; ++j)
        {
            if (VecEqual(e0, triB.vertices[j]))
                found0 = true;

            if (VecEqual(e1, triB.vertices[j])) 
                found1 = true;
        }

        if (found0 && found1)
            return e;
    }
    return -1;
}

void NavMesh::BuildAdjacency()
{
    size_t count = m_triangles.size();

    // Reset all neighbor slots to "no neighbor".
    for (auto& tri : m_triangles)
    {
        for (int e = 0; e < 3; ++e)
        {
            tri.neighbors[e] = -1;
        }
    }

    // Brute-force O(n²) comparison — acceptable for typical nav-meshes.
    for (size_t i = 0; i < count; ++i)
    {
        for (size_t j = i + 1; j < count; ++j)
        {
            if (SharedVertexCount(m_triangles[i], m_triangles[j]) < 2)
                continue;

            // Triangles i and j share an edge.
            int edgeI = FindSharedEdge(m_triangles[i], m_triangles[j]);
            int edgeJ = FindSharedEdge(m_triangles[j], m_triangles[i]);

            if (edgeI >= 0)
                m_triangles[i].neighbors[edgeI] = static_cast<int32_t>(j);

            if (edgeJ >= 0)
                m_triangles[j].neighbors[edgeJ] = static_cast<int32_t>(i);
        }
    }

    // Classify boundary edges once adjacency is known.
    ClassifyBoundaries();
}

// =============================================================================
// NavMesh — boundary classification
// =============================================================================

void NavMesh::ClassifyBoundaries()
{
    m_boundaryEdges.clear();

    // Collect every edge whose neighbor slot is -1.
    for (size_t t = 0; t < m_triangles.size(); ++t)
    {
        for (int e = 0; e < 3; ++e)
        {
            if (m_triangles[t].neighbors[e] != -1)
                continue;

            NavMeshEdge edge;
            edge.v0 = m_triangles[t].vertices[e];
            edge.v1 = m_triangles[t].vertices[(e + 1) % 3];
            edge.triangleIndex = t;
            edge.edgeIndex = e;
            edge.type = BoundaryType::External; // default
            m_boundaryEdges.push_back(edge);
        }
    }

    if (m_boundaryEdges.empty())
    {
        m_boundariesClassified = true;
        return;
    }

    // --- Build connected loops and classify via signed area -------------------
    // Chain boundary edges: an edge's v1 should match the next edge's v0.
    std::vector<bool> visited(m_boundaryEdges.size(), false);

    for (size_t start = 0; start < m_boundaryEdges.size(); ++start)
    {
        if (visited[start])
            continue;

        // Trace one connected loop.
        std::vector<size_t> loop;
        size_t current = start;

        while (!visited[current])
        {
            visited[current] = true;
            loop.push_back(current);

            // Find the next edge whose v0 ≈ current v1.
            bool found = false;
            for (size_t k = 0; k < m_boundaryEdges.size(); ++k)
            {
                if (!visited[k] && VecEqual(m_boundaryEdges[k].v0, m_boundaryEdges[current].v1))
                {
                    current = k;
                    found   = true;
                    break;
                }
            }

            if (!found)
                break;
        }

        // Shoelace signed area on the XZ plane.
        float signedArea = 0.f;
        for (size_t idx : loop)
        {
            const glm::vec3& a = m_boundaryEdges[idx].v0;
            const glm::vec3& b = m_boundaryEdges[idx].v1;
            signedArea += (a.x * b.z - b.x * a.z);
        }
        signedArea *= 0.5f;

        // Positive (CCW on XZ) = outer contour ;  negative (CW) = hole.
        BoundaryType type = (signedArea >= 0.f) ? BoundaryType::External : BoundaryType::Internal;

        for (size_t idx : loop)
            m_boundaryEdges[idx].type = type;
    }

    m_boundariesClassified = true;
}

// =============================================================================
// NavMesh — queries
// =============================================================================

size_t NavMesh::GetTriangleCount() const
{
    return m_triangles.size();
}

const NavMeshTriangle& NavMesh::GetTriangle(size_t index) const
{
    return m_triangles[index];
}

// ---- XZ-plane point-in-triangle ---------------------------------------------

float NavMesh::CrossXZ(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    // 2D cross product on the XZ plane:
    //   (b - a) × (c - a)  projected onto Y
    return (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
}

bool NavMesh::IsPointInTriangle(const glm::vec3& point, size_t triangleIndex) const
{
    const NavMeshTriangle& tri = m_triangles[triangleIndex];

    // Compute the three signed areas.  If they all have the same sign the
    // point is inside the triangle (or on its edge when one is zero).
    float d1 = CrossXZ(tri.vertices[0], tri.vertices[1], point);
    float d2 = CrossXZ(tri.vertices[1], tri.vertices[2], point);
    float d3 = CrossXZ(tri.vertices[2], tri.vertices[0], point);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // If signs are mixed the point is outside.
    return !(hasNeg && hasPos);
}

int32_t NavMesh::FindTriangle(const glm::vec3& point) const
{
    // Linear scan — fine for meshes with a moderate triangle count.
    for (size_t i = 0; i < m_triangles.size(); ++i)
    {
        if (IsPointInTriangle(point, i))
            return static_cast<int32_t>(i);
    }
    return -1; // Point is not on the mesh.
}

// =============================================================================
// NavMesh — boundary queries
// =============================================================================

bool NavMesh::IsBoundaryEdge(size_t triangleIndex, int edge) const
{
    if (triangleIndex >= m_triangles.size() || edge < 0 || edge > 2)
        return false;

    return m_triangles[triangleIndex].neighbors[edge] == -1;
}

const std::vector<NavMeshEdge>& NavMesh::GetBoundaryEdges() const
{
    return m_boundaryEdges;
}

int32_t NavMesh::FindClosestTriangle(const glm::vec3& point,
                                     float maxYDelta) const
{
    int32_t bestTri  = -1;
    float   bestDist = std::numeric_limits<float>::max();

    for (size_t i = 0; i < m_triangles.size(); ++i)
    {
        if (!IsPointInTriangle(point, i))
            continue;

        float yDist = std::abs(point.y - m_triangles[i].GetCenter().y);
        if (yDist <= maxYDelta && yDist < bestDist)
        {
            bestDist = yDist;
            bestTri  = static_cast<int32_t>(i);
        }
    }
    return bestTri;
}

glm::vec3 NavMesh::ConstrainToBounds(const glm::vec3& point,
                                     float agentRadius) const
{
    if (agentRadius <= 0.f || m_boundaryEdges.empty())
        return point;

    glm::vec3 result = point;

    for (const auto& edge : m_boundaryEdges)
    {
        // Project onto the edge segment in the XZ plane.
        glm::vec2 p(result.x, result.z);
        glm::vec2 a(edge.v0.x, edge.v0.z);
        glm::vec2 b(edge.v1.x, edge.v1.z);

        glm::vec2 ab = b - a;
        float len2 = glm::dot(ab, ab);

        if (len2 < kEpsilon * kEpsilon)
            continue;

        float t = glm::clamp(glm::dot(p - a, ab) / len2, 0.f, 1.f);
        glm::vec2 closest = a + t * ab;
        glm::vec2 diff = p - closest;
        float dist = glm::length(diff);

        if (dist < agentRadius)
        {
            if (dist > kEpsilon)
            {
                // Push outward from the edge.
                glm::vec2 pushDir = diff / dist;
                glm::vec2 newPos  = closest + pushDir * agentRadius;
                result.x = newPos.x;
                result.z = newPos.y;
            }

            else
            {
                // Point is exactly on the edge — push along edge normal
                // toward the owning triangle's centroid.
                glm::vec2 normal(-ab.y, ab.x);
                float nLen = glm::length(normal);
                if (nLen > kEpsilon)
                {
                    normal /= nLen;
                    glm::vec2 triC(m_triangles[edge.triangleIndex].GetCenter().x, m_triangles[edge.triangleIndex].GetCenter().z);

                    if (glm::dot(normal, triC - closest) < 0.f)
                        normal = -normal;

                    glm::vec2 newPos = closest + normal * agentRadius;
                    result.x = newPos.x;
                    result.z = newPos.y;
                }
            }
        }
    }

    return result;
}

// =============================================================================
// NavMesh — pathfinding (A* + funnel)
// =============================================================================

/**
 * @brief Internal A* node used during the graph search.
 */
struct AStarNode
{
    int32_t triangle = -1;  ///< Triangle index in the mesh.
    float gCost = 0.f; ///< Accumulated cost from start.
    float fCost = 0.f; ///< gCost + heuristic.

    /// @brief Comparison for the priority queue (lowest fCost first).
    bool operator>(const AStarNode& other) const 
    { 
        return fCost > other.fCost;
    }
};

std::vector<glm::vec3> NavMesh::FindPath(const glm::vec3& start, const glm::vec3& end) const
{
    // --- Step 0 : locate start / end triangles --------------------------------
    int32_t startTri = FindTriangle(start);
    int32_t endTri = FindTriangle(end);

    if (startTri < 0 || endTri < 0)
        return 
    {}; // One of the points is off-mesh — no path.

    if (startTri == endTri)
        return 
    { 
        start, end 
    }; // Trivial case: same triangle.

    // --- Step 1 : A* over the triangle adjacency graph ------------------------

    // Min-heap ordered by fCost.
    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;

    // gCost per triangle (best known).
    std::unordered_map<int32_t, float>   gCosts;
    // Parent map to reconstruct the triangle corridor.
    std::unordered_map<int32_t, int32_t> cameFrom;

    glm::vec3 endCenter = m_triangles[endTri].GetCenter();

    // Seed with the start triangle.
    gCosts[startTri] = 0.f;
    float h = glm::length(m_triangles[startTri].GetCenter() - endCenter);
    openSet.push({ startTri, 0.f, h });

    bool found = false;

    while (!openSet.empty())
    {
        AStarNode current = openSet.top();
        openSet.pop();

        // Reached the goal triangle.
        if (current.triangle == endTri)
        {
            found = true;
            break;
        }

        // Skip stale entries (a better path was already found).
        if (current.gCost > gCosts[current.triangle])
            continue;

        // Explore neighbors (up to 3 per triangle).
        const NavMeshTriangle& tri = m_triangles[current.triangle];
        for (int e = 0; e < 3; ++e)
        {
            int32_t nb = tri.neighbors[e];
            if (nb < 0)
                continue; // Boundary edge — no neighbor.

            // Cost = Euclidean distance between triangle centroids.
            float tentativeG = current.gCost + glm::length(tri.GetCenter() - m_triangles[nb].GetCenter());

            // Only keep this path if it improves the known cost.
            auto it = gCosts.find(nb);
            if (it != gCosts.end() && tentativeG >= it->second)
                continue;

            gCosts[nb]  = tentativeG;
            cameFrom[nb] = current.triangle;

            float heur = glm::length(m_triangles[nb].GetCenter() - endCenter);
            openSet.push({ nb, tentativeG, tentativeG + heur });
        }
    }

    if (!found)
        return {}; // No connected path exists.

    // --- Step 2 : reconstruct the triangle corridor ---------------------------

    std::vector<int32_t> triPath;
    {
        int32_t cur = endTri;
        while (cur != startTri)
        {
            triPath.push_back(cur);
            cur = cameFrom[cur];
        }
        triPath.push_back(startTri);
        std::reverse(triPath.begin(), triPath.end());
    }

    // --- Step 3 : funnel smoothing -------------------------------------------
    return FunnelPath(triPath, start, end);
}

// =============================================================================
// NavMesh — portal & funnel helpers
// =============================================================================

void NavMesh::GetPortal(size_t triA, size_t triB, glm::vec3& left, glm::vec3& right) const
{
    const NavMeshTriangle& a = m_triangles[triA];
    const NavMeshTriangle& b = m_triangles[triB];

    // Find the two shared vertices between the two triangles.
    glm::vec3 shared[2];
    int found = 0;

    for (int i = 0; i < 3 && found < 2; ++i)
    {
        for (int j = 0; j < 3 && found < 2; ++j)
        {
            if (VecEqual(a.vertices[i], b.vertices[j]))
                shared[found++] = a.vertices[i];
        }
    }

    if (found < 2)
    {
        // Fallback — should not happen if adjacency was built correctly.
        left = right = a.GetCenter();
        return;
    }

    // Order the portal so that 'left' is on the left side when walking
    // from triA's centroid toward triB's centroid.
    glm::vec3 forward = b.GetCenter() - a.GetCenter();
    float cross = (shared[1].x - shared[0].x) * forward.z - (shared[1].z - shared[0].z) * forward.x;

    if (cross > 0.f)
    {
        left  = shared[0];
        right = shared[1];
    }

    else
    {
        left  = shared[1];
        right = shared[0];
    }
}

std::vector<glm::vec3> NavMesh::FunnelPath(const std::vector<int32_t>& trianglePath, const glm::vec3& start, const glm::vec3& end) const
{
    // Trivial corridor — just go straight.
    if (trianglePath.size() <= 1)
        return { start, end };

    // --- Build the portal list ------------------------------------------------
    // Each portal is the shared edge between consecutive triangles.
    struct Portal { glm::vec3 left, right; };
    std::vector<Portal> portals;

    // First portal is the start point itself (degenerate portal).
    portals.push_back({ start, start });

    for (size_t i = 0; i + 1 < trianglePath.size(); ++i)
    {
        glm::vec3 l, r;
        GetPortal(static_cast<size_t>(trianglePath[i]), static_cast<size_t>(trianglePath[i + 1]), l, r);
        portals.push_back({ l, r });
    }

    // Last portal is the end point (degenerate portal).
    portals.push_back({ end, end });

    // --- Simple funnel algorithm (Mikko Mononen style) ------------------------
    // We maintain a funnel apex and two edges (left / right).  Each new
    // portal either narrows the funnel, keeps it, or forces us to emit a
    // new waypoint and restart.

    std::vector<glm::vec3> path;
    path.push_back(start);

    glm::vec3 apex = start;
    glm::vec3 funnelL = start; // Current left edge of the funnel.
    glm::vec3 funnelR = start; // Current right edge of the funnel.
    size_t apexIndex = 0;
    size_t leftIndex = 0;
    size_t rightIndex = 0;

    for (size_t i = 1; i < portals.size(); ++i)
    {
        const glm::vec3& pLeft  = portals[i].left;
        const glm::vec3& pRight = portals[i].right;

        // --- Update right side of the funnel ----------------------------------
        if (CrossXZ(apex, funnelR, pRight) <= 0.f)
        {
            // New right vertex tightens or keeps the funnel.
            if (VecEqual(apex, funnelR) || CrossXZ(apex, funnelL, pRight) > 0.f)
            {
                funnelR   = pRight;
                rightIndex = i;
            }
            else
            {
                // Right crossed over left — emit left vertex as waypoint.
                path.push_back(funnelL);
                apex = funnelL;
                apexIndex = leftIndex;

                // Reset funnel from new apex.
                funnelL = apex;
                funnelR = apex;
                leftIndex = apexIndex;
                rightIndex = apexIndex;

                // Re-scan from the portal after the new apex.
                i = apexIndex;
                continue;
            }
        }

        // --- Update left side of the funnel -----------------------------------
        if (CrossXZ(apex, funnelL, pLeft) >= 0.f)
        {
            if (VecEqual(apex, funnelL) || CrossXZ(apex, funnelR, pLeft) < 0.f)
            {
                funnelL  = pLeft;
                leftIndex = i;
            }
            else
            {
                // Left crossed over right — emit right vertex as waypoint.
                path.push_back(funnelR);
                apex = funnelR;
                apexIndex = rightIndex;

                funnelL = apex;
                funnelR = apex;
                leftIndex = apexIndex;
                rightIndex = apexIndex;

                i = apexIndex;
                continue;
            }
        }
    }

    // Add the final destination.
    if (path.empty() || !VecEqual(path.back(), end))
        path.push_back(end);

    return path;
}
