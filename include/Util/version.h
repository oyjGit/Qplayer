#ifndef __QPLAYER_VERSION_H__
#define __QPLAYER_VERSION_H__

/*
版本号的格式如下所示。可选组件显示在方括号（“[”和“]”）中：
主版本号.次版本号[.内部版本号[.修订号]]

应根据下面的约定使用这些部分：
Major：具有相同名称但不同主版本号的程序集不可互换。例如，这适用于对产品的大量重写，这些重写使得无法实现向后兼容性。
Minor：如果两个程序集的名称和主版本号相同，而次版本号不同，这指示显著增强，但照顾到了向后兼容性。例如，这适用于产品的修正版或完全向后兼容的新版本。
Build：内部版本号的不同表示对相同源所作的重新编译。这适合于更改处理器、平台或编译器的情况。
Revision：名称、主版本号和次版本号都相同但修订号不同的程序集应是完全可互换的。这适用于修复以前发布的程序集中的安全漏洞。
程序集的只有内部版本号或修订号不同的后续版本被认为是对先前版本的“快速修复工程”(QFE)   更新。如有必要，可以通过更改配置中的版本策略使内部版本号和修订号生效。

主版本或次版本
对版本号的主版本或次版本所做的更改表示不兼容的更改。因此，在这种约定下，2.0.0.0   版被视为与   1.0.0.0   版本不兼容。更改某些方法参数的类型，或者整个移除某个类型或方法都属于不兼容的更改。

内部版本
内部版本号通常用于区分每日版本或者改动较小的兼容版本。

修订号
修订号更改通常是为修复某个特定错误所需的增量编译保留的。有时，您会听到它被称为“紧急错误修复”号，因为当发送给客户针对某个特定错误的修复时，更改的通常是修订号。
因此，兼容性版本号为   2.0.0.0   的程序集被视为与兼容性版本号为   1.0.0.0   的程序集不兼容。同样，兼容性版本号   2.0.2.11   被视为兼容性版本号   2.0.2.1   的   QFE。
*/
#include <cstdint>
#include <string>

#define QPLAYER_MAJOR_VERSION	1	//8bit
#define QPLAYER_MINJOR_VERSION	1	//8bit
#define QPLAYER_BUILD_VERSION	0	//16bit

const std::string buildDate = __DATE__;
const std::string buildTime = __TIME__;

inline void getQPlayerVersionUint32(uint32_t& ver)
{
	uint8_t major = QPLAYER_MAJOR_VERSION;
	uint8_t minjar = QPLAYER_MINJOR_VERSION;
	uint16_t build = QPLAYER_BUILD_VERSION;
	ver = (major << 24) | (minjar << 16) | build;
}

inline void getQPlayerVersionString(std::string& version)
{
	std::string ver = std::to_string(QPLAYER_MAJOR_VERSION) \
					+ "." + std::to_string(QPLAYER_MINJOR_VERSION) \
					+ "." + std::to_string(QPLAYER_BUILD_VERSION);
	version = "Qplayer sdk version:" + ver + ",build time:" + buildDate + " " + buildTime;
}

#endif