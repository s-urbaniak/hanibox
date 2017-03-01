#include <alsa/asoundlib.h>
#include "rec-alsa-devices.h"

static void
rec_alsa_devices_destroy (gpointer data)
{
  RecAlsaDevice *dev = data;

  if (dev)
    {
      g_string_free (dev->card, TRUE);
      g_string_free (dev->device, TRUE);
      g_string_free (dev->name, TRUE);
    }
}

GQuark
rec_alsa_error_quark (void)
{
  return g_quark_from_static_string ("rec-alsa-error");
};

GArray*
rec_alsa_devices_new (GError **error)
{
  GArray *ret = g_array_new(FALSE, FALSE, sizeof (RecAlsaDevice));
  g_array_set_clear_func (ret, rec_alsa_devices_destroy);

  snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;

  snd_ctl_t *handle;
  int card, err, dev, idx;

  snd_ctl_card_info_t *info;
  snd_ctl_card_info_alloca (&info);

  snd_pcm_info_t *pcminfo;
  snd_pcm_info_alloca (&pcminfo);

  card = -1;
  if (snd_card_next (&card) < 0 || card < 0) {
    g_set_error (error,
                 REC_ALSA_ERROR,
                 REC_ALSA_ERROR_NO_SOUNDCARDS,
                 "no soundcards found");
    return NULL;
  }

  while (card >=0)
    {
      GString *name = g_string_new (NULL);
      g_string_printf(name, "hw:%d", card);

      if ((err = snd_ctl_open (&handle, name->str, 0)) < 0)
        {
          g_error ("control open (%i): %s", card, snd_strerror (err));
          goto next_card;
        }

      if ((err = snd_ctl_card_info (handle, info)) < 0)
        {
          g_error ("control hardware info (%i): %s", card, snd_strerror (err));
          snd_ctl_close (handle);
          goto next_card;
        }

      dev = -1;

      while (TRUE) {
        unsigned int count;

        if (snd_ctl_pcm_next_device (handle, &dev)<0)
          g_error("snd_ctl_pcm_next_device");

        if (dev < 0)
          break;
      }
    next_card:
      g_string_free (name, TRUE);
      if (snd_card_next (&card) < 0)
        {
          g_error ("snd_card_next");
          break;
      }
    }

  return ret;
}
