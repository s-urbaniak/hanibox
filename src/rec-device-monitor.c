#include "rec-device-monitor.h"

#include <gudev/gudev.h>
#include <glib.h>
#include <glib/gstdio.h>

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

enum {
  ADD,
  REMOVE,
  N_SIGNALS
};

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

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

  gboolean add = g_strcmp0 (action, "add") == 0;
  gboolean remove = g_strcmp0 (action, "remove") == 0;

  if (!(add || remove))
    {
      return;
    }

  if (remove)
    {
      g_signal_emit (self, signals[REMOVE], 0, g_udev_device_get_sysfs_path (device));
      return;
    }

  GString *sysfs_path = g_string_new (g_udev_device_get_sysfs_path (device));
  sysfs_path = g_string_append (sysfs_path, "/number");

  gsize len;
  gchar *content;
  if (g_file_get_contents (sysfs_path->str, &content, &len, NULL)) {
    g_strchomp (content);
    g_debug ("card number %s", content);
	g_free (content);
  }

  g_signal_emit (self, signals[ADD], 0, g_udev_device_get_sysfs_path (device));
  g_string_free (sysfs_path, TRUE);
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
  
  signals [ADD] =
    g_signal_new ("add",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);

  signals [REMOVE] =
    g_signal_new ("remove",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
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
