#include<cmath>
#include<Eigen/Core>
#include<iostream>

using namespace std;
using namespace Eigen;

#define PI acos(-1)

int main() {
    double theta = 45.0 / 180.0 * PI;

    Matrix3d rotation, translation;

    // 旋转矩阵
    rotation << cos(theta), -sin(theta), 0,
                sin(theta), cos(theta), 0,
                0, 0, 1;

    // 平移矩阵
    translation << 1, 0, 1,
                   0, 1, 2,
                   0, 0, 1;

    // 定义一个3行的列向量
    MatrixXd p(3, 1);
    p << 2.0, 1.0, 1;

    cout << translation * rotation * p << std::endl;

    return 0;
}