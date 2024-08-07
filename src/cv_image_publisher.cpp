#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <chrono>

class CvImagePublisher : public rclcpp::Node
{
public:
  CvImagePublisher()
  : Node("cv_image_publisher")
  {
    // Publisherの初期化
    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("cv_image", 10);
    
    // Webカメラのキャプチャ初期化
    cap.open(0);
    if (!cap.isOpened()) {
      RCLCPP_ERROR(this->get_logger(), "Failed to open camera.");
      rclcpp::shutdown();
    }

    // カメラのフレームレートを確認する
    RCLCPP_INFO(this->get_logger(), "Camera FPS set: %f", cap.set(cv::CAP_PROP_FPS, 10));
    RCLCPP_INFO(this->get_logger(), "Camera FPS: %f", cap.get(cv::CAP_PROP_FPS));

    // Timerの初期化
    timer_ = this->create_wall_timer(std::chrono::milliseconds(100), [this]() { publish_image(); }
    );
  }
private:
  void publish_image()
  {
    rclcpp::Time base_time = this->get_clock()->now();
    cv::Mat frame;
    cap >> frame; // カメラからフレームを取得

    // 画像の画素を落とす (リサイズしてから元のサイズに戻す)
    cv::Mat img_small;
    cv::resize(frame, img_small, cv::Size(frame.cols / 8, frame.rows / 8));
    // cv::resize(img_small, frame, cv::Size(frame.cols, frame.rows));

    // OpenCV画像をROS2のImageメッセージに変換
    auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", img_small).toImageMsg();
    auto elapsed_time = (this->get_clock()->now()) - base_time;
    std::cout << elapsed_time.seconds() << std::endl;
    // メッセージをパブリッシュ
    publisher_->publish(*msg);
  }

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  cv::VideoCapture cap;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CvImagePublisher>());
  rclcpp::shutdown();
  return 0;
}