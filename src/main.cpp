#include "core/scene.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>   
#include <vector>

// for random seeds every time; for soft shadows
#include <ctime>

template<typename T> 
Vec3<T> Reflect(const Vec3<T>& I, const Vec3<T>& N) {
    return I - T(2) * I.dot(N) * N;
}

template<typename T> 
bool Refract(const Vec3<T>& I, const Vec3<T>& N, T eta_i, T eta_T, Vec3<T>& refraction_direction) {
    T eta = eta_i / eta_T;
    T cos_i = std::min(std::max((-I).dot(N), T(0)), T(1));
    T k = T(1) - eta * eta * (T(1) - cos_i * cos_i);

    if (k < T(0)) {
        return false;
    }

    refraction_direction = eta * I + (eta * cos_i - std::sqrt(k)) * N;
    refraction_direction = refraction_direction.normalize();
    return true;
}

template<typename T>
T Schlick(T cos_theta, T eta_i, T eta_T) {
    T r0 = (eta_i - eta_T) / (eta_i + eta_T);
    r0 = r0 * r0;
    return r0 + (T(1) - r0) * std::pow(T(1) - cos_theta, T(5));
}

template<typename T>
T ShadowVisibility(const Scene<T>& scene, const Ray<T>& shadow_ray, T eps, T max_t) {
    T visibility = T(1);
    T tmin = eps;

    while (true) {
        Hit<T> h;
        if (!scene.objects.intersect(shadow_ray, tmin, max_t, h)) {
            break;
        }

        if (h.t < eps * 10) {
            tmin = h.t + eps;
            continue;
        }

        // Opaque object fully blocks light, transparent object only reduces it
        visibility *= (T(1) - h.material.alpha);

        if (visibility <= T(1e-3)) {
            return T(0);
        }

        tmin = h.t + eps;
    }

    return visibility;
}

