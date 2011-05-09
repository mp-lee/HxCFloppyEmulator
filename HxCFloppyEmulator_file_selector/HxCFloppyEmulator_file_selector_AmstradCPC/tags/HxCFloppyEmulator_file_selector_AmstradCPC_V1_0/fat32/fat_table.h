#ifndef __FAT_TABLE_H__
#define __FAT_TABLE_H__

#include "fat_opts.h"
#include "fat_misc.h"

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void	fatfs_fat_init(struct fatfs *fs);
int		fatfs_fat_purge(struct fatfs *fs);
UINT32	fatfs_find_next_cluster(struct fatfs *fs, UINT32 current_cluster);
void	fatfs_set_fs_info_next_free_cluster(struct fatfs *fs, UINT32 newValue);
int		fatfs_find_blank_cluster(struct fatfs *fs, UINT32 start_cluster, UINT32 *free_cluster);
int		fatfs_fat_set_cluster(struct fatfs *fs, UINT32 cluster, UINT32 next_cluster);
int		fatfs_fat_add_cluster_to_chain(struct fatfs *fs, UINT32 start_cluster, UINT32 newEntry);
int		fatfs_free_cluster_chain(struct fatfs *fs, UINT32 start_cluster);

#endif