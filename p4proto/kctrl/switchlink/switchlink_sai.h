/*
 * Copyright (c) 2022 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SWITCHLINK_SAI_H__
#define __SWITCHLINK_SAI_H__

#include <saitypes.h>
#include "switchlink.h"
#include "switchlink_link.h"
#include "switchlink_neigh.h"
#include "switchlink_db.h"

extern void switchlink_api_init(void);

extern int switchlink_vrf_create(switchlink_handle_t *vrf_h);

extern int switchlink_interface_create(switchlink_db_interface_info_t *intf,
                                       switchlink_handle_t *intf_h);

extern int switchlink_interface_delete(switchlink_db_interface_info_t *intf,
                                       switchlink_handle_t intf_h);

extern int switchlink_lag_create(switchlink_handle_t *lag_h);

extern int switchlink_mac_create(switchlink_mac_addr_t mac_addr,
                                 switchlink_handle_t bridge_h,
                                 switchlink_handle_t intf_h);

extern int switchlink_mac_delete(switchlink_mac_addr_t mac_addr,
                                 switchlink_handle_t bridge_h);

extern int switchlink_neighbor_create(switchlink_db_neigh_info_t *neigh_info);

extern int switchlink_neighbor_delete(switchlink_db_neigh_info_t *neigh_info);

extern int switchlink_nexthop_create(switchlink_db_nexthop_info_t *nexthop_info);

extern int switchlink_nexthop_delete(switchlink_handle_t nhop_h);

extern int switchlink_ecmp_create(switchlink_db_ecmp_info_t *ecmp_info);

extern int switchlink_ecmp_delete(switchlink_db_ecmp_info_t *ecmp_info);

extern int switchlink_route_create(switchlink_db_route_info_t *route_info);

extern int switchlink_route_delete(switchlink_db_route_info_t *route_info);

extern int switchlink_tunnel_interface_create(
                                switchlink_db_tunnel_interface_info_t *tnl_intf,
                                switchlink_handle_t *tnl_intf_h,
                                switchlink_handle_t *tnl_term_h);
extern int switchlink_tunnel_interface_delete(
                              switchlink_db_tunnel_interface_info_t *tnl_intf);

extern sai_status_t switchlink_create_tunnel(
                                switchlink_db_tunnel_interface_info_t *tnl_intf,
                                switchlink_handle_t *tnl_intf_h);

extern sai_status_t switchlink_remove_tunnel_term_table_entry(
                        switchlink_db_tunnel_interface_info_t *tnl_intf);

extern sai_status_t switchlink_remove_tunnel(
                        switchlink_db_tunnel_interface_info_t *tnl_intf);

extern sai_status_t switchlink_create_term_table_entry(
                            switchlink_db_tunnel_interface_info_t *tnl_intf,
                            switchlink_handle_t *tnl_term_intf_h);

#endif /* __SWITCHLINK_SAI_H__ */
