// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>

constexpr float SCAN_DISTANCE = 0.7071;

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

static bool MSAA_insideTriangle(float x, float y, const Vector3f* _v)
{
    Eigen::Vector3f p = Vector3f(x, y, 0);

    return (_v[1] - _v[0]).cross(p - _v[0]).z() > 0 &&
           (_v[2] - _v[1]).cross(p - _v[1]).z() > 0 &&
           (_v[0] - _v[2]).cross(p - _v[2]).z() > 0;
}

static float insideTriangle(int x, int y, const Vector3f* _v)
{
    Eigen::Vector3f p = Vector3f(x, y, 0);

    Eigen::Vector3f edges[3] = {
            _v[1] - _v[0],
            _v[2] - _v[1],
            _v[0] - _v[2]
    };

    Eigen::Vector3f v[3] = {
            p - _v[0],
            p - _v[1],
            p - _v[2]
    };


    bool flag = false;
    for (int i = 0; i < 3; i++) {
        Eigen::Vector3f e = edges[i] / edges[i].norm();
        v[i].z() = 0;

        float length = ((e.dot(v[i]) * e) - v[i]).norm();
        if (length < SCAN_DISTANCE) {
            flag = true;
            break;
        }
    }
    if (flag) {
        float alpha = 0;

        if (MSAA_insideTriangle(x - 0.25f, y - 0.25f, _v)) alpha += 0.25;
        if (MSAA_insideTriangle(x - 0.25f, y + 0.25f, _v)) alpha += 0.25;
        if (MSAA_insideTriangle(x + 0.25f, y + 0.25f, _v)) alpha += 0.25;
        if (MSAA_insideTriangle(x + 0.25f, y - 0.25f, _v)) alpha += 0.25;

        return alpha;
    } else if ((edges[0]).cross(v[0]).z() > 0 &&
               (edges[1]).cross(v[1]).z() > 0 &&
               (edges[2]).cross(v[2]).z() > 0) {
        return 1;
    }

    return 0;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = -vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();

    // 初始化边界，取最大和最小两个坐标表示边界框
    Eigen::Vector2i box_max = {INT_MIN, INT_MIN};
    Eigen::Vector2i box_min = {INT_MAX, INT_MAX};

    for (auto& p : v) {
        for(int i = 0; i < 2; i++) {
            auto value = p[i];
            int f = std::floor(value), c = std::ceil(value);
            if (box_min[i] > f) box_min[i] = f;
            if (box_max[i] < c) box_max[i] = c;
        }
    }


    for (int x = box_min[0]; x <= box_max[0]; x++) {
        for (int y = box_min[1]; y <= box_max[1]; y++) {
            float a = insideTriangle(x, y, t.v);

            // 判断是否在三角形内
            if (a == 0) continue;

            int index = get_index(x, y);

            // 获取深度插值
            auto [alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            float w_reciprocal = 1.0f / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;

            // 深度测试
            if (z_interpolated < depth_buf[index]) {
                depth_buf[index] = z_interpolated;
                frame_buf[index] = t.getColor() * a;
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on