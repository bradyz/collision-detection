#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <map>
#include <utility>

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "helpers/RandomUtils.h"

#include "data_structures/octree.h"

#include "geometry/Plane.h"
#include "geometry/Sphere.h"

#include "physics/Intersection.h"
#include "physics/Spring.h"

#include "render/Floor.h"
#include "render/LineSegment.h"
#include "render/OpenGLStuff.h"
#include "render/Phong.h"
#include "render/Shadow.h"
#include "render/Wire.h"

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

using namespace std;

const int DX[] = {-1, -1, -1,  0,  0,  1, 1, 1};
const int DY[] = {-1,  0,  1, -1,  1, -1, 0, 1};

const glm::vec4 FLOOR_COLOR(0.72, 0.60, 0.41, 1.0);
const glm::vec3 GRAVITY(0.0, -9.8, 0.0);
const size_t CELLS = 20;

const glm::mat4 I;

Sphere* grid[CELLS][CELLS];

PhongProgram phongP(&view_matrix, &projection_matrix);
FloorProgram floorP(&view_matrix, &projection_matrix);
ShadowProgram shadowP(&view_matrix, &projection_matrix);
LineSegmentProgram lineP(&view_matrix, &projection_matrix);
WireProgram wireP(&view_matrix, &projection_matrix);

vector<Sphere*> spheres;
vector<Plane> planes;
vector<RigidBody*> rigid_bodies;
vector<Spring*> springs;

vector<glm::vec4> sphere_vertices;
vector<glm::uvec3> sphere_faces;
vector<glm::vec4> sphere_normals;

void setupOpengl() {
  initOpenGL();

  phongP.setup();
  floorP.setup();
  shadowP.setup();
  lineP.setup();
  wireP.setup();
}

void setupCloth () {
  LoadOBJ("./obj/sphere.obj", sphere_vertices, sphere_faces, sphere_normals);
  fixSphereVertices(sphere_vertices);

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      double x = i * 0.1;
      double y = j * 0.1 + 1.0;
      grid[i][j] = new Sphere(0.01, glm::vec3(x, 2.0, y));
      spheres.push_back(grid[i][j]);
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (int k = 0; k < 8; k++) {
        int x = i + DX[k];
        int y = j + DY[k];
        if (x < 0 || x >= CELLS || y < 0 || y >= CELLS)
          continue;
        springs.push_back(new Spring(*grid[i][j], *grid[x][y]));
      }
    }
  }

  for (Sphere *s : spheres) {
    rigid_bodies.push_back((RigidBody*)s);
  }

  planes.push_back(Plane(glm::vec3(0.0, kFloorY-1e-3, 0.0)));
}

void cloth() {
  lineP.drawAxis();

  vector<glm::vec3> forces;

  for (Spring *spring : springs) {
    if (!timePaused)
      spring->step();
  }

  for (const Sphere *sphere : spheres) {
    glm::mat4 toWorld = sphere->toWorld();

    if (showWire) {
      wireP.draw(sphere_vertices, sphere_faces, toWorld, WHITE);
    }
    else {
      phongP.draw(sphere_vertices, sphere_faces, sphere_normals,
                  toWorld, sphere->color, glm::vec4(eye, 1.0f));
      shadowP.draw(sphere_vertices, sphere_faces, toWorld);
    }
  }

  for (const Plane& plane: planes) {
    if (showFloor && showWire == false) {
      phongP.draw(plane.vertices, plane.faces, plane.normals, I,
                  FLOOR_COLOR, glm::vec4(eye, 1.0f));
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (int k = 0; k < 8; k++) {
        int x = i + DX[k];
        int y = j + DY[k];
        if (x < 0 || x >= CELLS || y < 0 || y >= CELLS)
          continue;
        lineP.drawLineSegment(grid[i][j]->position,
                              grid[x][y]->position,
                              glm::vec4(0.0, 1.0, 0.0, 1.0));
      }

      if (j == 0 || j == CELLS-1)
        continue;
      Sphere *sphere = grid[i][j];
      if (!timePaused) {
        sphere->applyForce(GRAVITY * sphere->mass);
        sphere->step(forces);
      }
    }
  }
}

int main (int argc, char* argv[]) {
  ios_base::sync_with_stdio(false);

  setupOpengl();
  setupCloth();
  while (keepLoopingOpenGL()) {
    cloth();
    endLoopOpenGL();
  }
  cleanupOpenGL();
}
