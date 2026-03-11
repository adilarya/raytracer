#ifndef SCENE_H
#define SCENE_H

#include "geometry/object_list.h"
#include "geometry/sphere.h"
#include "geometry/cylinder.h"
#include "geometry/ellipsoid.h"
#include "geometry/cone.h"
#include "geometry/triangle.h"

#include "core/material.h"
#include "core/light.h"
#include "core/camera.h"
#include "core/texture.h"

#include "math/vec3.h"
#include "math/point2.h"

#include <vector>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <memory>

template <typename T>
class Scene {
    public:

        // attributes
        Camera<T> camera;
        ObjectList<T> objects;
        Vec3<T> bkgcolor;
        Material<T> current_material;
        std::vector<Light<T>> lights;
        std::vector<Point3<T>> vertices; // for triangles
        std::vector<Vec3<T>> normals; // for triangles
        std::vector<Point2<T>> texture_coords; 
        std::vector<Texture<T>> textures; 
        int current_texture_idx = -1;

        // depth cueing parameters
        bool depth_cueing_enabled = false; // useful in ShadeRay for checking if we need to apply depth cueing
        Vec3<T> dc = Vec3<T>(0, 0, 0); // depth cue color
        T alpha_min = 0;
        T alpha_max = 1;
        T dist_min = 0; 
        T dist_max = 1;

        // constructors
        Scene() : camera(), objects(), bkgcolor(T(0), T(0), T(0)), current_material(), lights(), vertices(), normals(), texture_coords(), textures(), current_texture_idx(-1) {}

        // methods
        void add_obj(std::shared_ptr<Object<T>> obj) {
            objects.add(obj);
        }
        void add_light(const Light<T>& light) {
            lights.push_back(light);
        }
        void add_vertex(const Point3<T>& vertex) {
            vertices.push_back(vertex);
        }
        void add_normal(const Vec3<T>& normal) {
            normals.push_back(normal);
        }
        void add_texture_coord(const Point2<T>& tex_coord) {
            texture_coords.push_back(tex_coord);
        }
        void add_texture(const Texture<T>& texture) {
            textures.push_back(texture);
        }

