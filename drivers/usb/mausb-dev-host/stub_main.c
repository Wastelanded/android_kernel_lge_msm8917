/*
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */

#include <linux/string.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>



#include "mausb_common.h"
#include "stub.h"
#include "mausb_util.h"

#define DRIVER_AUTHOR "Takahiro Hirofuchi"
#define DRIVER_DESC "USB/IP Host Driver"

#define DEVICE_BUS_ID	"1-1"

struct kmem_cache *stub_priv_cache;
struct kmem_cache *stub_mausb_pal_cache;

/*
 * busid_tables defines matching busids that mausb can grab. A user can change
 * dynamically what device is locally used and what device is exported to a
 * remote host.
 */
#define MAX_BUSID 16
static struct bus_id_priv busid_table[MAX_BUSID];
static spinlock_t busid_table_lock;



static void init_busid_table(void)
{
	/*
	 * This also sets the bus_table[i].status to
	 * MAUSB_STUB_BUSID_OTHER, which is 0.
	 */
	memset(busid_table, 0, sizeof(busid_table));

//	strncpy(busid_table[0].name, DEVICE_BUS_ID, BUSID_SIZE);
//	busid_table[0].status = MAUSB_STUB_BUSID_ADDED;
	

	spin_lock_init(&busid_table_lock);
	printk(KERN_INFO"lock3: %p",(void*)&busid_table_lock);
}

/*
 * Find the index of the busid by name.
 * Must be called with busid_table_lock held.
 */
static int get_busid_idx(const char *busid)
{
	int i;
	int idx = -1;

	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i].name[0]) {
			printk(KERN_INFO "busid: %s \n",busid_table[i].name);
			if (!strncmp(busid_table[i].name, busid, BUSID_SIZE)) {
				idx = i;
				break;
			}
		}	
	return idx;
}

struct bus_id_priv *get_busid_priv(const char *busid)
{
	int idx;
	struct bus_id_priv *bid = NULL;

	spin_lock(&busid_table_lock);
	idx = get_busid_idx(busid);
	if (idx >= 0)
		bid = &(busid_table[idx]);
	spin_unlock(&busid_table_lock);

	return bid;
}

static int add_match_busid(char *busid)
{
	int i;
	int ret = -1;
	printk(KERN_INFO "match busid: %s \n",busid);
	spin_lock(&busid_table_lock);
	/* already registered? */
	if (get_busid_idx(busid) >= 0) {
		ret = 0;
		goto out;
	}

	for (i = 0; i < MAX_BUSID; i++)
		if (!busid_table[i].name[0]) {
			strncpy(busid_table[i].name, busid, BUSID_SIZE);
			if ((busid_table[i].status != MAUSB_STUB_BUSID_ALLOC) &&
			    (busid_table[i].status != MAUSB_STUB_BUSID_REMOV))
				busid_table[i].status = MAUSB_STUB_BUSID_ADDED;
			ret = 0;
			break;
		}

out:
	spin_unlock(&busid_table_lock);

	return ret;
}

int del_match_busid(char *busid)
{
	int idx;
	int ret = -1;

	spin_lock(&busid_table_lock);
	idx = get_busid_idx(busid);
	if (idx < 0)
		goto out;

	/* found */
	ret = 0;

	if (busid_table[idx].status == MAUSB_STUB_BUSID_OTHER)
		memset(busid_table[idx].name, 0, BUSID_SIZE);

	if ((busid_table[idx].status != MAUSB_STUB_BUSID_OTHER) &&
	    (busid_table[idx].status != MAUSB_STUB_BUSID_ADDED))
		busid_table[idx].status = MAUSB_STUB_BUSID_REMOV;

out:
	spin_unlock(&busid_table_lock);

	return ret;
}

static ssize_t show_match_busid(struct device_driver *drv, char *buf)
{
	int i;
	char *out = buf;
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "---> %s", __func__);
	spin_lock(&busid_table_lock);
	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i].name[0])
			out += sprintf(out, "%s ", busid_table[i].name);
	spin_unlock(&busid_table_lock);
	out += sprintf(out, "\n");

	return out - buf;
}

static ssize_t store_match_busid(struct device_driver *dev, const char *buf,
				 size_t count)
{
	int len;
	char busid[BUSID_SIZE];

	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "\n---> %s",__func__);
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "\n count %zu  Buffer: %s", count, buf);
	if (count < 5)
		return -EINVAL;

	/* strnlen() does not include \0 */
	len = strnlen(buf + 4, BUSID_SIZE);

	/* busid needs to include \0 termination */
	if (!(len < BUSID_SIZE))
		return -EINVAL;

	strncpy(busid, buf + 4, BUSID_SIZE);
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "\n BusID: %s", busid);
	if (!strncmp(buf, "add ", 4)) {
		if (add_match_busid(busid) < 0)
			return -ENOMEM;

		pr_debug("add busid %s\n", busid);
		return count;
	}

	if (!strncmp(buf, "del ", 4)) {
		if (del_match_busid(busid) < 0)
			return -ENODEV;

		pr_debug("del busid %s\n", busid);
		return count;
	}

	return -EINVAL;
}
static DRIVER_ATTR(match_busid, S_IRUSR | S_IWUSR, show_match_busid,
		   store_match_busid);

