/* wsg_50_sim_driver
 * Copyright (c) 2012, Robotnik Automation, SLL
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Robotnik Automation, SLL. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \author Marc Benetó (mbeneto@robotnik.es)
 * \brief WSG-50 sim driver.
 */

/* wsg_50_sim_driver (ROS 2 Jazzy)
 * Migrated from ROS 1
 * \brief WSG-50 sim driver.
 */

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_srvs/srv/empty.hpp>

#include <wsg50_common/srv/move.hpp>
#include <wsg50_common/srv/incr.hpp>

#define GRIPPER_MAX_OPEN 110.0
#define GRIPPER_MIN_OPEN 0.0


class WSG50SimDriver : public rclcpp::Node {
	public:
		WSG50SimDriver() : Node("wsg_50_sim_driver"), current_opening_(0.0) {
			// parameters
			this->declare_parameter<std::string>("vel_pub_l_topic", "/wsg_50_gl/command");
			this->declare_parameter<std::string>("vel_pub_r_topic", "/wsg_50_gr/command");
			
			std::string vel_pub_l_topic = this->get_parameter("vel_pub_l_topic").as_string();
    		std::string vel_pub_r_topic = this->get_parameter("vel_pub_r_topic").as_string();

			// Publishers
			vel_pub_l_ = this->create_publisher<std_msgs::msg::Float64>(vel_pub_l_topic, 10);
			vel_pub_r_ = this->create_publisher<std_msgs::msg::Float64>(vel_pub_r_topic, 10);

			// Service Servers
			move_ss_ = this->create_service<wsg50_common::srv::Move>(
				"move",
				std::bind(
					&WSG50SimDriver::moveSrv,
					this,
					std::placeholders::_1,
					std::placeholders::_2
				)
			);

			move_inc_ss_ = this->create_service<wsg50_common::srv::Incr>(
				"move_incrementally",
				std::bind(
					&WSG50SimDriver::moveIncrementallySrv,
					this,
					std::placeholders::_1,
					std::placeholders::_2
				)
			);

			homing_ss_ = this->create_service<std_srvs::srv::Empty>(
				"homing",
				std::bind(
					&WSG50SimDriver::homingSrv,
					this,
					std::placeholders::_1,
					std::placeholders::_2
				)
			);

			grasp_ss_ = this->create_service<wsg50_common::srv::Move>(
				"grasp",
				std::bind(
					&WSG50SimDriver::graspSrv,
					this,
					std::placeholders::_1,
					std::placeholders::_2
				)
			);			
			RCLCPP_INFO(this->get_logger(), "WSG-50 Sim Driver Node Started.");
		}
	
	private:
		double current_opening_;
		
		rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr vel_pub_l_;
		rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr vel_pub_r_;
		
		rclcpp::Service<wsg50_common::srv::Move>::SharedPtr move_ss_;
		rclcpp::Service<wsg50_common::srv::Incr>::SharedPtr move_inc_ss_;
		rclcpp::Service<std_srvs::srv::Empty>::SharedPtr homing_ss_;
		rclcpp::Service<wsg50_common::srv::Move>::SharedPtr grasp_ss_;

		void move(double width)  {
			double open = width / 2.0;
			std_msgs::msg::Float64 lCommand, rCommand;
			
			rCommand.data = open / 1000.0;
			lCommand.data = rCommand.data * -1.0;
			
			vel_pub_r_->publish(rCommand);
			vel_pub_l_->publish(lCommand);
			
			current_opening_ = width;
		}

		void moveSrv(const std::shared_ptr<wsg50_common::srv::Move::Request> req,
               std::shared_ptr<wsg50_common::srv::Move::Response> res){
			
			(void)res;
			if (req->width >= GRIPPER_MIN_OPEN && req->width <= GRIPPER_MAX_OPEN) {
				RCLCPP_INFO(this->get_logger(), "Moving to %f position.", req->width);
				move(req->width);
			} else {
				RCLCPP_ERROR(this->get_logger(), "Impossible to move to this position. (Width values: [0.0 - 110.0])");
			}
		}

		void moveIncrementallySrv(const std::shared_ptr<wsg50_common::srv::Incr::Request> req,
                            std::shared_ptr<wsg50_common::srv::Incr::Response> res){
		
			(void)res;
			if (req->direction == "open") {
				float nextWidth = current_opening_ + req->increment;
				if (nextWidth <= GRIPPER_MAX_OPEN) {
					move(nextWidth);
				} else {
					move(GRIPPER_MAX_OPEN);
				}
			} else if (req->direction == "close") {
				float nextWidth = current_opening_ - req->increment;
				if (nextWidth >= GRIPPER_MIN_OPEN) {
					move(nextWidth);
				} else {
					move(GRIPPER_MIN_OPEN);
				}
			}
		}

		void homingSrv(const std::shared_ptr<std_srvs::srv::Empty::Request> req,
						std::shared_ptr<std_srvs::srv::Empty::Response> res) {

			(void)req;
			(void)res;
			RCLCPP_INFO(this->get_logger(), "Homing...");
			move(0.0);
			RCLCPP_INFO(this->get_logger(), "Home position reached.");
		}

		void graspSrv(const std::shared_ptr<wsg50_common::srv::Move::Request> req,
						std::shared_ptr<wsg50_common::srv::Move::Response> res) {
			
			(void)req;
			(void)res;
			RCLCPP_INFO(this->get_logger(), "Grasping...");
			// TODO: Increase finger force as per original code
			move(0.0);
			RCLCPP_INFO(this->get_logger(), "Object grasped");
		}
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<WSG50SimDriver>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
