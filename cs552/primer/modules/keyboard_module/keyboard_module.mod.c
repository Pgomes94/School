#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb79dfee1, "module_layout" },
	{ 0xd0d8621b, "strlen" },
	{ 0xf09c7f68, "__wake_up" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb00ccc33, "finish_wait" },
	{ 0x4292364c, "schedule" },
	{ 0xe75663a, "prepare_to_wait" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x7a2255dd, "per_cpu__current_task" },
	{ 0x86b82836, "create_proc_entry" },
	{ 0xfda85a7d, "request_threaded_irq" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x6644f130, "remove_proc_entry" },
	{ 0xb72397d5, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

