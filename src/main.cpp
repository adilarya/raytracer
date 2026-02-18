#include "core/scene.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>   

template<typename T>
Vec3<T> ShadeRay(const Ray<T>& ray, const Scene<T>& scene) {
    T eps = T(1e-4);
    T inf = std::numeric_limits<T>::infinity();

    Hit<T> hit;
    // PRIMARY RAY INTERSECTION
    if (!scene.objects.intersect(ray, eps, inf, hit)) { 
        return scene.bkgcolor; // return background color if no hit
    } 

    // STARTING WITH AMBIENT TERM
    Vec3<T> N = hit.normal.toVec3();
    Point3<T> P = hit.point;
    Vec3<T> V = (scene.camera.eye - P).normalize(); // view dir

    Vec3<T> color = hit.material.ka * hit.material.Od; // ambient component

    // LIGHT LOOP -- DIFFUSE AND SPECULAR IF NOT SHADOWED
    for (const auto& light : scene.lights) {
        Vec3<T> Lraw = light.direction_from(P);
        T dist = light.distance_from(P);

        Vec3<T> L = Lraw.normalize();

        // SHADOW TEST
        Point3<T> shadow_origin = P + N * eps; // offset to avoid self-intersection
        Ray<T> shadow_ray(shadow_origin, L);

        T tmax_shadow = dist;
        if (light.type) {
            tmax_shadow = dist - eps; // avoid hitting the light itself
        } else {
            tmax_shadow = inf; // directional light, check all the way to infinity
        }

        Hit<T> tmp;
        if (scene.objects.intersect(shadow_ray, eps, tmax_shadow, tmp)) {
            continue; // in shadow, skip this light
        }

        // DIFFUSE -- LAMBERT
        T ndotl = std::max(N.dot(L), T(0));
        Vec3<T> diffuse = hit.material.kd * hit.material.Od * ndotl;

        // SPECULAR -- BLINN-PHONG
        Vec3<T> H = (L + V).normalize(); // half-vector
        T ndoth = std::max(N.dot(H), T(0));
        T specular_factor = std::pow(ndoth, hit.material.n);
        Vec3<T> specular = hit.material.ks * hit.material.Os * specular_factor;

        // APPLYING LIGHT INTENSITY
        color += light.intensity * (diffuse + specular);

    }
    return color;
}

template<typename T>
void RenderPPM(const Scene<T>& scene, const std::string& filename) {
    char output_file[1024];

    if (strlen(filename.c_str()) >= sizeof(output_file) - 4) {
        printf("[ERROR] Output filename too long.\n");
        return;
    }

    strcpy(output_file, filename.c_str());

    for (int i = strlen(output_file) - 1; i >= 0; i--) {
        if (output_file[i] == '.') {
            output_file[i] = '\0'; // Terminate string at the dot
            break;
        }
    }

    strcat(output_file, ".ppm");
    FILE* ppm_file = fopen(output_file, "w");

    if (ppm_file == NULL) {
        printf("[ERROR] Could not create output file: %s\n", output_file);
        return;
    }

    int w = scene.camera.width;
    int h = scene.camera.height;

    fprintf(ppm_file, "P3\n");
    fprintf(ppm_file, "%d %d\n", w, h);
    fprintf(ppm_file, "255\n");

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray<T> r = scene.camera.get_ray(i, j);
            Vec3<T> c = ShadeRay(r, scene);
            
            // clamping to [0, 1] just in case
            c.x = std::min(std::max(c.x, T(0)), T(1));
            c.y = std::min(std::max(c.y, T(0)), T(1));
            c.z = std::min(std::max(c.z, T(0)), T(1));

            int red = static_cast<int>(c.x * 255.0f);
            int green = static_cast<int>(c.y * 255.0f);
            int blue = static_cast<int>(c.z * 255.0f);

            fprintf(ppm_file, "%d %d %d ", red, green, blue);
        }
        fprintf(ppm_file, "\n");
    }

    fclose(ppm_file);
    return;
}

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("[ERROR] Program requires a filename as an argument.\n");
        return 1;
    }

    // LOADING SCENE
    Scene<float> scene;
    if (!scene.parse(argv[1])) {
        printf("[ERROR] Failed to load scene from file: %s\n", argv[1]);
        return 1;
    }
    printf("[SUCCESS] Scene loaded successfully from file: %s\n", argv[1]);

    // RENDERING AND WRITING TO FILE
    RenderPPM(scene, argv[1]);
    return 0;
}