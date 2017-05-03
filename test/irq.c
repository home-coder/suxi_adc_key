/*
 * test key input event . example vol_up, wol_dow, vol_mute. \
 * 2017.5.2
 * 
 * - Base on Allwinner A33 platform;
 * 
 */  

#include <linux/module.h>  
#include <linux/slab.h>  
#include <linux/cdev.h>  
#include <linux/input.h>  
#include <linux/platform_device.h>  
#include <linux/interrupt.h>  
#include <asm/io.h>  
#include <asm/uaccess.h>  
/****************************************************************/  
#define dprintk(fmt, arg...) printk(fmt, ## arg)
#define KEY_IRQNO  62
#define PLATFORMNAME  "hello"

static struct platform_device keyadc_device = {  
	.name   = PLATFORMNAME,  
	.id = -1,  
};  

/****************************************************************/  
static irqreturn_t key_interrupt(int irq, void *pvoid)  
{  

	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  
	printk("---------------inter----\n");
	return IRQ_HANDLED;  
}  

static int keyadc_probe(struct platform_device *pdev)  
{  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	if (request_irq(KEY_IRQNO, key_interrupt, 0, "hello irq", NULL)) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to request_irq");  
	}  

	return 0;  
}  

static int keyadc_remove(struct platform_device *pdev)  
{  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

//	free_irq(KEY_IRQNO);       
	
	return 0;  
}  


static struct platform_driver keyadc_driver = {  
	.probe  = keyadc_probe,  
	.remove = keyadc_remove,  
	.driver = {  
		.name   = PLATFORMNAME,  
		.owner  = THIS_MODULE,  
	}  
};  

static int __init keyadc_init(void)  
{  
	int ret;  

	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	ret = platform_device_register(&keyadc_device);  
	if (ret < 0) {  
		dprintk("ERROR:Fail to platform_device_register!\n");  
		return ret;  
	}  

	ret = platform_driver_register(&keyadc_driver);   
	if (ret < 0) {  
		dprintk("ERROR:Fail to platform_driver_register!\n");  
		return ret;  
	}  

	return 0;  
}  

static void __exit keyadc_exit(void)  
{  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	platform_device_unregister(&keyadc_device);  
	platform_driver_unregister(&keyadc_driver);  
}  

MODULE_AUTHOR("home-coder, one_face@sina.com");
MODULE_LICENSE("GPL");  

module_init(keyadc_init);  
module_exit(keyadc_exit);  
