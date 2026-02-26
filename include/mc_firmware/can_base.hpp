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

#include "status.hpp"
#include <condition_variable>
#include <functional>

namespace mcan {

static constexpr uint32_t CAN_ANY_FRAME = 0;

/// @brief CAN data frame.
struct CanFrame
{

  /// @brief CAN ID of the frame. Either standard (11 bits) or extended (29 bits).
  uint32_t id;

  /// @brief Length of the data in the frame. Maximum length is 64 bytes.
  uint8_t size;

  /// @brief Data of the frame. Maximum length is 64 bytes.
  uint8_t data[8];

  /// @brief Flag to indicate if the frame is a remote request.
  bool is_remote_request;

  /// @brief Flag to indicate if the frame is an extended frame.
  bool is_extended;
};

class CanBase
{
 public:
  using can_callback_type = std::function<void(CanBase&, const CanFrame&, void*)>;
  virtual ~CanBase(){};

  /// @brief Send a CAN frame to the CAN bus.
  /// @param frame The CAN frame to send.
  /// @note The frame will be sent to the CAN bus immediately if the driver is not
  /// threaded.
  /// @return Status of the operation.
  virtual Status send(const CanFrame& frame) = 0;

  /// @brief Send can frame and wait for response with specific CAN ID.
  /// @param frame The CAN frame to send.
  /// @param response_id The CAN ID of the expected response frame. If CAN_ANY_FRAME is
  /// used the first received frame will be returned.
  /// @param timeout_ms  The timeout in milliseconds to wait for the response frame.
  /// @return Result containing the received CAN frame or an error status.
  virtual Result<CanFrame> send_await_response(const CanFrame& frame,
                                               uint32_t response_id,
                                               uint32_t timeout_ms = 1000) = 0;

  /// @brief Add a callback for a specific CAN ID.
  /// @param id The CAN ID to listen for, if you want to receive callback for a remote
  /// request you have to set by adding specific bit to ID
  /// @param callback The callback function to call when a frame with the specified ID is
  /// received.
  /// @param args Optional arguments to pass to the callback function.
  /// @return Status of the operation.
  virtual Status add_callback(uint32_t id,
                              can_callback_type callback,
                              void* args = nullptr) = 0;

  /// @brief Add a callback for a specific CAN IDs with mask.
  /// so one callback can be used for multiple CAN IDs by using mask to specify which bits
  /// of the ID should be matched.
  /// @param id_base The base CAN ID to listen for, the bits that are not
  /// masked will be ignored when matching incoming frames.
  /// @param id_mask The mask to apply to incoming CAN IDs when matching, bits set
  /// to 1 in the mask will be compared to the corresponding bits in the base ID, bits set
  /// to 0 will be ignored.
  /// @param callback The callback function to call when a frame with the specified ID is
  /// received.
  /// @param args Optional arguments to pass to the callback function.
  /// @return Status of the operation.
  /// @note When using masked callbacks, if a id matches a masked callback and a
  /// non-masked callback, only the regural callback will be called, and the masked one
  /// will be ignored. if matching multiple masked callbacks, only the first one that
  /// matches will be called, the order of matching is not guaranteed, so if you have
  /// multiple masked callbacks that can match the same ID, you should make sure that the
  /// most specific one (the one with the most bits set in the mask) is added first, to
  /// ensure that it will be matched before less specific ones. will be called.
  virtual Status add_callback_masked(uint32_t id_base,
                                     uint32_t id_mask,
                                     can_callback_type callback,
                                     void* args = nullptr) = 0;

  /// @brief Remove a callback for a specific CAN ID.
  /// @param id The CAN ID to remove the callback for.
  /// @return Status of the operation.
  virtual Status remove_callback(uint32_t id) = 0;

  /// @brief Remove a masked callback for a specific CAN ID.
  /// @param id_base The base CAN ID of the callback to remove.
  /// @param id_mask The mask of the callback to remove.
  /// @return Status of the operation.
  virtual Status remove_callback_masked(uint32_t id_base, uint32_t id_mask) = 0;

  /// @brief Open the CAN socket.
  /// this should create two threads to handle CAN tx and rx with callbacks.
  /// @return Status of the operation.
  virtual Status open_can() = 0;

  /// @brief Close the CAN .
  /// @return Status of the operation.
  virtual Status close_can() = 0;
};

}; // namespace mcan