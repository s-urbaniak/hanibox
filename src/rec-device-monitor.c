#include "rec-device-monitor.h"
#include "gudev/gudev.h"

struct _RecDeviceMonitor
{
  GObject parent_instance;

  GUdevClient *udev_client;
};

G_DEFINE_TYPE (RecDeviceMonitor, rec_device_monitor, G_TYPE_OBJECT)

enum {
  PROP_0,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

RecDeviceMonitor *
rec_device_monitor_new (void)
{
  return g_object_new (REC_TYPE_DEVICE_MONITOR, NULL);
}

static void
rec_device_monitor_uevent_handler (GUdevClient *client,
                                   gchar       *action,
                                   GUdevDevice *device,
                                   gpointer     user_data)
{
  RecDeviceMonitor *self = (RecDeviceMonitor *)user_data;

  g_debug ("sysfs path %s", g_udev_device_get_sysfs_path (device));
}

static void
rec_device_monitor_dispose (GObject *object)
{
  RecDeviceMonitor *self = (RecDeviceMonitor *)object;

  g_clear_object (&self->udev_client);
}

static void
rec_device_monitor_finalize (GObject *object)
{
  RecDeviceMonitor *self = (RecDeviceMonitor *)object;

  G_OBJECT_CLASS (rec_device_monitor_parent_class)->finalize (object);
}

static void
rec_device_monitor_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  RecDeviceMonitor *self = REC_DEVICE_MONITOR (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
rec_device_monitor_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  RecDeviceMonitor *self = REC_DEVICE_MONITOR (object);

  switch (prop_id)
    {
	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
rec_device_monitor_class_init (RecDeviceMonitorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = rec_device_monitor_finalize;
  object_class->dispose = rec_device_monitor_dispose;
  object_class->get_property = rec_device_monitor_get_property;
  object_class->set_property = rec_device_monitor_set_property;
}

static void
rec_device_monitor_init (RecDeviceMonitor *self)
{
  const gchar *subsystems[] = { "sound", NULL };
  self->udev_client = g_udev_client_new (subsystems);

  g_signal_connect (self->udev_client,
                    "uevent",
                    G_CALLBACK (rec_device_monitor_uevent_handler),
                    self);
}
