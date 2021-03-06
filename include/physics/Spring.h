#ifndef SPRING_H
#define SPRING_H

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "geometry/Sphere.h"

struct Spring {
  Sphere& sphereA;
  Sphere& sphereB;
  float kHook;
  float kDamp;
  float rLength;

  Spring (Sphere& sa, Sphere& sb, double k=10.0, double l=-1.0, double d=0.9) :
    sphereA(sa), sphereB(sb), kHook(k), kDamp(d) {
      rLength = (l < 0.0) ? glm::length(sa.position - sb.position) : l;
  }

  double getLength() const;

  void step ();
};

#endif
