#include <iostream>
#include <vector>

#include "ISSTracker.h"

int main()
{
    double mylat;
    double mylong;
    double satlat;
    double satlong;
    std::vector<std::vector<double>> satvector;

    std::cout << "Enter a latitude: " << std::endl;
    std::cin >> mylat;
    std::cout << "Enter a longitude: " << std::endl;
    std::cin >> mylong;
    std::cout << "Enter satellite latitude: " << std::endl;
    std::cin >> satlat;
    std::cout << "Enter satellite longitude: " << std::endl;
    std::cin >> satlong;

    ISSTracker tracker(mylat, mylong);

    satvector = tracker.satUnitVector(satlat, satlong);
    int i;
    for (i = 0; i < 3; i++) {
        std::cout << satvector[i][0] << std::endl;
    }

    satvector.clear();
    return 0;
}