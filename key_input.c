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
#define MAGIC '8'  
#define KEYADC_SET      _IOW(MAGIC, 0, unsigned long)  
#define KEYADC_GET      _IOR(MAGIC, 1, unsigned long)  

#define KEYADC_VOLUP    (0x11)  
#define KEYADC_VOLDOWN  (0x12)  
#define KEYADC_HOME     (0x13)  
#define KEYADC_BACK     (0x14)  

//arch/arm/mach-sunxi/include/mach/sun8i/irqs-sun8iw7p1.h 如果是h2平台中有对中断向量定义
#define KEY_IRQNO       (62) 

#define INPUTNAME       "keyadc_input"
#define PLATFORMNAME    "platform_keyadc"
#define DEVNAME         "keyadc"
#define WORKQUEUENAME   "keyadc_workqueue"
#define IRQNAME         "keyadc_irq"

#define DEBUGMASK   
#ifdef DEBUGMASK  
#define dprintk(fmt, arg...) printk(fmt, ## arg)  
#else  
#define dprintk(fmt, arg...)  
#endif  

/****************************************************************/  
#define LRADC_BASE_ADDR         (0xf1c22800)
#define REG_ADDR(offset)		(volatile void __iomem *)(LRADC_BASE_ADDR + offset)

#define LRADC_CTRL				REG_ADDR(0x00)
#define LRADC_INTC				REG_ADDR(0x04)
#define LRADC_INTS				REG_ADDR(0x08)
#define LRADC_DATA0				REG_ADDR(0x0c)
/*****************************************************************/
#define  FIRST_CONCERT_DLY  (0<<24)  
#define  CHAN           (0x3)  
#define  ADC_CHAN_SELECT    (CHAN<<22)  
#define  LRADC_KEY_MODE     (0)  
#define  KEY_MODE_SELECT    (LRADC_KEY_MODE<<12)  
#define  LEVELB_VOL     (0<<4)  

#define  LRADC_HOLD_EN      (1<<6)  

#define  LRADC_SAMPLE_32HZ  (3<<2)  
#define  LRADC_SAMPLE_62HZ  (2<<2)  
#define  LRADC_SAMPLE_125HZ (1<<2)  
#define  LRADC_SAMPLE_250HZ (0<<2)  

#define  LRADC_EN       (1<<0)  

#define  LRADC_ADC1_UP_EN   (1<<12)  
#define  LRADC_ADC1_DOWN_EN (1<<9)  
#define  LRADC_ADC1_DATA_EN (1<<8)  

#define  LRADC_ADC0_UP_EN   (1<<4)  
#define  LRADC_ADC0_DOWN_EN (1<<1)  
#define  LRADC_ADC0_DATA_EN (1<<0)  

#define  LRADC_ADC1_UPPEND  (1<<12)  
#define  LRADC_ADC1_DOWNPEND    (1<<9)  
#define  LRADC_ADC1_DATAPEND    (1<<8)  

#define  LRADC_ADC0_UPPEND  (1<<4)  
#define  LRADC_ADC0_DOWNPEND    (1<<1)  
#define  LRADC_ADC0_DATAPEND    (1<<0)  

struct sunxi_adc_reg_addr{
	volatile void __iomem *ctrl;
	volatile void __iomem *intc;
	volatile void __iomem *ints;
	volatile void __iomem *data0;
};

static struct sunxi_adc_reg_addr sunxi_adc_reg = {
	LRADC_CTRL,
	LRADC_INTC,
	LRADC_INTS,
	LRADC_DATA0,
};

static struct keyadc_dev{  
	dev_t devid;  
	struct cdev chrdev;  
	struct class *class;  
	struct input_dev *input;  

	struct workqueue_struct *keyadc_workqueue;  
	struct work_struct keyadc_work;  

	unsigned int keycode;  
	struct sunxi_adc_reg_addr *regs;
};

struct timer_list keyadc_event_timer;

static struct keyadc_dev keyadc_pdata = {  
	.regs = &sunxi_adc_reg,
};  

static struct platform_device keyadc_device = {  
	.name   = PLATFORMNAME,  
	.id = -1,  
	.dev    = {  
		.platform_data = &keyadc_pdata,  
	}         
};  


struct keyadc_dev *gpdata;     

/****************************************************************/  
static int keyadc_open(struct inode *inode, struct file *filp)  
{  
	struct keyadc_dev *pdata;     

	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	pdata = container_of(inode->i_cdev, struct keyadc_dev, chrdev);  
	filp->private_data = pdata;  

	return 0;  
}  

