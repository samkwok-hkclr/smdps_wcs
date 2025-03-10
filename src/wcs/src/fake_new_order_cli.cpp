#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include "smdps_msgs/action/new_order.hpp"
#include "smdps_msgs/msg/order_request.hpp"
#include "smdps_msgs/msg/material_box_slot.hpp"
#include "smdps_msgs/msg/dispensing_detail.hpp"
#include "std_msgs/msg/header.hpp"

using NewOrder = smdps_msgs::action::NewOrder;
using OrderRequest = smdps_msgs::msg::OrderRequest;
using MaterialBoxSlot = smdps_msgs::msg::MaterialBoxSlot;
using DispensingDetail = smdps_msgs::msg::DispensingDetail;
using Drug = smdps_msgs::msg::Drug;
using DrugLocation = smdps_msgs::msg::DrugLocation;

using GoalHandleNewOrder = rclcpp_action::ClientGoalHandle<NewOrder>;

using namespace std::placeholders;

class NewOrderClient : public rclcpp::Node {
public:
  explicit NewOrderClient()
  : Node("new_order_cli") 
  {
    action_client_ = rclcpp_action::create_client<NewOrder>(this, "new_order");
    timer_ = this->create_wall_timer(std::chrono::milliseconds(1000), std::bind(&NewOrderClient::send_goal, this));
  }

private:
  rclcpp_action::Client<NewOrder>::SharedPtr action_client_;
  rclcpp::TimerBase::SharedPtr timer_;

  size_t map_index(size_t index)
  {
    return (index / 7) + (index % 7) * 4;
  }

  void send_goal() {
    timer_->cancel();

    while (!action_client_->wait_for_action_server(std::chrono::seconds(1))) {
      RCLCPP_INFO(this->get_logger(), "Waiting for action server to be available...");
    }

    auto goal_msg = NewOrder::Goal();

    goal_msg.request.header.stamp = this->now();
    goal_msg.request.order_id = 1;
    goal_msg.request.start_date = "2023-01-20";
    goal_msg.request.start_meal = OrderRequest::MEAL_MORNING;

    for (size_t i = 0; i < 28; i++) 
    {
      if (map_index(i) < 7)
      {
        MaterialBoxSlot slot;
        Drug drug;
        DrugLocation location;
        location.dispenser_station = 1;
        location.dispenser_unit = 2;
        drug.amount = 1;
        drug.name = "drug_name";
        drug.drug_id = "1_2";
        drug.locations.push_back(location);
        slot.drugs.push_back(drug);
        goal_msg.request.material_box.slots[i] = slot;
      }
      if (map_index(i) >= 7 && map_index(i) < 14)
      {
        MaterialBoxSlot slot;
        Drug drug;
        DrugLocation location;
        location.dispenser_station = 3;
        location.dispenser_unit = 6;
        drug.amount = 1;
        drug.name = "drug_name";
        drug.drug_id = "3_6";
        drug.locations.push_back(location);
        slot.drugs.push_back(drug);
        goal_msg.request.material_box.slots[i] = slot;
      }
      if (map_index(i) >= 14 && map_index(i) < 21)
      {
        MaterialBoxSlot slot;
        Drug drug;
        DrugLocation location;
        location.dispenser_station = 7;
        location.dispenser_unit = 6;
        drug.amount = 1;
        drug.name = "drug_name";
        drug.drug_id = "3_6";
        drug.locations.push_back(location);
        slot.drugs.push_back(drug);
        Drug drug2;
        DrugLocation location2;
        location2.dispenser_station = 7;
        location2.dispenser_unit = 8;
        drug2.amount = 1;
        drug2.name = "drug_name";
        drug2.drug_id = "7_8";
        drug2.locations.push_back(location2);
        slot.drugs.push_back(drug2);
        goal_msg.request.material_box.slots[i] = slot;
      }
      if (map_index(i) >= 21 && map_index(i) < 28)
      {
        MaterialBoxSlot slot;
        Drug drug;
        DrugLocation location;
        location.dispenser_station = 10;
        location.dispenser_unit = 3;
        drug.amount = 1;
        drug.name = "drug_name";
        drug.drug_id = "10_3";
        drug.locations.push_back(location);
        slot.drugs.push_back(drug);
        Drug drug2;
        DrugLocation location2;
        location2.dispenser_station = 12;
        location2.dispenser_unit = 7;
        drug2.amount = 1;
        drug2.name = "drug_name";
        drug2.drug_id = "12_7";
        drug2.locations.push_back(location2);
        slot.drugs.push_back(drug2);
        goal_msg.request.material_box.slots[i] = slot;
      }
    }

    RCLCPP_INFO(this->get_logger(), "Sending goal...");

    auto send_goal_options = rclcpp_action::Client<NewOrder>::SendGoalOptions();
    send_goal_options.goal_response_callback =
      std::bind(&NewOrderClient::goal_response_callback, this, _1);
    send_goal_options.feedback_callback =
      std::bind(&NewOrderClient::feedback_callback, this, _1, _2);
    send_goal_options.result_callback =
      std::bind(&NewOrderClient::result_callback, this, _1);

    action_client_->async_send_goal(goal_msg, send_goal_options);
  }

  void goal_response_callback(const GoalHandleNewOrder::SharedPtr &goal_handle)
  {
    if (goal_handle) 
      RCLCPP_INFO(this->get_logger(), "Goal accepted by server, waiting for result");
    else 
      RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
  }

  void feedback_callback(
    GoalHandleNewOrder::SharedPtr,
    const std::shared_ptr<const NewOrder::Feedback> feedback)
  {
    RCLCPP_INFO(this->get_logger(), "Running: %s", feedback->running ? "True" : "False");
  }

  void result_callback(const GoalHandleNewOrder::WrappedResult &result) 
  {
    switch (result.code) 
    {
    case rclcpp_action::ResultCode::SUCCEEDED:
      RCLCPP_INFO(this->get_logger(), "Goal succeeded");
      RCLCPP_INFO(this->get_logger(), "material_box_id: %d", result.result->response.material_box_id);
      RCLCPP_INFO(this->get_logger(), "Success: %d", result.result->response.success);
      RCLCPP_INFO(this->get_logger(), "Message: %s", result.result->response.message.c_str());
      break;
    case rclcpp_action::ResultCode::ABORTED:
      RCLCPP_ERROR(this->get_logger(), "Goal aborted");
      break;
    case rclcpp_action::ResultCode::CANCELED:
      RCLCPP_ERROR(this->get_logger(), "Goal canceled");
      break;
    default:
      RCLCPP_ERROR(this->get_logger(), "Unknown result code");
      break;
    }
    RCLCPP_INFO(this->get_logger(), "The node will be shutdown now");
    rclcpp::shutdown();
  }
};

int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<NewOrderClient>());
    return 0;
}