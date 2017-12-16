#include "message_slot.h" /* replace it with your own header if needed */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h> /* open */
#include <sys/ioctl.h>  /* ioctl */
#include <unistd.h> /* read, write */
#include <errno.h>
#define BUFF_SIZE 129

/* tries to read an empty channel */
void read_no_message(int fd) {
	printf("\n----- read_no_message ---------- \n");
	fflush(stdout);
	char bffr[10];
	int rc = ioctl(fd, MSG_SLOT_CHANNEL, 20);
	if (rc == -1) {
		fprintf(stderr, "read_no_message: ioctl error is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "read_no_message: ioctl error is ENOMEM: %d, errno: %d\n", (errno == ENOMEM), errno);
		return;
	}
	rc = read(fd, bffr, 5);
	if (rc == -1) {
		fprintf(stderr, "write_read_null: read-error is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "write_read_null: read-error is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
		fprintf(stderr, "write_read_null: read-error is EWOULDBLOCK: %d, errno: %d\n", (errno == EWOULDBLOCK), errno);
		fprintf(stderr, "write_read_null: read-error is ENOSPC: %d, errno: %d\n", (errno == ENOSPC), errno);
	}
}

/* tries to read / write to / from a NULL pointer */
void write_read_null(int fd) {
	printf("\n----- write_read_null ---------- \n");
	fflush(stdout);
	sleep(1);
	char *null_bffr = NULL;
	int rc = ioctl(fd, MSG_SLOT_CHANNEL, 10);
	if (rc == -1) {
		fprintf(stderr, "write_read_null: ioctl error is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "write_read_null: ioctl error is ENOMEM: %d, errno: %d\n", (errno == ENOMEM), errno);
		return;
	}
	rc = write(fd, null_bffr, 100);
	if (rc == -1) {
		fprintf(stderr, "write_read_null: write-error is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	}
	rc = read(fd, null_bffr, 100);
	if (rc == -1) {
		fprintf(stderr, "write_read_null: read-error is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	}
}

/* messing with the buffer size */
void error_buffer_size(int fd) {
	printf("\n----- error_buffer_size ---------- \n");
	fflush(stdout);
	char *long_bffr = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	char short_bffr[10];
	int rc = write(fd, long_bffr, 200);
	if (rc == -1) {
		fprintf(stderr, "buffer_size: write-error is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
	}
	rc = write(fd, long_bffr, 100);
	if (rc == -1) {
		fprintf(stderr, "buffer_size: write-error is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	}
	rc = read(fd, short_bffr, 120);
	if (rc == -1) {
		fprintf(stderr, "buffer_size: read-error is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	}
	rc = read(fd, short_bffr, 10);
	if (rc == -1) {
		fprintf(stderr, "buffer_size: read-error is ENOSPC: %d, errno: %d\n", (errno == ENOSPC), errno);
	}
}

/* tries to read / write before evoking ioctl() command */
void write_read_before_ioctl(int fd) {
	printf("\n----- write_read_before_ioctl ---------- \n");
	fflush(stdout);
	char *bffr = "abc";
	char bffr_read[10];
	int rc = write(fd, bffr, 3);
	if (rc == -1) {
			fprintf(stderr, "write_before_ioctl: error-write is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
	}
	rc = read(fd, bffr_read, 3);
	if (rc == -1) {
		fprintf(stderr, "write_before_ioctl: error-read is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
	}
	rc = ioctl(fd, MSG_SLOT_CHANNEL, 10);
	if (rc == -1) {
		fprintf(stderr, "write_before_ioctl: ioctl error is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "write_before_ioctl: ioctl error is ENOMEM: %d, errno: %d\n", (errno == ENOMEM), errno);
	}
	rc = write(fd, bffr, 3);
	if (rc == -1) {
		fprintf(stderr, "write_before_ioctl: error-write is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "buffer_size: error-write is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	} else {
		printf("could write after ioctl\n");
	}
	rc = read(fd, bffr_read, 3);
	if (rc == -1) {
		fprintf(stderr, "write_before_ioctl: error-read is EINVAL: %d, errno: %d\n", (errno == EINVAL), errno);
		fprintf(stderr, "buffer_size: error-read is EFAULT: %d, errno: %d\n", (errno == EFAULT), errno);
	} else {
		bffr_read[rc] = '\0'; /* terminating the string */
		printf("could read after ioctl: %s\n", bffr_read);
	}
}

int main(int argc, char *argv[]) {
	int fd = open(argv[1], O_RDWR); /* argv[1] is a device created beforehand */
	int ret_val;
	if (fd < 0) {
	    fprintf(stderr, "Can't open device file: %s\n", argv[1]);
	    return -1;
	}
	write_read_before_ioctl(fd);
	//error_buffer_size(fd); /* causes a "smashing stack" fault on my machine */
	close(fd);
	fd = open(argv[1], O_RDWR);
	read_no_message(fd);
	write_read_null(fd);
	close(fd);
	return 0;
}
