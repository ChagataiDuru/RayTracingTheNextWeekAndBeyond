// obj_loader.h
#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "rt.h"
#include "hittable.h"
#include "material.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

class Triangle : public hittable {
public:
    Triangle(const point3& v0, const point3& v1, const point3& v2, shared_ptr<material> mat)
        : vertex0(v0), vertex1(v1), vertex2(v2), mat_ptr(mat) {
        normal = unit_vector(cross(vertex1 - vertex0, vertex2 - vertex0));
    }

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Implement Möller–Trumbore intersection algorithm
        vec3 edge1 = vertex1 - vertex0;
        vec3 edge2 = vertex2 - vertex0;
        vec3 h = cross(r.direction(), edge2);
        double a = dot(edge1, h);

        if (a > -1e-8 && a < 1e-8)
            return false;

        double f = 1.0 / a;
        vec3 s = r.origin() - vertex0;
        double u = f * dot(s, h);

        if (u < 0.0 || u > 1.0)
            return false;

        vec3 q = cross(s, edge1);
        double v = f * dot(r.direction(), q);

        if (v < 0.0 || u + v > 1.0)
            return false;

        double t = f * dot(edge2, q);

        if (t > ray_t.min && t < ray_t.max) {
            rec.t = t;
            rec.p = r.rayPos(t);
            rec.set_face_normal(r, normal);
            rec.mat = mat_ptr;
            return true;
        }

        return false;
    }

    aabb bounding_box() const override {
		point3 min = point3(
			std::min({ vertex0.x(), vertex1.x(), vertex2.x() }),
			std::min({ vertex0.y(), vertex1.y(), vertex2.y() }),
			std::min({ vertex0.z(), vertex1.z(), vertex2.z() })
		);

		point3 max = point3(
			std::max({ vertex0.x(), vertex1.x(), vertex2.x() }),
			std::max({ vertex0.y(), vertex1.y(), vertex2.y() }),
			std::max({ vertex0.z(), vertex1.z(), vertex2.z() })
		);

		return aabb(min, max);
	}

    void update(double time) override {};

private:
    point3 vertex0, vertex1, vertex2;
    vec3 normal;
    shared_ptr<material> mat_ptr;
};

class OBJLoader {
public:
    static std::vector<shared_ptr<hittable>> load_obj(const std::string& filename, shared_ptr<material> mat) {
        std::vector<shared_ptr<hittable>> triangles;
        std::vector<vec3> vertices;
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return triangles;
        }

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "v") {
                double x, y, z;
                iss >> x >> y >> z;
                vertices.emplace_back(x, y, z);
            }
            else if (type == "f") {
                int v1, v2, v3;
                iss >> v1 >> v2 >> v3;
                triangles.push_back(make_shared<Triangle>(
                    vertices[v1 - 1], vertices[v2 - 1], vertices[v3 - 1], mat
                ));
            }
        }

        return triangles;
    }
};

#endif // OBJ_LOADER_H