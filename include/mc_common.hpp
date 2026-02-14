/*
Copyright (c) 2025 Patryk Dudziński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Authors: Patryk Dudziński
 */


#pragma once

#include "can_base.hpp"
#include <cstring>
#include <bitset>
#include <type_traits>
#include <tuple>

/*

EXTEND CAN FRAME HAVE 29 bits
so we will use it like this

| 21 bits unique id |  8 bits node id |

BUT node ID=0,1 will be reserved for broadcast messages
so valid node IDs are 2-255, which gives us 254 nodes per CAN bus.
Or as some other could say master have always node ID=1
SO messages with NODE ID=1
will be mostly fro configuration protocol, global command broadcast
While unconfigured node will use NODE ID=0 to send some status messages

*/

namespace mcan {

#define CAN_REMOTE_REQUEST_FLAG 0x40000000

static constexpr size_t MAX_STRUCT_SIZE = 16320;

enum class DeviceMode : std::uint8_t {
  UNDEFINED     = 0,
  NORMAL        = 1,
  CONFIGURATION = 2,
};


template <typename T> struct CanMultiPackageFrame {
  static_assert(sizeof(T) <= 16320, "Struct size too big to send over CAN");
  static constexpr size_t expected_index_count = sizeof(T) / 7 + ((sizeof(T) % 7) ? 1 : 0);
  using Type                                   = T::Type;
  Type value;
  std::bitset<expected_index_count> received;
};

inline constexpr uint32_t mcan_connect_msg_id_with_node_id(uint32_t uid_21_bit, uint8_t node_id, bool remote = false) {
  return (uid_21_bit << 8) | node_id | (remote ? CAN_REMOTE_REQUEST_FLAG : 0);
}

template <typename T>
Status mcan_pack_send_msg(mcan::CanBase &can_interface, const T &struct_to_send, int node_id) {
  static_assert(
  std::is_member_pointer_v<decltype(&T::k_base_address)> || requires { T::k_base_address; },
  "Type T must have k_base_address member or constant");
  static_assert(sizeof(T) <= MAX_STRUCT_SIZE, "Struct size too big to send over CAN");
  CanFrame frame;
  frame.id = mcan_connect_msg_id_with_node_id(T::k_base_address, node_id);

  if constexpr(sizeof(T::value) <= 8) {
    frame.size = sizeof(T::value);
    std::memcpy(frame.data, reinterpret_cast<const uint8_t *>(&struct_to_send.value), sizeof(T::value));
    frame.is_extended       = true;
    frame.is_remote_request = false;
    return can_interface.send(frame);
  } else {
    // now since we have to send more than 8 bytes we will have to split the message into
    // multiple can frames, but sine the receiver knows which can id corresponds to which message
    // we can just send them one after another with adding index in the data.
    int total_frames = (sizeof(T)) / 7;
    if((sizeof(T) % 7) != 0) {
      total_frames += 1;
    }
    const uint8_t *data_ptr = reinterpret_cast<const uint8_t *>(&struct_to_send.value);
    for(int frame_index = 0; frame_index < total_frames; ++frame_index) {
      frame.size    = (frame_index == total_frames - 1) ? ((sizeof(T::value) - (frame_index * 7)) + 1) : 8;
      frame.data[0] = static_cast<uint8_t>(frame_index); // first byte is frame index
      std::memcpy(&frame.data[1], &data_ptr[frame_index * 7], frame.size - 1);
      frame.is_extended       = true;
      frame.is_remote_request = false;
      ARI_RETURN_ON_ERROR(can_interface.send(frame));
    }
  }
  return Status::OK();
}

template <typename T>
Status mcan_request_msg(mcan::CanBase &can_interface, const T &struct_to_send, int node_id) {
  (void)struct_to_send; // in case struct is empty and we are not using it to get base address
  static_assert(
  std::is_member_pointer_v<decltype(&T::k_base_address)> || requires { T::k_base_address; },
  "Type T must have k_base_address member or constant");
  static_assert(sizeof(T) <= MAX_STRUCT_SIZE, "Struct size too big to send over CAN");
  CanFrame frame;
  frame.id = mcan_connect_msg_id_with_node_id(T::k_base_address, node_id, true);

  frame.size              = 0;
  frame.is_extended       = true;
  frame.is_remote_request = true;
  return can_interface.send(frame);
}

/// @brief Unpack a received CAN frame into the provided structure.
/// @tparam T The type of the structure to unpack into.
/// @param frame The received CAN frame.
/// @param struct_to_receive The structure to unpack the data into.
/// @return Status of the operation.
/// Status can be Cancelled if more frames are needed to complete the message. Ok if message is complete.
///
template <typename T>
Status mcan_unpack_msg(const CanFrame &frame, CanMultiPackageFrame<T> &struct_to_receive) {
  static_assert(
  std::is_member_pointer_v<decltype(T::k_base_address)> || requires { T::k_base_address; },
  "Type T must have k_base_address member or constant");
  if constexpr(sizeof(T::value) <= 8) {
    if(frame.size != sizeof(T::value)) {
      return Status::Invalid("Received CAN frame size does not match expected size");
    }
    std::memcpy(reinterpret_cast<uint8_t *>(&struct_to_receive.value), frame.data, sizeof(T::value));
    return Status::OK();
  } else {
    // we have to receive multiple frames to reconstruct the message
    size_t index = frame.data[0];
    if(index >= CanMultiPackageFrame<T>::expected_index_count) {
      struct_to_receive.received.reset();
      struct_to_receive.value = {};
      return Status::Invalid("Received CAN frame index out of bounds");
    }
    size_t data_size = frame.size - 1;
    std::memcpy(reinterpret_cast<uint8_t *>(&struct_to_receive.value) + (index * 7), &frame.data[1], data_size);
    struct_to_receive.received.set(index);
    if(struct_to_receive.received.all()) {
      return Status::OK();
    } else {
      return Status::Cancelled("Waiting for more CAN frames to complete the message");
    };
  }
}

} // namespace mcan