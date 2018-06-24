#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAP_SIZE 0x200000

static const int gen2_devices[] = {
	0x2582,
	0x258a,
	0x2592,
	0x2772,
	0x27a2,
	0x27ae,
	0x29b2,
	0x29c2,
	0x29d2,
	0xa001,
	0xa011
};

static int is_gen2_device(int id) {
	int i;
	for (i = 0; i < sizeof(gen2_devices) / sizeof(gen2_devices[0]); i++) {
		if (gen2_devices[i] == id) {
			return 1;
		}
	}
	return 0;
}

static int get_pci_info(const char * device, const char * name, int * out) {
	int result = 0;
	if (device && device[0] != '.' && strlen(device) < 20) {
		char buffer[100];
		sprintf(buffer, "/sys/bus/pci/devices/%s/%s", device, name);
		int fd = open(buffer, O_RDONLY);
		if (fd >= 0) {
			int len;
			if ((len = read(fd, buffer, 20)) > 0) {
				buffer[len] = '\0';
				*out = (int) strtoul(buffer, 0, 0);
				result = 1;
			}
			close(fd);
		}
	}
	return result;
}

static int test_pci_info(const char * device, const char * name, int what) {
	int value;
	return get_pci_info(device, name, &value) && value == what;
}

static int test_intel_gpu(const char * device) {
	return test_pci_info(device, "vendor", 0x8086) &&
		test_pci_info(device, "class", 0x30000);
}

static int get_intel_gpu_pci_fd() {
	int fd = -1;
	DIR * dir = opendir("/sys/bus/pci/devices");
	if (dir) {
		struct dirent * entry;
		while (entry = readdir(dir)) {
			if (test_intel_gpu(entry->d_name)) {
				int id;
				if (get_pci_info(entry->d_name, "device", &id)) {
					int resource = is_gen2_device(id) ? 1 : 0;
					char buffer[100];
					sprintf(buffer, "/sys/bus/pci/devices/%s/resource%d",
						entry->d_name, resource);
					fd = open(buffer, O_RDWR | O_SYNC);
					break;
				}
			}
		}
		closedir(dir);
	}
	return fd;
}

int main(int argc, char ** argv) {
	if (argc <= 1 || argc >= 4) {
		fprintf(stderr, "usage: %s {address} [value]\n", argv[0]);
		return 1;
	}
	uint32_t addr = (uint32_t) strtoul(argv[1], 0, 0);
	int fd = get_intel_gpu_pci_fd();
	if (fd < 0) {
		fprintf(stderr, "failed to open intel gpu pci resource file\n");
		return 1;
	}
	void * base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!base || !(base + 1)) {
		close(fd);
		fprintf(stderr, "failed to map memory\n");
		return 1;
	}
	if (argc >= 3) {
		uint32_t value = (uint32_t) strtoul(argv[2], 0, 0);
		*((uint32_t *) (base + addr)) = value;
	}
	uint32_t value = *((uint32_t *) (base + addr));
	printf("0x%08x\n", value);
	munmap(base, MAP_SIZE);
	close(fd);
	return 1;
}
