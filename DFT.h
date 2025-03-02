#include <complex>
#include <vector>

using namespace std;

vector<double> discreteFourierTransform(vector<double> points)
{
    vector<double> DFT(points.size());
    for(int k = 0; k < points.size(); k++)
    {
        complex sum(0.0,0.0);
        for(int j = 0; j < points.size(); j++)
        {
            double arg = 2*M_PI*j*k/points.size();
            sum += points[j] * complex(cos(arg),-sin(arg));
        }
        DFT[k] = sum.real();
    }

    return DFT;
}