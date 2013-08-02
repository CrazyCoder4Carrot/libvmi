/* The LibVMI Library is an introspection library that simplifies access to 
 * memory in a target virtual machine or in a file containing a dump of 
 * a system's physical memory.  LibVMI is based on the XenAccess Library.
 *
 * Copyright 2011 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 *
 * Author: Bryan D. Payne (bdpayne@acm.org)
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/mount.h>

#define MYMODNAME "VMITest"

static int my_init_module(
    void);
static void my_cleanup_module(
    void);

static int
my_init_module(
    void)
{
    struct task_struct *p = NULL;
    unsigned long commOffset;
    unsigned long tasksOffset;
    unsigned long mmOffset;
    unsigned long pidOffset;
    unsigned long stateOffset;
    unsigned long fsOffset;
    unsigned long filesOffset;
    unsigned long pgdOffset;
    unsigned long addrOffset;

    printk(KERN_ALERT "Module %s loaded.\n\n", MYMODNAME);
    p = current;

    if (p != NULL) {
        commOffset = (unsigned long) (&(p->comm)) - (unsigned long) (p);
        tasksOffset =
            (unsigned long) (&(p->tasks)) - (unsigned long) (p);
        mmOffset = (unsigned long) (&(p->mm)) - (unsigned long) (p);
        pidOffset = (unsigned long) (&(p->pid)) - (unsigned long) (p);
        stateOffset = (unsigned long) (&(p->state)) - (unsigned long) (p);
        fsOffset = (unsigned long) (&(p->fs)) - (unsigned long) (p);
        filesOffset = (unsigned long) (&(p->files)) - (unsigned long) (p);
        pgdOffset =
            (unsigned long) (&(p->mm->pgd)) - (unsigned long) (p->mm);
        addrOffset =
            (unsigned long) (&(p->mm->start_code)) -
            (unsigned long) (p->mm);

        printk(KERN_ALERT "[domain name] {\n");
        printk(KERN_ALERT "    ostype = \"Linux\";\n");
        printk(KERN_ALERT "    sysmap = \"[insert path here]\";\n");
        printk(KERN_ALERT "    linux_name = 0x%x;\n",
               (unsigned int) commOffset);
        printk(KERN_ALERT "    linux_tasks = 0x%x;\n",
               (unsigned int) tasksOffset);
        printk(KERN_ALERT "    linux_mm = 0x%x;\n",
               (unsigned int) mmOffset);
        printk(KERN_ALERT "    linux_pid = 0x%x;\n",
               (unsigned int) pidOffset);
        printk(KERN_ALERT "    linux_state = 0x%x;\n",
               (unsigned int) stateOffset);
        printk(KERN_ALERT "    linux_fs = 0x%x;\n",
               (unsigned int) fsOffset);
        printk(KERN_ALERT "    linux_files = 0x%x;\n",
               (unsigned int) filesOffset);
        printk(KERN_ALERT "    linux_pgd = 0x%x;\n",
               (unsigned int) pgdOffset);
        printk(KERN_ALERT "    fdtable = 0x%x;\n",
			   (unsigned long) (&(p->files->fdtab)) - (unsigned long) (p->files));
        printk(KERN_ALERT "    max_fds = 0x%x;\n",
			   (unsigned long) (&(p->files->fdtab.max_fds)) - (unsigned long) (p->files));
        printk(KERN_ALERT "}\n");

        printk(KERN_ALERT "    fd_array = 0x%x;\n",
               (unsigned int) (&(p->files->fd_array)));
        printk(KERN_ALERT "    fd = 0x%x;\n",
               (unsigned int) (p->files->fdt->fd));
		printk(KERN_ALERT "current process-%d-%s opened file count is %d\n", 
			   p->pid, p->comm, p->files->fdt->max_fds);	
		if (p->files->fdt->max_fds >= 4) {
			struct file *fp = p->files->fd_array[0];
			printk(KERN_ALERT "process %d opened file 0 is:\nmntname-%s\ndname-%s\nmode-0x%x\ncount-%ld\nflags-0x%x\n", 
			   p->pid, fp->f_path.mnt->mnt_root->d_name.name, 
			   fp->f_path.dentry->d_name.name, fp->f_mode, fp->f_count.counter, fp->f_flags);	
		}
    }
    else {
        printk(KERN_ALERT
               "%s: found no process to populate task_struct.\n",
               MYMODNAME);
    }

    return 0;
}

static void
my_cleanup_module(
    void)
{
    printk(KERN_ALERT "Module %s unloaded.\n", MYMODNAME);
}

module_init(my_init_module);
module_exit(my_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nilushan Silva");
MODULE_DESCRIPTION("task_struct offset Finder");
