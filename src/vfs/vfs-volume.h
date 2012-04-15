/*
*  C Interface: vfs-volume
*
* Description: 
*
*
* Author: Hong Jen Yee (PCMan) <pcman.tw (AT) gmail.com>, (C) 2006
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#ifndef _VFS_VOLUME_H_
#define _VFS_VOLUME_H_

#include <glib.h>

G_BEGIN_DECLS

typedef struct _VFSVolume VFSVolume;

typedef enum{
    VFS_VOLUME_ADDED,
    VFS_VOLUME_REMOVED,
    VFS_VOLUME_MOUNTED, /* Not implemented */
    VFS_VOLUME_UNMOUNTED, /* Not implemented */
    VFS_VOLUME_CHANGED
}VFSVolumeState;

typedef void ( *VFSVolumeCallback ) ( VFSVolume* vol,
                                      VFSVolumeState state,
                                      gpointer user_data );

gboolean vfs_volume_init();

gboolean vfs_volume_clean();

VFSVolume** vfs_volume_get_all_volumes( int* num );

void vfs_volume_add_callback( VFSVolumeCallback cb, gpointer user_data );

void vfs_volume_remove_callback( VFSVolumeCallback cb, gpointer user_data );

gboolean vfs_volume_mount( VFSVolume* vol );

gboolean vfs_volume_umount( VFSVolume *vol );

gboolean vfs_volume_eject( VFSVolume *vol );

const char* vfs_volume_get_disp_name( VFSVolume *vol );

const char* vfs_volume_get_mount_point( VFSVolume *vol );

const char* vfs_volume_get_device( VFSVolume *vol );

const char* vfs_volume_get_fstype( VFSVolume *vol );

const char* vfs_volume_get_icon( VFSVolume *vol );

gboolean vfs_volume_is_removable( VFSVolume *vol );

gboolean vfs_volume_is_mounted( VFSVolume *vol );

gboolean vfs_volume_requires_eject( VFSVolume *vol );

G_END_DECLS

#endif
