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
    addr_t list_head = 0, current_list_entry = 0, next_list_entry = 0, files_struct_list = 0, fdt_struct_list = 0;
    addr_t current_process = 0, files_struct = 0, fdt_struct = 0;
    addr_t tmp_next = 0;
    vmi_pid_t pid, mypid = 0;
    vmi_state_t state = 0;
    unsigned long tasks_offset, pid_offset, name_offset, state_offset, files_offset;
    status_t status;

    /* this is the VM or file that we are looking at */
    if (argc != 3) {
        printf("Usage: %s <vmname> <pid>\n", argv[0]);
        return 1;
    } // if

    char *name = argv[1];
    mypid = atoi(argv[2]);

    /* initialize the libvmi library */
    if (vmi_init(&vmi, VMI_AUTO | VMI_INIT_COMPLETE, name) == VMI_FAILURE) {
        printf("Failed to init LibVMI library.\n");
        return 1;
    }

    /* init the offset values */
    if (VMI_OS_LINUX == vmi_get_ostype(vmi)) {
        tasks_offset = vmi_get_offset(vmi, "linux_tasks");
        name_offset = vmi_get_offset(vmi, "linux_name");
        pid_offset = vmi_get_offset(vmi, "linux_pid");
        files_offset = vmi_get_offset(vmi, "linux_files");
        state_offset = vmi_get_offset(vmi, "linux_state");
		printf("Offset: tasks-0x%x name-0x%x pid-0x%x state-0x%x files-0x%x\n", 
			   tasks_offset, name_offset, pid_offset, state_offset, files_offset);
    }
    else if (VMI_OS_WINDOWS == vmi_get_ostype(vmi)) {
        tasks_offset = vmi_get_offset(vmi, "win_tasks");
        if (0 == tasks_offset) {
            printf("Failed to find win_tasks\n");
            goto error_exit;
        }
        name_offset = vmi_get_offset(vmi, "win_pname");
        if (0 == name_offset) {
            printf("Failed to find win_pname\n");
            goto error_exit;
        }
        pid_offset = vmi_get_offset(vmi, "win_pid");
        if (0 == pid_offset) {
            printf("Failed to find win_pid\n");
            goto error_exit;
        }
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
        current_process = vmi_translate_ksym2v(vmi, "init_task");
    }
    else if (VMI_OS_WINDOWS == vmi_get_ostype(vmi)) {

        // find PEPROCESS PsInitialSystemProcess
        vmi_read_addr_ksym(vmi, "PsInitialSystemProcess", &current_process);

    }

    /* walk the task list */
    list_head = current_process + tasks_offset;
    current_list_entry = list_head;

    status = vmi_read_addr_va(vmi, current_list_entry, 0, &next_list_entry);
    if (status == VMI_FAILURE) {
        printf("Failed to read next pointer at 0x%lx before entering loop\n",
                current_list_entry);
        goto error_exit;
    }

    do {
        /* Note: the task_struct that we are looking at has a lot of
         * information.  However, the process name and id are burried
         * nice and deep.  Instead of doing something sane like mapping
         * this data to a task_struct, I'm just jumping to the location
         * with the info that I want.  This helps to make the example
         * code cleaner, if not more fragile.  In a real app, you'd
         * want to do this a little more robust :-)  See
         * include/linux/sched.h for mode details */

        /* NOTE: _EPROCESS.UniqueProcessId is a really VOID*, but is never > 32 bits,
         * so this is safe enough for x64 Windows for example purposes */
        vmi_read_32_va(vmi, current_process + pid_offset, 0, &pid);

		/* print out the current running process id */
		if (pid == mypid) {
        	char *procname = vmi_read_str_va(vmi, current_process + name_offset, 0);
        	printf("process-%d-%s opened files information:\n", pid, procname);
			files_struct_list = current_process + files_offset;
			vmi_read_addr_va(vmi, files_struct_list, 0, &files_struct);
			int count = 0;
			vmi_read_32_va(vmi, files_struct, 0, &count);
			printf("count is %d\n", count);
			fdt_struct_list = files_struct + 0x10;
			int max_fds = 0;
			vmi_read_32_va(vmi, fdt_struct_list, 0, &max_fds);
			printf("max_fds is %d\n", max_fds);
        
			break;
		}

        current_list_entry = next_list_entry;
        current_process = current_list_entry - tasks_offset;

        /* follow the next pointer */

        status = vmi_read_addr_va(vmi, current_list_entry, 0, &next_list_entry);
        if (status == VMI_FAILURE) {
            printf("Failed to read next pointer in loop at %lx\n", current_list_entry);
            goto error_exit;
        }

    } while (next_list_entry != list_head);

    error_exit:

    /* resume the vm */
    vmi_resume_vm(vmi);

    /* cleanup any memory associated with the LibVMI instance */
    vmi_destroy(vmi);

    return 0;
}