template<typename T>
Vec3<T> ComputeLocalColor(const Ray<T>& ray, const Scene<T>& scene, const Hit<T>& hit, const Point3<T>& P, const Vec3<T>& Ng, const Vec3<T>& N, const Vec3<T>& V, const Vec3<T>& material_Od, T eps, T inf) {
    Vec3<T> color = hit.material.ka * material_Od;

    for (const auto& light : scene.lights) {
        if (!light.type) { // directional
            Vec3<T> L = light.direction_from(P).normalize();
            Point3<T> origin = P + Ng * eps;
            Ray<T> shadow_ray(origin, L);

            T visibility = ShadowVisibility(scene, shadow_ray, eps, inf);

            Vec3<T> diffuse = hit.material.kd * material_Od * std::max(N.dot(L), T(0));
            Vec3<T> H = (L + V).normalize();
            Vec3<T> spec = hit.material.ks * hit.material.Os *
                        std::pow(std::max(N.dot(H), T(0)), hit.material.n);

            // ambient stays untouched; only diffuse/spec get shadowed
            color += light.intensity * visibility * (diffuse + spec);
        } else { // point light
            Point3<T> origin = P + Ng * eps;
            Vec3<T> sum(0, 0, 0);

            T radius = T(0.1);
            int S = 32;

            for (int s = 0; s < S; s++) {
                Vec3<T> random_vec(
                    (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius,
                    (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius,
                    (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius
                );

                while (random_vec.length() > radius) {
                    random_vec = Vec3<T>(
                        (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius,
                        (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius,
                        (static_cast<T>(rand()) / RAND_MAX - T(0.5)) * T(2) * radius
                    );
                }

                Point3<T> pos(light.direction.x, light.direction.y, light.direction.z);
                Point3<T> Q = pos + random_vec;

                Vec3<T> Ls = Q - origin;
                T dist_s = Ls.length();
                Ls = Ls.normalize();

                Ray<T> shadow_ray(origin, Ls);

                T visibility = ShadowVisibility(
                    scene,
                    shadow_ray,
                    eps,
                    std::max(dist_s - eps, eps)
                );

                T ndotl = std::max(N.dot(Ls), T(0));
                Vec3<T> diffuse = hit.material.kd * material_Od * ndotl;
                Vec3<T> Hs = (Ls + V).normalize();
                T ndoth = std::max(N.dot(Hs), T(0));
                Vec3<T> spec = hit.material.ks * hit.material.Os *
                            std::pow(ndoth, hit.material.n);

                T att = T(1);
                if (light.has_attenuation) {
                    T den = light.c1 + light.c2 * dist_s + light.c3 * dist_s * dist_s;
                    den = std::max(den, T(1e-6));
                    att = T(1) / den;
                }

                sum += att * visibility * (diffuse + spec);
            }

            color += light.intensity * (sum / static_cast<T>(S));
        }
    }

    if (scene.depth_cueing_enabled) {
        T d = (P - scene.camera.eye).length();
        if (scene.camera.is_parallel) {
            d = std::abs((P - scene.camera.eye).dot(scene.camera.forward));
        }
        T t = std::min(std::max((d - scene.dist_min) / (scene.dist_max - scene.dist_min), T(0)), T(1));
        T a = scene.alpha_min + t * (scene.alpha_max - scene.alpha_min);
        color = (T(1) - a) * color + a * scene.dc;
    }

    return color;
}

template<typename T>
Vec3<T> ShadeRay(const Ray<T>& ray, const Scene<T>& scene, int depth, const std::vector<T>& eta_stack) {
    T eps = T(1e-4);
    T inf = std::numeric_limits<T>::infinity();

    T current_eta = eta_stack.empty() ? scene.bkg_eta : eta_stack.back();

    const int MAX_DEPTH = 10;
    if (depth >= MAX_DEPTH) {
        return Vec3<T>(0, 0, 0);
    }

    Hit<T> hit;
    if (!scene.objects.intersect(ray, eps, inf, hit)) {
        return scene.bkgcolor;
    }

    Vec3<T> Ng = hit.normal.toVec3();
    Vec3<T> N = Ng;
    Point3<T> P = hit.point;
    Vec3<T> V = (-ray.direction).normalize();

    Vec3<T> material_Od = hit.material.Od;

    if (hit.texture_idx >= 0 && hit.texture_idx < static_cast<int>(scene.textures.size())) {
        const Texture<T>& tex = scene.textures[hit.texture_idx];
        Point2<T> uv = hit.uv;
        material_Od = tex.sample(uv.x, uv.y);
    }

    if (hit.bump_map_idx >= 0 && hit.bump_map_idx < static_cast<int>(scene.bump_maps.size())) {
        const Texture<T>& bump = scene.bump_maps[hit.bump_map_idx];
        Point2<T> uv = hit.uv;

        Vec3<T> bump_rgb = bump.sample(uv.x, uv.y);

        Vec3<T> n_tangent(
            T(2) * bump_rgb.x - T(1),
            T(2) * bump_rgb.y - T(1),
            T(2) * bump_rgb.z - T(1)
        );
        n_tangent = n_tangent.normalize();

        Vec3<T> Tvec = hit.tangent.normalize();
        Vec3<T> Nbase = N.normalize();
        Tvec = (Tvec - Nbase * Tvec.dot(Nbase)).normalize();
        Vec3<T> Bvec = Nbase.cross(Tvec);

        Vec3<T> bumpedN =
            Tvec * n_tangent.x +
            Bvec * n_tangent.y +
            Nbase * n_tangent.z;

        N = bumpedN.normalize();
    }

    Vec3<T> local_color = ComputeLocalColor(
        ray, scene, hit, P, Ng, N, V, material_Od, eps, inf
    );

    Vec3<T> I = ray.direction.normalize();
    Vec3<T> Nuse = N.normalize();

    Vec3<T> reflected_color(0, 0, 0);
    Vec3<T> refracted_color(0, 0, 0);

    bool can_refract = hit.material.alpha < T(1);
    bool can_reflect = (hit.material.ks > T(0)) || can_refract;

    T eta_i = current_eta;
    T eta_t = current_eta;
    std::vector<T> next_stack = eta_stack;

    if (can_refract) {
        if (hit.front_face) {
            eta_t = hit.material.eta;
            next_stack.push_back(hit.material.eta);
        } else {
            if (!next_stack.empty()) {
                next_stack.pop_back();
            }
            eta_t = next_stack.empty() ? scene.bkg_eta : next_stack.back();
        }
    }

    T cos_theta = std::min(std::max((-I).dot(Nuse), T(0)), T(1));
    T fresnel = can_refract ? Schlick(cos_theta, eta_i, eta_t) : T(0);

    if (can_reflect) {
        Vec3<T> reflection_direction = Reflect(I, Nuse).normalize();
        Point3<T> reflection_origin = P + reflection_direction * eps;

        reflected_color = ShadeRay(
            Ray<T>(reflection_origin, reflection_direction),
            scene,
            depth + 1,
            eta_stack
        );
    }

    if (can_refract) {
        Vec3<T> refraction_direction;
        if (Refract(I, Nuse, eta_i, eta_t, refraction_direction)) {
            Point3<T> refraction_origin = P + refraction_direction * eps;

            refracted_color = ShadeRay(
                Ray<T>(refraction_origin, refraction_direction),
                scene,
                depth + 1,
                next_stack
            );
        } else {
            fresnel = T(1); // total internal reflection
        }
    }

    Vec3<T> final_color(0, 0, 0);

    if (can_refract) {
        Vec3<T> recursive_color = fresnel * reflected_color
                                + (T(1) - fresnel) * refracted_color;
        final_color = hit.material.alpha * local_color + (T(1) - hit.material.alpha) * recursive_color;
    } else if (can_reflect) {
        final_color = local_color + hit.material.ks * fresnel * reflected_color;
    } else {
        final_color = local_color;
    }

    return final_color;
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

    std::vector<T> initial_eta_stack;
    initial_eta_stack.push_back(scene.bkg_eta);

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray<T> r = scene.camera.get_ray(i, j);
            Vec3<T> c = ShadeRay(r, scene, 0, initial_eta_stack);
            
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