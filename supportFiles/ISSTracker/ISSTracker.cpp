/*
    ISSTracker.cpp Library for finding relative
    satellite locations
*/

#include "ISSTracker.h"

// Dependencies
#include <math.h>
#include <vector>

#define radius 3959 // Earth's radius in miles
#define alt 256 // ISS Average Altitude in miles
#define PI 3.1415926535

ISSTracker::ISSTracker(double latitude, double longitude)
{
    mylat = latitude;
    mylong = longitude;
}

ISSTracker::~ISSTracker(void)
{
    //
}

std::vector<std::vector<double>> ISSTracker::mapLatLong(double latitude, double longitude, double distance)
{
    std::vector<std::vector<double>> coords(3, std::vector<double>(1));
    coords[2][0] = (distance) * sin(latitude * PI / 180.0);
    coords[0][0] = (distance) * cos(latitude * PI / 180.0) * cos(longitude * PI / 180.0);
    coords[1][0] = (distance) * cos(latitude * PI / 180.0) * sin(longitude * PI / 180.0);
    return coords;
}

std::vector<std::vector<double>> ISSTracker::satUnitVector(double satlat, double satlong)
{
    std::vector<std::vector<double>> enuVec = ecef2enu(satlat, satlong);
    std::vector<std::vector<double>> difference = matrixSubtract(mapLatLong(satlat, satlong, radius + alt),
                                                                 mapLatLong(mylat, mylong, radius));
    std::vector<std::vector<double>> product = matrixMultiply(enuVec, difference);

    enuVec.clear();
    difference.clear();

    double mag = sqrt(pow(product[0][0], 2) + pow(product[1][0], 2) + pow(product[2][0], 2));
    product[0][0] = product[0][0] / mag;
    product[1][0] = product[1][0] / mag;
    product[2][0] = product[2][0] / mag;
    return product;
}

std::vector<std::vector<double>> ISSTracker::ecef2enu(double satlat, double satlong)
{
    return matrixMultiply(r1(90 - satlat), r3(90 + satlong));
}

/***************************ROTATION MATRIX FUNCTIONS********************************/

std::vector<std::vector<double>> ISSTracker::r1(double theta)
{
    std::vector<std::vector<double>> rotmat {
        {1, 0, 0},
        {0, cos(theta * PI / 180.0), sin(theta * PI / 180.0)},
        {0, -1 * sin(theta * PI / 180.0), cos(theta * PI / 180.0)}
    };
    return rotmat;
}

std::vector<std::vector<double>> ISSTracker::r2(double theta)
{
    std::vector<std::vector<double>> rotmat {
        {cos(theta * PI / 180.0), 0, -1 * sin(theta * PI / 180.0)},
        {0, 1, 0},
        {sin(theta * PI / 180.0), 0, cos(theta * PI / 180.0)}
    };
    return rotmat;
}

std::vector<std::vector<double>> ISSTracker::r3(double theta)
{
    std::vector<std::vector<double>> rotmat {
        {cos(theta * PI / 180.0), sin(theta * PI / 180.0), 0},
        {-1 * sin(theta * PI / 180.0), cos(theta * PI / 180.0), 0},
        {0, 0, 1}
    };
    return rotmat;
}

/*************************************************************************************/

/**
 * Precondition: The inner dimensions of the two matrices must be equal.
 */
std::vector<std::vector<double>> ISSTracker::matrixMultiply(const std::vector<std::vector<double>> m1,
                                                const std::vector<std::vector<double>> m2)
{
    int numr1 = m1.size();
    int numc1 = m1[0].size();
    int numr2 = m2.size();
    int numc2 = m2[0].size();

    std::vector<std::vector<double>> product(numr1, std::vector<double>(numc2));
    int r, c;
    for (r = 0; r < numr1; r++) {
        for (c = 0; c < numc2; c++) {
            for (int i = 0; i < numr2; i++) {
                product[r][c] += m1[r][i] * m2[i][c];
            }
        }
    }
    return product;
}

/**
 * Precondition: Both m1, and m2 are of the same dimensions.
 */
std::vector<std::vector<double>> ISSTracker::matrixAdd(const std::vector<std::vector<double>> m1,
                                                       const std::vector<std::vector<double>> m2)
{
    int numr1 = m1.size();
    int numc1 = m1[0].size();
    int numr2 = m2.size();
    int numc2 = m2[0].size();

    std::vector<std::vector<double>> mysum(numr1, std::vector<double>(numc1));
    for (int r = 0; r < numr1; r++) {
        for (int c = 0; c < numc1; c++) {
            mysum[r][c] = m1[r][c] + m2[r][c];
        }
    }
    return mysum;
}

/**
 * Precondition: Both m1, and m2 are of the same dimensions.
 */
std::vector<std::vector<double>> ISSTracker::matrixSubtract(const std::vector<std::vector<double>> m1,
                                                            const std::vector<std::vector<double>> m2)
{
    int numr1 = m1.size();
    int numc1 = m1[0].size();
    int numr2 = m2.size();
    int numc2 = m2[0].size();

    std::vector<std::vector<double>> mysum(numr1, std::vector<double>(numc1));
    for (int r = 0; r < numr1; r++) {
        for (int c = 0; c < numc1; c++) {
            mysum[r][c] = m1[r][c] - m2[r][c];
        }
    }
    return mysum;
}