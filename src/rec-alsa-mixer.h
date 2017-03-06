#ifndef __REC_ALSA_MIXER_H__
#define __REC_ALSA_MIXER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define REC_ALSA_MIXER_ERROR (rec_alsa_mixer_error_quark())

typedef enum
{
  REC_ALSA_MIXER_ERROR_OPEN,
} RecAlsaMixerError;

#define REC_TYPE_ALSA_MIXER (rec_alsa_mixer_get_type())

G_DECLARE_FINAL_TYPE (RecAlsaMixer, rec_alsa_mixer, REC, ALSA_MIXER, GObject)

RecAlsaMixer *rec_alsa_mixer_new (void);
gboolean rec_alsa_mixer_open (RecAlsaMixer *self, GError **error);

G_END_DECLS

#endif /* REC_ALSA_MIXER_H */