static struct stub_priv *stub_priv_pop_from_listhead(struct list_head *listhead)
{
	struct stub_priv *priv, *tmp;

	list_for_each_entry_safe(priv, tmp, listhead, list) {
		list_del(&priv->list);
		return priv;
	}

	return NULL;
}


static struct stub_mausb_pal *stub_mausb_pal_pop(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_mausb_pal *pal;

	spin_lock_irqsave(&sdev->mausb_pal_lock, flags);

	pal = (struct stub_mausb_pal *)stub_priv_pop_from_listhead(&sdev->mausb_pal_in_init);
	if (pal)
		goto done1;
	
	pal = (struct stub_mausb_pal *)stub_priv_pop_from_listhead(&sdev->mausb_pal_out_init);
	if (pal)
		goto done1;

	pal = (struct stub_mausb_pal *)stub_priv_pop_from_listhead(&sdev->mausb_pal_mgmt_init);
	if (pal)
		goto done1;

	pal = (struct stub_mausb_pal *)stub_priv_pop_from_listhead(&sdev->mausb_pal_tx);
	if (pal)
		goto done1;
	
	pal = (struct stub_mausb_pal *)stub_priv_pop_from_listhead(&sdev->mausb_pal_free);
	if (pal)
		goto done1;
	
done1:	
	spin_unlock_irqrestore(&sdev->mausb_pal_lock, flags);
	return pal;
	
}


static struct stub_priv *stub_priv_pop(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv = NULL;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	priv = stub_priv_pop_from_listhead(&sdev->priv_init);
	if (priv)
		goto done;

	priv = stub_priv_pop_from_listhead(&sdev->priv_tx);
	if (priv)
		goto done;

	priv = stub_priv_pop_from_listhead(&sdev->priv_free);

done:
	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return priv;
}

void stub_device_cleanup_urbs(struct stub_device *sdev)
{
	struct stub_priv *priv;
	struct stub_mausb_pal *pal;
	struct urb *urb;

	printk(KERN_INFO " %s  \n",__func__);
	
	if(sdev==NULL)
	{
		return;
	}
	dev_dbg(&sdev->udev->dev, "free sdev %p\n", sdev);



	while ((pal = stub_mausb_pal_pop(sdev))) {
		urb = pal->urb;
		dev_dbg(&sdev->udev->dev, "free urb %p\n", urb);
		if (urb)
			usb_kill_urb(urb);
		printk(KERN_INFO " %s  pal %p\n",__func__,pal);

		kmem_cache_free(stub_mausb_pal_cache, pal);
		if (urb){
			if (urb->transfer_buffer)
				kfree(urb->transfer_buffer);
			if (urb->setup_packet)
				kfree(urb->setup_packet);
			usb_free_urb(urb);
		}
	}

	while ((priv = stub_priv_pop(sdev))) {
		urb = priv->urb;
		dev_dbg(&sdev->udev->dev, "free urb %p\n", urb);
		if (urb)
			usb_kill_urb(urb);

		kmem_cache_free(stub_priv_cache, priv);
		if (urb){
			if (urb->transfer_buffer)
				kfree(urb->transfer_buffer);
			if (urb->setup_packet)
				kfree(urb->setup_packet);
			usb_free_urb(urb);
		}
	}
}

