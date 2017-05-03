# suxi_adc_key
此为ADC 按键 驱动，based on A33. 原始代码参考网络A20平台，需转为A33，并添加KEY 的MAP，adc检测输入&lt;-->一对多按键。

	|* 手册中寄存器的物理地址0x06001800   *|
在Linux kernel 中，物理地址是不能直接使用的，ARM9可以最大寻址范围为1G，其从0X00000000开始都是物理地址。。必须通过转换才可以。
转换分为两种， 静态和动态。
静态就是下面那种，不过，静态的地址转换，还需要在kernel 初始化的时候作映射。  
动态映射是使用 ioremap 函数 。

/*以下定义是错误的，如果是0x06001800，那么是正确的*/

#define KEY_ADCBASE         0xf6001800

#define __REG_VAL(offset) 	(*(volatile unsigned *))(KEY_ADCBASE + offset)
#define LRADC_CTRL			__REG_VAL(0x04)

/*以上地址是通过手册计算得到的映射地址,代码中无需ioremap等操作，但是这里必须如下定义*/

#define __REG_VAL(offset)   (*(volatile void __iomem *)(KEY_ADCBASE + offset))


/*如果ioremap,则如下,将整个寄存器的地址范围映射到虚拟地址,起始地址base_addr*/
#define KEY_ADCBASE			0x06001800

static void __iomem *base_addr;
base_addr = ioremap(KEY_ADCBASE, offset_all);

-----------------------------------------------------------------------------------------------
#下面的两行代码，理解其思路。
#239     struct keyadc_dev *pdata = pdev->dev.platform_data; 
#240     pdata->regs = (struct sun7i_lradc_regs *)LRADC_BASE_ADDR;
#
#分析是不是最好的办法。
#
#
----------------------------------------------------------------------------------------------------
#
#如果有其它的驱动注册了同一个中断号irqno，并且不是共享的，那么另一个试图再注册同一个中断的模块会使系统挂掉
#更正：不会的，这是由于adc寄存器设置出错，导致中断产生出问题
#
----------------------------------------------------------------------
#如何删除test目录下的垃圾文件


