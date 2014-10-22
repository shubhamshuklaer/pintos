#ifndef DEVICES_DISK_H
#define DEVICES_DISK_H

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include "threads/synch.h"

/* Size of a disk sector in bytes. */
#define DISK_SECTOR_SIZE 512

/* Index of a disk sector within a disk.
   Good enough for disks up to 2 TB. */
typedef uint32_t disk_sector_t;

/* Format specifier for printf(), e.g.:
   printf ("sector=%"PRDSNu"\n", sector); */
#define PRDSNu PRIu32


/* An ATA device. */
struct disk 
  {
    char name[8];               /* Name, e.g. "hd0:1". */
    struct channel *channel;    /* Channel disk is on. */
    int dev_no;                 /* Device 0 or 1 for master or slave. */

    bool is_ata;                /* 1=This device is an ATA disk. */
    disk_sector_t capacity;     /* Capacity in sectors (if is_ata). */

    long long read_cnt;         /* Number of sectors read. */
    long long write_cnt;        /* Number of sectors written. */
  };

/* An ATA channel (aka controller).
   Each channel can control up to two disks. */
struct channel 
  {
    char name[8];               /* Name, e.g. "hd0". */
    uint16_t reg_base;          /* Base I/O port. */
    uint8_t irq;                /* Interrupt in use. */

    struct lock lock;           /* Must acquire to access the controller. */
    bool expecting_interrupt;   /* True if an interrupt is expected, false if
                                   any interrupt would be spurious. */
    struct semaphore completion_wait;   /* Up'd by interrupt handler. */

    struct disk devices[2];     /* The devices on this channel. */
  };



void disk_init (void);
void disk_print_stats (void);

struct disk *disk_get (int chan_no, int dev_no);
disk_sector_t disk_size (struct disk *);
void disk_read (struct disk *, disk_sector_t, void *);
void disk_write (struct disk *, disk_sector_t, const void *);

#endif /* devices/disk.h */