static ssize_t mausb_bind_write(struct file *fp, const char __user *buf,
	size_t count, loff_t *pos)
{
  int ret = 0;
  char *argv_bind[] = { "/data/mausb", "bind", "--busid=1-1", NULL };
  char *argv_upnp[] = {"/data/upnp-server", "&", NULL};
  char *argv_mausbd[] = {"/data/mausbd", "-D", NULL};
  
  char *argv_unbind[] = { "/data/mausb", "unbind", "--busid=1-1", NULL };
  char *argv_kill_mausbd[] = {"/data/busybox20_0", "pkill", "/data/mausbd", NULL};
  char *argv_kill_upnp[] = {"/data/busybox20_0", "pkill","-15", "/data/upnp-server", NULL};
  
  char *argv_upnp_client[] = {"/data/upnp-client", "&", NULL};
  char *argv_kill_upnp_client[] = {"/data/busybox20_0", "pkill","-15", "/data/upnp-client", NULL};
  char *argv_detach[] = {"/data/mausb","detach", "--port=0", NULL};
 
  static char *envp[] = {
		"HOME=/data",
		"TERM=linux",
		"SHELL=/system/bin/sh",
		"LD_LIBRARY_PATH=/vendor/lib:/system/lib",
		"MKSH=/system/bin/sh",
		"PATH=/sbin:/vendor/bin:/system/sbin:/system/bin:/system/xbin", NULL };

  LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "---> %s",__func__);	
  if (!strcmp(buf,"bind"))	
  {
	  printk(KERN_INFO "bind command received \n");
  	ret  = call_usermodehelper( argv_bind[0], argv_bind, envp, UMH_WAIT_EXEC );
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "ret: %d\n",ret);
	ret  = call_usermodehelper( argv_upnp[0], argv_upnp, envp, UMH_WAIT_EXEC );
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "ret: %d\n",ret);	
	ret  = call_usermodehelper( argv_mausbd[0], argv_mausbd, envp, UMH_WAIT_EXEC );
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "ret: %d\n",ret);	
	
  }
  else if (!strcmp(buf,"unbind"))
  {
  
	ret  = call_usermodehelper( argv_kill_upnp[0], argv_kill_upnp, envp, UMH_WAIT_EXEC );
	ret  = call_usermodehelper( argv_unbind[0], argv_unbind, envp, UMH_WAIT_EXEC );
	ret  = call_usermodehelper( argv_kill_mausbd[0], argv_kill_mausbd, envp, UMH_WAIT_EXEC );
  }
  else if (!strcmp(buf,"detach"))
  {
	ret  = call_usermodehelper( argv_detach[0], argv_detach, envp, UMH_WAIT_EXEC );
	ret  = call_usermodehelper( argv_kill_upnp_client[0], argv_kill_upnp_client, envp, UMH_WAIT_EXEC );
  }
  else if (!strcmp(buf,"attach"))
  {
	printk(KERN_INFO "Attach command received \n");
	ret  = call_usermodehelper( argv_upnp_client[0], argv_upnp_client, envp, UMH_WAIT_EXEC );	  
  }
  
  LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "<-- %s",__func__);
  return ret;
}



static int mausb_bind_open(struct inode *ip, struct file *fp)
{
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "---> %s",__func__);
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "<-- %s",__func__);
	return 0;
}
static int mausb_bind_release(struct inode *ip, struct file *fp)
{
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "---> %s",__func__);
	LG_PRINT(DBG_LEVEL_MEDIUM,DATA_TRANS_MAIN, "<-- %s",__func__);
	return 0;
}

static const char mausb_shortname[] = "mausb_bind";

/* file operations for /dev/mausb_bind */
static const struct file_operations mausb_bind_ops = {
	.owner = THIS_MODULE,
	.write = mausb_bind_write,
	.open = mausb_bind_open,
	.release = mausb_bind_release,
};

static struct miscdevice mausb_bind_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = mausb_shortname,
	.fops = &mausb_bind_ops,
};

static int __init mausb_device_init(void)
{
	int ret;

	init_busid_table();

	stub_priv_cache = KMEM_CACHE(stub_priv, SLAB_HWCACHE_ALIGN);
	if (!stub_priv_cache) {
		pr_err("kmem_cache_create failed\n");
		return -ENOMEM;
	}

	stub_mausb_pal_cache = KMEM_CACHE(stub_mausb_pal, SLAB_HWCACHE_ALIGN);
	if (!stub_mausb_pal_cache) {
		pr_err("kmem_cache_create failed\n");
		return -ENOMEM;
	}


	ret = usb_register(&stub_driver);
	if (ret) {
		pr_err("usb_register failed %d\n", ret);
		goto err_usb_register;
	}

	ret = driver_create_file(&stub_driver.drvwrap.driver,
				 &driver_attr_match_busid);
	if (ret) {
		pr_err("driver_create_file failed\n");
		goto err_create_file;
	}

	ret = misc_register(&mausb_bind_device);
	
	if (ret)
			goto err_create_file;
	
	pr_info(DRIVER_DESC " v" MAUSB_VERSION "\n");

	return ret;

err_create_file:
	usb_deregister(&stub_driver);
err_usb_register:
	kmem_cache_destroy(stub_priv_cache);
	kmem_cache_destroy(stub_mausb_pal_cache);
	return ret;
}

static void __exit mausb_device_exit(void)
{
	misc_deregister(&mausb_bind_device);
	driver_remove_file(&stub_driver.drvwrap.driver,
			   &driver_attr_match_busid);

	/*
	 * deregister() calls stub_disconnect() for all devices. Device
	 * specific data is cleared in stub_disconnect().
	 */
	usb_deregister(&stub_driver);

	kmem_cache_destroy(stub_priv_cache);
	kmem_cache_destroy(stub_mausb_pal_cache);
}

module_init(mausb_device_init);
module_exit(mausb_device_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(MAUSB_VERSION);
