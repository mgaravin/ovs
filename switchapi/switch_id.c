/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2019 Barefoot Networks, Inc.

 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Barefoot Networks,
 * Inc.
 * and its suppliers and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is
 * strictly forbidden unless prior written permission is obtained from
 * Barefoot Networks, Inc.
 *
 * No warranty, explicit or implicit is provided, unless granted under a
 * written agreement with Barefoot Networks, Inc.
 *
 * $Id: $
 *
 ******************************************************************************/
#include "switch_id.h"
#include "switch_internal.h"
#include <openvswitch/vlog.h>
#include <config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define __FILE_ID__ SWITCH_ID
VLOG_DEFINE_THIS_MODULE(switch_id);

static switch_status_t switch_api_id_allocator_new_internal(
    switch_device_t device,
    switch_uint32_t initial_size,
    bool zero_based,
    switch_id_allocator_t **allocator) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  *allocator = SWITCH_MALLOC(device, sizeof(switch_id_allocator_t), 1);
  if (*allocator == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    VLOG_ERR("id alloc failed %s", switch_error_to_string(status));
    return status;
  }

  (*allocator)->data =
      SWITCH_MALLOC(device, sizeof(switch_uint32_t), initial_size);
  if ((*allocator)->data == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_FREE(device, *allocator);
    VLOG_ERR("id alloc failed %s", switch_error_to_string(status));
    return status;
  }

  (*allocator)->n_words = initial_size;
  (*allocator)->zero_based = zero_based;
  SWITCH_MEMSET((*allocator)->data, 0, initial_size * sizeof(switch_uint32_t));
  return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_api_id_allocator_destroy_internal(
    switch_device_t device, switch_id_allocator_t *allocator) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    VLOG_ERR("id destroy failed %s", switch_error_to_string(status));
    return status;
  }

  SWITCH_FREE(device, allocator->data);
  SWITCH_FREE(device, allocator);
  return SWITCH_STATUS_SUCCESS;
}

static inline switch_int32_t switch_api_id_fit_width(switch_uint32_t val,
                                                     switch_uint32_t width) {
  switch_uint32_t offset = 32;
  switch_uint32_t mask = 0;
  switch_uint32_t b = 0;

  while (offset >= width) {
    mask = (((switch_uint32_t)1 << width) - 1) << (offset - width);
    b = val & mask;
    if (!b) {
      return offset;
    }
    offset = __builtin_ctz(b);
  }
  return -1;
}

static switch_status_t switch_api_id_allocator_allocate_contiguous_internal(
    switch_device_t device,
    switch_id_allocator_t *allocator,
    switch_uint8_t count,
    switch_uint32_t *id) {
  switch_uint32_t n_words = 0;
  switch_uint32_t i = 0;
  switch_int32_t pos = -1;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    VLOG_ERR("id alloc failed %s", switch_error_to_string(status));
    return status;
  }

  for (i = 0; i < allocator->n_words; i++) {
    if (allocator->data[i] != 0xFFFFFFFF) {
      pos = switch_api_id_fit_width(allocator->data[i], count);
      if (pos > 0) {
        // set the bitmap to 1s
        allocator->data[i] |= (0xFFFFFFFF << (pos - count)) & 0xFFFFFFFF;
        *id = 32 * i + (32 - pos) + (allocator->zero_based ? 0 : 1);
        return status;
      }
    }
  }

  n_words = allocator->n_words;
  allocator->data = SWITCH_REALLOC(
      device, allocator->data, n_words * 2 * sizeof(switch_uint32_t));

  SWITCH_MEMSET(
      &allocator->data[n_words], 0, n_words * sizeof(switch_uint32_t));
  allocator->n_words = n_words * 2;
  allocator->data[n_words] |= (0xFFFFFFFF << (32 - count)) & 0xFFFFFFFF;
  *id = 32 * n_words + (allocator->zero_based ? 0 : 1);
  return status;
}

static switch_status_t switch_api_id_allocator_allocate_internal(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t *id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    VLOG_ERR("id alloc failed %s", switch_error_to_string(status));
    return status;
  }

  return switch_api_id_allocator_allocate_contiguous(device, allocator, 1, id);
}

static switch_status_t switch_api_id_allocator_release_internal(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!allocator) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    VLOG_ERR("id release failed %s", switch_error_to_string(status));
    return status;
  }

  if (allocator->zero_based != true) {
    id = id > 0 ? id - 1 : 0;
  }
  allocator->data[id >> 5] &= ~(1U << (31 - id % 32));
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_id_allocator_destroy(
    switch_device_t device, switch_id_allocator_t *allocator) {
  return switch_api_id_allocator_destroy_internal(device, allocator);
}

switch_status_t switch_api_id_allocator_new(switch_device_t device,
                                            switch_uint32_t initial_size,
                                            bool zero_based,
                                            switch_id_allocator_t **allocator) {
  return switch_api_id_allocator_new_internal(device, initial_size, zero_based,
                                       allocator);
}

switch_status_t switch_api_id_allocator_allocate(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t *id) {
    return switch_api_id_allocator_allocate_internal(device, allocator, id);
}

switch_status_t switch_api_id_allocator_release(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t id) {
    return switch_api_id_allocator_release_internal(device, allocator, id);
}

switch_status_t switch_api_id_allocator_allocate_contiguous(
    switch_device_t device,
    switch_id_allocator_t *allocator,
    switch_uint8_t count,
    switch_id_t *id) {
  return switch_api_id_allocator_allocate_contiguous_internal(device, allocator,
                                                       count, id);
}
