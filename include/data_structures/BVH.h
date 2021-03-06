#ifndef BVH_H
#define BVH_H

#include <vector>

#include "physics/Intersection.h"
#include "physics/RigidBody.h"
#include "geometry/BoundingBox.h"
#include "geometry/Sphere.h"
#include "geometry/Ray.h"

struct BVHNode {
  BoundingBox box;
  BVHNode* left;
  BVHNode* right;
  std::vector<RigidBody*> objects;

  BVHNode (const std::vector<RigidBody*>& newObjects, int level=0);

  void getAllBoxes (std::vector<BoundingBox>& allBoxes) const;
  void getAllBoxesDebug (std::vector<BoundingBox>& allBoxes,
                         std::vector<bool>& isleft) const;

  bool getIntersection (const Sphere& obj, Intersection& isect) const;
  bool getIntersection (const Ray& ray, Intersection& isect) const;

  ~BVHNode () {
    if (left != NULL)
      delete left;
    if (right != NULL)
      delete right;
  }
};

#endif
