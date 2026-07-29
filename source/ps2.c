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
#define QUAKE_PAK_RELATIVE_PATH "id1/pak0.pak"
/* Keep this in sync with Quake's MAX_OSPATH-sized base-directory buffers. */
#define BASE_PATH_SIZE 128

static char launch_base_path[BASE_PATH_SIZE];
static char probe_path[BASE_PATH_SIZE + sizeof(QUAKE_PAK_RELATIVE_PATH) + 1];

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

static int GameDataReady(const char *base_path)
{
	FILE *file;

	snprintf(probe_path, sizeof(probe_path), "%s/%s",
		base_path, QUAKE_PAK_RELATIVE_PATH);
	file = fopen(probe_path, "rb");
	if (file == NULL)
		return 0;

	fclose(file);
	return 1;
}

static const char *FindHostBasePath(void)
{
	static const char *host_candidates[] = {
		"host:",
		"host:."
	};
	unsigned int i;

	for (i = 0; i < sizeof(host_candidates) / sizeof(host_candidates[0]); ++i)
	{
		if (GameDataReady(host_candidates[i]))
			return host_candidates[i];
	}

	return NULL;
}

static const char *FindMassLaunchBasePath(int argc, char **argv)
{
	char *slash;
	char *backslash;
	size_t length;

	if (argc < 1 || argv == NULL || argv[0] == NULL ||
		strncmp(argv[0], "mass:", 5) != 0)
		return "mass:";

	length = strlen(argv[0]);
	if (length >= sizeof(launch_base_path))
		return "mass:";

	memcpy(launch_base_path, argv[0], length + 1);
	slash = strrchr(launch_base_path, '/');
	backslash = strrchr(launch_base_path, '\\');
	if (backslash != NULL && (slash == NULL || backslash > slash))
		slash = backslash;

	if (slash == NULL)
		return "mass:";

	*slash = '\0';
	if (launch_base_path[0] == '\0')
		return "mass:";

	return launch_base_path;
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

const char *loadmodules(int argc, char **argv)
{
	const char *base_path;
	const char *host_base_path;
	int waited;

	/*
	 * PCSX2 exposes the directory containing the launched ELF through host:.
	 * Probe it before resetting the IOP, because the emulator supplies the
	 * host filesystem without the USB/fileXio module stack.
	 */
	host_base_path = FindHostBasePath();
	if (host_base_path != NULL)
	{
		/* Match the Quake II host path: keep the existing IOP, but ensure
		 * the ROM-resident controller modules are available. */
		SifInitRpc(0);
		LoadRomModule("rom0:SIO2MAN");
		LoadRomModule("rom0:PADMAN");
		printf("Game data found beside the ELF at %s/id1\n", host_base_path);
		return host_base_path;
	}

	printf("No host: game data found; bringing up USB mass storage...\n");
#ifdef _IOPRESET
	IOP_reset();
#endif

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

	base_path = FindMassLaunchBasePath(argc, argv);
	printf("Waiting for game data beside the ELF at %s/id1...\n", base_path);
	for (waited = 0; waited <= USB_WAIT_TOTAL_MSEC; waited += USB_WAIT_STEP_MSEC)
	{
		if (GameDataReady(base_path))
		{
			printf("USB game data found at %s/id1 after about %d ms\n",
				base_path, waited);
			return base_path;
		}

		/*
		 * Some launchers do not pass the ELF path in argv[0]. Keep the
		 * traditional mass:/ root as a compatibility fallback.
		 */
		if (strcmp(base_path, "mass:") != 0 && GameDataReady("mass:"))
		{
			printf("USB game data found at mass:/id1 after about %d ms\n",
				waited);
			return "mass:";
		}
		DelayThread(USB_WAIT_STEP_MSEC * 1000);
	}

	Sys_Error(
		"No Quake game data found beside the ELF.\n"
		"PCSX2: enable Host Filesystem and place id1 next to the ELF.\n"
		"PS2: place id1 next to the ELF on a FAT32 USB drive.");
	return NULL;
}
