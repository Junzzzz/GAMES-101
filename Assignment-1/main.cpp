#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // 角度转弧度
    float rad = rotation_angle / 180.0 * MY_PI;

    // 绕Z轴旋转
    Eigen::Matrix4f rotation;
    rotation << std::cos(rad), -std::sin(rad), 0, 0,
                std::sin(rad), std::cos(rad), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;

    return rotation * model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // zNear 和 zFar 为正数，这里理解为摄像机朝Z轴正方向的话显然是看不见三角形的
    // 因此这里理解为摄像头正方向的 near 和 far
    float n, f, width, height;
    n = -zNear;
    f = -zFar;

    // 角度需要转弧度
    height = std::abs(n) * std::tan(eye_fov / 180 / 2 * MY_PI) * 2;
    width = height * aspect_ratio;

    Eigen::Matrix4f projection, ortho_translate, ortho_scale, persp2ortho;

    // 默认镜头 x,y 中心已在原点
    ortho_translate << 1, 0, 0, 0,
                       0, 1, 0, 0,
                       0, 0, 1, -(n + f) / 2,
                       0, 0, 0, 1;

    ortho_scale << 2 / width, 0, 0, 0,
                   0, 2 / height, 0, 0,
                   0, 0, 2 / (n - f), 0,
                   0, 0, 0, 1;

    persp2ortho << n, 0, 0, 0,
                    0, n, 0, 0,
                    0, 0, n + f, -n * f,
                    0, 0, 1, 0;

    projection = ortho_scale * ortho_translate * persp2ortho;
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
