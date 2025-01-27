"""

DPDK Action Profile CLI Test

"""

# in-built module imports
from itertools import count
import time
import sys

# Unittest related imports
import unittest

# ptf related imports
import ptf
#import ptf.dataplane as dataplane
from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf import config

# scapy related imports
from scapy.packet import *
from scapy.fields import *
from scapy.all import *

# framework related imports
import common.utils.ovsp4ctl_utils as ovs_p4ctl
import common.utils.test_utils as test_utils
from common.utils.config_file_utils import get_config_dict, get_gnmi_params_simple
from common.utils.gnmi_cli_utils import gnmi_cli_set_and_verify, gnmi_set_params


class DPDK_Action_Profile_CLI(BaseTest):

    def setUp(self):
        BaseTest.setUp(self)
        self.result = unittest.TestResult()
        
        test_params = test_params_get()
        config_json = test_params['config_json']

        self.config_data = get_config_dict(config_json)

        self.gnmicli_params = get_gnmi_params_simple(self.config_data)


    def runTest(self):
        if not test_utils.gen_dep_files_p4c_ovs_pipeline_builder(self.config_data):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to generate P4C artifacts or pb.bin")

        if not gnmi_cli_set_and_verify(self.gnmicli_params):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to configure gnmi cli ports")


        port_list = self.config_data['port_list']

        # set pipe line
        if not ovs_p4ctl.ovs_p4ctl_set_pipe(self.config_data['switch'], self.config_data['pb_bin'], self.config_data['p4_info']):
            self.result.addFailure(self, sys.exc_info())
            self.fail("Failed to set pipe")
        
        table = self.config_data['table'][0]

        print("Add action profile members")
        member_count = 0
        for member in table['member_details']:
            if not ovs_p4ctl.ovs_p4ctl_add_member_and_verify(table['switch'],table['name'],member):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to add member {member}")
            member_count+=1


        print("Getting action profile members")
        for mem_id in table['del_member']:
            if not ovs_p4ctl.ovs_p4ctl_get_member(table['switch'],table['name'],member_id=mem_id):
                self.result.addFailure(self, sys.exc_info())
                self.fail(f"Failed to get member {mem_id}")

                 
        print("Deleting members")    
        for index,del_mem in enumerate(table['del_member']):
            if index == 0 or index == 7:
                ovs_p4ctl.ovs_p4ctl_del_member(table['switch'],table['name'],del_mem)


        print("Checking deleted members")    
        for index,mem_id in enumerate(table['del_member']):
            if index == 0 or index == 7:
                if ovs_p4ctl.ovs_p4ctl_get_member(table['switch'],table['name'],member_id=mem_id):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Deleted member check failed for {mem_id}")
                else:
                    print("PASS: Expected failure message for deleted member")


        print("Checking undeleted member")
        for index,mem_id in enumerate(table['del_member']):
            if index in range(1,7): 
                if not ovs_p4ctl.ovs_p4ctl_get_member(table['switch'],table['name'],member_id=mem_id):
                    self.result.addFailure(self, sys.exc_info())
                    self.fail(f"Failed to get member {mem_id}")

    def tearDown(self):

        if self.result.wasSuccessful():
            print("Test has PASSED")
        else:
            print("Test has FAILED")

 
