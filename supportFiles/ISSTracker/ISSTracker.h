/*
    Written by Shrivathsav Seshan
    Library used to find relative location of the ISS from an observer.
    Also returns euler angles and quaternions
    to determine how to change orientations.
*/

#ifndef ISSTracker_h
#define ISSTracker_h

// Dependencies
#include <vector>

class ISSTracker
{
public:
    ISSTracker(double latitude, double longitude);
    ~ISSTracker();
    std::vector<std::vector<double>> satUnitVector(double satlat, double satlong);
    std::vector<std::vector<double>> mapLatLong(double lat, double longitude, double distance);
    std::vector<std::vector<double>> ecef2enu(double satlat, double satlong);
private:
    double mylat;
    double mylong;
    std::vector<std::vector<double>> r1(double theta);
    std::vector<std::vector<double>> r2(double theta);
    std::vector<std::vector<double>> r3(double theta);
    std::vector<std::vector<double>> matrixMultiply(std::vector<std::vector<double>>,
                                                    std::vector<std::vector<double>>);
    std::vector<std::vector<double>> matrixAdd(std::vector<std::vector<double>>,
                                               std::vector<std::vector<double>>);
    std::vector<std::vector<double>> matrixSubtract(std::vector<std::vector<double>>,
                                                    std::vector<std::vector<double>>);
};

#endif