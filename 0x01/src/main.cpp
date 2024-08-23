#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) // 调整摄像头到原点
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
/*逐个元素地构建模型变换矩阵并返回该矩阵。在此函数中，你只需要实现三维中绕 z 轴旋转的变换矩阵，而不用处理平移与缩放。 */
Eigen::Matrix4f get_model_matrix(float rotation_angle) //绕z轴旋转
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f translate;
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float radian = rotation_angle * MY_PI / 180.0f;
    float cosin = cos(radian);
    float sinin = sin(radian);
    translate << cosin, -sinin, 0, 0, 
                sinin, cosin, 0, 0, 
                0, 0, 1, 0,
                0, 0, 0, 1;
    model = translate * model;
    return model;
}
/*使用给定的参数逐个元素地构建透视投影矩阵并返回该矩阵。 */
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    float angel = eye_fov / 180.0 * MY_PI;
    float t = zNear * std::tan(angel / 2);
    float r = t * aspect_ratio;
    float l = -r;
    float b = -t;

    Eigen::Matrix4f MorthoScale(4, 4); //坐标缩放为-1到1区间
    MorthoScale << 2 / (r - l), 0, 0, 0,
                    0, 2 / (t - b), 0, 0,
                    0, 0, 2 / (zFar - zNear), 0,
                    0, 0, 0, 1;

    Eigen::Matrix4f MorthoPos(4, 4); //移到原点
    MorthoPos << 1, 0, 0, -(r + l) / 2,
                0, 1, 0, -(t + b) / 2,
                0, 0, 1, -(zNear + zFar) / 2,
                0, 0, 0, 1;

    Eigen::Matrix4f Mpersp2ortho(4, 4); //透视投影

    Mpersp2ortho << zNear, 0, 0, 0,
                    0, zNear, 0, 0,
                    0, 0, zNear + zFar, -zNear * zFar,
                    0, 0, 1, 0;

    // 为了使得三角形是正着显示的，这里需要把透视矩阵乘以下面这样的矩阵
    // 参考：http://games-cn.org/forums/topic/%e4%bd%9c%e4%b8%9a%e4%b8%89%e7%9a%84%e7%89%9b%e5%80%92%e8%bf%87%e6%9d%a5%e4%ba%86/
    Eigen::Matrix4f Mt(4, 4); // 翻转z坐标
    Mt << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1;
    Mpersp2ortho = Mpersp2ortho * Mt;

    projection = MorthoScale * MorthoPos * Mpersp2ortho * projection;
    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0; // 旋转角度（默认为0度）
    bool command_line = false;
    std::string filename = "output.png"; // 默认输出图片文件名

    if (argc >= 3) {//用于设定旋转角度
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);//图片保存路径
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700); // 初始化光栅化器（rasterizer），宽高为700像素。

    Eigen::Vector3f eye_pos = {0, 0, 5}; // 设置相机的位置信息（eye_pos）

    // 定义对象的顶点位置和索引
    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    // 将顶点位置和索引加载到光栅化器中
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    // 初始化键盘输入变量和帧计数器
    int key = 0;
    int frame_count = 0;
    // 如果通过命令行提供了旋转角度，则执行以下操作：
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle)); // 设置模型矩阵，用于调整视角和对象的位置。
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }
    // 若程序未通过命令行接收输入，则进入交互模式：
    while (key != 27) {// 'esc'键退出循环
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        // 进行绘制操作
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image); // 显示实时更新的图像并等待用户输入
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
        // 按下'a'键增加角度，按下'd'键减少角度（控制旋转）
        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
