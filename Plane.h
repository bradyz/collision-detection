#ifndef PLANE_H
#define PLANE_H

#include <vector>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "BoundingBox.h"
#include "RigidBody.h"
#include "RandomUtils.h"

struct Plane : RigidBody {
  glm::vec3 position;
  glm::vec3 normal;

  std::vector<glm::vec4> vertices;
  std::vector<glm::uvec3> faces;
  std::vector<glm::vec4> normals;

  double len;
  double wid;

  Plane (const glm::vec3& p, const glm::vec3& n, double l, double w) :
    position(p), normal(glm::normalize(n)), len(l), wid(w) {
    vertices.push_back(glm::vec4(-w, 0.0, -l, 1.0));
    vertices.push_back(glm::vec4(-w, 0.0, l, 1.0));
    vertices.push_back(glm::vec4(w, 0.0, -l, 1.0));
    vertices.push_back(glm::vec4(w, 0.0, l, 1.0));

    faces.push_back(glm::uvec3(0, 1, 3));
    faces.push_back(glm::uvec3(0, 3, 2));

    normals.push_back(glm::vec4(normal, 0.0));
    normals.push_back(glm::vec4(normal, 0.0));
    normals.push_back(glm::vec4(normal, 0.0));
    normals.push_back(glm::vec4(normal, 0.0));

    glm::vec3 original(0.0, 1.0, 0.0);
    glm::vec3 axis = glm::cross(original, normal);

    if (std::abs(glm::length2(axis)) > 1e-5) {
      float angle = std::acos(glm::dot(normal, original));
      glm::mat4 R = glm::rotate(angle, axis);

      for (glm::vec4& vertex: vertices)
        vertex = R * vertex;

      for (glm::vec4& vertexNormal: normals)
        vertexNormal = R * vertexNormal;
    }

    glm::mat4 T = glm::translate(p);

    for (glm::vec4& vertex: vertices)
      vertex = T * vertex;
  }

  BoundingBox getBoundingBox () {
    return BoundingBox(vertices);
  }
};

#endif