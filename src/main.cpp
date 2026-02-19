#include "core/scene.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>   

// for random seeds every time; for soft shadows
#include <ctime>

template<typename T>
Vec3<T> ShadeRay(const Ray<T>& ray, const Scene<T>& scene) {
    // currently implements area-light Monte Carlo visibility

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
    if (scene.camera.is_parallel) {
        V = (-ray.direction).normalize();
    }
    

    Vec3<T> color = hit.material.ka * hit.material.Od; // ambient component

    // LIGHT LOOP -- DIFFUSE AND SPECULAR IF NOT SHADOWED
    for (const auto& light : scene.lights) {

        if (!light.type) { // directional == 0
            Vec3<T> L = light.direction_from(P).normalize();
            Point3<T> origin = P + N * eps;
            Ray<T> shadow_ray(origin, L);

            Hit<T> tmp;
            if (scene.objects.intersect(shadow_ray, eps, inf, tmp)) {
                continue; // in shadow, skip this light
            }

            Vec3<T> diffuse = hit.material.kd * hit.material.Od * std::max(N.dot(L), T(0));
            Vec3<T> H = (L + V).normalize();
            Vec3<T> spec = hit.material.ks * hit.material.Os * std::pow(std::max(N.dot(H), T(0)), hit.material.n);
            color += light.intensity * (diffuse + spec);

        } else { // point light == 1
            Point3<T> origin = P + N * eps;
            Vec3<T> sum(0, 0, 0);

            T radius = T(0.1); // hyperparameter, radius of sphere light for soft shadows
            int S = 32; // hyperparameter, number of samples for soft shadows
            for (int s = 0; s < S; s++) {
                
                // fixing square sampling to be within sphere radius of point light
                Vec3<T> random_vec(
                    (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius,
                    (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius,
                    (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius
                );
                while (random_vec.length() > radius) { // resample until within sphere
                    random_vec = Vec3<T>(
                        (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius,
                        (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius,
                        (static_cast<T>(rand()) / RAND_MAX - 0.5f) * 2.0f * radius
                    );
                }
                Point3<T> pos(light.direction.x, light.direction.y, light.direction.z);
                Point3<T> Q = pos + random_vec; // jittered light position;

                Vec3<T> Ls = Q - origin;
                T dist_s = Ls.length();
                Ls = Ls.normalize();

                Ray<T> shadow_ray(origin, Ls);

                Hit<T> tmp;
                if (scene.objects.intersect(shadow_ray, eps, std::max(dist_s - eps, eps), tmp)) {
                    continue; // in shadow, skip this sample
                }

                T ndotl = std::max(N.dot(Ls), T(0));
                Vec3<T> diffuse = hit.material.kd * hit.material.Od * ndotl;
                Vec3<T> Hs = (Ls + V).normalize();
                T ndoth = std::max(N.dot(Hs), T(0));
                Vec3<T> spec = hit.material.ks * hit.material.Os * std::pow(ndoth, hit.material.n);

                // LIGHT ATTENUATION FOR POINT LIGHTS
                T att = T(1);
                if (light.has_attenuation) {
                    T den = light.c1 + light.c2 * dist_s + light.c3 * dist_s * dist_s;
                    den = std::max(den, T(1e-6)); // prevent division by zero or very small numbers
                    att = 1 / den;
                }

                sum += att * (diffuse + spec);
            }
            color += light.intensity * (sum / static_cast<T>(S)); 
        }
    }

    // IMPLEMENTING DEPTH CUEING
    if (scene.depth_cueing_enabled) {
        T d = (P - scene.camera.eye).length();
        if (scene.camera.is_parallel) {
            d = std::abs((P - scene.camera.eye).dot(scene.camera.forward));
        }
        T t = std::min(std::max((d - scene.dist_min) / (scene.dist_max - scene.dist_min), T(0)), T(1));
        T a = scene.alpha_min + t * (scene.alpha_max - scene.alpha_min);
        color = (1 - a) * color + a * scene.dc;
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

    // INITIALIZING RANDOM SEED FOR SOFT SHADOWS
    srand(static_cast<unsigned int>(time(0)));

    // RENDERING AND WRITING TO FILE
    RenderPPM(scene, argv[1]);
    return 0;
}