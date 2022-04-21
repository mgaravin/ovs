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

#include <sainexthop.h>
#include "saiinternal.h"
#include <config.h>
#include <switchapi/switch_nhop.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_tunnel.h>
#include <openvswitch/vlog.h>

VLOG_DEFINE_THIS_MODULE(sainexthop);

//static sai_api_t api_id = SAI_API_NEXT_HOP;

static switch_nhop_type_t sai_nhop_type_to_switch_nhop_type(
    sai_next_hop_type_t sai_type) {
  switch (sai_type) {
    case SAI_NEXT_HOP_TYPE_IP:
      return SWITCH_NHOP_TYPE_IP;
    case SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP:
      return SWITCH_NHOP_TYPE_TUNNEL;
    case SAI_NEXT_HOP_TYPE_MPLS:
      return SWITCH_NHOP_TYPE_MPLS;
    case SAI_NEXT_HOP_TYPE_SRV6_SIDLIST:
    default:
      return SWITCH_NHOP_TYPE_NONE;
  }
}

/*
* Routine Description:
*    Create next hop
*
* Arguments:
*    [out] next_hop_id - next hop id
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
static sai_status_t sai_create_next_hop_entry(_Out_ sai_object_id_t *next_hop_id,
                                       _In_ sai_object_id_t switch_id,
                                       _In_ uint32_t attr_count,
                                       _In_ const sai_attribute_t *attr_list) {

  const sai_attribute_t *attribute;
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t index = 0;
  const sai_ip_address_t *sai_ip_addr;
  switch_api_nhop_info_t api_nhop_info;
  sai_next_hop_type_t nhtype = SAI_NEXT_HOP_TYPE_IP;
  switch_handle_t next_hop_handle = SWITCH_API_INVALID_HANDLE;
  *next_hop_id = SAI_NULL_OBJECT_ID;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    VLOG_ERR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  memset(&api_nhop_info, 0, sizeof(switch_api_nhop_info_t));
  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_NEXT_HOP_ATTR_TYPE:
        nhtype = attribute->value.s32;
        api_nhop_info.nhop_type =
            sai_nhop_type_to_switch_nhop_type(attribute->value.s32);
        break;
      case SAI_NEXT_HOP_ATTR_IP:
        sai_ip_addr = &attribute->value.ipaddr;
        sai_ip_addr_to_switch_ip_addr(sai_ip_addr, &api_nhop_info.ip_addr);
        break;
      case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
        api_nhop_info.rif_handle = (switch_handle_t)attribute->value.oid;
        break;
      case SAI_NEXT_HOP_ATTR_TUNNEL_ID:
        api_nhop_info.nhop_tunnel_type = SWITCH_NHOP_TUNNEL_TYPE_VRF;
        api_nhop_info.tunnel_handle = (switch_handle_t)attribute->value.oid;
        break;
      case SAI_NEXT_HOP_ATTR_TUNNEL_VNI:
        api_nhop_info.tunnel_vni = attribute->value.u32;
        break;
      case SAI_NEXT_HOP_ATTR_TUNNEL_MAC:
        memcpy(&api_nhop_info.mac_addr,
               attribute->value.mac,
               sizeof(switch_mac_addr_t));
        break;
      default:
        return SAI_STATUS_INVALID_PARAMETER;
    }
  }

  api_nhop_info.nhop_type = sai_nhop_type_to_switch_nhop_type(nhtype);

  VLOG_INFO("Calling switch api nhop create");
  status = switch_api_nhop_create(0, &api_nhop_info, &next_hop_handle);
  if (status != SAI_STATUS_SUCCESS) {
    VLOG_ERR("failed to create nexthop: %s", sai_status_to_string(status));
  } else {
    *next_hop_id = next_hop_handle;
  }

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Remove next hop
*
* Arguments:
*    [in] next_hop_id - next hop id
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
static sai_status_t sai_remove_next_hop_entry(_In_ sai_object_id_t next_hop_id) {

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  if (sai_object_type_query(next_hop_id) != SAI_OBJECT_TYPE_NEXT_HOP) {
    VLOG_ERR("nexthop remove failed: invalid nexthop handle %lx\n",
                  next_hop_id);
    return SAI_STATUS_INVALID_PARAMETER;
  }

  VLOG_INFO("Calling switch api nhop delete");
  switch_status = switch_api_nhop_delete(0, (switch_handle_t)next_hop_id);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SAI_STATUS_SUCCESS) {
    VLOG_ERR("failed to remove nexthop %lx: %s",
                  next_hop_id,
                  sai_status_to_string(status));
  }

  return (sai_status_t)status;
}

/*
*  Next Hop methods table retrieved with sai_api_query()
*/
sai_next_hop_api_t nhop_api = {
    .create_next_hop = sai_create_next_hop_entry,
    .remove_next_hop = sai_remove_next_hop_entry};

sai_status_t sai_next_hop_initialize(sai_api_service_t *sai_api_service) {
  VLOG_DBG("Initializing nexthop");
  sai_api_service->nhop_api = nhop_api;
  return SAI_STATUS_SUCCESS;
}
