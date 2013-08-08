/* The LibVMI Library is an introspection library that simplifies access to 
 * memory in a target virtual machine or in a file containing a dump of 
 * a system's physical memory.  LibVMI is based on the XenAccess Library.
 *
 * Copyright 2011 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 *
 * Author: Yutao Liu (mctrain016@gmail.com)
 *
 * This file is part of LibVMI.
 *
 * LibVMI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * LibVMI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with LibVMI.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libvmi/libvmi.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>

int main (int argc, char **argv)
{
    vmi_instance_t vmi;
    unsigned char *memory = NULL;
    uint32_t offset;
    addr_t sys_table_addr = 0;
    status_t status;
    FILE *f = NULL;

    /* this is the VM or file that we are looking at */
    if (argc != 3) {
        printf("Usage: %s <vmname> <filename>\n", argv[0]);
        return 1;
    } // if

    char *name = argv[1];
    char *filename = argv[2];

    /* initialize the libvmi library */
    if (vmi_init(&vmi, VMI_AUTO | VMI_INIT_COMPLETE, name) == VMI_FAILURE) {
        printf("Failed to init LibVMI library.\n");
        return 1;
    }

    /* pause the vm for consistent memory access */
    if (vmi_pause_vm(vmi) != VMI_SUCCESS) {
        printf("Failed to pause VM\n");
        goto error_exit;
    } // if

    /* demonstrate name and id accessors */
    char *name2 = vmi_get_name(vmi);

    if (VMI_FILE != vmi_get_access_mode(vmi)) {
        unsigned long id = vmi_get_vmid(vmi);

        printf("Process listing for VM %s (id=%lu)\n", name2, id);
    }
    else {
        printf("Process listing for file %s\n", name2);
    }
    free(name2);

    /* get the head of the list */
    if (VMI_OS_LINUX == vmi_get_ostype(vmi)) {
        /* Begin at PID 0, the 'swapper' task. It's not typically shown by OS
         *  utilities, but it is indeed part of the task list and useful to
         *  display as such.
         */
        sys_table_addr = vmi_translate_ksym2v(vmi, "ia32_sys_call_table");
        printf("ia32_sys_call_table virtual address is 0x%x\n", (int)sys_table_addr);
    }

    int i = 0;
    addr_t valid_sys_call_addr = 0, sys_call_table_addr = 0;;

    /* open the file for reading*/
    f = fopen(filename, "r");
    do {
        fscanf(f, "%d %x\n", &i, &valid_sys_call_addr);
        vmi_read_32_va(vmi, sys_table_addr, 0, &sys_call_table_addr);
        if (valid_sys_call_addr != sys_call_table_addr) {
            printf("%dth entry (0x%08x) modified from 0x%x to 0x%x\n", i, 
                    sys_table_addr, valid_sys_call_addr, sys_call_table_addr);
            goto error_exit;
        }
        printf("sys_call_table_entry-%d-0x%08x: 0x%08x - 0x%08x\n", i, (int)sys_table_addr, 
                valid_sys_call_addr, sys_call_table_addr);
        sys_table_addr += 8;
    } while (i < 324);

error_exit:

    fclose(f);
    /* resume the vm */
    vmi_resume_vm(vmi);

    /* cleanup any memory associated with the LibVMI instance */
    vmi_destroy(vmi);

    return 0;
}
