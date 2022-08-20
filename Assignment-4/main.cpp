#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void MSAA(cv::Point2f point, cv::Mat &window, int channel, float weight)
{
    weight = std::clamp(weight, 0.0f, 1.0f);

    int floor_x = std::floor(point.x);
    int floor_y = std::floor(point.y);
    float rate_x = point.x - floor_x;
    float rate_y = point.y - floor_y;

    auto &a = window.at<cv::Vec3b>(floor_y, floor_x);
    auto &b = window.at<cv::Vec3b>(floor_y, floor_x + 1);
    auto &c = window.at<cv::Vec3b>(floor_y + 1, floor_x);
    auto &d = window.at<cv::Vec3b>(floor_y + 1, floor_x + 1);

    // 透明度叠加
    float alpha1, alpha2;
    alpha1 = a[channel] / 255.0f;
    alpha2 = (1 - rate_x) * (1 - rate_y);

    // 设置绿色
    a[channel] = std::floor((alpha1 + alpha2 - alpha1 * alpha2) * 255 * weight);

    alpha1 = b[channel] / 255.0f;
    alpha2 = rate_x * (1 - rate_y);
    b[channel] = std::floor((alpha1 + alpha2 - alpha1 * alpha2) * 255 * weight);

    alpha1 = c[channel] / 255.0f;
    alpha2 = (1 - rate_x) * rate_y;

    c[channel] = std::floor((alpha1 + alpha2 - alpha1 * alpha2) * 255 * weight);

    alpha1 = d[channel] / 255.0f;
    alpha2 = rate_x * rate_y;
    d[channel] = std::floor((alpha1 + alpha2 - alpha1 * alpha2) * 255 * weight);
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;
        // 无抗锯齿（对比）
        window.at<cv::Vec3b>(point.y + 200, point.x)[2] = 255;

        // 抗锯齿
        MSAA(point, window, 2, 1.0f);
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    if (control_points.size() < 2) {
        // 递归出口
        return control_points[0];
    }
    std::vector<cv::Point2f> points;
    for (int i = 1; i < control_points.size(); i++) {
        auto a = control_points[i - 1];
        auto b = control_points[i];

        // 插值
        points.push_back(a + (b - a) * t);
    }
    // 递归
    return recursive_bezier(points,t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    for (double t = 0.0; t <= 1.0; t += 0.001) {
        auto point = recursive_bezier(control_points, t);
        // 颜色数组按 BGR 的顺序（这个用来对比）
        window.at<cv::Vec3b>(point.y + 200, point.x)[1] = 255;

        // 抗锯齿
        MSAA(point, window, 1, 1.0f);
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
