#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <vector>
#include <cstdint>
#include <limits>

struct NavMeshTriangle
{
    glm::vec3 vertices[3]{};
    int32_t neighbors[3]{ -1, -1, -1 };

    glm::vec3 GetCenter() const;
};


enum class BoundaryType
{
    None,
    External,
    Internal
};

struct NavMeshEdge
{
    glm::vec3 v0{};
    glm::vec3 v1{};
    size_t triangleIndex{};
    int edgeIndex{};
    BoundaryType type = BoundaryType::External;
};


class NavMesh
{
public:
    NavMesh() = default;

    // -------------------------------------------------------------------------
    // CONSTRUCTION
    // -------------------------------------------------------------------------

    void AddTriangle(const NavMeshTriangle& tri);

    void BuildFromMesh(const std::vector<glm::vec3>& vertices,const std::vector<uint32_t>& indices);

    void BuildAdjacency();

    // -------------------------------------------------------------------------
    // QUERIES
    // -------------------------------------------------------------------------

    size_t GetTriangleCount() const;

    const NavMeshTriangle& GetTriangle(size_t index) const;

    int32_t FindTriangle(const glm::vec3& point) const;

    bool IsPointInTriangle(const glm::vec3& point, size_t triangleIndex) const;

    // -------------------------------------------------------------------------
    // BOUNDARY QUERIES
    // -------------------------------------------------------------------------

    bool IsBoundaryEdge(size_t triangleIndex, int edge) const;

    const std::vector<NavMeshEdge>& GetBoundaryEdges() const;

    int32_t FindClosestTriangle(const glm::vec3& point, float maxYDelta = std::numeric_limits<float>::max()) const;

    glm::vec3 ConstrainToBounds(const glm::vec3& point, float agentRadius) const;

    // -------------------------------------------------------------------------
    // PATHFINDING
    // -------------------------------------------------------------------------

    std::vector<glm::vec3> FindPath(const glm::vec3& start, const glm::vec3& end) const;

private:
    std::vector<NavMeshTriangle> m_triangles;
    std::vector<NavMeshEdge> m_boundaryEdges;
    bool m_boundariesClassified = false;

    // ---- internal helpers ---------------------------------------------------

    void ClassifyBoundaries();

    static float CrossXZ(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    void GetPortal(size_t triA, size_t triB, glm::vec3& left, glm::vec3& right) const;

    std::vector<glm::vec3> FunnelPath(const std::vector<int32_t>& trianglePath, const glm::vec3& start, const glm::vec3& end) const;
};


struct NavMeshComponent
{
    NavMesh* navMesh = nullptr;
};
