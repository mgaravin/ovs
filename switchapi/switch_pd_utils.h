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

#include "switch_base_types.h"
#include "switch_handle.h"
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
#include "bf_pal/bf_pal_port_intf.h"

#ifndef __SWITCH_PD_UTILS_H__
#define __SWITCH_PD_UTILS_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define PROGRAM_NAME "linux_networking"

// Currently this value is picked from dpdk_port_config.pb.txt
#define MAX_NO_OF_PORTS 56

#define RMAC_BASE 0
#define RMAC_BYTES_OFFSET 2
#define RMAC_START_OFFSET RMAC_BASE
#define RMAC_MID_OFFSET RMAC_START_OFFSET+RMAC_BYTES_OFFSET
#define RMAC_END_OFFSET RMAC_MID_OFFSET+RMAC_BYTES_OFFSET


bf_status_t switch_pd_allocate_handle_session(const bf_dev_id_t device_id,
                                              const char *pipeline_name,
                                              bf_rt_info_hdl **bfrt_info_hdl_t,
                                              bf_rt_session_hdl **session_t);

bf_status_t switch_pd_deallocate_handle_session(bf_rt_table_key_hdl *key_hdl_t,
                                                bf_rt_table_data_hdl *data_hdl_t,
                                                bf_rt_session_hdl *session_t,
                                                bool entry_type);

int switch_pd_to_get_port_id(uint32_t rif_ifindex);

#ifdef  __cplusplus
}
#endif

#endif

