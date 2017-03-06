#include <alsa/asoundlib.h>
#include "rec-alsa-mixer.h"

GQuark
rec_alsa_mixer_error_quark (void)
{
  return g_quark_from_static_string ("rec-alsa-mixer-error");
};

struct _RecAlsaMixer
{
  GObject parent_instance;

  GString *device;
  snd_mixer_t *handle;
};

G_DEFINE_TYPE (RecAlsaMixer, rec_alsa_mixer, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_DEVICE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

RecAlsaMixer *
rec_alsa_mixer_new (void)
{
  return g_object_new (REC_TYPE_ALSA_MIXER, NULL);
}

static void
rec_alsa_mixer_finalize (GObject *object)
{
  RecAlsaMixer *self = (RecAlsaMixer *)object;

  g_string_free (self->device, TRUE);

  if (self->handle)
    {
      snd_mixer_close (self->handle);
      self->handle = NULL;
    }

  G_OBJECT_CLASS (rec_alsa_mixer_parent_class)->finalize (object);
}

static void
rec_alsa_mixer_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  RecAlsaMixer *self = REC_ALSA_MIXER (object);

  switch (prop_id)
    {
    case PROP_DEVICE:
      g_value_set_string (value, self->device->str);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
rec_alsa_mixer_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  RecAlsaMixer *self = REC_ALSA_MIXER (object);

  switch (prop_id)
    {
    case PROP_DEVICE:
      g_string_free (self->device, TRUE);
      self->device = g_string_new (g_value_dup_string (value));
      break;

	default:
	  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
rec_alsa_mixer_class_init (RecAlsaMixerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = rec_alsa_mixer_finalize;
  object_class->get_property = rec_alsa_mixer_get_property;
  object_class->set_property = rec_alsa_mixer_set_property;

  properties [PROP_DEVICE] =
    g_param_spec_string ("device",
                         "Device",
                         "Device",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_DEVICE,
                                   properties [PROP_DEVICE]);
}

static void
rec_alsa_mixer_init (RecAlsaMixer *self)
{
  self->device = g_string_new (NULL);
}

gboolean
rec_alsa_mixer_open (RecAlsaMixer *self, GError **error)
{
  gint err;
  snd_ctl_t *ctl;
  snd_ctl_card_info_t *card_info;

  if (self->handle != NULL)
    {
      g_set_error (error, REC_ALSA_MIXER_ERROR,
                   REC_ALSA_MIXER_ERROR_OPEN,
                   "mixer already open");
      return FALSE;
    }

  err = snd_mixer_open (&self->handle, 0);

  if (err < 0 || self->handle == NULL)
    {
      g_set_error (error, REC_ALSA_MIXER_ERROR,
                   REC_ALSA_MIXER_ERROR_OPEN,
                   "cannot open mixer: %s",
                   snd_strerror (err));
      goto error;
    }

  if ((err = snd_mixer_attach (self->handle, self->device->str)) < 0)
    {
      g_set_error (error, REC_ALSA_MIXER_ERROR,
                   REC_ALSA_MIXER_ERROR_OPEN,
                   "cannot attach to mixer: %s",
                   snd_strerror (err));
      goto error;
    }

  return TRUE;

 error:
  snd_mixer_close (self->handle);
  self->handle = NULL;
  return FALSE;
}
