#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include <map>
#include <functional>
#include <memory>
#include <thread>
#include <queue>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/component_manager.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "rcl_interfaces/msg/parameter.hpp"
#include "rcl_interfaces/msg/parameter_value.hpp"

#include "composition_interfaces/srv/load_node.hpp"
#include "composition_interfaces/srv/unload_node.hpp"
#include "composition_interfaces/srv/list_nodes.hpp"

#include "std_srvs/srv/set_bool.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "smdps_msgs/msg/packaging_machine_status.hpp"
#include "smdps_msgs/msg/packaging_result.hpp"
#include "smdps_msgs/msg/unbind_request.hpp"
#include "smdps_msgs/msg/packaging_machine_info.hpp"
#include "smdps_msgs/msg/motor_status.hpp"

#include "smdps_msgs/srv/packaging_order.hpp"
#include "smdps_msgs/srv/u_int8.hpp"

#define CELLS 28

using namespace std::chrono_literals;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class PackagingMachineManager : public rclcpp::Node
{
public:
  using SetBool = std_srvs::srv::SetBool;
  using Trigger = std_srvs::srv::Trigger;
  
  using PackagingMachineStatus = smdps_msgs::msg::PackagingMachineStatus;
  using PackagingMachineInfo = smdps_msgs::msg::PackagingMachineInfo;
  using MotorStatus = smdps_msgs::msg::MotorStatus;
  using PackagingResult = smdps_msgs::msg::PackagingResult;
  using UnbindRequest = smdps_msgs::msg::UnbindRequest;

  using PackagingOrderSrv = smdps_msgs::srv::PackagingOrder;
  using UInt8Srv = smdps_msgs::srv::UInt8;

  using LoadNode = composition_interfaces::srv::LoadNode;
  using UnloadNode = composition_interfaces::srv::UnloadNode;
  using ListNodes = composition_interfaces::srv::ListNodes;

  explicit PackagingMachineManager(
    std::weak_ptr<rclcpp::Executor> executor,
    const std::string & node_name,
    const std::string & node_namespace,
    const rclcpp::NodeOptions & options);
  ~PackagingMachineManager() = default;

  void packaging_order_handle(
    const std::shared_ptr<PackagingOrderSrv::Request> request, 
    std::shared_ptr<PackagingOrderSrv::Response> response);
  void release_blocking_handle(
    const std::shared_ptr<Trigger::Request> request, 
    std::shared_ptr<Trigger::Response> response);
  void income_mtrl_box_handle(
    const std::shared_ptr<UInt8Srv::Request> request, 
    std::shared_ptr<UInt8Srv::Response> response);
  void con_mtrl_box_handle(
    const std::shared_ptr<UInt8Srv::Request> request, 
    std::shared_ptr<UInt8Srv::Response> response);  
  void manually_release_handle(
    const std::shared_ptr<Trigger::Request> request, 
    std::shared_ptr<Trigger::Response> response);

  void release_blocking_cb(void);
  void queue_handler_cb(void);

  void status_cb(const PackagingMachineStatus::SharedPtr msg);
  void motor_status_cb(const MotorStatus::SharedPtr msg);
  void info_cb(const PackagingMachineInfo::SharedPtr msg);
  void packaging_result_cb(const PackagingResult::SharedPtr msg);

private:
  std::mutex mutex_;
  size_t no_of_pkg_mac;

  std::queue<rclcpp::Time> release_blk_;
  std::queue<std::pair<uint8_t, rclcpp::Time>> income_box_;
  std::queue<std::pair<uint8_t, rclcpp::Time>> packaging_order_;
  uint8_t last_pkg_mac_scan_1;
  uint8_t last_pkg_mac_scan_2;

  // order_id, unique_id
  std::vector<std::pair<uint32_t, uint64_t>> curr_client_;

  rclcpp::CallbackGroup::SharedPtr timer_cbg_;
  rclcpp::CallbackGroup::SharedPtr srv_cli_cbg_;
  rclcpp::CallbackGroup::SharedPtr srv_ser_cbg_;

  rclcpp::Service<PackagingOrderSrv>::SharedPtr service_;
  rclcpp::Service<Trigger>::SharedPtr release_blk_srv_;
  rclcpp::Service<UInt8Srv>::SharedPtr income_box_srv_;
  rclcpp::Service<UInt8Srv>::SharedPtr con_box_srv_;
  rclcpp::Service<Trigger>::SharedPtr manually_release_srv_;

  rclcpp::Subscription<PackagingMachineStatus>::SharedPtr status_sub_;
  rclcpp::Subscription<MotorStatus>::SharedPtr motor_status_sub_;
  rclcpp::Subscription<PackagingMachineInfo>::SharedPtr info_sub_;
  rclcpp::Subscription<PackagingResult>::SharedPtr packaging_result_sub_;

  rclcpp::Publisher<UnbindRequest>::SharedPtr unbind_order_id_pub_;
  
  rclcpp::Client<LoadNode>::SharedPtr load_node_client_;
  rclcpp::Client<UnloadNode>::SharedPtr unload_node_client_;
  rclcpp::Client<ListNodes>::SharedPtr list_node_client_;
  std::map<uint8_t, std::pair<rclcpp::Client<SetBool>::SharedPtr, rclcpp::Client<SetBool>::SharedPtr>> conveyor_stopper_client_;
  
  rclcpp::TimerBase::SharedPtr release_blocking_timer_;
  rclcpp::TimerBase::SharedPtr queue_handler_timer_;

  // packaging_machine_id, PackagingMachineStatus
  std::map<uint8_t, PackagingMachineStatus> pkg_mac_status_;
  std::map<uint8_t, MotorStatus> motor_status_;
  std::map<uint8_t, PackagingMachineInfo> info_;
  
  const std::string action_client_manager_node_name = "action_client_manager";
  const std::string load_node_service_name = action_client_manager_node_name + "/_container/load_node";
  const std::string unload_node_service_name = action_client_manager_node_name + "/_container/unload_node";
  const std::string list_nodes_service_name = action_client_manager_node_name + "/_container/list_nodes";

  const std::string packaging_order_service_name = "packaging_order";
  const std::string release_blocking_service_name = "release_blocking";
  const std::string income_box_service_name = "income_material_box";
  const std::string con_box_service_name = "container_material_box";
  const std::string manually_release_service_name = "manually_release";

protected:
  std::shared_ptr<rclcpp::Executor> executor_;
  std::shared_ptr<rclcpp_components::ComponentManager> action_client_manager_;

};


#endif  // MANAGER_HPP_