        // general parsing method
        bool parse(const std::string& filename) {
            objects.clear(); // clear any existing objects in the scene
            lights.clear();  // clear any existing lights in the scene
            vertices.clear(); // clear any existing vertices in the scene
            normals.clear(); // clear any existing normals in the scene
            texture_coords.clear(); // clear any existing texture coordinates
            textures.clear(); // clear any existing textures in the scene
            current_texture_idx = -1; // reset current texture index

            FILE* file = fopen(filename.c_str(), "r");
            if (file == NULL) {
                std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
                return false;
            }

            // getting variables to build scene

            // for camera
            Point3<T> eye;
            Vec3<T> viewdir;
            Vec3<T> updir;
            T vfov = T(90); // default value, will be overridden if specified in file
            int width, height;

            T frustum_height = T(0); // only used if parallel is specified

            // flags to check if required parameters are set
            bool eye_set = false;
            bool viewdir_set = false;
            bool updir_set = false;
            bool vfov_set = false;
            bool imsize_set = false;
            bool bkgcolor_set = false;
            bool mtlcolor_set = false;
            bool parallel_set = false;

            char line[256]; // max line length

            while(fgets(line, sizeof(line), file)) {
                char keyword[20];

                if (sscanf(line, "%19s", keyword) != 1) { // catches blank line and EOF
                    continue;
                }

                if (keyword[0] == '#') { 
                    continue; // skip comments
                } else if (strcmp(keyword, "eye") == 0) {
                    float ex, ey, ez;
                    if (sscanf(line, "%*s %f %f %f", &ex, &ey, &ez) < 3) {
                        printf("[ERROR] Invalid eye parameters.\n");
                        fclose(file);
                        return false;
                    }
                    eye = Point3<T>(static_cast<T>(ex), static_cast<T>(ey), static_cast<T>(ez));
                    eye_set = true;
                } else if (strcmp(keyword, "viewdir") == 0) {
                    float vx, vy, vz;
                    if (sscanf(line, "%*s %f %f %f", &vx, &vy, &vz) < 3) {
                        printf("[ERROR] Invalid viewdir parameters.\n");
                        fclose(file);
                        return false;
                    }
                    viewdir = Vec3<T>(static_cast<T>(vx), static_cast<T>(vy), static_cast<T>(vz));
                    viewdir_set = true;
                } else if (strcmp(keyword, "updir") == 0) {
                    float ux, uy, uz;
                    if (sscanf(line, "%*s %f %f %f", &ux, &uy, &uz) < 3) {
                        printf("[ERROR] Invalid updir parameters.\n");
                        fclose(file);
                        return false;
                    }
                    updir = Vec3<T>(static_cast<T>(ux), static_cast<T>(uy), static_cast<T>(uz));
                    updir_set = true;
                } else if (strcmp(keyword, "vfov") == 0) {
                    float vf;
                    if (sscanf(line, "%*s %f", &vf) < 1) {
                        printf("[ERROR] Invalid vfov parameters.\n");
                        fclose(file);
                        return false;
                    }

                    // vfov range
                    if (vf <= 0.0f || vf >= 180.0f) {
                        printf("[ERROR] vfov must be in (0, 180).\n");
                        fclose(file);
                        return false;
                    }

                    vfov = static_cast<T>(vf);
                    vfov_set = true;
                } else if (strcmp(keyword, "parallel") == 0) {
                    float fh;
                    if (sscanf(line, "%*s %f", &fh) < 1) {
                        printf("[ERROR] Invalid parallel parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (fh <= 0.0f) {
                        printf("[ERROR] Frustum height must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    frustum_height = static_cast<T>(fh);
                    parallel_set = true;
                } else if (strcmp(keyword, "imsize") == 0) {
                    if (sscanf(line, "%*s %d %d", &width, &height) < 2) {
                        printf("[ERROR] Invalid imsize parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (width <= 0 || height <= 0) {
                        printf("[ERROR] Image dimensions must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    imsize_set = true;
                } else if (strcmp(keyword, "bkgcolor") == 0) {
                    float bx, by, bz;
                    if (sscanf(line, "%*s %f %f %f", &bx, &by, &bz) < 3) {
                        printf("[ERROR] Invalid bkgcolor parameters.\n");
                        fclose(file);
                        return false;
                    }

                    // bkgcolor range
                    if (bx < 0.0f || bx > 1.0f ||
                        by < 0.0f || by > 1.0f ||
                        bz < 0.0f || bz > 1.0f) {
                        printf("[ERROR] bkgcolor values must be in [0, 1].\n");
                        fclose(file);
                        return false;
                    }

                    bkgcolor = Vec3<T>(static_cast<T>(bx), static_cast<T>(by), static_cast<T>(bz));
                    bkgcolor_set = true;
                } else if (strcmp(keyword, "light") == 0) {
                    float lx, ly, lz, intensity;
                    int ltype;
                    if (sscanf(line, "%*s %f %f %f %d %f", &lx, &ly, &lz, &ltype, &intensity) < 5) {
                        printf("[ERROR] Invalid light parameters.\n");
                        fclose(file);
                        return false;
                    }
                    
                    // checking params before adding light
                    if (ltype != 0 && ltype != 1) {
                        printf("[ERROR] Invalid light type. Must be 0 (directional) or 1 (point).\n");
                        fclose(file);
                        return false;
                    }
                    if (intensity < 0.0f) {
                        printf("[ERROR] Light intensity must be non-negative.\n");
                        fclose(file);
                        return false;
                    }

                    Light<T> new_light;
                    new_light.direction = Vec3<T>(static_cast<T>(lx), static_cast<T>(ly), static_cast<T>(lz));
                    new_light.type = ltype != 0; // treat any non-zero as point light
                    new_light.intensity = static_cast<T>(intensity);
                    add_light(new_light);
                } else if (strcmp(keyword, "attlight") == 0) {
                    float lx, ly, lz, intensity, lc1, lc2, lc3;
                    int ltype;
                    if (sscanf(line, "%*s %f %f %f %d %f %f %f %f", 
                        &lx, &ly, &lz, &ltype, &intensity, &lc1, &lc2, &lc3) < 8) {
                        printf("[ERROR] Invalid attlight parameters.\n");
                        fclose(file);
                        return false;
                    }

                    // checking params before adding light
                    if (ltype != 0 && ltype != 1) {
                        printf("[ERROR] Invalid light type. Must be 0 (directional) or 1 (point).\n");
                        fclose(file);
                        return false;
                    }
                    if (intensity < 0.0f) {
                        printf("[ERROR] Light intensity must be non-negative.\n");
                        fclose(file);
                        return false;
                    }

                    if (lc1 < 0.0f || lc2 < 0.0f || lc3 < 0.0f) {
                        printf("[ERROR] Light attenuation coefficients must be non-negative.\n");
                        fclose(file);
                        return false;
                    }

                    if (lc1 == 0.0f && lc2 == 0.0f && lc3 == 0.0f) {
                        printf("[ERROR] At least one attenuation coefficient must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    Vec3<T> dir = Vec3<T>(static_cast<T>(lx), static_cast<T>(ly), static_cast<T>(lz));
                    bool t = ltype != 0; 
                    Light<T> new_light(dir, t, static_cast<T>(intensity), static_cast<T>(lc1), static_cast<T>(lc2), static_cast<T>(lc3));
                    add_light(new_light);
                    
                } else if (strcmp(keyword, "depthcueing") == 0){
                    float dcr, dcg, dcb, amin, amax, distmin, distmax;
                    if (sscanf(line, "%*s %f %f %f %f %f %f %f", 
                        &dcr, &dcg, &dcb, 
                        &amin, &amax, 
                        &distmin, &distmax) < 7) {
                        printf("[ERROR] Invalid depthcueing parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (dcr < 0.0f || dcr > 1.0f ||
                        dcg < 0.0f || dcg > 1.0f ||
                        dcb < 0.0f || dcb > 1.0f) {
                        printf("[ERROR] Depth cueing color values must be in [0, 1].\n");
                        fclose(file);
                        return false;
                    }

                    if (amin < 0.0f || amin > 1.0f ||
                        amax < 0.0f || amax > 1.0f ||
                        distmin < 0.0f || distmax < 0.0f) {
                        printf("[ERROR] Depth cueing depth and distance values must be non-negative.\n");
                        fclose(file);
                        return false;
                    }

                    if (amin > amax) {
                        printf("[ERROR] Depth cueing depth min must be less than or equal to depth max.\n");
                        fclose(file);
                        return false;
                    }

                    if (distmin >= distmax) {
                        printf("[ERROR] Depth cueing distance min must be less than distance max.\n");
                        fclose(file);
                        return false;
                    }

                    Vec3<T> depth_cue = Vec3<T>(static_cast<T>(dcr), static_cast<T>(dcg), static_cast<T>(dcb));
                    alpha_min = static_cast<T>(amin);
                    alpha_max = static_cast<T>(amax);
                    dist_min = static_cast<T>(distmin);
                    dist_max = static_cast<T>(distmax);
                    dc = depth_cue;
                    depth_cueing_enabled = true;
                } else if (strcmp(keyword, "mtlcolor") == 0) {
                    float Odx, Ody, Odz, Osx, Osy, Osz, ka, kd, ks, n;
                    if (sscanf(line, "%*s %f %f %f %f %f %f %f %f %f %f", 
                        &Odx, &Ody, &Odz, 
                        &Osx, &Osy, &Osz, 
                        &ka, &kd, &ks, 
                        &n) < 10) {
                        printf("[ERROR] Invalid mtlcolor parameters.\n");
                        fclose(file);
                        return false;
                    }

                    Material<T> current;
                    current.Od = Vec3<T>(static_cast<T>(Odx), static_cast<T>(Ody), static_cast<T>(Odz));
                    current.Os = Vec3<T>(static_cast<T>(Osx), static_cast<T>(Osy), static_cast<T>(Osz));
                    current.ka = static_cast<T>(ka);
                    current.kd = static_cast<T>(kd);
                    current.ks = static_cast<T>(ks);
                    current.n = static_cast<T>(n);

                    // need to check if mtlcolor values are in [0, 1]
                    if (current.Od.x < T(0.0f) || current.Od.x > T(1.0f) ||
                        current.Od.y < T(0.0f) || current.Od.y > T(1.0f) ||
                        current.Od.z < T(0.0f) || current.Od.z > T(1.0f) ||
                        current.Os.x < T(0.0f) || current.Os.x > T(1.0f) ||
                        current.Os.y < T(0.0f) || current.Os.y > T(1.0f) ||
                        current.Os.z < T(0.0f) || current.Os.z > T(1.0f) ||
                        current.ka < T(0.0f) || current.ka > T(1.0f) ||
                        current.kd < T(0.0f) || current.kd > T(1.0f) ||
                        current.ks < T(0.0f) || current.ks > T(1.0f) || 
                        current.n < T(0.0f) || current.n > T(128.0f)) { // commonly used range for shininess is [0, 128]    
                        printf("[ERROR] mtlcolor values must be in [0, 1], shininess must be in [0, 128].\n");
                        fclose(file);
                        return false;
                    }

                    mtlcolor_set = true;
                    current_material = current; // update current material
                } else if (strcmp(keyword, "sphere") == 0) {
                    if (!mtlcolor_set) {
                        printf("[ERROR] Material properties must be defined before objects.\n");
                        fclose(file);
                        return false;
                    }
                    Point3<T> center;
                    T radius;

                    float cx, cy, cz, r;
                    if (sscanf(line, "%*s %f %f %f %f", &cx, &cy, &cz, &r) < 4) {
                        printf("[ERROR] Invalid sphere parameters.\n");
                        fclose(file);
                        return false;
                    }
                    center = Point3<T>(static_cast<T>(cx), static_cast<T>(cy), static_cast<T>(cz));
                    radius = static_cast<T>(r);
                    // checking radius
                    if (radius <= T(0.0)) {
                        printf("[ERROR] Sphere radius must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    if (current_texture_idx >= 0) {
                        add_obj(std::make_shared<Sphere<T>>(center, radius, current_material, current_texture_idx));
                    } else {
                        add_obj(std::make_shared<Sphere<T>>(center, radius, current_material));
                    }
                } else if (strcmp(keyword, "cylinder") == 0) {
                    if (!mtlcolor_set) {
                        printf("[ERROR] Material properties must be defined before objects.\n");
                        fclose(file);
                        return false;
                    }
                    
                    float cx, cy, cz, dx, dy, dz, r, l;
                    if (sscanf(line, "%*s %f %f %f %f %f %f %f %f", 
                        &cx, &cy, &cz, 
                        &dx, &dy, &dz, 
                        &r, &l) < 8) {
                        printf("[ERROR] Invalid cylinder parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (r <= 0.0f) {
                        printf("[ERROR] Cylinder radius must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    if (l <= 0.0f) {
                        printf("[ERROR] Cylinder length must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    if (dx == 0.0f && dy == 0.0f && dz == 0.0f) {
                        printf("[ERROR] Cylinder direction cannot be zero vector.\n");
                        fclose(file);
                        return false;
                    }
                    
                    Point3<T> center = Point3<T>(static_cast<T>(cx), static_cast<T>(cy), static_cast<T>(cz));
                    Vec3<T> direction = Vec3<T>(static_cast<T>(dx), static_cast<T>(dy), static_cast<T>(dz));
                    T radius = static_cast<T>(r);
                    T length = static_cast<T>(l);

                    if (current_texture_idx >= 0) {
                        add_obj(std::make_shared<Cylinder<T>>(center, direction, radius, length, current_material, current_texture_idx));
                    } else {
                        add_obj(std::make_shared<Cylinder<T>>(center, direction, radius, length, current_material));
                    }
                } else if (strcmp(keyword, "ellipsoid") == 0) {
                    if (!mtlcolor_set) {
                        printf("[ERROR] Material properties must be defined before objects.\n");
                        fclose(file);
                        return false;
                    }

                    float cx, cy, cz, rx, ry, rz;
                    if (sscanf(line, "%*s %f %f %f %f %f %f", 
                        &cx, &cy, &cz,
                        &rx, &ry, &rz) < 6) {
                        printf("[ERROR] Invalid ellipsoid parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (rx <= 0.0f || ry <= 0.0f || rz <= 0.0f) {
                        printf("[ERROR] Ellipsoid radii must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    Point3<T> center = Point3<T>(static_cast<T>(cx), static_cast<T>(cy), static_cast<T>(cz));
                    Vec3<T> radii = Vec3<T>(static_cast<T>(rx), static_cast<T>(ry), static_cast<T>(rz));
                    add_obj(std::make_shared<Ellipsoid<T>>(center, radii, current_material));
                } else if (strcmp(keyword, "cone") == 0) {
                    if (!mtlcolor_set) {
                        printf("[ERROR] Material properties must be defined before objects.\n");
                        fclose(file);
                        return false;
                    }

                    float tx, ty, tz, dx, dy, dz, angle, height;
                    if (sscanf(line, "%*s %f %f %f %f %f %f %f %f", 
                        &tx, &ty, &tz,
                        &dx, &dy, &dz,
                        &angle, &height) < 8) {
                        printf("[ERROR] Invalid cone parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (angle <= 0.0f || angle >= 90.0f) {
                        printf("[ERROR] Cone angle must be in (0, 90).\n");
                        fclose(file);
                        return false;
                    }

                    if (height <= 0.0f) {
                        printf("[ERROR] Cone height must be positive.\n");
                        fclose(file);
                        return false;
                    }

                    if (dx == 0.0f && dy == 0.0f && dz == 0.0f) {
                        printf("[ERROR] Cone direction cannot be zero vector.\n");
                        fclose(file);
                        return false;
                    }

                    Point3<T> tip = Point3<T>(static_cast<T>(tx), static_cast<T>(ty), static_cast<T>(tz));
                    Vec3<T> direction = Vec3<T>(static_cast<T>(dx), static_cast<T>(dy), static_cast<T>(dz));
                    T cone_angle = static_cast<T>(T(angle) * std::acos(T(-1)) / T(180)); // convert to radians
                    T cone_height = static_cast<T>(height);
                    add_obj(std::make_shared<Cone<T>>(tip, direction, cone_angle, cone_height, current_material));
                } else if (strcmp(keyword, "v") == 0) {
                    float vx, vy, vz;
                    if (sscanf(line, "%*s %f %f %f", &vx, &vy, &vz) < 3) {
                        printf("[ERROR] Invalid vertex parameters.\n");
                        fclose(file);
                        return false;
                    }
                    Point3<T> vertex = Point3<T>(static_cast<T>(vx), static_cast<T>(vy), static_cast<T>(vz));
                    add_vertex(vertex);
                } else if (strcmp(keyword, "vn") == 0) {
                    float nx, ny, nz;
                    if (sscanf(line, "%*s %f %f %f", &nx, &ny, &nz) < 3) {
                        printf("[ERROR] Invalid normal parameters.\n");
                        fclose(file);
                        return false;
                    }
                    Vec3<T> normal = Vec3<T>(static_cast<T>(nx), static_cast<T>(ny), static_cast<T>(nz));
                    add_normal(normal.normalize()); // normalize the normal vector before storing
                } else if (strcmp(keyword, "vt") == 0) {
                    float u, v;
                    if (sscanf(line, "%*s %f %f", &u, &v) < 2) {
                        printf("[ERROR] Invalid texture coordinate parameters.\n");
                        fclose(file);
                        return false;
                    }
                    Point2<T> tex_coord = Point2<T>(static_cast<T>(u), static_cast<T>(v));
                    add_texture_coord(tex_coord);
                } else if (strcmp(keyword, "texture") == 0) {
                    char filename[100];
                    if (sscanf(line, "%*s %99s", filename) < 1) {
                        printf("[ERROR] Invalid texture parameters.\n");
                        fclose(file);
                        return false;
                    }
                    Texture<T> new_texture;
                    if (!new_texture.load(filename)) {
                        printf("[ERROR] Failed to load texture from file: %s\n", filename);
                        fclose(file);
                        return false;
                    }
                    add_texture(new_texture);
                    current_texture_idx++; // update current texture index to the newly added texture
                } else if (strcmp(keyword, "f") == 0) {
                    if (!mtlcolor_set) {
                        printf("[ERROR] Material properties must be defined before objects.\n");
                        fclose(file);
                        return false;
                    }

                    char token1[20], token2[20], token3[20];
                    if (sscanf(line, "%*s %19s %19s %19s", token1, token2, token3) < 3) {
                        printf("[ERROR] Invalid face parameters.\n");
                        fclose(file);
                        return false;
                    }

                    if (strstr(token1, "//") != NULL || strstr(token2, "//") != NULL || strstr(token3, "//") != NULL) {
                        if (strstr(token1, "//") == NULL || strstr(token2, "//") == NULL || strstr(token3, "//") == NULL) {
                            printf("[ERROR] Inconsistent face format. All vertices must have normals or none should have normals.\n");
                            fclose(file);
                            return false;
                        }

                        int v1, v2, v3;
                        int vn1, vn2, vn3;

                        if (sscanf(token1, "%d//%d", &v1, &vn1) < 2 ||
                            sscanf(token2, "%d//%d", &v2, &vn2) < 2 ||
                            sscanf(token3, "%d//%d", &v3, &vn3) < 2) {
                            printf("[ERROR] Invalid face parameters. Expected format: f v//vn v//vn v//vn\n");
                            fclose(file);
                            return false;
                        }

                        // check if vertex and normal indices are valid
                        if (v1 <= 0 || v2 <= 0 || v3 <= 0 || 
                            vn1 <= 0 || vn2 <= 0 || vn3 <= 0 || 
                            v1 > static_cast<int>(vertices.size()) || 
                            v2 > static_cast<int>(vertices.size()) || 
                            v3 > static_cast<int>(vertices.size()) || 
                            vn1 > static_cast<int>(normals.size()) || 
                            vn2 > static_cast<int>(normals.size()) || 
                            vn3 > static_cast<int>(normals.size())) {
                            printf("[ERROR] Face vertex or normal index out of bounds.\n");
                            fclose(file);
                            return false;
                        }

                        Point3<T> vert1 = vertices[v1 - 1]; // convert to 0-based index
                        Point3<T> vert2 = vertices[v2 - 1];
                        Point3<T> vert3 = vertices[v3 - 1];

                        Vec3<T> norm1 = normals[vn1 - 1];
                        Vec3<T> norm2 = normals[vn2 - 1];
                        Vec3<T> norm3 = normals[vn3 - 1];

                        add_obj(std::make_shared<Triangle<T>>(vert1, vert2, vert3, norm1, norm2, norm3, current_material));
                    } else if (strchr(token1, '/') != NULL || strchr(token2, '/') != NULL || strchr(token3, '/') != NULL) {
                        if (strchr(token1, '/') == NULL || strchr(token2, '/') == NULL || strchr(token3, '/') == NULL) {
                            printf("[ERROR] Inconsistent face format. All vertices must have texture coordinates or none should have texture coordinates.\n");
                            fclose(file);
                            return false;
                        }

                        // deciding whether format is v/vt or v/vt/vn based on occurrences of '/' 
                        int count = 0;
                        for (char* c = token1; *c != '\0'; c++) {
                            if (*c == '/') count++;
                        }

                        // if count is 2, format is v/vt/vn, if count is 1, format is v/vt
                        if (count == 2) {
                            int v1, v2, v3;
                            int vt1, vt2, vt3;
                            int vn1, vn2, vn3;

                            if (sscanf(token1, "%d/%d/%d", &v1, &vt1, &vn1) < 3 ||
                                sscanf(token2, "%d/%d/%d", &v2, &vt2, &vn2) < 3 ||
                                sscanf(token3, "%d/%d/%d", &v3, &vt3, &vn3) < 3) {
                                printf("[ERROR] Invalid face parameters. Expected format: f v/vt/vn v/vt/vn v/vt/vn\n");
                                fclose(file);
                                return false;
                            }

                            if (v1 <= 0 || v2 <= 0 || v3 <= 0 || 
                                vt1 <= 0 || vt2 <= 0 || vt3 <= 0 || 
                                vn1 <= 0 || vn2 <= 0 || vn3 <= 0 ||
                                v1 > static_cast<int>(vertices.size()) || 
                                v2 > static_cast<int>(vertices.size()) || 
                                v3 > static_cast<int>(vertices.size()) || 
                                vt1 > static_cast<int>(texture_coords.size()) || 
                                vt2 > static_cast<int>(texture_coords.size()) || 
                                vt3 > static_cast<int>(texture_coords.size()) ||
                                vn1 > static_cast<int>(normals.size()) ||
                                vn2 > static_cast<int>(normals.size()) ||
                                vn3 > static_cast<int>(normals.size())) {
                                printf("[ERROR] Face vertex, texture coordinate, or normal index out of bounds.\n");
                                fclose(file);
                                return false;
                            }

                            Point3<T> vert1 = vertices[v1 - 1]; // convert to 0-based index
                            Point3<T> vert2 = vertices[v2 - 1];
                            Point3<T> vert3 = vertices[v3 - 1];

                            Point2<T> tex1 = texture_coords[vt1 - 1];
                            Point2<T> tex2 = texture_coords[vt2 - 1];
                            Point2<T> tex3 = texture_coords[vt3 - 1];

                            Vec3<T> norm1 = normals[vn1 - 1];
                            Vec3<T> norm2 = normals[vn2 - 1];
                            Vec3<T> norm3 = normals[vn3 - 1];

                            if (current_texture_idx < 0) {
                                printf("[ERROR] Texture must be defined before faces that use textures.\n");
                                fclose(file);
                                return false;
                            }

                            add_obj(std::make_shared<Triangle<T>>(vert1, vert2, vert3, norm1, norm2, norm3, tex1, tex2, tex3, current_material, current_texture_idx));
                        } else {
                            int v1, v2, v3;
                            int vt1, vt2, vt3;
                            
                            if (sscanf(token1, "%d/%d", &v1, &vt1) < 2 ||
                                sscanf(token2, "%d/%d", &v2, &vt2) < 2 ||
                                sscanf(token3, "%d/%d", &v3, &vt3) < 2) {
                                printf("[ERROR] Invalid face parameters. Expected format: f v/vt v/vt v/vt\n");
                                fclose(file);
                                return false;
                            }

                            // check if vertex and texture coordinate indices are valid
                            if (v1 <= 0 || v2 <= 0 || v3 <= 0 || 
                                vt1 <= 0 || vt2 <= 0 || vt3 <= 0 || 
                                v1 > static_cast<int>(vertices.size()) || 
                                v2 > static_cast<int>(vertices.size()) || 
                                v3 > static_cast<int>(vertices.size()) || 
                                vt1 > static_cast<int>(texture_coords.size()) || 
                                vt2 > static_cast<int>(texture_coords.size()) || 
                                vt3 > static_cast<int>(texture_coords.size())) {
                                printf("[ERROR] Face vertex or texture coordinate index out of bounds.\n");
                                fclose(file);
                                return false;
                            }

                            Point3<T> vert1 = vertices[v1 - 1]; // convert to 0-based index
                            Point3<T> vert2 = vertices[v2 - 1];
                            Point3<T> vert3 = vertices[v3 - 1];

                            Point2<T> tex1 = texture_coords[vt1 - 1];
                            Point2<T> tex2 = texture_coords[vt2 - 1];
                            Point2<T> tex3 = texture_coords[vt3 - 1];

                            if (current_texture_idx < 0) {
                                printf("[ERROR] Texture must be defined before faces that use textures.\n");
                                fclose(file);
                                return false;
                            }

                            add_obj(std::make_shared<Triangle<T>>(vert1, vert2, vert3, tex1, tex2, tex3, current_material, current_texture_idx));
                        }
                    } else {
                        int v1, v2, v3;
                        if (sscanf(token1, "%d", &v1) < 1 ||
                            sscanf(token2, "%d", &v2) < 1 ||
                            sscanf(token3, "%d", &v3) < 1) {
                            printf("[ERROR] Invalid face parameters. Expected format: f v v v\n");
                            fclose(file);
                            return false;
                        }

                        // check if vertex indices are valid
                        if (v1 <= 0 || v2 <= 0 || v3 <= 0 || 
                            v1 > static_cast<int>(vertices.size()) || 
                            v2 > static_cast<int>(vertices.size()) || 
                            v3 > static_cast<int>(vertices.size())) {
                            printf("[ERROR] Face vertex index out of bounds.\n");
                            fclose(file);
                            return false;
                        }

                        Point3<T> vert1 = vertices[v1 - 1]; // convert to 0-based index
                        Point3<T> vert2 = vertices[v2 - 1];
                        Point3<T> vert3 = vertices[v3 - 1];

                        add_obj(std::make_shared<Triangle<T>>(vert1, vert2, vert3, current_material));
                    }


                } else {
                    printf("[WARNING] Unknown keyword: %s\n", keyword);
                }
            }

            fclose(file);

            if (!eye_set) {
                printf("[ERROR] Missing eye parameters.\n");
                return false;
            } else if (!viewdir_set) {
                printf("[ERROR] Missing viewdir parameters.\n");
                return false;
            } else if (!updir_set) {
                printf("[ERROR] Missing updir parameters.\n");
                return false;
            } else if (!vfov_set && !parallel_set) { // parallel overrides if both are set, but at least one must be set
                printf("[ERROR] Missing BOTH vfov and parallel parameters.\n");
                return false;
            } else if (!imsize_set) {
                printf("[ERROR] Missing imsize parameters.\n");
                return false;
            } else if (!bkgcolor_set) {
                printf("[ERROR] Missing bkgcolor parameters.\n");
                return false;
            } 

            // making the camera after parsing all params
            camera = Camera<T>(eye, viewdir, updir, vfov, width, height, parallel_set, frustum_height);

            // checking if params are valid 
            T eps = T(1e-6);

            // checking if viewdir and updir are non-zero vectors
            T viewdir_len = camera.viewdir.length();
            T updir_len = camera.updir.length();
            if (viewdir_len < eps) {
                printf("[ERROR] viewdir cannot be zero vector.\n");
                return false;
            }
            if (updir_len < eps) {
                printf("[ERROR] updir cannot be zero vector.\n");
                return false;
            }

            // collinearity of viewdir and updir
            Vec3<T> cp = camera.viewdir.cross(camera.updir);
            if (cp.length() < eps) {
                printf("[ERROR] viewdir and updir cannot be collinear.\n");
                return false;
            }

            return true;
        }
};

#endif
