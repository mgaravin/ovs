/*
Copyright 2013-present Barefoot Networks, Inc.
Copyright(c) 2021 Intel Corporation.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <openvswitch/vlog.h>
#include "switch_internal.h"
#include "switch_base_types.h"
#include "switch_pd_utils.h"
#include <config.h>

#include <bf_types/bf_types.h>
#include <port_mgr/dpdk/bf_dpdk_port_if.h>
#include "bf_rt/bf_rt_common.h"
#include "bf_rt/bf_rt_session.h"
#include "bf_rt/bf_rt_init.h"
#include "bf_rt/bf_rt_info.h"
#include "bf_rt/bf_rt_table.h"
#include "bf_rt/bf_rt_table_key.h"
#include "bf_rt/bf_rt_table_data.h"

VLOG_DEFINE_THIS_MODULE(switch_pd_utils);

// TODO: check if session can be created only once and re0use it
bf_status_t switch_pd_allocate_handle_session(const bf_dev_id_t device_id,
                                              const char *pipeline_name,
                                              bf_rt_info_hdl **bfrt_info_hdl_t,
                                              bf_rt_session_hdl **session_t) {
  bf_status_t status;
  
  VLOG_INFO("%s", __func__);

  status = bf_rt_session_create(session_t);
  if(status != BF_SUCCESS) {
      VLOG_ERR("Cannot create bfrt session");
      return status;
  }

  *bfrt_info_hdl_t = (bf_rt_info_hdl *)malloc(sizeof(bf_rt_info_hdl));
  status = bf_rt_info_get(device_id, pipeline_name,
                          (const bf_rt_info_hdl **)bfrt_info_hdl_t);
  if(status != BF_SUCCESS) {
      VLOG_ERR("Cannot get bfrt handle for pipeline: %s", pipeline_name);
      return status;
  }

  return status;
}

bf_status_t switch_pd_deallocate_handle_session(bf_rt_table_key_hdl *key_hdl_t,
                                                bf_rt_table_data_hdl *data_hdl_t,
                                                bf_rt_session_hdl *session_t,
                                                bool entry_type) {

  bf_status_t status;

  VLOG_INFO("%s", __func__);

  if (entry_type) {
      // Data handle is created only when entry is added to backend
      status = bf_rt_table_data_deallocate(data_hdl_t);
      if(status != BF_SUCCESS) {
          VLOG_ERR("Cannot deallocate data handler");
          return status;
      }
  }

  status = bf_rt_table_key_deallocate(key_hdl_t);
  if(status != BF_SUCCESS) {
      VLOG_ERR("Cannot deallocate key handler");
      return status;
  }

  status = bf_rt_session_destroy(session_t);
  if(status != BF_SUCCESS) {
      VLOG_ERR("Cannot destroy session");
      return status;
  }

  return status;
}

