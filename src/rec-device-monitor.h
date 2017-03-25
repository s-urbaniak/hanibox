#ifndef __REC_DEVICE_MONITOR_H__
#define __REC_DEVICE_MONITOR_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define REC_TYPE_DEVICE_MONITOR (rec_device_monitor_get_type())

G_DECLARE_FINAL_TYPE (RecDeviceMonitor, rec_device_monitor, REC, DEVICE_MONITOR, GObject)

RecDeviceMonitor *
rec_device_monitor_new (void);

G_END_DECLS

#endif /* REC_DEVICE_MONITOR_H */
