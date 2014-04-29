#ifndef MESH_HPP_INCLUDED
#define MESH_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "systems/ResourceSystem.h"

namespace trillek { namespace resource {
    // Vertex data used for rendering or other purposes.
    struct VertexData {
        VertexData() : position(0.0f, 0.0f, 0.0f) { }
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    // Face information
    struct Face {
        unsigned int index[3]; // Vertex index
    };

    // Container for holding sub-mesh groups.
    struct MeshGroup {
        std::vector<VertexData> verts;
        std::vector<Face> indexList;
    };

    class Mesh : public ResourceBase {
    public:
        Mesh() { }
        ~Mesh() { }

        /**
        * \brief Returns a sub-mesh group.
        *
        * \param[in] const unsigned int& meshGroup The index of the mesh group to retrieve.
        * \return std::weak_ptr<MeshGroup> The mesh group requested
        */
        std::weak_ptr<MeshGroup> GetMeshGroup(const unsigned int& meshGroup);
    protected:
        std::vector<std::shared_ptr<MeshGroup>> meshGroups;
    };
}}

#endif