static int keyadc_release(struct inode *inode, struct file *filp)  
{  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	filp->private_data = NULL;  

	return 0;  
}  

static long keyadc_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)  
{  
	long ret;  
	void __user *uarg;  
	unsigned long karg;  

	struct keyadc_dev *pdata = filp->private_data;  

	dprintk("=====[%s(%d)]:cmd=%d\n", __FUNCTION__, __LINE__, cmd);  

	if (KEYADC_SET == cmd) {  
		uarg = (void __user *)arg;  
		ret = copy_from_user(&karg, uarg, sizeof(unsigned long));  
		if(0 == ret){  
			dprintk("copy_from_user successed!\n");  
		}  

		switch (karg) {  
			case KEYADC_VOLUP:  
				pdata->keycode = KEY_VOLUMEUP;  
				break;  
			case KEYADC_VOLDOWN:  
				pdata->keycode = KEY_VOLUMEDOWN;  
				break;  
			case KEYADC_HOME:  
				pdata->keycode = KEY_HOME;  
				break;  
			case KEYADC_BACK:  
				pdata->keycode = KEY_BACK;  
				break;  
			default:  
				dprintk("this is defalt\n");  
				break;  
		}  
	}  
	if (KEYADC_GET == cmd){  
		karg = pdata->keycode;  
		dprintk("send keycode to usr space: pdata->keycode=0x%x, karg=0x%x\n", (unsigned int)pdata->keycode, (unsigned int)karg);  
		ret = copy_to_user((void __user *)arg, &karg, sizeof(unsigned long));  
		if(0 == ret){  
			dprintk("copy_to_user successed!\n");  
		}  
	}  

	return 0;  
}  

static struct file_operations keyadc_fops = {  
	.owner      = THIS_MODULE,  
	.open       = keyadc_open,  
	.release    = keyadc_release,  
	.unlocked_ioctl = keyadc_unlocked_ioctl,  
};  

static void keyadc_do_work(struct work_struct *work)  
{  
	struct keyadc_dev *pdata;  
	u32 dval;
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);
	pdata = (struct keyadc_dev *)container_of(work, struct keyadc_dev, keyadc_work);

	//data[5:0]
	dval = *(volatile unsigned __iomem *)(pdata->regs->data0) & 0x3f;
	printk("%d\n", dval);
	if (dval < 9) {
		pdata->keycode = KEY_VOLUMEUP;
		printk("------------------up-----------------\n");
	} else if (dval > 9 && dval < 20) {
		pdata->keycode = KEY_VOLUMEDOWN;
		printk("------------------down-----------------\n");
	}

	//1. 按下 2.取消； TODO 查看是否是两个都必要
	input_report_key(pdata->input, pdata->keycode, 1);
	input_report_key(pdata->input, pdata->keycode, 0);
	input_sync(pdata->input);
}  

static void keyadc_event_function(unsigned long data)
{
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);
	if (!gpdata) {
		printk("pdata is null exit -1");
		return ;
	}
	
	if (!work_pending(&gpdata->keyadc_work)) {
		queue_work(gpdata->keyadc_workqueue, &gpdata->keyadc_work);
	}

	mod_timer(&keyadc_event_timer, jiffies + 1* HZ);
}

static void keyadc_init_timer(void)
{
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);
	if (!gpdata) {
		printk("pdata is null exit -1");
		return ;
	}

	init_timer(&keyadc_event_timer);
	keyadc_event_timer.data = (unsigned long)gpdata;
	keyadc_event_timer.function = keyadc_event_function;
	add_timer(&keyadc_event_timer);
}

static irqreturn_t key_interrupt(int irq, void *pvoid)  
{  
	unsigned int reg_val;  
	int ret = 0;
	struct keyadc_dev *pdata    = (struct keyadc_dev *)pvoid;  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	reg_val  = readl(pdata->regs->ints);  
	printk ("===== 'reg_val=%d' ===\n", reg_val);
	//判断中断状态是否是到来
	// if () {
	ret = mod_timer(&keyadc_event_timer, jiffies + (HZ) / 2); //500ms
	if (ret < 0) {
		printk("mod_timer");
	}
	//}

	//严格来讲，需要清楚中断状态位 clr_ints
	//writel(reg_val, pdata->regs->ints);  

	return IRQ_HANDLED;  
}  

