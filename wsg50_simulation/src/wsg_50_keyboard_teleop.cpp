/* wsg_50_keyboard_teleop
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
 * \brief WSG-50 keyboard teleop
 */

/* wsg_50_keyboard_teleop (ROS 2 Jazzy)
 * Migrated from ROS 1
 * \brief WSG-50 keyboard teleop
 */

#include <termios.h>
#include <signal.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>

#define KEYCODE_A 0x61
#define KEYCODE_D 0x64
#define KEYCODE_S 0x73
#define KEYCODE_W 0x77 
#define KEYCODE_Q 0x71
#define KEYCODE_E 0x65
#define KEYCODE_SPACEBAR 0x49

#define MAX_GRIPPER_OPEN 0.056
#define MIN_GRIPPER_OPEN 0.0

int kfd = 0;
struct termios cooked, raw;

// Function to restore terminal settings on exit
void quit(int sig) {
  (void)sig;
  tcsetattr(kfd, TCSANOW, &cooked);
  rclcpp::shutdown();
  exit(0);
}

class Wsg50Teleop : public rclcpp::Node {
  public:
    Wsg50Teleop() : Node("wsg_50_teleop"), current_pos_(0.0) { 
      cmd.data = 0;

      vel_pub_r_ = this->create_publisher<std_msgs::msg::Float64>("/wsg_50_gr/command", 1);
      vel_pub_l_ = this->create_publisher<std_msgs::msg::Float64>("/wsg_50_gl/command", 1);

      this->declare_parameter<double>("open_increment", 0.001);
      this->get_parameter("open_increment", open_increment);
    }
    
    void keyboardLoop() {
      char c;
      bool dirty=false;
      current_pos_ = 0.0;

      // get the console in raw mode
      tcgetattr(kfd, &cooked);
      memcpy(&raw, &cooked, sizeof(struct termios));
      raw.c_lflag &=~ (ICANON | ECHO);
      // Setting a new line, then end of file
      raw.c_cc[VEOL] = 1;
      raw.c_cc[VEOF] = 2;
      tcsetattr(kfd, TCSANOW, &raw);

      puts("Reading from keyboard");
      puts("---------------------------");
      puts("Use 'W' to open the gripper");
      puts("Use 'S' to close the gripper");

      while(rclcpp::ok()) {
        // get the next event from the keyboard
        if(read(kfd, &c, 1) < 0) {
          perror("read():");
          exit(-1);
        }
        cmd.data = current_pos_;

        switch(c){
          case KEYCODE_W: // Open gripper
            if (current_pos_ < MAX_GRIPPER_OPEN){
              current_pos_ = current_pos_ + open_increment;
              cmd.data = current_pos_;
              dirty = true;
            }
            break;
          case KEYCODE_S: // Close gripper
            if (current_pos_ > MIN_GRIPPER_OPEN){
              current_pos_ = current_pos_ - open_increment;
              cmd.data = current_pos_;
              dirty = true;
            }
            break;
        }
        
        if (dirty == true){
          vel_pub_r_->publish(cmd);
          cmd.data = cmd.data * -1.0; // Adapt for the left gripper
          vel_pub_l_->publish(cmd);
        }
      }
    }

  private:
    double open_increment, close_increment, grasp_increment, force;
    double current_pos_;
    std_msgs::msg::Float64 cmd;

    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr vel_pub_r_, vel_pub_l_;
};


int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  signal(SIGINT,quit);

  auto teleop_node = std::make_shared<Wsg50Teleop>();
  teleop_node->keyboardLoop();

  rclcpp::shutdown();  
  return(0);
}