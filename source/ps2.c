#include "ps2.h"
#include "sys.h"

#include <delaythread.h>
#include <fileXio_rpc.h>

extern unsigned char iomanx_irx[];
extern unsigned int size_iomanx_irx;
extern unsigned char filexio_irx[];
extern unsigned int size_filexio_irx;
extern unsigned char bdm_irx[];
extern unsigned int size_bdm_irx;
extern unsigned char bdmfs_fatfs_irx[];
extern unsigned int size_bdmfs_fatfs_irx;
extern unsigned char usbd_irx[];
extern unsigned int size_usbd_irx;
extern unsigned char usbmass_bd_irx[];
extern unsigned int size_usbmass_bd_irx;
extern unsigned char ps2kbd_irx[];
extern unsigned int size_ps2kbd_irx;
extern unsigned char ps2mouse_irx[];
extern unsigned int size_ps2mouse_irx;

#define USB_WAIT_TOTAL_MSEC 5000
#define USB_WAIT_STEP_MSEC 100
#define QUAKE_PAK_PATH "mass:/id1/pak0.pak"

static void ExecIopModule(const char *name, void *image, unsigned int size)
{
	int id;
	int result;

	id = SifExecModuleBuffer(image, size, 0, NULL, &result);
	if (id < 0 || result == 1)
		Sys_Error("IOP module '%s' failed (id %d, result %d)", name, id, result);

	printf("IOP: started %s (id %d)\n", name, id);
}

static void LoadRomModule(const char *name)
{
	int id;

	id = SifLoadModule(name, 0, NULL);
	if (id < 0)
		Sys_Error("IOP ROM module '%s' failed (id %d)", name, id);
}

static int UsbGameDataReady(void)
{
	int file;

	file = fileXioOpen(QUAKE_PAK_PATH, O_RDONLY);
	if (file < 0)
		return 0;

	fileXioClose(file);
	return 1;
}

void IOP_reset(void)
{
	SifInitRpc(0);
	while (!SifIopReset("", 0))
		;
	while (!SifIopSync())
		;
	SifInitRpc(0);

	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
}

void loadmodules(void)
{
	int waited;

	ExecIopModule("iomanX", iomanx_irx, size_iomanx_irx);
	ExecIopModule("fileXio", filexio_irx, size_filexio_irx);
	ExecIopModule("bdm", bdm_irx, size_bdm_irx);
	ExecIopModule("bdmfs_fatfs", bdmfs_fatfs_irx, size_bdmfs_fatfs_irx);
	ExecIopModule("usbd", usbd_irx, size_usbd_irx);
	ExecIopModule("usbmass_bd", usbmass_bd_irx, size_usbmass_bd_irx);
	ExecIopModule("ps2kbd", ps2kbd_irx, size_ps2kbd_irx);
	ExecIopModule("ps2mouse", ps2mouse_irx, size_ps2mouse_irx);

	LoadRomModule("rom0:SIO2MAN");
	LoadRomModule("rom0:PADMAN");

	if (fileXioInit() < 0)
		Sys_Error("Unable to initialize fileXio");

	printf("Waiting for USB game data at %s...\n", QUAKE_PAK_PATH);
	for (waited = 0; waited <= USB_WAIT_TOTAL_MSEC; waited += USB_WAIT_STEP_MSEC)
	{
		if (UsbGameDataReady())
		{
			printf("USB game data ready after about %d ms\n", waited);
			return;
		}
		DelayThread(USB_WAIT_STEP_MSEC * 1000);
	}

	Sys_Error(
		"No Quake game data found on USB.\n"
		"Use a FAT32 drive and place id1/pak0.pak at its root.");
}