static int keyadc_probe(struct platform_device *pdev)  
{  
	int ret;  
	struct cdev *pchrdev;  
	u32 intc_reg;
	struct keyadc_dev *pdata = pdev->dev.platform_data;  

	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  
	gpdata = pdata;

	alloc_chrdev_region(&pdata->devid, 0, 1, DEVNAME);  
	pchrdev = cdev_alloc();  
	pdata->chrdev = *pchrdev;  
	cdev_init(&pdata->chrdev, &keyadc_fops);  
	pdata->chrdev.owner = THIS_MODULE;  
	ret = cdev_add(&pdata->chrdev, pdata->devid, 1);  
	if (ret) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to cdev_add!");  
		goto fail0;  
	}  

	pdata->input = input_allocate_device();    
	if (!pdata->input) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to input_allocate_device");  
		goto fail1;  
	}  
	pdata->input->name = INPUTNAME;  
	set_bit(EV_KEY,     pdata->input->evbit);  
	set_bit(KEY_VOLUMEDOWN, pdata->input->keybit);  
	set_bit(KEY_VOLUMEUP,   pdata->input->keybit);  
	set_bit(KEY_HOME,   pdata->input->keybit);  
	set_bit(KEY_BACK,   pdata->input->keybit);  
	ret = input_register_device(pdata->input);  
	if (ret) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to input_register_device");  
		goto fail2;  
	}  

	pdata->class = class_create(THIS_MODULE, DEVNAME);  
	if (IS_ERR(pdata->class)) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to class_create");  
		goto fail3;  
	}  
	device_create(pdata->class, NULL, pdata->devid, NULL, DEVNAME);  
	
	printk("ctrl =%p\n", pdata->regs->ctrl);
	printk("intc =%p\n", pdata->regs->intc);
	printk("ints =%p\n", pdata->regs->ints);
	
	intc_reg = LRADC_ADC0_DOWN_EN|LRADC_ADC0_UP_EN|LRADC_ADC0_DATA_EN;
	printk("intc_reg =%d\n", intc_reg);
	writel(LRADC_ADC0_DOWN_EN|LRADC_ADC0_UP_EN|LRADC_ADC0_DATA_EN, pdata->regs->intc);

	printk("%d %d %d %d %d %d %d\n", FIRST_CONCERT_DLY,LEVELB_VOL,KEY_MODE_SELECT,LRADC_HOLD_EN,ADC_CHAN_SELECT,LRADC_SAMPLE_250HZ,LRADC_EN);
	writel(FIRST_CONCERT_DLY|LEVELB_VOL|KEY_MODE_SELECT|LRADC_HOLD_EN|LRADC_SAMPLE_250HZ|LRADC_EN, pdata->regs->ctrl);


	if (request_irq(KEY_IRQNO, key_interrupt, 0, IRQNAME, pdata)) {
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to request_irq");
		goto fail3;
	}  

	pdata->keyadc_workqueue = create_singlethread_workqueue(WORKQUEUENAME);    
	if (NULL == pdata->keyadc_workqueue) {  
		dprintk("=====[%s(%d)]:%s\n", __FUNCTION__, __LINE__, "fail to create_singlethread_workqueue");  
		goto fail4;  
	}     
	INIT_WORK(&pdata->keyadc_work, keyadc_do_work);  

	//创建消抖定时器
	keyadc_init_timer();

	return 0;  

fail4:  
	destroy_workqueue(pdata->keyadc_workqueue);    
fail3:  
	free_irq(KEY_IRQNO, pdata);  
	device_destroy(pdata->class, pdata->devid);  
	class_destroy(pdata->class);  
fail2:  
	input_unregister_device(pdata->input);  
fail1:  
	input_free_device(pdata->input);  
fail0:  
	cdev_del(&pdata->chrdev);  
	kfree(pdata);  

	return ret;  
}  

static int keyadc_remove(struct platform_device *pdev)  
{  
	struct keyadc_dev *pdata = pdev->dev.platform_data;  
	dprintk("=====[%s(%d)]\n", __FUNCTION__, __LINE__);  

	destroy_workqueue(pdata->keyadc_workqueue);  
	device_destroy(pdata->class, pdata->devid);         
	class_destroy(pdata->class);  
	input_free_device(pdata->input);  
	cdev_del(&pdata->chrdev);  
	free_irq(KEY_IRQNO, pdata);       
	//kfree(pdata);  
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

module_init(keyadc_init);  
module_exit(keyadc_exit);  
//refer to this coder 成龙 based on A20
//MODULE_AUTHOR("Jack Chen, chwenj@gmail.com");  
MODULE_AUTHOR("home-coder, one_face@sina.com");
MODULE_LICENSE("GPL");  
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("adc key support");
