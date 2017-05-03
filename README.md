# suxi_adc_key
此为ADC 按键 驱动，based on A33. 原始代码参考网络A20平台，需转为A33，并添加KEY 的MAP，adc检测输入&lt;-->一对多按键。

	|* 手册中寄存器的物理地址0x06001800   *|

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
