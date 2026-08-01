#pragma once

#include <cstdint>
#include "core.hpp"
namespace game {
namespace hk {

namespace ai {
typedef hkUint32 hkaiPackedKeyStorage;
typedef hkaiPackedKeyStorage hkaiPackedKey;
typedef hkInt32 hkaiNavMeshFaceData;
typedef hkInt32 hkaiNavMeshEdgeData;
typedef hkUint32 hkaiSectionUid;
typedef int32_t hkaiRuntimeIndex;
typedef hkUint32 hkaiLayer;
typedef hkInt32 hkaiNavMeshVertexIndex;
typedef hkInt32 hkaiNavMeshEdgeIndex;
typedef hkInt32 hkaiNavMeshNodeIndex;
typedef hkInt32 hkaiNavMeshFaceIndex;
typedef hkInt32 hkaiNavMeshCellIndex;

class hkaiReferenceFrame {
  hkTransform m_transform;
  hkVector4 m_linearVelocity;
  hkVector4 m_angularVelocity;
};

class hkaiStreamingSet {
public:
  typedef hkaiNavMeshFaceIndex FaceIndex;
  typedef hkaiNavMeshEdgeIndex EdgeIndex;
  typedef hkaiNavMeshNodeIndex NodeIndex;
  typedef hkaiNavMeshCellIndex CellIndex;
  typedef hkUint32 GraphEdgeData;

  struct NavMeshConnection {
    hkaiStreamingSet::FaceIndex m_faceIndex;
    hkaiStreamingSet::EdgeIndex m_edgeIndex;
    hkaiStreamingSet::FaceIndex m_oppositeFaceIndex;
    hkaiStreamingSet::EdgeIndex m_oppositeEdgeIndex;
  };

  PACKED(struct GraphConnection {
    NodeIndex m_nodeIndex;
    NodeIndex m_oppositeNodeIndex;
    GraphEdgeData m_edgeData;
    hkHalf m_edgeCost;
    uint8_t _padding0E[2];
  });

  struct VolumeConnection {
    CellIndex m_cellIndex;
    CellIndex m_oppositeCellIndex;
  };

  hkaiSectionUid m_thisUid;
  hkaiSectionUid m_oppositeUid;
  hkArray<hkaiStreamingSet::NavMeshConnection, hkContainerHeapAllocator>
      m_meshConnections;
  hkArray<hkaiStreamingSet::GraphConnection, hkContainerHeapAllocator>
      m_graphConnections;
  hkArray<hkaiStreamingSet::VolumeConnection, hkContainerHeapAllocator>
      m_volumeConnections;
};

class hkaiNavMesh : public hkReferencedObject {
public:
  typedef hkaiNavMeshVertexIndex VertexIndex;
  enum class EdgeFlagBits : uint32_t {
    SILHOUETTE = 0x1,
    RETRIANGULATED = 0x2,
    ORIGINAL = 0x4,
    OPPOSITE_UNLOADED_UNUSED = 0x8,
    USER = 0x10,
    BLOCKED = 0x20,
    EXTERNAL_OPPOSITE = 0x40,
  };

  typedef hkFlags<hkaiNavMesh::EdgeFlagBits, hkUint8> EdgeFlags;

  typedef hkaiNavMeshEdgeIndex EdgeIndex;

  struct Face {
    hkaiNavMesh::EdgeIndex m_startEdgeIndex;
    hkaiNavMesh::EdgeIndex m_startUserEdgeIndex;
    hkInt16 m_numEdges;
    hkInt16 m_numUserEdges;
    hkInt16 m_clusterIndex;
    hkUint16 m_padding;
  };

  class Edge {
    hkaiNavMesh::VertexIndex m_a;
    hkaiNavMesh::VertexIndex m_b;
    hkaiPackedKey m_oppositeEdge;
    hkaiPackedKey m_oppositeFace;
    hkaiNavMesh::EdgeFlags m_flags;
    hkUint8 m_paddingByte;
    hkHalf m_userEdgeCost;
  };

  typedef hkaiNavMeshEdgeData EdgeData;

  typedef hkaiNavMeshFaceData FaceData;

  enum class FaceFlagBits : uint32_t {
    HIDDEN = 0x1,
    CUT = 0x2,
    STREAMING = 0x4,
  };

  enum class NavMeshFlagBits : uint32_t {
    NONE = 0x0,
    CLIMBING = 0x1,
  };

