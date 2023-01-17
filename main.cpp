#include <iostream>
#include <utility>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "cal_main.h"

extern camera_parameters cp;
extern calibration_constants cc;


#ifdef DEBUG
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif

struct Input_args {
    const std::string coords_file;
    const std::string camera_params;
    const std::string calibration_consts;

    Input_args(std::string  coordFile, std::string cameraParams, std::string calibrationConsts)
            : coords_file(std::move(coordFile)), camera_params(std::move(cameraParams)), calibration_consts(std::move(calibrationConsts)) {}
};

Input_args parse_args(std::vector<std::string> args) {
    if (args.size() != 4) {
        return {"x", "a", "b"};
    }

    return {args.at(1), args.at(2), args.at(3)};
}

void load_camera_params(camera_parameters &cameraParameters, const std::string &params) {
    std::vector<double> v_cp;

    std::stringstream ss(params);

    while (ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        v_cp.push_back(strtod(substr.c_str(), nullptr));
    }

    //Ncx, Nfx, dx, dy, dpx, dpy, Cx, Cy, sx
    cameraParameters.Ncx = v_cp.at(0);
    cameraParameters.Nfx = v_cp.at(1);
    cameraParameters.dx = v_cp.at(2);
    cameraParameters.dy = v_cp.at(3);
    cameraParameters.dpx = v_cp.at(4);
    cameraParameters.dpy = v_cp.at(5);
    cameraParameters.Cx = v_cp.at(6);
    cameraParameters.Cy = v_cp.at(7);
    cameraParameters.sx = v_cp.at(8);
}

void load_cal_consts(calibration_constants &calibrationConstants, const std::string &cal_consts) {
    std::vector<double> v_cp;

    std::stringstream ss(cal_consts);

    while (ss.good()) {
        std::string substr;
        getline(ss, substr, ',');
        v_cp.push_back(strtod(substr.c_str(), nullptr));
    }

    calibrationConstants.f = v_cp.at(0);
    calibrationConstants.kappa1 = v_cp.at(1);
    calibrationConstants.p1 = v_cp.at(2);
    calibrationConstants.p2 = v_cp.at(3);
    calibrationConstants.Tx = v_cp.at(4);
    calibrationConstants.Ty = v_cp.at(5);
    calibrationConstants.Tz = v_cp.at(6);
    calibrationConstants.Rx = v_cp.at(7);
    calibrationConstants.Ry = v_cp.at(8);
    calibrationConstants.Rz = v_cp.at(9);

    double sa, ca, sb, cb, sg, cg;

    SINCOS (calibrationConstants.Rx, sa, ca);
    SINCOS (calibrationConstants.Ry, sb, cb);
    SINCOS (calibrationConstants.Rz, sg, cg);

    calibrationConstants.r1 = cb * cg;
    calibrationConstants.r2 = cg * sa * sb - ca * sg;
    calibrationConstants.r3 = sa * sg + ca * cg * sb;
    calibrationConstants.r4 = cb * sg;
    calibrationConstants.r5 = sa * sb * sg + ca * cg;
    calibrationConstants.r6 = ca * sb * sg - cg * sa;
    calibrationConstants.r7 = -sb;
    calibrationConstants.r8 = cb * sa;
    calibrationConstants.r9 = ca * cb;
}

template<typename T>
std::vector<std::vector<T>> split_vector(const std::vector<T>& source, size_t chunkSize)
{
    std::vector<std::vector<T>> result;
    result.reserve((source.size() + chunkSize - 1) / chunkSize);

    auto start = source.begin();
    auto end = source.end();

    while (start != end) {
        auto next = std::distance(start, end) >= chunkSize
                    ? start + chunkSize
                    : end;

        result.emplace_back(start, next);
        start = next;
    }

    return result;
}

void calc_from_file(const std::string &file_name) {
    std::ifstream infile(file_name);
    std::ofstream outfile(file_name.substr(0, file_name.size()-4) + "_a.csv", std::ofstream::out | std::ofstream::trunc);

    std::vector<std::vector<double>> result;

    std::string line;
    std::getline(infile, line);

    while (std::getline(infile, line)) {
        std::vector<double> v_before;
        std::vector<double> v_after;

        std::stringstream ss(line);
        while( ss.good() ) {
            std::string substr;
            std::getline( ss, substr, ',' );
            v_before.push_back(strtod(substr.c_str(), nullptr));
        }

        auto time = v_before.front();
        v_before.erase(v_before.begin());

        auto x = split_vector(v_before, 3);
        outfile << time;
        for (auto item : x) {
            double xf = 0;
            double yf = 0;
            world_coord_to_image_coord(item[0], item[1], item[2], &xf, &yf);
            result.push_back({xf, yf});
#ifdef DEBUG
            std::cout << item[0] << "," << item[1] << "," << item[2];
            std::cout << "=>{" << xf << ", " << yf << "} \n";
#endif
            outfile << "," << round(xf) << "," << round(yf);
        }
        outfile << "\n";
    }

    infile.close();
    outfile.close();
}

int main(int argc, char *argv[]) {

    std::vector<std::string> args;
    args.assign(argv, argv + argc);

    std::cout << "args: ";
    for (auto const &i: args)
        std::cout << i.c_str() << ", ";
    std::cout << "\n\n";

    Input_args inputArgs = parse_args(args);

    std::cout << inputArgs.coords_file.c_str() << "\n";
    std::cout << inputArgs.calibration_consts.c_str() << "\n";
    std::cout << inputArgs.camera_params.c_str() << "\n";

    load_camera_params(cp, inputArgs.camera_params);
    load_cal_consts(cc, inputArgs.calibration_consts);

    calc_from_file(inputArgs.coords_file);
    return 0;
}
