#ifndef SCENE_H
#define SCENE_H

#include "core/camera.h"
#include "geometry/object_list.h"
#include "core/material.h"
#include "core/light.h"
#include "geometry/sphere.h"
#include "math/vec3.h"

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

        // constructors
        Scene() : camera(), objects(), bkgcolor(T(0), T(0), T(0)), current_material(), lights() {}

        // methods
        void add_obj(std::shared_ptr<Object<T>> obj) {
            objects.add(obj);
        }
        void add_light(const Light<T>& light) {
            lights.push_back(light);
        }

        // general parsing method
        bool parse(const std::string& filename) {
            objects.clear(); // clear any existing objects in the scene
            lights.clear();  // clear any existing lights in the scene

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
            T vfov;
            int width, height;

            // flags to check if required parameters are set
            bool eye_set = false;
            bool viewdir_set = false;
            bool updir_set = false;
            bool vfov_set = false;
            bool imsize_set = false;
            bool bkgcolor_set = false;
            bool mtlcolor_set = false;

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
                    vfov = static_cast<T>(vf);
                    vfov_set = true;
                } else if (strcmp(keyword, "imsize") == 0) {
                    if (sscanf(line, "%*s %d %d", &width, &height) < 2) {
                        printf("[ERROR] Invalid imsize parameters.\n");
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
                        printf("[ERROR] mtlcolor values must be in [0, 1].\n");
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

                    add_obj(std::make_shared<Sphere<T>>(center, radius, current_material));
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
            } else if (!vfov_set) {
                printf("[ERROR] Missing vfov parameters.\n");
                return false;
            } else if (!imsize_set) {
                printf("[ERROR] Missing imsize parameters.\n");
                return false;
            } else if (!bkgcolor_set) {
                printf("[ERROR] Missing bkgcolor parameters.\n");
                return false;
            } 

            // making the camera after parsing all params
            camera = Camera<T>(eye, viewdir, updir, vfov, width, height);

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

            // vfov range
            if (vfov <= 0.0f || vfov >= 180.0f) {
                printf("[ERROR] vfov must be in (0, 180).\n");
                return false;
            }

            // imsize positive
            if (width <= 0 || height <= 0) {
                printf("[ERROR] imsize dimensions must be positive.\n");
                return false;
            }

            // bkgcolor range
            if (bkgcolor.x < 0.0f || bkgcolor.x > 1.0f ||
                bkgcolor.y < 0.0f || bkgcolor.y > 1.0f ||
                bkgcolor.z < 0.0f || bkgcolor.z > 1.0f) {
                printf("[ERROR] bkgcolor values must be in [0, 1].\n");
                return false;
            }


            return true;
        }
};

#endif
