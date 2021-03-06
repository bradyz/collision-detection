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

const glm::vec3 GRAVITY(0.0, -9.8, 0.0);
const size_t CELLS = 100;

const glm::mat4 I;

Sphere* grid[CELLS][CELLS];
vector<Spring*> springs[CELLS][CELLS];

PhongProgram phongP(&view_matrix, &projection_matrix);
ShadowProgram shadowP(&view_matrix, &projection_matrix);
LineSegmentProgram lineP(&view_matrix, &projection_matrix);
WireProgram wireP(&view_matrix, &projection_matrix);

vector<Sphere*> spheres;
vector<Plane> planes;
vector<RigidBody*> rigid_bodies;

vector<glm::vec4> sphere_vertices;
vector<glm::uvec3> sphere_faces;
vector<glm::vec4> sphere_normals;

vector<glm::vec4> cloth_vertices;
vector<glm::uvec3> cloth_faces;
vector<glm::vec4> cloth_normals;

double max_stretch = 0.0;

void setupOpengl() {
  initOpenGL();

  phongP.setup();
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

      if (i + 1 < CELLS && j + 1 < CELLS) {
        int a = i * CELLS + j;
        int b = i * CELLS + j + 1;
        int c = (i + 1) * CELLS + j + 1;
        int d = (i + 1) * CELLS + j;
        cloth_faces.push_back(glm::vec3(a, b, c));
        cloth_faces.push_back(glm::vec3(a, c, d));
      }
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (int k = 0; k < 8; k++) {
        int x = i + DX[k];
        int y = j + DY[k];
        if (x < 0 || x >= CELLS || y < 0 || y >= CELLS)
          continue;
        springs[i][j].push_back(new Spring(*grid[i][j], *grid[x][y]));

        x = i + 2 * DX[k];
        y = j + 2 * DY[k];
        if (x < 0 || x >= CELLS || y < 0 || y >= CELLS)
          continue;
      }
    }
  }

  for (Sphere *s : spheres) {
    rigid_bodies.push_back((RigidBody*)s);
  }
}

void cloth() {
  // Rendering stuff.
  lineP.drawAxis();

  if (!showWire) {
    cloth_vertices.clear();
    cloth_normals.clear();
    for (int i = 0; i < CELLS; i++)
      for (int j = 0; j < CELLS; j++)
        cloth_vertices.push_back(glm::vec4(grid[i][j]->position, 1.0));
    cloth_normals = getVertexNormals(cloth_vertices, cloth_faces);
    phongP.draw(cloth_vertices, cloth_faces, cloth_normals,
                I, glm::vec4(1.0, 0.0, 0.0, 1.0), glm::vec4(eye, 1.0f));
  }

  for (const Sphere *sphere : spheres) {
    if (showWire) {
      glm::mat4 toWorld = sphere->toWorld();
      // phongP.draw(sphere_vertices, sphere_faces, sphere_normals,
      //             toWorld, sphere->color, glm::vec4(eye, 1.0f));
      // shadowP.draw(sphere_vertices, sphere_faces, toWorld);
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (Spring* spring : springs[i][j]) {
        double stretch = spring->getLength() - spring->rLength;
        max_stretch = max(max_stretch, abs(stretch));
      }
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (Spring* spring : springs[i][j]) {
        if (showWire) {
          double stretch = spring->getLength() - spring->rLength;
          lineP.drawLineSegment(spring->sphereA.position,
                                spring->sphereB.position,
                                jet(stretch / max_stretch));
        }
      }
    }
  }

  // Simulation stuff.
  vector<glm::vec3> forces;
  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      for (Spring* spring : springs[i][j]) {
        if (!timePaused)
          spring->step();
      }
    }
  }

  for (int i = 0; i < CELLS; i++) {
    for (int j = 0; j < CELLS; j++) {
      if (j == 0 || j == (CELLS - 1))
        continue;
      // if ((i == 0 || i == CELLS - 1) && (j == 0 || j == CELLS - 1))
      //   continue;
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