  typedef hkFlags<FaceFlagBits, unsigned char> FaceFlags;
  typedef hkFlags<hkaiNavMesh::NavMeshFlagBits, unsigned char> NavMeshFlags;

  hkArray<hkaiNavMesh::Face, hkContainerHeapAllocator> m_faces;
  hkArray<hkaiNavMesh::Edge, hkContainerHeapAllocator> m_edges;
  hkArray<hkVector4f, hkContainerHeapAllocator> m_vertices;
  hkArray<hkaiStreamingSet, hkContainerHeapAllocator> m_streamingSets;
  hkArray<int, hkContainerHeapAllocator> m_faceData;
  hkArray<int, hkContainerHeapAllocator> m_edgeData;
  int m_faceDataStriding;
  int m_edgeDataStriding;
  hkaiNavMesh::NavMeshFlags m_flags;
  __attribute__((aligned(8))) hkAabb m_aabb;
  hkReal m_erosionRadius;
  hkUlong m_userData;
};

class hkaiNavMeshClearanceCache : hkReferencedObject {
  typedef hkUint8 ClearanceIntegerType;
  typedef ClearanceIntegerType NodeData;
  typedef hkUint8 McpInterpolantIntegerType;

  class McpDataInteger {
    hkaiNavMeshClearanceCache::McpInterpolantIntegerType m_interpolant;
    hkaiNavMeshClearanceCache::ClearanceIntegerType m_clearance;
  };

  hkReal m_clearanceCeiling;
  hkReal m_clearanceIntToRealMultiplier;
  hkReal m_clearanceRealToIntMultiplier;
  hkArray<NodeData, hkContainerHeapAllocator> m_faceOffsets;
  hkArray<ClearanceIntegerType, hkContainerHeapAllocator> m_edgePairClearances;
  int m_unusedEdgePairElements;
  hkArray<hkaiNavMeshClearanceCache::McpDataInteger, hkContainerHeapAllocator>
      m_mcpData;
  hkArray<ClearanceIntegerType, hkContainerHeapAllocator> m_vertexClearances;
  int m_edgePairsUncalculatedLowerBound;
};

class hkaiNavMeshInstance : hkReferencedObject {
public:
  typedef hkUint16 CutInfo;

  const hkaiNavMesh::Face *m_originalFaces;
  int m_numOriginalFaces;
  const hkaiNavMesh::Edge *m_originalEdges;
  int m_numOriginalEdges;
  const hkVector4 *m_originalVertices;
  int m_numOriginalVertices;
  const hkaiNavMesh::FaceData *m_originalFaceData;
  int m_faceDataStriding;
  const hkaiNavMesh::EdgeData *m_originalEdgeData;
  int m_edgeDataStriding;
  hkRefPtr<const hkaiNavMesh> m_originalMesh;
  __attribute__((aligned(16))) hkaiReferenceFrame m_referenceFrame;
  hkArray<int, hkContainerHeapAllocator> m_edgeMap;
  hkArray<int, hkContainerHeapAllocator> m_faceMap;
  hkArray<hkaiNavMesh::Face, hkContainerHeapAllocator> m_instancedFaces;
  hkArray<hkaiNavMesh::Edge, hkContainerHeapAllocator> m_instancedEdges;
  hkArray<hkaiNavMesh::Face, hkContainerHeapAllocator> m_ownedFaces;
  hkArray<hkaiNavMesh::Edge, hkContainerHeapAllocator> m_ownedEdges;
  hkArray<hkVector4f, hkContainerHeapAllocator> m_ownedVertices;
  hkArray<hkaiNavMesh::FaceFlags, hkContainerHeapAllocator> m_faceFlags;
  hkArray<CutInfo, hkContainerHeapAllocator> m_cuttingInfo;
  hkArray<int, hkContainerHeapAllocator> m_instancedFaceData;
  hkArray<int, hkContainerHeapAllocator> m_instancedEdgeData;
  hkArray<int, hkContainerHeapAllocator> m_ownedFaceData;
  hkArray<int, hkContainerHeapAllocator> m_ownedEdgeData;
  hkaiSectionUid m_sectionUid;
  hkaiRuntimeIndex m_runtimeId;
  hkaiLayer m_layer;
  hkArray<hkRefPtr<hkaiNavMeshClearanceCache>, hkContainerHeapAllocator>
      m_clearanceCaches;
};
} // namespace ai
} // namespace hk
} // namespace game