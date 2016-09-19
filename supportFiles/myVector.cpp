#include <iostream>
#include <vector>
using namespace std;

vector<vector<double>> matrixMultiply(vector<vector<double>>,
                                      vector<vector<double>>);

int main()
{
    int numr1, numr2, numc1, numc2;
    cout << "Matrix 1, how many rows?" << endl;
    cin >> numr1;
    cout << "Matrix 1: how many cols?" << endl;
    cin >> numc1;
    cout << "Matrix 2: how many cols?" << endl;
    cin >> numc2;

    vector<vector<double>> m1(numr1, vector<double>(numc1));
    vector<vector<double>> m2(numc1, vector<double>(numc2));

    for (int r = 0; r < numr1; r++) {
        for (int c = 0; c < numc1; c++) {
            cout << "Enter the number for the row: ";
            cin >> m1[r][c];
        }
    }

    for (int r = 0; r < numc1; r++) {
        for (int c = 0; c < numc2; c++) {
            cout << "Enter the number for the row: ";
            cin >> m2[r][c];
        }
    }

    vector<vector<double>> product = matrixMultiply(m1, m2);
    for (int r = 0; r < numr1; r++) {
        for (int c = 0; c < numc2; c++) {
            cout << product[r][c] << " ";
        }
        cout << endl;
    }
}

vector<vector<double>> matrixMultiply(vector<vector<double>> m1,
                                      vector<vector<double>> m2)
{
    int numr1 = m1.size();
    int numc1 = m1[0].size();
    int numr2 = m2.size();
    int numc2 = m2[0].size();

    vector<vector<double>> product(numr1, vector<double>(numc2));
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