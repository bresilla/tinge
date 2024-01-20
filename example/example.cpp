#include <fmt/core.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>
#include <random>
#include <string>
#include <vector>

#include "ColorSpace.h"
#include "Comparison.h"
#include "Conversion.h"

namespace fs = std::filesystem;
namespace cs = ColorSpace;

class Pixel {
  public:
    uchar b, g, r; // 8 bit, 0-255
    Pixel(uchar b, uchar g, uchar r) {
        this->b = b;
        this->g = g;
        this->r = r;
    }
};

template <> struct fmt::formatter<Pixel> {
    formatter<int> int_formatter;
    template <typename ParseContext> constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }
    template <typename FormatContext> auto format(const Pixel &pixel, FormatContext &ctx) {
        return format_to(ctx.out(), "rgb({},{},{})", pixel.r, pixel.g, pixel.b);
    }
};

const auto mappy = std::map<std::string, Pixel>{
    {"10", Pixel(119, 23, 35)}, // R10
    {"9", Pixel(101, 16, 18)},  // R9
    {"8", Pixel(163, 23, 30)},  // R8
    {"7", Pixel(200, 25, 34)},  // R7
    {"6", Pixel(178, 53, 31)},  // R6
    {"5", Pixel(170, 52, 31)},  // R5
    {"4", Pixel(163, 78, 31)},  // R4
    {"3", Pixel(164, 116, 40)}, // R3
    {"2", Pixel(184, 154, 41)}, // R2
    {"1", Pixel(162, 152, 52)}, // R1
};

class KMeans {
  private:
    std::vector<Pixel> clusterCentres;
    std::vector<int> clusterCentres_int;
    std::map<Pixel, int> clusterCount;
    cv::Mat image;
    int K;

  public:
    cv::Mat labels;
    // Intialise cluster centres as random pixels from the image
    KMeans(cv::Mat image, int K) {
        this->image = image;
        this->K = K;
        labels = cv::Mat::zeros(cv::Size(image.cols, image.rows), CV_8UC1);
        for (int i = 0; i < K; i++) {
            int randRow = random(image.rows - 1);
            int randCol = random(image.cols - 1);
            cv::Vec3b bgr_pixel = image.at<cv::Vec3b>(randRow, randCol);
            uchar b = bgr_pixel[0];
            uchar g = bgr_pixel[1];
            uchar r = bgr_pixel[2];
            clusterCentres.push_back(Pixel(b, g, r));
        }
        assignNewClusterCentres();
    }
    auto train(int iterations) -> void {
        // fmt::print("Training...\n");
        for (int i = 0; i < iterations; i++) {
            computeCentroids();
            assignNewClusterCentres();
            // fmt::print("Training step {} done\n", i);
        }
    }

    auto print_colors() -> void {
        for (Pixel p : clusterCentres) {
            fmt::print("{}\n", p);
        }
    }

    auto get_colors() -> std::vector<Pixel> { return clusterCentres; }

    auto get_distence() -> int { return 1; }

  private:
    auto assignNewClusterCentres() -> void {
        for (int r = 0; r < image.rows; r++) {
            for (int c = 0; c < image.cols; c++) {
                int centroidLabel = 0;
                uchar b, g, r1;
                cv::Vec3b bgr_pixel = image.at<cv::Vec3b>(r, c);
                b = bgr_pixel[0];
                g = bgr_pixel[1];
                r1 = bgr_pixel[2];
                double distance, min_dist;
                min_dist = euclideanDistance(clusterCentres[0].b, clusterCentres[0].g, clusterCentres[0].r, b, g, r1);
                for (int i = 1; i < K; i++) {
                    distance =
                        euclideanDistance(clusterCentres[i].b, clusterCentres[i].g, clusterCentres[i].r, b, g, r1);
                    if (distance < min_dist) {
                        min_dist = distance;
                        centroidLabel = i;
                        labels.at<uchar>(r, c) = (uchar)centroidLabel;
                    }
                }
            }
        }
    }

  private:
    auto computeCentroids() -> void {
        for (int i = 0; i < K; i++) {
            double mean_b = 0.0, mean_g = 0.0, mean_r = 0.0;
            int n = 0;
            for (int r = 0; r < image.rows; r++) {
                for (int c = 0; c < image.cols; c++) {
                    if (labels.at<uchar>(r, c) == i) {
                        cv::Vec3b bgr_pixel = image.at<cv::Vec3b>(r, c);
                        mean_b += bgr_pixel[0];
                        mean_g += bgr_pixel[1];
                        mean_r += bgr_pixel[2];
                        n++;
                    }
                }
            }
            mean_b /= n;
            mean_g /= n;
            mean_r /= n;
            clusterCentres.at(i) = Pixel(mean_b, mean_g, mean_r);
        }
    }

    static auto euclideanDistance(int x1, int y1, int c1, int x2, int y2, int c2) -> double {
        return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(c1 - c2, 2));
    }

    static auto random(int lim) -> int {
        std::default_random_engine dre(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> uid{0, lim};
        return uid(dre);
    }
};

int main(int argc, char **argv) {
    std::string imgFileName;
    std::string outFileName;
    int nColorVectors = 3;
    std::vector<std::string> images;

    std::ofstream csv("data.csv");
    if (!csv) {
        fmt::print("Error: Could not open file for writing.\n");
        return 1;
    }

    if (argc < 1) {
        fmt::print("Please specify input filename(or path)\n");
        return 1;
    }
    fs::path file = argv[1];
    if (fs::is_directory(file)) {
        for (const auto &f : fs::directory_iterator(file)) {
            images.push_back(f.path().string());
        }
    } else if (std::filesystem::is_regular_file(file)) {
        images.push_back(file.string());
    } else {
        fmt::print("Please specify a valid directory");
        return 1;
    }

    for (const std::string &image_path : images) {
        // outFileName = argv[2];
        auto image = cv::imread(image_path);
        if (image.empty()) {
            continue;
        }

        auto kmeans = KMeans(image, nColorVectors);
        kmeans.train(10);
        // kmeans.print_colors();

        auto colors = kmeans.get_colors();
        auto col = cs::Rgb(colors[0].r, colors[0].g, colors[0].b);
        // fmt::print("{}\n", image_path);
        auto biggest = 0;
        std::string score;
        for (const auto &[k, c] : mappy) {
            auto scl = cs::Rgb(c.r, c.g, c.b);
            fmt::print("{}\n", scl);
            auto val = cs::Cie2000Comparison::Compare(&scl, &col);
            if (val > biggest) {
                biggest = val;
                score = k;
            }
        }
        // fmt::print("{}\n", score);
        csv << image_path << ',' << score << '\n';
        // cv::imwrite(outFileName, image);
    }

    return 0;
}